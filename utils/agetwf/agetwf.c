#include "../../library/agilent_user.h"

#ifndef	BOOL
#define	BOOL	int
#endif
#ifndef TRUE
#define	TRUE	1
#endif
#ifndef FALSE
#define	FALSE	0
#endif

BOOL	sc(char*, char*);

int	main(int argc, char *argv[]) {

VXI11_CLIENT	*client;
VXI11_LINK	*link;
static char	*progname;
static char	*serverIP;
char		chnl; /* we use '1' to '4' for channels, and 'A' to 'D' for FUNC[1...4] */
FILE		*f_wf;
char 		wfname[256];
char 		wfiname[256];
char 		cmd[256];
long		buf_size;
char		*buf;
unsigned long	timeout=10000; /* in ms (= 10 seconds) */

long		bytes_returned;
BOOL		clear_sweeps=FALSE;
BOOL		got_ip=FALSE;
BOOL		got_scope_channel=FALSE;
BOOL		got_file=FALSE;
int		index=1;
double		s_rate=0;
long		npoints=0;
double		actual_s_rate;
long		actual_npoints;

	progname = argv[0];

	while(index<argc){
		if(sc(argv[index],"-filename")||sc(argv[index],"-f")||sc(argv[index],"-file")){
			snprintf(wfname,256,"%s.wf",argv[++index]);
			snprintf(wfiname,256,"%s.wfi",argv[index]);
			got_file=TRUE;
			}

		if(sc(argv[index],"-ip")||sc(argv[index],"-ip_address")||sc(argv[index],"-IP")){
			serverIP = argv[++index];
			got_ip = TRUE;
			}

		if(sc(argv[index],"-channel")||sc(argv[index],"-c")||sc(argv[index],"-scope_channel")){
			sscanf(argv[++index],"%c",&chnl);
			got_scope_channel=TRUE;
			}

		if(sc(argv[index],"-sample_rate")||sc(argv[index],"-s")||sc(argv[index],"-rate")){
			sscanf(argv[++index],"%lg",&s_rate); /* %g in sscanf is a float, so use %lg for double. %g in printf is a double. Great. */
			}

		if(sc(argv[index],"-no_points")||sc(argv[index],"-n")||sc(argv[index],"-points")){
			sscanf(argv[++index],"%lu",&npoints);
			}
			
		if(sc(argv[index],"-timeout")||sc(argv[index],"-t")){
			sscanf(argv[++index],"%lu",&timeout);
			}
			
		index++;
		}

	if(got_file==FALSE||got_scope_channel==FALSE||got_ip==FALSE){
		printf("%s: grabs a waveform from an Agilent scope via ethernet, by Steve (June 06)\n",progname);
		printf("Run using %s [arguments]\n\n",progname);
		printf("REQUIRED ARGUMENTS:\n");
		printf("-ip    -ip_address     -IP      : IP address of scope (eg 128.243.74.232)\n");
		printf("-f     -filename       -file    : filename (without extension)\n");
		printf("-c     -scope_channel  -channel : scope channel (1,2,3,4,A,B,C,D)\n");
		printf("OPTIONAL ARGUMENTS:\n");
		printf("-t     -timeout                 : timout (in milliseconds)\n");
		printf("-s     -sample_rate    -rate    : set sample rate (eg 1e9 = 1GS/s)\n");
		printf("-n     -no_points      -points  : set minimum no of points\n\n");
		printf("OUTPUTS:\n");
		printf("filename.wf  : binary data of waveform\n");
		printf("filename.wfi : waveform information (text)\n\n");
		printf("In Matlab, use loadwf or similar to load and process the waveform\n\n");
		printf("EXAMPLE:\n");
		printf("%s -ip 128.243.74.232 -f test -c 2 -s 1e9\n",progname);
		exit(1);
		}

	f_wf=fopen(wfname,"w");
	if (f_wf > 0) {
	/* This utility illustrates the general idea behind how data is acquired.
	 * First we open the device, referenced by an IP address, and obtain
	 * a client id, and a link id. Each client can have more than one link */

		if (vxi11_open_device(serverIP,&client,&link) != 0) {
			printf("Quitting...\n");
			exit(2);
			}
	/* Next we do some trivial initialisation. This sets LSB first, binary 
	 * word transfer, etc. A good opportunity to check we can talk to the scope. */
		if (agilent_init(client, link) !=0 ) {
			printf("Quitting...\n");
			exit(2);
			}
	/* Now we set up the sampling rate and number of acquisition points. We
	 * can choose to let the scope do both for us, set the sampling rate only,
	 * or set the number of points (actually the minimum number of points) only.
	 * The important thing to note is that the time range of the acquired data
	 * will _always_ be the same as the time range displayed on the scope screen.
	 * This is NOT the default behaviour of Agilent scopes; however we find this
	 * approach a lot less confusing. You may have your own opinions. Passing
	 * values <=0 for s_rate or npoints means they will be assigned automatically;
	 * stating positive values for both, the sample rate takes precedence. */
		agilent_set_for_capture(client, link, s_rate, npoints, timeout);

	/* We need to know how big an array we need to hold all the data. This is
	 * not necessarily simply related to npoints; it may depend on whether
	 * interpolation (sinx/x) is turned on, the timebase, available memory
	 * etc. I have worked out a formula for calculating the number of bytes
	 * needed, but in order to do this we need to know the "WAV:XINC" (the
	 * amount of time between each data point). In order to find this out, we
	 * must :DIG (digitise) the waveform. This is NOT the same as _acquiring_ the
	 * waveform, it merely freezes does what ever is necessary for the scope
	 * itself to acquire a legitimate data set (averaging etc), then stops the
	 * acquisition. Once a digitisation has been done, the data is there to be
	 * read, until either the next acquisition, or the scope is set to :RUN
	 * again.
	 * Another aspect of recording a waveform is knowing what the binary numbers
	 * mean. We have a standard text file, a "wfi_file" that accompanies each
	 * (or each set of) binary waveform (wf) files. This contains enough
	 * information to reconstruct the trace. It does not contain all the info
	 * of the Agilent "preamble" data, but it does contain extra info, such
	 * as the name of the program used to capture the data, and how many traces
	 * are contained in the wf file.
	 * So, since we need the number of bytes for the wfi file AND to know how
	 * big an array we need, we combine a "find out how many points we need"
	 * function with a "write the wfi file" function, to save on a bit of
	 * overhead. So, in _this_ instance, the actual acquisition (digitisation)
	 * is performed here. */
		buf_size = agilent_write_wfi_file(client, link, wfiname, chnl, progname, 1, timeout);
		buf=new char[buf_size];
	/* This is where we transfer the data from the scope to the PC. Note the
	 * fourth argument, "0"; this tells the function not to do a digitisation.
	 * Normally, calling this function repetetively to acquire several traces,
	 * you would either use "1" for this argument, or miss it out altogether
	 * (i.e. there is an overloaded wrapper function). This is all for 
	 * optimisation; if you have a large npoints, many averages, and a slow-
	 * triggered signal, you do not want to be waiting 2 or 3 times for no
	 * good reason! */
		bytes_returned = agilent_get_data(client, link, chnl, 0, buf, buf_size, timeout);
		if (bytes_returned <=0) {
			printf("Problem reading the data, quitting...\n");
			exit(2);
			}

		//agilent_report_status(client, link, timeout);

		strcpy(cmd, ":ACQ:SRAT?");
		actual_s_rate = vxi11_obtain_double_value(client, link, cmd);
		strcpy(cmd, ":ACQ:POINTS?");
		actual_npoints = vxi11_obtain_long_value(client, link, cmd);
		printf("Sample rate used: %g (%g GSa/s); acquisition points: %ld\n",actual_s_rate, (actual_s_rate/1e9),actual_npoints);

	/* Now that everything is done, we can return the scope back to its usual
	 * "free-running" mode, where the sample rate and npoints are automatically 
	 * set. This allows you to adjust the timebase manually without fear of
	 * running out of points, sample rate, or the scope becoming sluggish.
	 * You would only want to do this after everything is done, do not call
	 * this function if you want to do any more acquisition. */
		agilent_set_for_auto(client, link);
		fwrite(buf, sizeof(char), bytes_returned, f_wf);
		fclose(f_wf);
		delete[] buf;

	/* Finally we sever the link to the client. */
		vxi11_close_device(serverIP,client,link);
		}
	else {
		printf("error: could not open file for writing, quitting...\n");
		exit(3);
		}
	}

/* Extra notes:
 * Another way of going through the acquisition loop, which is more relevant to
 * acquiring multiple waveforms, is as follows:

	vxi11_open_device(serverIP,&client,&link);
	agilent_init(client, link);
	agilent_set_for_capture(client, link, s_rate, npoints, timeout);
	buf_size = agilent_calculate_no_of_bytes(client, link, chnl, timeout); // performs :DIG
	buf = new char[buf_size];
	count=0;
	do { // note for first acquisition no :DIG is done, then it is for each one after
		bytes_returned = agilent_get_data(client, link, chnl, count++, buf, buf_size, timeout);
		<append trace to wf file>;
		} while (<some condition>);

	// Note we are sending the function the buf_size instead of the chnl; no digitisation
	// needs to be done (however you must take care that the scope has not been adjusted
	// since the last acquisition)
	ret = agilent_write_wfi_file(client, link, wfiname, buf_size, progname, count, timeout);
	agilent_set_for_auto(client, link);
	delete[] buf;
	vxi11_close_device(serverIP,client,link);
 */

/* string compare (sc) function for parsing... ignore */
BOOL	sc(char *con,char *var){
	if(strcmp(con,var)==0){
		return TRUE;
		}
	return FALSE;
	}

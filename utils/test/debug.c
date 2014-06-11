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

static char	*progname;
static char	*serverIP;
char		chnl; /* we use '1' to '4' for channels, and 'A' to 'D' for FUNC[1...4] */
FILE		*f_wf;
char 		wfname[256];
char 		wfiname[256];
long		buf_size;
char		*buf;
unsigned long	timeout=10000; /* in ms (= 10 seconds) */

long		bytes_returned;
BOOL		clear_sweeps=FALSE;
BOOL		got_ip=FALSE;
BOOL		got_scope_channel=FALSE;
BOOL		got_file=FALSE;
BOOL		got_no_averages=FALSE;
int		no_averages;
int		index=1;
double		s_rate=0;
long		npoints=0;
double		actual_s_rate;
long		actual_npoints;
CLINK		*clink; /* client link (actually a structure contining CLIENT and VXI11_LINK pointers) */
CLINK		*clink2;
char		locbuf[256];
CLIENT		*client;
VXI11_LINK	*link1;
VXI11_LINK	*link2;
int		ret;
CLIENT		*client_addr;
	clink = new CLINK; /* allocate some memory */
	clink2 = new CLINK; /* allocate some memory */

	//client	= new CLIENT;
	//link1	= new VXI11_LINK;
	//link2	= new VXI11_LINK;

	progname = argv[0];

	while(index<argc){
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
			sscanf(argv[++index],"%ld",&npoints);
			}
			
		if(sc(argv[index],"-averages")||sc(argv[index],"-a")||sc(argv[index],"-aver")){
			sscanf(argv[++index],"%d",&no_averages);
			got_no_averages=TRUE;
			}
			
		if(sc(argv[index],"-timeout")||sc(argv[index],"-t")){
			sscanf(argv[++index],"%lu",&timeout);
			}
			
		index++;
		}

	if(got_scope_channel==FALSE||got_ip==FALSE){
		printf("%s: grabs a waveform from an Agilent scope via ethernet, by Steve (June 06)\n",progname);
		printf("Run using %s [arguments]\n\n",progname);
		printf("REQUIRED ARGUMENTS:\n");
		printf("-ip    -ip_address     -IP      : IP address of scope (eg 128.243.74.232)\n");
		printf("-f     -filename       -file    : filename (without extension)\n");
		printf("-c     -scope_channel  -channel : scope channel (1,2,3,4,A,B,C,D)\n");
		printf("OPTIONAL ARGUMENTS:\n");
		printf("-t     -timeout                 : timout (in milliseconds)\n");
		printf("-s     -sample_rate    -rate    : set sample rate (eg 1e9 = 1GS/s)\n");
		printf("-n     -no_points      -points  : set minimum no of points\n");
		printf("-a     -averages       -aver    : set no of averages (<=0 means none)\n\n");
		printf("OUTPUTS:\n");
		printf("filename.wf  : binary data of waveform\n");
		printf("filename.wfi : waveform information (text)\n\n");
		printf("In Matlab, use loadwf or similar to load and process the waveform\n\n");
		printf("EXAMPLE:\n");
		printf("%s -ip 128.243.74.232 -f test -c 2 -s 1e9\n",progname);
		exit(1);
		}

/*	if (vxi11_open_device(serverIP, &client, &link1) != 0) {
		printf("Quitting...\n");
		exit(2);
		}
	client_addr = client;
	clink->client=client_addr;
	clink->link=link1;
	printf("client = %ld, client_addr=%ld, clink->client=%ld\n",client, client_addr, clink->client);
*/
	if (vxi11_open_device(serverIP, clink) != 0) {
		printf("Quitting...\n");
		exit(2);
		}

	if (agilent_init(clink) !=0 ) {
		printf("Quitting...\n");
		exit(2);
		}
	vxi11_send_and_receive(clink,"*IDN?",locbuf,256,500);
	printf("sent *IDN? to clink, received: '%s'\n",locbuf);

/*	if (vxi11_open_link(serverIP,&client,&link2) != 0) {
		printf("Quitting...\n");
		exit(2);
		}
	clink2->client=client;
	clink2->link=link2;
*/
	if (vxi11_open_device(serverIP, clink2) != 0) {
		printf("Quitting...\n");
		exit(2);
		}

	printf("Just opened clink2.\n");
	vxi11_send_and_receive(clink,":WAV:SOURCE?",locbuf,256,500);
	printf("sent :WAV:SOURCE? to clink, received: '%s'\n",locbuf);


/* Next we do some trivial initialisation. This sets LSB first, binary 
 * word transfer, etc. A good opportunity to check we can talk to the scope. */
	if (agilent_init(clink2) !=0 ) {
		printf("Quitting...\n");
		exit(2);
		}
	actual_s_rate = vxi11_obtain_double_value(clink, ":ACQ:SRAT?");
	actual_npoints = vxi11_obtain_long_value(clink, ":ACQ:POINTS?");
	printf("Sample rate used: %g (%g GSa/s); acquisition points: %ld\n",actual_s_rate, (actual_s_rate/1e9),actual_npoints);

	actual_s_rate = vxi11_obtain_double_value(clink2, ":ACQ:SRAT?");
	actual_npoints = vxi11_obtain_long_value(clink2, ":ACQ:POINTS?");
	printf("Sample rate used: %g (%g GSa/s); acquisition points: %ld\n",actual_s_rate, (actual_s_rate/1e9),actual_npoints);

	actual_s_rate = vxi11_obtain_double_value(clink, ":ACQ:SRAT?");
	actual_npoints = vxi11_obtain_long_value(clink, ":ACQ:POINTS?");
	printf("Sample rate used: %g (%g GSa/s); acquisition points: %ld\n",actual_s_rate, (actual_s_rate/1e9),actual_npoints);


	/* Finally we sever the link to the client. */
	//	vxi11_close_link(serverIP,client, link1);
	//	vxi11_close_device(serverIP,client, link1);
	//	vxi11_close_device(serverIP,client, link2);
	vxi11_close_device(serverIP,clink);
	vxi11_close_device(serverIP,clink2);
	}

/* Extra notes:
 * Another way of going through the acquisition loop, which is more relevant to
 * acquiring multiple waveforms, is as follows:

	vxi11_open_device(serverIP,clink);
	agilent_init(clink);
	agilent_set_for_capture(clink, s_rate, npoints, timeout);
	buf_size = agilent_calculate_no_of_bytes(clink, chnl, timeout); // performs :DIG
	buf = new char[buf_size];
	count=0;
	do { // note for first acquisition no :DIG is done, then it is for each one after
		bytes_returned = agilent_get_data(clink, chnl, count++, buf, buf_size, timeout);
		<append trace to wf file>;
		} while (<some condition>);

	// Note we are sending the function the buf_size instead of the chnl; no digitisation
	// needs to be done (however you must take care that the scope has not been adjusted
	// since the last acquisition)
	ret = agilent_write_wfi_file(clink, wfiname, buf_size, progname, count, timeout);
	agilent_set_for_auto(clink);
	delete[] buf;
	vxi11_close_device(serverIP,clink);
 */

/* string compare (sc) function for parsing... ignore */
BOOL	sc(char *con,char *var){
	if(strcmp(con,var)==0){
		return TRUE;
		}
	return FALSE;
	}


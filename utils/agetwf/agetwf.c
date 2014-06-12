/* agetwf.c
 * Copyright (C) 2006 Steve D. Sharples
 *
 * Command line utility to acquire traces from Agilent Infiniium oscilloscopes.
 * After compiling, run it without any arguments for help info.
 * For historical reasons, we have our own data format for scope trace data.
 * Each trace consists of a trace.wf file that contains the binary data, and a
 * trace.wfi text file that contains the waveform info. We then use a very
 * cheesy Matlab script, loadwf.m to load the data into Matlab. The wfi file
 * does not contain all the information that Agilent's own "preamble"
 * information contains; on the other hand, you can have multiple traces in
 * the same wf file.
 *
 * The source is extensively commented and from this, and a look at the 
 * agilent_user.c library, you will begin to understand the approach to 
 * acquiring data that I've taken.
 *
 * You will also need the
 * vxi11_X.XX.tar.gz source, currently available from:
 * http://optics.eee.nottingham.ac.uk/vxi11/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * The author's email address is steve.sharples@nottingham.ac.uk
 */

#include <stdio.h>
#include <string.h>

#include "agilent_user.h"

#ifndef	BOOL
#define	BOOL	int
#endif
#ifndef TRUE
#define	TRUE	1
#endif
#ifndef FALSE
#define	FALSE	0
#endif

BOOL sc(const char *, const char *);

int main(int argc, char *argv[])
{

	static char *progname;
	static char *serverIP;
	char chnl;		/* we use '1' to '4' for channels, and 'A' to 'D' for FUNC[1...4] */
	FILE *f_wf;
	char wfname[256];
	char wfiname[256];
	long buf_size;
	char *buf;
	unsigned long timeout = 10000;	/* in ms (= 10 seconds) */

	long bytes_returned;
	BOOL clear_sweeps = FALSE;
	BOOL got_ip = FALSE;
	BOOL got_scope_channel = FALSE;
	BOOL got_file = FALSE;
	BOOL got_no_averages = FALSE;
	int no_averages;
	BOOL got_no_segments = FALSE;
	int no_segments;
	int index = 1;
	double s_rate = 0;
	long npoints = 0;
	double actual_s_rate;
	long actual_npoints;
	VXI11_CLINK *clink;		/* client link (actually a structure contining CLIENT and VXI11_LINK pointers) */
	int l;
	char cmd[256];

	progname = argv[0];

	while (index < argc) {
		if (sc(argv[index], "-filename") || sc(argv[index], "-f")
		    || sc(argv[index], "-file")) {
			snprintf(wfname, 256, "%s.wf", argv[++index]);
			snprintf(wfiname, 256, "%s.wfi", argv[index]);
			got_file = TRUE;
		}

		if (sc(argv[index], "-ip") || sc(argv[index], "-ip_address")
		    || sc(argv[index], "-IP")) {
			serverIP = argv[++index];
			got_ip = TRUE;
		}

		if (sc(argv[index], "-channel") || sc(argv[index], "-c")
		    || sc(argv[index], "-scope_channel")) {
			sscanf(argv[++index], "%c", &chnl);
			got_scope_channel = TRUE;
		}

		if (sc(argv[index], "-sample_rate") || sc(argv[index], "-s")
		    || sc(argv[index], "-rate")) {
			sscanf(argv[++index], "%lg", &s_rate);	/* %g in sscanf is a float, so use %lg for double. %g in printf is a double. Great. */
		}

		if (sc(argv[index], "-no_points") || sc(argv[index], "-n")
		    || sc(argv[index], "-points")) {
			sscanf(argv[++index], "%ld", &npoints);
		}

		if (sc(argv[index], "-averages") || sc(argv[index], "-a")
		    || sc(argv[index], "-aver")) {
			sscanf(argv[++index], "%d", &no_averages);
			got_no_averages = TRUE;
		}

		if (sc(argv[index], "-segmented") || sc(argv[index], "-seg")
		    || sc(argv[index], "-segm")) {
			sscanf(argv[++index], "%d", &no_segments);
			got_no_segments = TRUE;
		}

		if (sc(argv[index], "-timeout") || sc(argv[index], "-t")) {
			sscanf(argv[++index], "%lu", &timeout);
		}

		index++;
	}

	if (got_file == FALSE || got_scope_channel == FALSE || got_ip == FALSE) {
		printf
		    ("%s: grabs a waveform from an Agilent scope via ethernet, by Steve (June 06)\n",
		     progname);
		printf("Run using %s [arguments]\n\n", progname);
		printf("REQUIRED ARGUMENTS:\n");
		printf
		    ("-ip    -ip_address     -IP      : IP address of scope (eg 128.243.74.232)\n");
		printf
		    ("-f     -filename       -file    : filename (without extension)\n");
		printf
		    ("-c     -scope_channel  -channel : scope channel (1,2,3,4,A,B,C,D)\n");
		printf("OPTIONAL ARGUMENTS:\n");
		printf
		    ("-t     -timeout                 : timout (in milliseconds)\n");
		printf
		    ("-s     -sample_rate    -rate    : set sample rate (eg 1e9 = 1GS/s)\n");
		printf
		    ("-n     -no_points      -points  : set minimum no of points\n");
		printf
		    ("-a     -averages       -aver    : set no of averages (<=0 means none)\n\n");
		printf
		    ("-seg   -segmented      -segm    : set no of segments (<=0 means none)\n\n");
		printf("OUTPUTS:\n");
		printf("filename.wf  : binary data of waveform\n");
		printf("filename.wfi : waveform information (text)\n\n");
		printf
		    ("In Matlab, use loadwf or similar to load and process the waveform\n\n");
		printf("EXAMPLE:\n");
		printf("%s -ip 128.243.74.232 -f test -c 2 -s 1e9\n", progname);
		exit(1);
	}

	f_wf = fopen(wfname, "w");
	if (f_wf > 0) {
		/* This utility illustrates the general idea behind how data is acquired.
		 * First we open the device, referenced by an IP address, and obtain
		 * a client id, and a link id, all contained in a "VXI11_CLINK" structure.  Each
		 * client can have more than one link. For simplicity we bundle them together. */

		if (agilent_open(&clink, serverIP) != 0) {	// could also use "vxi11_open_device()"
			printf("Quitting...\n");
			exit(2);
		}

		/* Next we do some trivial initialisation. This sets LSB first, binary 
		 * word transfer, etc. A good opportunity to check we can talk to the scope. */
		if (agilent_init(clink) != 0) {
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
		agilent_set_for_capture(clink, s_rate, npoints, timeout);

		/* If we've specified the number of averages, then set it. Otherwise, just
		 * leave the scope in the condition it's in, in that respect. */
		if (got_no_averages == TRUE)
			agilent_set_averages(clink, no_averages);

		/* We make sure the channel we want to grab data from is turned on */
		agilent_display_channel(clink, chnl, 1);

		/* If number of segments specified, then set to segmented mode */

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
		if (got_no_segments == FALSE) {
			buf_size =
			    agilent_write_wfi_file(clink, wfiname, chnl,
						   progname, 1, timeout);
			buf = new char[buf_size];

			/* This is where we transfer the data from the scope to the PC. Note the
			 * fourth argument, "0"; this tells the function not to do a digitisation.
			 * Normally, calling this function repetetively to acquire several traces,
			 * you would either use "1" for this argument, or miss it out altogether
			 * (i.e. there is an overloaded wrapper function). This is all for 
			 * optimisation; if you have a large npoints, many averages, and a slow-
			 * triggered signal, you do not want to be waiting 2 or 3 times for no
			 * good reason! */
			bytes_returned =
			    agilent_get_data(clink, chnl, 0, buf, buf_size,
					     timeout);
			if (bytes_returned <= 0) {
				printf
				    ("Problem reading the data, quitting...\n");
				exit(2);
			}
		} else {
			buf_size =
			    agilent_write_wfi_file(clink, wfiname, chnl,
						   progname, no_segments,
						   timeout);
			vxi11_send_str(clink, ":ACQ:MODE SEGMENTED");
			sprintf(cmd, ":ACQ:SEGM:COUNT %d", no_segments);
			vxi11_send_str(clink, cmd);
			buf = new char[buf_size * no_segments];

			for (l = 0; l < no_segments; l++) {
				sprintf(cmd, ":ACQ:SEGM:INDEX %d", l + 1);
				vxi11_send_str(clink, cmd);
				bytes_returned =
				    agilent_get_data(clink, chnl, 0,
						     buf + (l * buf_size),
						     buf_size, timeout);
				if (bytes_returned <= 0) {
					printf
					    ("Problem reading the data, quitting...\n");
					exit(2);
				}
			}
		}

		//agilent_report_status(client, link, timeout);

		actual_s_rate = vxi11_obtain_double_value(clink, ":ACQ:SRAT?");
		actual_npoints = vxi11_obtain_long_value(clink, ":ACQ:POINTS?");
		printf
		    ("Sample rate used: %g (%g GSa/s); acquisition points: %ld\n",
		     actual_s_rate, (actual_s_rate / 1e9), actual_npoints);

		/* Now that everything is done, we can return the scope back to its usual
		 * "free-running" mode, where the sample rate and npoints are automatically 
		 * set. This allows you to adjust the timebase manually without fear of
		 * running out of points, sample rate, or the scope becoming sluggish.
		 * You would only want to do this after everything is done, do not call
		 * this function if you want to do any more acquisition. */
		agilent_set_for_auto(clink);
		if (got_no_segments == FALSE) {
			fwrite(buf, sizeof(char), bytes_returned, f_wf);
		} else {
			fwrite(buf, sizeof(char), bytes_returned * no_segments,
			       f_wf);
		}

		fclose(f_wf);
		delete[]buf;

		/* Finally we sever the link to the client. */
		agilent_close(clink, serverIP);	// could also use "vxi11_close_device()"
	} else {
		printf("error: could not open file for writing, quitting...\n");
		exit(3);
	}
}

/* Extra notes:
 * Another way of going through the acquisition loop, which is more relevant to
 * acquiring multiple waveforms, is as follows:

	agilent_open(serverIP,clink);
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
	agilent_close(serverIP,clink);
 */

/* string compare (sc) function for parsing... ignore */
BOOL sc(const char *con, const char *var)
{
	if (strcmp(con, var) == 0) {
		return TRUE;
	}
	return FALSE;
}

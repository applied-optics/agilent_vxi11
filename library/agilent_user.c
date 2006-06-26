#include "agilent_user.h"

/* Set up some fundamental settings for data transfer. It's possible
 * (although not certain) that some or all of these would be reset after
 * a system reset. It's a very tiny overhead right at the beginning of your
 * acquisition that's performed just once. */
int	agilent_init(CLIENT *client, VXI11_LINK *link) {
char	cmd[256];
int	ret;
	sprintf(cmd,":SYSTEM:HEADER 0");
	ret=vxi11_send(client, link, cmd, strlen(cmd));
	if(ret < 0) {
		printf("error in agilent init, could not send command '%s'\n",cmd);
		return ret;
		}
	sprintf(cmd,":ACQUIRE:COMPLETE 100");
	vxi11_send(client, link, cmd, strlen(cmd));
	sprintf(cmd,":WAVEFORM:BYTEORDER LSBFIRST");
	vxi11_send(client, link, cmd, strlen(cmd));
	sprintf(cmd,":WAVEFORM:FORMAT BINARY");
	vxi11_send(client, link, cmd, strlen(cmd));
	return 0;
	}

/* Debugging function whilst developing the library. Might as well leave it here */
int	agilent_report_status(CLIENT *client, VXI11_LINK *link, unsigned long timeout) {
char	cmd[256];
char	buf[256];
double	dval;
long	lval;

	memset(buf, 0, 256);
	strcpy(cmd, ":wav:source?");
	if (vxi11_send_and_receive(client, link, cmd, strlen(cmd), buf, 256, timeout) != 0) return -1;
	printf("Source:        %s", buf);

	strcpy(cmd, ":tim:range?");
	dval = vxi11_obtain_double_value(client, link, cmd, strlen(cmd), timeout);
	printf("Time range:    %g (%g us)\n",dval, (dval*1e6)); 

	strcpy(cmd, ":acq:srat?");
	dval=vxi11_obtain_double_value(client, link, cmd, strlen(cmd), timeout);
	strcpy(cmd, ":acq:srat:auto?");
	lval=vxi11_obtain_long_value(client, link, cmd, strlen(cmd), timeout);
	printf("Sample rate:   %g (%g GS/s), set to ", dval, (dval/1e9));
	if(lval == 0) printf("MANUAL\n"); else printf("AUTO\n");

	strcpy(cmd, ":acq:points?");
	lval=vxi11_obtain_long_value(client, link, cmd, strlen(cmd), timeout);
	printf("No of points:  %ld, set to ", lval);
	strcpy(cmd, ":acq:points:auto?");
	lval=vxi11_obtain_long_value(client, link, cmd, strlen(cmd), timeout);
	if(lval == 0) printf("MANUAL\n"); else printf("AUTO\n");

	strcpy(cmd, ":acq:INT?");
	lval=vxi11_obtain_long_value(client, link, cmd, strlen(cmd), timeout);
	if(lval == 0) printf("(sin x)/x:     OFF\n"); else printf("(sin x)/x:     ON\n");

	strcpy(cmd, ":wav:xinc?");
	dval = vxi11_obtain_double_value(client, link, cmd, strlen(cmd), timeout);
	printf("xinc:          %g (%g us, %g ps)\n",dval, (dval*1e6), (dval*1e12)); 

	return 0;
	}

/* Gets the system setup for the scope, dumps it in an array. Can be saved for later. */
int	agilent_get_setup(CLIENT *client, VXI11_LINK *link, char *buf, unsigned long buf_len) {
char	cmd[256];
int	ret;
long	bytes_returned;

	sprintf(cmd,":SYSTEM:SETUP?");
	ret=vxi11_send(client, link, cmd, strlen(cmd));
	if(ret < 0) {
		printf("error, could not ask for system setup, quitting...\n");
		return ret;
		}
	bytes_returned=vxi11_receive_data_block(client, link, buf, buf_len, VXI11_READ_TIMEOUT);

	return (int) bytes_returned;
	}

/* Sends a previously saved system setup back to the scope. */
int	agilent_send_setup(CLIENT *client, VXI11_LINK *link, char *buf, unsigned long buf_len) {
char	cmd[256];
int	ret;
long	bytes_returned;

	sprintf(cmd,":SYSTEM:SETUP ");
	ret=vxi11_send_data_block(client, link, cmd, buf, buf_len);
	if (ret < 0) {
		printf("error, could not send system setup, quitting...\n");
		return ret;
		}
	return 0;
	}

/* Fairly important function. Calculates the number of bytes that digitised
 * data will take up, based on timebase settings, whether interpolation is
 * turned on etc. Can be used before you actually acquire any data, to find
 * out how big you need your data buffer to be. */
long	agilent_calculate_no_of_bytes(CLIENT *client, VXI11_LINK *link, char chan, unsigned long timeout) {
char	cmd[256];
char	source[20];
double	hinterval,time_range;
double	srat;
long	no_of_bytes;
char	etim_result[256];

	// First we need to digitize, to get the correct values for the
	// waveform data. This is a pain in the arse.
	agilent_scope_channel_str(chan, source);
	sprintf(cmd,":WAV:SOURCE %s",source);
	vxi11_send(client, link, cmd, strlen(cmd));
	strcpy(cmd,":DIG");
	vxi11_send(client, link, cmd, strlen(cmd));

	/* Now find the info we need to calculate the number of points */
	strcpy(cmd, ":WAV:XINC?");
	hinterval = vxi11_obtain_double_value(client, link, cmd, strlen(cmd),timeout);
	strcpy(cmd, ":ACQ:SRAT?");
	srat = vxi11_obtain_double_value(client, link, cmd, strlen(cmd));

	/* Are we in equivalent time (ETIM) mode? If so, the value of ACQ:SRAT will
	 * be meaningless, and there's a different formula */
	strcpy(cmd, ":ACQ:MODE?");
	vxi11_send_and_receive(client, link, cmd, strlen(cmd), etim_result, 256, VXI11_READ_TIMEOUT);

	/* Equivalent time (ETIM) mode: */
	if (strncmp("ETIM",etim_result,4) == 0) {
		no_of_bytes = (long) ( 2*((time_range / hinterval) + 0.5) );
		}
	else {
		strcpy(cmd, ":TIM:RANGE?");
		time_range = vxi11_obtain_double_value(client, link, cmd, strlen(cmd));

		no_of_bytes = (long) (2*(((time_range-(1/srat)) / hinterval)+1) + 0.5);
		/* 2x because 2 bytes per data point
		 * +0.5 to round up when casting as a long
		 * -(1/srat) and +1 so that both raw data, and interpolated (sinx/x) data works */
		}
	return no_of_bytes;
	}

/* This function, agilent_write_wfi_file(), saves useful (to us!)
 * information about the waveforms. This is NOT the full set of
 * "preamble" data - things like the data and other bits of info you may find
 * useful are not included, and there are extra elements like the number of
 * waveforms acquired. 
 *
 * It is up to the user to ensure that the scope is in the same condition as
 * when the data was taken before running this function, otherwise the values
 * returned may not reflect those during your data capture; ie if you run
 * agilent_set_for_capture() before you grab the data, then either call that
 * function again with the same values, or, run this function straight after
 * you've acquired your data.
 *
 * Actually there are two agilent_write_wfi_file() functions; the first does
 * a :DIG and finds out how many bytes of data will be collected. This may
 * be useful if you're only capturing one trace; you might as well call this
 * function at the start, find out how many bytes you need for your data
 * array, and write the wfi file at the same time.
 * 
 * But maybe you won't know how many traces you're taking until the end, in
 * which case you'll want to write the wfi file at the end. In that case,
 * you should already know how many bytes per trace there are, and have 
 * also probably recently done a :DIG to capture your data; hence the 
 * second agilent_write_wfi_file() function. */
long	agilent_write_wfi_file(CLIENT *client, VXI11_LINK *link, char *wfiname, char chan, char *captured_by, int no_of_traces, unsigned long timeout) {
long	no_of_bytes;

	/* First we'll calculate the number of bytes. This involves doing a :DIG
	 * command, which (depending on how your scope's set up) may take a fraction
	 * of a second, or a very long time. */
	no_of_bytes = agilent_calculate_no_of_bytes(client, link, chan, timeout);

	/* Now we'll pass this to the _real_ function that does all the work 
	 * (no_of_bytes will stay the same unless there is trouble writing the file) */
	no_of_bytes = agilent_write_wfi_file(client, link, wfiname, no_of_bytes, captured_by, no_of_traces, timeout);

	return no_of_bytes;
	}

/* This version of the agilent_write_wfi_file() function is passed the
 * no_of_bytes, from an earlier calling of the agilent_calculate_no_of_bytes()
 * function. Useful if you want to write your wfi file at the end of your
 * acquisition(s), but don't want to do another digitisation. */
long	agilent_write_wfi_file(CLIENT *client, VXI11_LINK *link, char *wfiname, long no_of_bytes, char *captured_by, int no_of_traces, unsigned long timeout) {
FILE	*wfi;
char	cmd[256];
double	vgain,voffset,hinterval,hoffset;
int	ret;

	/* Now find the other info we need to write the wfi-file */
	/* (those paying attention will notice some repetition here,
	 * it's just a bit more logical this way) */
	strcpy(cmd, ":WAV:XINC?");
	hinterval = vxi11_obtain_double_value(client, link, cmd, strlen(cmd),timeout);
	strcpy(cmd, ":WAV:XORIGIN?");
	hoffset = vxi11_obtain_double_value(client, link, cmd, strlen(cmd));
	strcpy(cmd, ":WAV:YINC?");
	vgain = vxi11_obtain_double_value(client, link, cmd, strlen(cmd));
	strcpy(cmd, ":WAV:YORIGIN?");
	voffset = vxi11_obtain_double_value(client, link, cmd, strlen(cmd));

	wfi = fopen(wfiname,"w");
	if (wfi > 0) {
		fprintf(wfi,"%% %s\n",wfiname);
		fprintf(wfi,"%% Waveform captured using %s\n\n",captured_by);
		fprintf(wfi,"%% Number of bytes:\n%d\n\n",no_of_bytes);
		fprintf(wfi,"%% Vertical gain:\n%g\n\n",vgain);
		fprintf(wfi,"%% Vertical offset:\n%g\n\n",-voffset);
		fprintf(wfi,"%% Horizontal interval:\n%g\n\n",hinterval);
		fprintf(wfi,"%% Horizontal offset:\n%g\n\n",hoffset);
		fprintf(wfi,"%% Number of traces:\n%d\n\n",no_of_traces);
		fprintf(wfi,"%% Number of bytes per data-point:\n%d\n\n",2); /* always 2 on Agilent scopes */
		fprintf(wfi,"%% Keep all datapoints (0 or missing knocks off 1 point, legacy lecroy):\n%d\n\n",1);
		fclose(wfi);
		}
	else {
		printf("error: agilent_write_wfi_file: could not open %s for writing\n",wfiname);
		return -1;
		}
	return no_of_bytes;
	}

/* The following function is more a demonstration of the sequence of capturing
 * data, than a useful function in itself (though it can be used as-is).
 * Agilent scopes, when either sample rate or no of acquisition points (or both)
 * are set to "auto", sets the time period for acquisition to be greater than
 * what is displayed on the screen. We find this way of working confusing, and
 * so we set the sample rate and no of acquisition points manually, such that
 * the acquisition period matches the time range (i.e. the period of time
 * displayed on the screen).
 * We may also want to control either the sample rate or the no of acquisition
 * points (we can't control both, otherwise that would change the time range!);
 * or, we may want the scope to select sensible values for us. So we have a
 * function agilent_set_for_capture() which sets everthing up for us.
 * Once this is done, we have a very basic (and hence fast) function that
 * actually digitises the data in binary form, agilent_get_data().
 * Finally, we like to return the scope back to automatic mode, whereby it
 * chooses sensible values for the sample rate and no of acquisition points,
 * and also restarts the acquisition (which gets frozen after digitisation).
 *
 * If you would like to grab the preamble information - see the function
 * agilent_get_preamble() - or write a wfi file - agilent_write_wfi_file() -
 * make sure that you do this BEFORE you return the scope to automatic mode.
 */
long	agilent_get_screen_data(CLIENT *client, VXI11_LINK *link, char chan, char *buf, unsigned long buf_len, unsigned long timeout, double s_rate, long npoints) {
long	returned_bytes;

	agilent_set_for_capture(client, link, s_rate, npoints, timeout);
	returned_bytes = agilent_get_data(client, link, chan, buf, buf_len, timeout);
	agilent_set_for_auto(client, link);
	return returned_bytes;
	}


/* See comments for above function, and comments within this function, for a
 * description of what it does.
 */
int	agilent_set_for_capture(CLIENT *client, VXI11_LINK *link, double s_rate, long npoints, unsigned long timeout) {
long	actual_npoints;		/* actual number of points returned */
double	time_range;
double	auto_srat;		/* sample rate whilst on auto setting */
long	auto_npoints;		/* no of points whilst on auto setting */
double	expected_s_rate;	/* based on s_rate passed to us, or npoints */
double	actual_s_rate;		/* what it ends up as */
double	xinc;			/* xincrement (only need for ETIM mode) */
char	cmd[256];
char	etim_result[256];
int	cmp;
int	ret_val	= 0;
int	not_enough_memory = 0;

	/* First we need to find out if we're in "ETIM" (equivalent time) mode,
	 * because things are done differently. You can't set the sample rate,
	 * and if you query it, you get a meaningless answer. You must work out
	 * what the effective sample rate is from the waveform xincrement. A
	 * pain in the arse, quite frankly. */

	strcpy(cmd, ":ACQ:MODE?");
	vxi11_send_and_receive(client, link, cmd, strlen(cmd), etim_result, 256, VXI11_READ_TIMEOUT);

	/* Equivalent time (ETIM) mode: */
	if (strncmp("ETIM",etim_result,4) == 0) {
		/* Find out the time range displayed on the screen */
		strcpy(cmd, ":TIM:RANGE?");
		time_range = vxi11_obtain_double_value(client, link, cmd, strlen(cmd));

		/* Find the xincrement, whilst we're still in auto (points) mode */
		strcpy(cmd, ":ACQ:POINTS?");
		auto_npoints = vxi11_obtain_long_value(client, link, cmd, strlen(cmd));

		/* Set the no of acquisition points to manual */
		strcpy(cmd, ":ACQ:POINTS:AUTO 0");
		vxi11_send(client, link, cmd, strlen(cmd));

		if (npoints <= 0) { // if we've not been passed a value for npoints
			npoints = auto_npoints;
			}
		/* Remember we want at LEAST the number of points specified.
		 * To some extent, the xinc value is determined by the
		 * number of points. So to get the best xinc value we ask
		 * for double what we actually want. */
		sprintf(cmd, ":ACQ:POINTS %ld",(2*npoints)-1);
		vxi11_send(client, link, cmd, strlen(cmd));

		/* Unfortunately we have to do a :dig, to make sure our changes have
		 * been registered */
		strcpy(cmd, ":DIG");
		vxi11_send(client, link, cmd, strlen(cmd));

		/* Find the xincrement is now*/
		strcpy(cmd, ":WAV:XINC?");
		xinc = vxi11_obtain_double_value(client, link, cmd, strlen(cmd), timeout);

		/* Work out the number of points there _should_ be to cover the time range */
		actual_npoints = (long) ( (time_range / xinc) + 0.5);

		/* Set the number of points accordingly. Hopefully the
		 * xincrement won't have changed! */
		sprintf(cmd, ":ACQ:POINTS %ld",actual_npoints);
		vxi11_send(client, link, cmd, strlen(cmd));

		/* This is a bit anal... we can work out very easily what the equivalent
		 * sampling rate is (1 / xinc); the scope seems to store this value
		 * somewhere, even though it doesn't use it. We may as well write it
		 * to the scope, in case some user program asks for it while in
		 * equivalent time mode. Should not be depended upon, though! */

		sprintf(cmd, ":ACQ:SRAT %G", (1/xinc));
		vxi11_send(client, link, cmd, strlen(cmd));
		}

	/* Real time (RTIM, NORM or PDET) mode: */
	else {
		/* First find out what the sample rate is set to.
		 * Each time you switch from auto to manual for either of these, the
		 * scope remembers the values from last time you set these manually.
		 * This is not very useful to us. We want to be able to set either the
		 * sample rate (and derive npoints from that and the timebase), or the
		 * minimum number of points (and derive the sample rate) or let
		 * the scope choose sensible values for both of these. We only want to
		 * capture the data for the time period displayed on the scope screen,
		 * which is equal to the time range. If you leave the scope to do
		 * everything auto, then it always acquires a bit more than what's on
		 * the screen.
		 */
		strcpy(cmd, ":ACQ:SRAT?");
		auto_srat = vxi11_obtain_double_value(client, link, cmd, strlen(cmd));

		/* Set the sample rate (SRAT) and no of acquisition points to manual */
		sprintf(cmd, ":ACQ:SRAT:AUTO 0;:ACQ:POINTS:AUTO 0");
		vxi11_send(client, link, cmd, strlen(cmd));

		/* Find out the time range displayed on the screen */
		strcpy(cmd, ":TIM:RANGE?");
		time_range = vxi11_obtain_double_value(client, link, cmd, strlen(cmd));

		/* If we've not been passed a sample rate (ie s_rate <= 0) then... */
		if (s_rate <= 0 ) {
			/* ... if we've not been passed npoints, let scope set rate */
			if (npoints <= 0) {
				s_rate = auto_srat;
				}
			/* ... otherwise set the sample rate based on no of points. */
			else {
				s_rate = (double) npoints / time_range;
				}
			}
		/* We make a note here of what we're expecting the sample rate to be.
		 * If it has to change for any reason (dodgy value, or not enough
		 * memory) we will know about it.
		 */
		expected_s_rate = s_rate;

		/* Now we set the number of points to acquire. Of course, the scope
		 * may not have enough memory to acquire all the points, so we just
		 * sit in a loop, reducing the sample rate each time, until it's happy.
		 */
		do {
			/* Send scope our desired sample rate. */
			sprintf(cmd, ":ACQ:SRAT %G",s_rate);
			vxi11_send(client, link, cmd, strlen(cmd));
			/* Scope will choose next highest allowed rate.
			 * Find out what this is */
			strcpy(cmd, ":ACQ:SRAT?");
			actual_s_rate = vxi11_obtain_double_value(client, link, cmd, strlen(cmd));

			/* Calculate the number of points on display (and round up for rounding errors) */
			npoints = (long) ((time_range * actual_s_rate)+0.5);

			/* Set the number of points accordingly */
			/* Note this won't necessarily be the no of points you receive, eg if you have
			 * sin(x)/x interpolation turned on, you will probably get more. */
			sprintf(cmd, ":ACQ:POINTS %ld",npoints);
			vxi11_send(client, link, cmd, strlen(cmd));

			/* We should do a check, see if there's enough memory */
			strcpy(cmd, ":ACQ:POINTS?");
			actual_npoints = vxi11_obtain_long_value(client, link, cmd, strlen(cmd));

			if (actual_npoints < npoints) {
				not_enough_memory = 1;
				ret_val = -1;		/* We should report this fact to the calling function */
				s_rate = s_rate * 0.75 * (double) ( (double) actual_npoints / (double) npoints );
				}
			else {
				not_enough_memory = 0;
				}
			} while (not_enough_memory == 1);
		/* Will possibly remove the explicit printf's here, maybe leave it up to the
		 * calling function to spot potential problems (the user may not care!) */
		if (actual_s_rate != expected_s_rate) {
			printf("Warning: the sampling rate has been adjusted,\n");
			printf("from %g to %g, because ", expected_s_rate, actual_s_rate);
			if (ret_val == -1) {
				printf("there was not enough memory.\n");
				}
			else {
				printf("because %g Sa/s is not a valid sample rate.\n",expected_s_rate);
				ret_val = -2;
				}
			}
		}
	return ret_val;
	}
	

void	agilent_set_for_auto(CLIENT *client, VXI11_LINK *link) {
char	cmd[256];
	/* Return the scope to its auto condition */
	sprintf(cmd, ":ACQ:SRAT:AUTO 1;:ACQ:POINTS:AUTO 1;:RUN");
	vxi11_send(client, link, cmd, strlen(cmd));
	}


void	agilent_scope_channel_str(char chan, char *source){
	switch (chan) {
		case 'A' :
		case 'a' : strcpy(source,"FUNC1");
			break;
		case 'B' :
		case 'b' : strcpy(source,"FUNC2");
			break;
		case 'C' :
		case 'c' : strcpy(source,"FUNC3");
			break;
		case 'D' :
		case 'd' : strcpy(source,"FUNC4");
			break;
		case '1' : strcpy(source,"CHAN1");
			break;
		case '2' : strcpy(source,"CHAN2");
			break;
		case '3' : strcpy(source,"CHAN3");
			break;
		case '4' : strcpy(source,"CHAN4");
			break;
		default :  printf("error: unknown channel '%c', using channel 1\n",chan);
			   strcpy(source,"CHAN1");
			break;
		}
	}

/* Wrapper function; implicitely does the :dig command before grabbing the data
 * (safer but possibly slower, if you've already done a dig to get your wfi_file
 * info) */
long	agilent_get_data(CLIENT *client, VXI11_LINK *link, char chan, char *buf, unsigned long buf_len, unsigned long timeout) {
	return agilent_get_data(client, link, chan, 1, buf, buf_len, timeout);
	}

long	agilent_get_data(CLIENT *client, VXI11_LINK *link, char chan, int digitise, char *buf, unsigned long buf_len, unsigned long timeout) {
char	source[20];
char	cmd[256];
int	ret;
long	bytes_returned;

	if (digitise != 0) {
		agilent_scope_channel_str(chan, source);
		sprintf(cmd,":wav:source %s",source);
		ret=vxi11_send(client, link, cmd, strlen(cmd));
		sprintf(cmd,":DIG");
		ret=vxi11_send(client, link, cmd, strlen(cmd));
		if(ret < 0) {
			printf("error, could not send :DIG cmd, quitting...\n");
			return ret;
			}
		}
	memset(cmd,0,256);
	sprintf(cmd,":WAV:DATA?");
	ret=vxi11_send(client, link, cmd, strlen(cmd));
	bytes_returned=vxi11_receive_data_block(client, link, buf, buf_len, timeout);
	return bytes_returned;
	}




int	agilent_get_preamble(CLIENT *client, VXI11_LINK *link, char* buf, unsigned long buf_len) {
char	cmd[256];
int	ret;
long	bytes_returned;

	sprintf(cmd,":WAV:PRE?");
	ret=vxi11_send(client, link, cmd, strlen(cmd));
	if(ret < 0) {
		printf("error, could not send :WAV:PRE? cmd, quitting...\n");
		return ret;
		}

	bytes_returned=vxi11_receive(client, link, buf, buf_len);

	return (int) bytes_returned;
	}

/* $Id: agilent_load_setup.c,v 1.2 2006/07/07 07:32:50 sds Exp $ */

/*
 * $Log: agilent_load_setup.c,v $
 * Revision 1.2  2006/07/07 07:32:50  sds
 * added revision info, short description, and GNU GPL license.
 *
 */

/* agilent_load_setup.c
 * Copyright (C) 2006 Steve D. Sharples
 *
 * Command line utility to load a previously saved Agilent Scope Setup (.ass)
 * file, and upload it to an Agilent Infiniium (and possibly other model)
 * oscilloscope. As well as performing a useful function, it also illustrates
 * the very simple steps required to begin communicating with your scope
 * from Linux over ethernet, via the VXI11 RPC protocol.
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

#include "../../library/agilent_user.h"
#define BUF_LEN 30000

int	main(int argc, char *argv[]) {

static char *device_ip;
static char *filename;
char	buf[BUF_LEN];
FILE	*fi;
long	bytes_returned;
CLINK	*clink;

	clink = new CLINK;
	if (argc != 3) {
		printf("usage: %s www.xxx.yyy.zzz filename.ass\n",argv[0]);
		printf("Uploads the .ass (Agilent Scope Setup) file to an Agilent scope\n");
		exit(1);
		}
	device_ip = argv[1];
	filename = argv[2];

	fi=fopen(filename,"r");
	if (fi > 0) {
		bytes_returned=fread(buf, sizeof(char),BUF_LEN,fi);
		fclose(fi);

		if (agilent_open(device_ip,clink) != 0) {
			printf("Quitting...\n");
			exit(2);
			}

		agilent_send_setup(clink, buf, bytes_returned);

		agilent_close(device_ip,clink);
		}
	else {
		printf("error: could not open file for reading, quitting...\n");
		exit(3);
		}
	}

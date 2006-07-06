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

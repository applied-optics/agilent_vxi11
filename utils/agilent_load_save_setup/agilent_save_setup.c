#include "../../library/agilent_user.h"
#define BUF_LEN 30000

int	main(int argc, char *argv[]) {

static char *device_ip;
static char *filename;
char	buf[BUF_LEN];
FILE	*fo;
long	bytes_returned;
CLINK	*clink;

	clink = new CLINK;

	if (argc != 3) {
		printf("usage: %s www.xxx.yyy.zzz filename.ass\n",argv[0]);
		printf("Saves the current Agilent scope setup as a .ass (Agilent Scope Setup) file\n");
		exit(1);
		}
	device_ip = argv[1];
	filename = argv[2];

	fo=fopen(filename,"w");
	if (fo > 0) {
		if (agilent_open(device_ip, clink) != 0) {
			printf("Quitting...\n");
			exit(2);
			}

		bytes_returned=agilent_get_setup(clink, buf, BUF_LEN);
		if (bytes_returned <= 0) {
			printf("Problem reading the setup, quitting...\n");
			exit(2);
			}

		fwrite(buf, sizeof(char), bytes_returned, fo);
		fclose(fo);
		agilent_close(device_ip, clink);
		}
	else {
		printf("error: could not open file for writing, quitting...\n");
		exit(3);
		}
	}

#include "../../library/agilent_user.h"

int main(int argc, char *argv[])
{

	static char *device_ip;
	int ret = 0;
	int n;
	CLINK *clink;

	clink = new CLINK;

	if (argc < 2) {
		printf("usage: %s 128.243.74.106 (or other IP address) then optionally a number N.\n",
		     argv[0]);
		printf("Reports, about once every couple of seconds, status of an Agilent scope.\n");
		printf("If N is not stated, then sits in a loop until ctrl-c is pressed, otherwise\n");
		printf("Performs the task N times.\n");
		exit(1);
	}
	device_ip = argv[1];
	if (argc == 3)
		n = sscanf(argv[2], "%d", &n);
	else
		n = -1;

	if (vxi11_open_device(device_ip, clink) != 0) {
		printf("Quitting...\n");
		exit(2);
	}

	while (ret == 0 && n != 0) {
		printf("\n****** STATS FROM %s PRESS CTRL-C TO QUIT ******\n",
		       device_ip);
		ret = agilent_report_status(clink, 1000);	/* 1000ms = 1 second timeout */
		n--;
		if (n != 0)
			sleep(2);
	}

	vxi11_close_device(device_ip, clink);
}

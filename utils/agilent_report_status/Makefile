CFLAGS = -g

all:	agilent_report_status
.PHONY:	all clean install

agilent_report_status: agilent_report_status.o agilent_user.o vxi11_user.o vxi11_clnt.o vxi11_xdr.o
	g++ $(CFLAGS) -o agilent_report_status agilent_report_status.o agilent_user.o vxi11_user.o vxi11_clnt.o vxi11_xdr.o

agilent_report_status.o: agilent_report_status.c ../../library/agilent_user.c ../../../vxi11/vxi11_user.cc ../../../vxi11/vxi11.h
	g++ $(CFLAGS) -c agilent_report_status.c -o agilent_report_status.o

agilent_user.o:	../../library/agilent_user.c
	g++ $(CFLAGS) -c ../../library/agilent_user.c -o agilent_user.o

vxi11_user.o: ../../../vxi11/vxi11_user.cc ../../../vxi11/vxi11.h
	g++ $(CLFAGS) -c ../../../vxi11/vxi11_user.cc -o vxi11_user.o 

vxi11_clnt.o: ../../../vxi11/vxi11_clnt.c
	g++ $(CLFAGS) -c ../../../vxi11/vxi11_clnt.c -o vxi11_clnt.o

vxi11_xdr.o: ../../../vxi11/vxi11_xdr.c
	g++ $(CLFAGS) -c ../../../vxi11/vxi11_xdr.c -o vxi11_xdr.o

clean:
	rm -f *.o agilent_report_status

install:
	cp -f agilent_report_status /usr/local/bin


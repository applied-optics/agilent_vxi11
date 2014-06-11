CFLAGS = -g

all:	agilent_load_setup agilent_save_setup
.PHONY:	all clean install

agilent_load_setup: agilent_load_setup.o agilent_user.o vxi11_user.o vxi11_clnt.o vxi11_xdr.o
	g++ $(CFLAGS) -o agilent_load_setup agilent_load_setup.o agilent_user.o vxi11_user.o vxi11_clnt.o vxi11_xdr.o

agilent_load_setup.o: agilent_load_setup.c ../../library/agilent_user.c ../../../vxi11/vxi11_user.cc ../../../vxi11/vxi11.h
	g++ $(CFLAGS) -c agilent_load_setup.c -o agilent_load_setup.o

agilent_save_setup: agilent_save_setup.o agilent_user.o vxi11_user.o vxi11_clnt.o vxi11_xdr.o
	g++ $(CFLAGS) -o agilent_save_setup agilent_save_setup.o agilent_user.o vxi11_user.o vxi11_clnt.o vxi11_xdr.o

agilent_save_setup.o: agilent_save_setup.c ../../library/agilent_user.c ../../../vxi11/vxi11_user.cc ../../../vxi11/vxi11.h
	g++ $(CFLAGS) -c agilent_save_setup.c -o agilent_save_setup.o

agilent_user.o:	../../library/agilent_user.c
	g++ $(CFLAGS) -c ../../library/agilent_user.c -o agilent_user.o

vxi11_user.o: ../../../vxi11/vxi11_user.cc ../../../vxi11/vxi11.h
	g++ $(CLFAGS) -c ../../../vxi11/vxi11_user.cc -o vxi11_user.o 

vxi11_clnt.o: ../../../vxi11/vxi11_clnt.c
	g++ $(CLFAGS) -c ../../../vxi11/vxi11_clnt.c -o vxi11_clnt.o

vxi11_xdr.o: ../../../vxi11/vxi11_xdr.c
	g++ $(CLFAGS) -c ../../../vxi11/vxi11_xdr.c -o vxi11_xdr.o

clean:
	rm -f *.o agilent_load_setup agilent_save_setup *.ass

install:
	cp -f agilent_load_setup /usr/local/bin
	cp -f agilent_save_setup /usr/local/bin


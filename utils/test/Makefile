CFLAGS = -g

all:	debug
.PHONY:	all clean install

debug: debug.o agilent_user.o vxi11_user.o vxi11_clnt.o vxi11_xdr.o
	g++ $(CFLAGS) -o debug debug.o agilent_user.o vxi11_user.o vxi11_clnt.o vxi11_xdr.o

debug.o: debug.c ../../library/agilent_user.c ../../../vxi11/vxi11_user.cc ../../../vxi11/vxi11.h
	g++ $(CFLAGS) -c debug.c -o debug.o

agilent_user.o:	../../library/agilent_user.c
	g++ $(CFLAGS) -c ../../library/agilent_user.c -o agilent_user.o

vxi11_user.o: ../../../vxi11/vxi11_user.cc ../../../vxi11/vxi11.h
	g++ $(CLFAGS) -c ../../../vxi11/vxi11_user.cc -o vxi11_user.o 

vxi11_clnt.o: ../../../vxi11/vxi11_clnt.c
	g++ $(CLFAGS) -c ../../../vxi11/vxi11_clnt.c -o vxi11_clnt.o

vxi11_xdr.o: ../../../vxi11/vxi11_xdr.c
	g++ $(CLFAGS) -c ../../../vxi11/vxi11_xdr.c -o vxi11_xdr.o

clean:
	rm -f *.o test.* debug

install:
	cp -f debug /usr/local/bin


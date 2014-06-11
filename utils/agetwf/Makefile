CFLAGS = -g

all:	agetwf agetwf_debug
.PHONY:	all clean install

agetwf: agetwf.o agilent_user.o vxi11_user.o vxi11_clnt.o vxi11_xdr.o
	g++ $(CFLAGS) -o agetwf agetwf.o agilent_user.o vxi11_user.o vxi11_clnt.o vxi11_xdr.o

agetwf.o: agetwf.c ../../library/agilent_user.c ../../../vxi11/vxi11_user.cc ../../../vxi11/vxi11.h
	g++ $(CFLAGS) -c agetwf.c -o agetwf.o

agetwf_debug: agetwf_debug.o agilent_user.o vxi11_user.o vxi11_clnt.o vxi11_xdr.o
	g++ $(CFLAGS) -o agetwf_debug agetwf_debug.o agilent_user.o vxi11_user.o vxi11_clnt.o vxi11_xdr.o

agetwf_debug.o: agetwf_debug.c ../../library/agilent_user.c ../../../vxi11/vxi11_user.cc ../../../vxi11/vxi11.h
	g++ $(CFLAGS) -c agetwf_debug.c -o agetwf_debug.o

agilent_user.o:	../../library/agilent_user.c
	g++ $(CFLAGS) -c ../../library/agilent_user.c -o agilent_user.o

vxi11_user.o: ../../../vxi11/vxi11_user.cc ../../../vxi11/vxi11.h
	g++ $(CLFAGS) -c ../../../vxi11/vxi11_user.cc -o vxi11_user.o 

vxi11_clnt.o: ../../../vxi11/vxi11_clnt.c
	g++ $(CLFAGS) -c ../../../vxi11/vxi11_clnt.c -o vxi11_clnt.o

vxi11_xdr.o: ../../../vxi11/vxi11_xdr.c
	g++ $(CLFAGS) -c ../../../vxi11/vxi11_xdr.c -o vxi11_xdr.o

clean:
	rm -f *.o test.* agetwf agetwf_debug

install:
	cp -f agetwf /usr/local/bin
	cp -f agetwf_debug /usr/local/bin


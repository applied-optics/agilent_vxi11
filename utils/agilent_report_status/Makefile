include ../../config.mk

CFLAGS:=$(CFLAGS) -I../../library

all:	agilent_report_status
.PHONY:	all clean install

agilent_report_status: agilent_report_status.o ../../library/libagilent_user.so.0
	$(CXX) $(LDFLAGS) -o $@ $^

agilent_report_status.o: agilent_report_status.c ../../library/libagilent_user.so.0
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o agilent_report_status

install: all
	$(INSTALL) agilent_report_status $(DESTDIR)$(prefix)/bin


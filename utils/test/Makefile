include ../../config.mk

CFLAGS:=$(CFLAGS) -I../../library

all:	agilent_debug
.PHONY:	all clean install

agilent_debug: debug.o ../../library/libagilent_user.so.0
	$(CXX) $(LDFLAGS) -o $@ $^

debug.o: debug.c ../../library/libagilent_user.so.0
	$(CXX) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o test.* debug

install: all
	$(INSTALL) agilent_debug $(DESTDIR)$(prefix)/bin


include ../../config.mk

CFLAGS:=$(CFLAGS) -I../../library

all:	agilent_debug
.PHONY:	all clean install

agilent_debug: debug.o ../../library/$(full_libname)
	$(CXX) $(LDFLAGS) -o $@ $^ -lvxi11

debug.o: debug.c ../../library/$(full_libname)
	$(CXX) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o test.* debug

install: all
	$(INSTALL) agilent_debug $(DESTDIR)$(prefix)/bin


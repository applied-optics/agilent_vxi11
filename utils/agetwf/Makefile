include ../../config.mk

CFLAGS:=$(CFLAGS) -I../../library

all:	agetwf agetwf_debug
.PHONY:	all clean install

agetwf: agetwf.o ../../library/libagilent_user.so.0
	$(CXX) $(LDFLAGS) -o $@ $^

agetwf.o: agetwf.c ../../library/libagilent_user.so.0
	$(CXX) $(CFLAGS) -c -o $@ $<

agetwf_debug: agetwf_debug.o ../../library/libagilent_user.so.0
	$(CXX) $(LDFLAGS) -o $@ $^

agetwf_debug.o: agetwf_debug.c ../../library/libagilent_user.so.0
	$(CXX) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o test.* agetwf agetwf_debug

install: all
	$(INSTALL) agetwf $(DESTDIR)$(prefix)/bin
	$(INSTALL) agetwf_debug $(DESTDIR)$(prefix)/bin


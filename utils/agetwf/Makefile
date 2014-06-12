include ../../config.mk

CFLAGS:=$(CFLAGS) -I../../library

all:	agetwf agetwf_debug
.PHONY:	all clean install

agetwf: agetwf.o ../../library/$(full_libname)
	$(CXX) $(LDFLAGS) -o $@ $^

agetwf.o: agetwf.c ../../library/$(full_libname)
	$(CXX) $(CFLAGS) -c -o $@ $<

agetwf_debug: agetwf_debug.o ../../library/$(full_libname)
	$(CXX) $(LDFLAGS) -o $@ $^

agetwf_debug.o: agetwf_debug.c ../../library/$(full_libname)
	$(CXX) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o test.* agetwf agetwf_debug

install: all
	$(INSTALL) agetwf $(DESTDIR)$(prefix)/bin
	$(INSTALL) agetwf_debug $(DESTDIR)$(prefix)/bin


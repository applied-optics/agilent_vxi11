include ../../config.mk

CFLAGS:=$(CFLAGS) -I../../library

all:	agilent_load_setup agilent_save_setup
.PHONY:	all clean install

agilent_load_setup: agilent_load_setup.o ../../library/$(full_libname)
	$(CXX) $(LDFLAGS) -o $@ $^

agilent_load_setup.o: agilent_load_setup.c ../../library/$(full_libname)
	$(CXX) $(CFLAGS) -c $< -o $@

agilent_save_setup: agilent_save_setup.o ../../library/$(full_libname)
	$(CXX) $(LDFLAGS) -o $@ $^

agilent_save_setup.o: agilent_save_setup.c ../../library/$(full_libname)
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o agilent_load_setup agilent_save_setup *.ass

install: all
	$(INSTALL) agilent_load_setup $(DESTDIR)$(prefix)/bin
	$(INSTALL) agilent_save_setup $(DESTDIR)$(prefix)/bin


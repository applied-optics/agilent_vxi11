include ../config.mk

.PHONY : all install clean dist distclean

all : $(full_libname)

$(full_libname) : agilent_vxi11.o
	$(CC) -shared -Wl,-soname,$(full_libname) $^ -o $@ -lvxi11

agilent_vxi11.o: agilent_vxi11.c agilent_vxi11.h
	$(CXX) -fPIC $(CFLAGS) -c $< -o $@

TAGS: $(wildcard *.c) $(wildcard *.h) $(wildcard *.c)
	etags $^

clean:
	rm -f *.o $(full_libname) TAGS

install: all
	$(INSTALL) -d $(DESTDIR)$(prefix)/lib${LIB_SUFFIX}/
	$(INSTALL) $(full_libname) $(DESTDIR)$(prefix)/lib${LIB_SUFFIX}/
	ln -sf $(full_libname) $(DESTDIR)$(prefix)/lib${LIB_SUFFIX}/$(libname)
	$(INSTALL) -d $(DESTDIR)$(prefix)/include/
	$(INSTALL) agilent_vxi11.h $(DESTDIR)$(prefix)/include/


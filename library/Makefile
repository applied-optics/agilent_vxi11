include ../config.mk

.PHONY : all install clean dist distclean

all : libagilent_user.so.0

libagilent_user.so.0 : agilent_user.o
	$(CC) -shared -Wl,-soname,libagilent_user.so.0 $^ -o $@

agilent_user.o: agilent_user.c agilent_user.h
	$(CC) -fPIC $(CFLAGS) -c $< -o $@

TAGS: $(wildcard *.c) $(wildcard *.h) $(wildcard *.c)
	etags $^

clean:
	rm -f *.o libagilent_user.so.0 TAGS

install: all
	$(INSTALL) -d $(DESTDIR)$(prefix)/lib${LIB_SUFFIX}/
	$(INSTALL) libagilent_user.so.0 $(DESTDIR)$(prefix)/lib${LIB_SUFFIX}/
	ln -sf libagilent_user.so.0 $(DESTDIR)$(prefix)/lib${LIB_SUFFIX}/libagilent_user.so
	$(INSTALL) -d $(DESTDIR)$(prefix)/include/
	$(INSTALL) agilent_user.h $(DESTDIR)$(prefix)/include/


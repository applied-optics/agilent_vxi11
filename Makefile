VERSION=1.04

include config.mk

DIRS=library utils

.PHONY : all clean install

all :
	for d in ${DIRS}; do $(MAKE) -C $${d}; done

clean :
	for d in ${DIRS}; do $(MAKE) -C $${d} clean; done

install : all
	for d in ${DIRS}; do $(MAKE) -C $${d} install; done

dist : distclean
	mkdir aglient_scope-$(VERSION)
	cp -pr library utils aglient_scope-$(VERSION)/
	cp -p Makefile CMakeLists.txt CHANGELOG.txt README.txt GNU_General_Public_License.txt aglient_scope-$(VERSION)/
	tar -zcf aglient_scope-$(VERSION).tar.gz aglient_scope-$(VERSION)

distclean : 
	rm -rf aglient_scope-$(VERSION)
	rm -f aglient_scope-$(VERSION).tar.gz

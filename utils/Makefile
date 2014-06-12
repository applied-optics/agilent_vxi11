include ../config.mk

DIRS=agetwf agilent_load_save_setup agilent_report_status test

.PHONY : all clean install

all :
	for d in ${DIRS}; do $(MAKE) -C $${d}; done

clean:
	for d in ${DIRS}; do $(MAKE) -C $${d} clean; done

install:
	$(INSTALL) -d $(DESTDIR)$(prefix)/bin/
	for d in ${DIRS}; do $(MAKE) -C $${d} install; done


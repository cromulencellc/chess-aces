include ../config.mk

CC=cc
CFLAGS=-I../src -I../include -I. -I../ -Wall -ggdb -DDEBUG -DWITH_CLIENT
LDFLAGS=
SOVERSION=1

ifeq ($(WITH_SHARED_LIBRARIES),yes)
SHARED_DEP:=../lib/libmosquitto.so.${SOVERSION}
endif

ifeq ($(WITH_SHARED_LIBRARIES),yes)
ALL_DEPS:= pov
else
endif


.PHONY: all test clean

all : ${ALL_DEPS}

pov : pov.o
	${CROSS_COMPILE}${CC} $(CLIENT_LDFLAGS) $^ -o $@ $(CLIENT_LDADD)

pov.o : pov.c
	${CC} $(CFLAGS) -c $< -o $@

../lib/libmosquitto.so.${SOVERSION} :
	$(MAKE) -C ../lib

../lib/libmosquitto.a :
	$(MAKE) -C ../lib libmosquitto.a

install : all
	$(INSTALL) -d "${DESTDIR}$(prefix)/bin"
	$(INSTALL) ${STRIP_OPTS} pov "${DESTDIR}${prefix}/bin/pov"

uninstall :
	-rm -f "${DESTDIR}${prefix}/bin/pov"

reallyclean : clean

clean :
	-rm -f *.o pov *.gcda *.gcno

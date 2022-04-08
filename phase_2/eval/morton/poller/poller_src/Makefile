include ../config.mk

CC=cc
CFLAGS=-I../src -I../include -I. -I../ -Wall -ggdb -DDEBUG -DWITH_CLIENT
LDFLAGS=
SOVERSION=1

ifeq ($(WITH_SHARED_LIBRARIES),yes)
SHARED_DEP:=../lib/libmosquitto.so.${SOVERSION}
endif

ifeq ($(WITH_SHARED_LIBRARIES),yes)
ALL_DEPS:= poller
else
endif


.PHONY: all test clean

all : ${ALL_DEPS}

poller : poller.o
	${CROSS_COMPILE}${CC} $(CLIENT_LDFLAGS) $^ -o $@ $(CLIENT_LDADD)

poller.o : poller.c
	${CC} $(CFLAGS) -c $< -o $@

../lib/libmosquitto.so.${SOVERSION} :
	$(MAKE) -C ../lib

../lib/libmosquitto.a :
	$(MAKE) -C ../lib libmosquitto.a

install : all
	$(INSTALL) -d "${DESTDIR}$(prefix)/bin"
	$(INSTALL) ${STRIP_OPTS} poller "${DESTDIR}${prefix}/bin/poller"

uninstall :
	-rm -f "${DESTDIR}${prefix}/bin/poller"

reallyclean : clean

clean :
	-rm -f *.o poller *.gcda *.gcno

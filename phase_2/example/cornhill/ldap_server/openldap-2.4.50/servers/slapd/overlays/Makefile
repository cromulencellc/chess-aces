# $OpenLDAP$
## This work is part of OpenLDAP Software <http://www.openldap.org/>.
##
## Copyright 1998-2020 The OpenLDAP Foundation.
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted only as authorized by the OpenLDAP
## Public License.
##
## A copy of this license is available in the file LICENSE in the
## top-level directory of the distribution or, alternatively, at
##---------------------------------------------------------------------------
#
# Top-level Makefile template
#

PACKAGE= OpenLDAP
VERSION= 2.4.50
RELEASEDATE= 2020/04/28


SHELL = /bin/sh

top_builddir = ../../..

srcdir = .
top_srcdir = ../../..

prefix = /usr/local
exec_prefix = ${prefix}
ldap_subdir = /openldap

bindir = ${exec_prefix}/bin
datarootdir = ${prefix}/share
datadir = ${datarootdir}$(ldap_subdir)
includedir = ${prefix}/include
infodir = ${datarootdir}/info
libdir = ${exec_prefix}/lib
libexecdir = ${exec_prefix}/libexec
localstatedir = ${prefix}/var
mandir = ${datarootdir}/man
moduledir = ${exec_prefix}/libexec$(ldap_subdir)
sbindir = ${exec_prefix}/sbin
sharedstatedir = ${prefix}/com
sysconfdir = ${prefix}/etc$(ldap_subdir)
schemadir = $(sysconfdir)/schema

PLAT = UNIX
EXEEXT = 
OBJEXT = o

BUILD_LIBS_DYNAMIC = static

SHTOOL = $(top_srcdir)/build/shtool

INSTALL = $(SHTOOL) install -c
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL) -m 644
INSTALL_SCRIPT = $(INSTALL)

STRIP = -s

LINT = lint
5LINT = 5lint

MKDEP = $(top_srcdir)/build/mkdep $(MKDEPFLAG) \
	-d "$(srcdir)" -c "$(MKDEP_CC)" -m "$(MKDEP_CFLAGS)"
MKDEP_CC	= cc
MKDEP_CFLAGS = -M

MKVERSION = $(top_srcdir)/build/mkversion -v "$(VERSION)"

LIBTOOL = $(SHELL) $(top_builddir)/libtool
LIBRELEASE = 2.4
LIBVERSION = 12:13:10
LTVERSION = -release $(LIBRELEASE) -version-info $(LIBVERSION)

# libtool --only flag for libraries: platform specific
NT_LTONLY_LIB = # --only-$(BUILD_LIBS_DYNAMIC)
LTONLY_LIB = $(UNIX_LTONLY_LIB)

# libtool --only flag for modules: depends on linkage of module
# The BUILD_MOD macro is defined in each backend Makefile.in file
LTONLY_yes = --tag=disable-shared
LTONLY_mod = --tag=disable-static
LTONLY_MOD = $(LTONLY_$(BUILD_MOD))

# platform-specific libtool flags
NT_LTFLAGS_LIB = -no-undefined -avoid-version -rpath $(libdir)
NT_LTFLAGS_MOD = -no-undefined -avoid-version -rpath $(moduledir)
UNIX_LTFLAGS_LIB = $(LTVERSION) -rpath $(libdir)
UNIX_LTFLAGS_MOD = $(LTVERSION) -rpath $(moduledir)

# libtool flags
LTFLAGS     = $(UNIX_LTFLAGS)
LTFLAGS_LIB = $(UNIX_LTFLAGS_LIB)
LTFLAGS_MOD = $(UNIX_LTFLAGS_MOD)

# LIB_DEFS defined in liblber and libldap Makefile.in files.
# MOD_DEFS defined in backend Makefile.in files.

# platform-specific LINK_LIBS defined in various Makefile.in files.
# LINK_LIBS referenced in library and module link commands.
LINK_LIBS = $(MOD_LIBS) $(UNIX_LINK_LIBS)

LTSTATIC = -static

LTLINK   = $(LIBTOOL) --mode=link \
	$(CC) $(LTSTATIC) $(LT_CFLAGS) $(LDFLAGS) $(LTFLAGS)

LTCOMPILE_LIB = $(LIBTOOL) $(LTONLY_LIB) --mode=compile \
	$(CC) $(LT_CFLAGS) $(LT_CPPFLAGS) $(LIB_DEFS) -c

LTLINK_LIB = $(LIBTOOL) $(LTONLY_LIB) --mode=link \
	$(CC) $(LT_CFLAGS) $(LDFLAGS) $(LTFLAGS_LIB)

LTCOMPILE_MOD = $(LIBTOOL) $(LTONLY_MOD) --mode=compile \
	$(CC) $(LT_CFLAGS) $(LT_CPPFLAGS) $(MOD_DEFS) -c

LTLINK_MOD = $(LIBTOOL) $(LTONLY_MOD) --mode=link \
	$(CC) $(LT_CFLAGS) $(LDFLAGS) $(LTFLAGS_MOD)

LTINSTALL = $(LIBTOOL) --mode=install $(INSTALL) 
LTFINISH = $(LIBTOOL) --mode=finish

# Misc UNIX commands used in build environment
AR = ar
BASENAME = basename
CAT = cat
CHMOD = chmod
DATE = date
ECHO = $(SHTOOL) echo
HOSTNAME = $(SHTOOL) echo -e "%h%d"
LN = $(SHTOOL) mkln
LN_H = $(LN)
LN_S = $(LN) -s
MAKEINFO = ${SHELL} /vagrant/chess/cornhill/ldap_server/openldap-2.4.50/build/missing --run makeinfo
MKDIR = $(SHTOOL) mkdir -p
MV = $(SHTOOL) move
PWD = pwd
RANLIB = ranlib
RM = rm -f
SED = sed
SUBST = $(SHTOOL) subst

# For manual pages
# MANCOMPRESS=@MANCOMPRESS@
# MANCOMPRESSSUFFIX=@MANCOMPRESSSUFFIX@
MANCOMPRESS=$(CAT)
MANCOMPRESSSUFFIX=

SOELIM=soelim

INCLUDEDIR= $(top_srcdir)/include
LDAP_INCPATH= -I$(LDAP_INCDIR) -I$(INCLUDEDIR)
LDAP_LIBDIR= $(top_builddir)/libraries

LUTIL_LIBS = 
LTHREAD_LIBS =  -pthread

BDB_LIBS =  -ldb-5.3
SLAPD_NDB_LIBS = 

LDAP_LIBLBER_LA = $(LDAP_LIBDIR)/liblber/liblber.la
LDAP_LIBLDAP_LA = $(LDAP_LIBDIR)/libldap/libldap.la
LDAP_LIBLDAP_R_LA = $(LDAP_LIBDIR)/libldap_r/libldap_r.la

LDAP_LIBREWRITE_A = $(LDAP_LIBDIR)/librewrite/librewrite.a
LDAP_LIBLUNICODE_A = $(LDAP_LIBDIR)/liblunicode/liblunicode.a
LDAP_LIBLUTIL_A = $(LDAP_LIBDIR)/liblutil/liblutil.a

LDAP_L = $(LDAP_LIBLUTIL_A) \
	$(LDAP_LIBLDAP_LA) $(LDAP_LIBLBER_LA)
SLAPD_L = $(LDAP_LIBLUNICODE_A) $(LDAP_LIBREWRITE_A) \
	$(LDAP_LIBLUTIL_A) $(LDAP_LIBLDAP_R_LA) $(LDAP_LIBLBER_LA)

WRAP_LIBS = 
# AutoConfig generated 
AC_CC	= cc
AC_CFLAGS = -g -O2
AC_DEFS =  # -DHAVE_CONFIG_H
AC_LDFLAGS = 
AC_LIBS = -lresolv 

KRB4_LIBS = 
KRB5_LIBS = 
KRB_LIBS =  
SASL_LIBS = -lsasl2
TLS_LIBS = -lssl -lcrypto
AUTH_LIBS = 
SECURITY_LIBS = $(SASL_LIBS) $(KRB_LIBS) $(TLS_LIBS) $(AUTH_LIBS)

MODULES_CPPFLAGS = 
MODULES_LDFLAGS = 
MODULES_LIBS = 
SLAPD_PERL_LDFLAGS = 

SLAPD_SQL_LDFLAGS = 
SLAPD_SQL_INCLUDES = 
SLAPD_SQL_LIBS = 

SLAPD_LIBS =  $(BDB_LIBS)     

# Our Defaults
CC = $(AC_CC)
DEFS = $(LDAP_INCPATH) $(XINCPATH) $(XDEFS) $(AC_DEFS) $(DEFINES)
CFLAGS = $(AC_CFLAGS) $(DEFS)
LDFLAGS = $(LDAP_LIBPATH) $(AC_LDFLAGS) $(XLDFLAGS)
LIBS = $(XLIBS) $(XXLIBS) $(AC_LIBS) $(XXXLIBS)

LT_CFLAGS = $(AC_CFLAGS)
LT_CPPFLAGS = $(DEFS)

all:		all-common all-local FORCE
install:	install-common install-local FORCE
clean:		clean-common clean-local FORCE
veryclean:	veryclean-common veryclean-local FORCE
depend:		depend-common depend-local FORCE

# empty common rules
all-common:
install-common:
clean-common:
veryclean-common:	clean-common FORCE
depend-common:
lint-common:
lint5-common:

# empty local rules
all-local:
install-local:
clean-local:
veryclean-local:	clean-local FORCE
depend-local:
lint-local:
lint5-local:

veryclean: FORCE
	$(RM) Makefile
	$(RM) -r .libs

Makefile: Makefile.in $(top_srcdir)/build/top.mk

pathtest:
	$(SHTOOL) --version

# empty rule for forcing rules
FORCE:

##---------------------------------------------------------------------------

# Makefile.in for overlays
# $OpenLDAP$
## This work is part of OpenLDAP Software <http://www.openldap.org/>.
##
## Copyright 2003-2020 The OpenLDAP Foundation.
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted only as authorized by the OpenLDAP
## Public License.
##
## A copy of this license is available in the file LICENSE in the
## top-level directory of the distribution or, alternatively, at
## <http://www.OpenLDAP.org/license.html>.

SRCS = overlays.c \
	accesslog.c \
	auditlog.c \
	constraint.c \
	dds.c \
	deref.c \
	dyngroup.c \
	dynlist.c \
	memberof.c \
	pcache.c \
	collect.c \
	ppolicy.c \
	refint.c \
	retcode.c \
	rwm.c rwmconf.c rwmdn.c rwmmap.c \
	seqmod.c \
	sssvlv.c \
	syncprov.c \
	translucent.c \
	unique.c \
	valsort.c
OBJS = statover.o \
	 syncprov.o \
	overlays.o

# Add here the objs that are needed by overlays, but do not make it
# into SLAPD_STATIC_OVERLAYS...
OBJDEP=rwm.o rwmconf.o rwmdn.o rwmmap.o

LTONLY_MOD = $(LTONLY_mod)
LDAP_INCDIR= ../../../include       
LDAP_LIBDIR= ../../../libraries

MOD_DEFS = -DSLAPD_IMPORT

shared_LDAP_LIBS = $(LDAP_LIBLDAP_R_LA) $(LDAP_LIBLBER_LA)
NT_LINK_LIBS = -L.. -lslapd $(static_LDAP_LIBS)
UNIX_LINK_LIBS = $(static_LDAP_LIBS)

LIBRARY = ../liboverlays.a
PROGRAMS = 

XINCPATH = -I.. -I$(srcdir)/..
XDEFS = $(MODULES_CPPFLAGS)

static:	$(LIBRARY)

dynamic: $(PROGRAMS)

accesslog.la : accesslog.lo
	$(LTLINK_MOD) -module -o $@ accesslog.lo version.lo $(LINK_LIBS)

auditlog.la : auditlog.lo
	$(LTLINK_MOD) -module -o $@ auditlog.lo version.lo $(LINK_LIBS)

constraint.la : constraint.lo
	$(LTLINK_MOD) -module -o $@ constraint.lo version.lo $(LINK_LIBS)

dds.la : dds.lo
	$(LTLINK_MOD) -module -o $@ dds.lo version.lo $(LINK_LIBS)

deref.la : deref.lo
	$(LTLINK_MOD) -module -o $@ deref.lo version.lo $(LINK_LIBS)

dyngroup.la : dyngroup.lo
	$(LTLINK_MOD) -module -o $@ dyngroup.lo version.lo $(LINK_LIBS)

dynlist.la : dynlist.lo
	$(LTLINK_MOD) -module -o $@ dynlist.lo version.lo $(LINK_LIBS)

memberof.la : memberof.lo
	$(LTLINK_MOD) -module -o $@ memberof.lo version.lo $(LINK_LIBS)

pcache.la : pcache.lo
	$(LTLINK_MOD) -module -o $@ pcache.lo version.lo $(LINK_LIBS)

collect.la : collect.lo
	$(LTLINK_MOD) -module -o $@ collect.lo version.lo $(LINK_LIBS)

ppolicy.la : ppolicy.lo
	$(LTLINK_MOD) -module -o $@ ppolicy.lo version.lo $(LINK_LIBS) $(MODULES_LIBS)

refint.la : refint.lo
	$(LTLINK_MOD) -module -o $@ refint.lo version.lo $(LINK_LIBS)

retcode.la : retcode.lo
	$(LTLINK_MOD) -module -o $@ retcode.lo version.lo $(LINK_LIBS)

rwm_x.o: rwm.o rwmconf.o rwmdn.o rwmmap.o
	$(LD) -r -o $@ rwm.o rwmconf.o rwmdn.o rwmmap.o

rwm.la : rwm.lo rwmconf.lo rwmdn.lo rwmmap.lo
	$(LTLINK_MOD) -module -o $@ rwm.lo rwmconf.lo rwmdn.lo rwmmap.lo version.lo $(LINK_LIBS)

seqmod.la : seqmod.lo
	$(LTLINK_MOD) -module -o $@ seqmod.lo version.lo $(LINK_LIBS)

sssvlv.la : sssvlv.lo
	$(LTLINK_MOD) -module -o $@ sssvlv.lo version.lo $(LINK_LIBS)

syncprov.la : syncprov.lo
	$(LTLINK_MOD) -module -o $@ syncprov.lo version.lo $(LINK_LIBS)

translucent.la : translucent.lo
	$(LTLINK_MOD) -module -o $@ translucent.lo version.lo $(LINK_LIBS)

unique.la : unique.lo
	$(LTLINK_MOD) -module -o $@ unique.lo version.lo $(LINK_LIBS)

valsort.la : valsort.lo
	$(LTLINK_MOD) -module -o $@ valsort.lo version.lo $(LINK_LIBS)

install-local:	$(PROGRAMS)
	@if test -n "$?" ; then \
		$(MKDIR) $(DESTDIR)$(moduledir); \
		$(LTINSTALL) $(INSTALLFLAGS) -m 755 $? $(DESTDIR)$(moduledir);\
	fi

MKDEPFLAG = -l

.SUFFIXES: .c .o .lo

.c.lo:
	$(LTCOMPILE_MOD) $<

statover.o: statover.c $(srcdir)/../slap.h

$(LIBRARY): $(OBJS) version.lo
	$(AR) rs $@ $(OBJS)

# Must fixup depends for non-libtool objects
depend-local: depend-common
	@if test -n "$(OBJS)"; then \
	OBJ2=`echo $(OBJS) $(OBJDEP) | $(SED) -e 's/\.o//g'`; \
	SCR=''; for i in $$OBJ2; do SCR="$$SCR -e s/^$$i.lo:/$$i.o:/"; done; \
	mv Makefile Makefile.bak; $(SED) $$SCR Makefile.bak > Makefile && \
	$(RM) Makefile.bak; fi

veryclean-local:
	$(RM) statover.c

# $OpenLDAP$
## This work is part of OpenLDAP Software <http://www.openldap.org/>.
##
## Copyright 1998-2020 The OpenLDAP Foundation.
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted only as authorized by the OpenLDAP
## Public License.
##
## A copy of this license is available in the file LICENSE in the
## top-level directory of the distribution or, alternatively, at
## <http://www.OpenLDAP.org/license.html>.
##---------------------------------------------------------------------------
#
# Makefile Template for Libraries
#

all-common: $(LIBRARY) $(PROGRAMS)

version.c: Makefile
	$(RM) $@
	$(MKVERSION) $(LIBRARY) > $@

version.o version.lo: version.c $(OBJS)

install-common: FORCE

lint: lint-local FORCE
	$(LINT) $(DEFS) $(DEFINES) $(SRCS)

lint5: lint5-local FORCE
	$(5LINT) $(DEFS) $(DEFINES) $(SRCS)

#
# In the mingw/cygwin environment, the so and dll files must be
# deleted separately, instead of using the {.so*,*.dll} construct
# that was previously used. It just didn't work.
#
clean-common: 	FORCE
	$(RM) $(LIBRARY) ../$(LIBRARY) $(XLIBRARY) \
		$(PROGRAMS) $(XPROGRAMS) $(XSRCS) $(XXSRCS) \
		*.o *.lo a.out *.exe core version.c .libs/*

depend-common: FORCE
	$(MKDEP) $(DEFS) $(DEFINES) $(SRCS) $(XXSRCS)

lint-local: FORCE
lint5-local: FORCE

Makefile: $(top_srcdir)/build/lib.mk

# DO NOT DELETE THIS LINE -- mkdep uses it.
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.


#
# files: -I../../../include -I../../../include -I.. -I./.. overlays.c accesslog.c auditlog.c constraint.c dds.c deref.c dyngroup.c dynlist.c memberof.c pcache.c collect.c ppolicy.c refint.c retcode.c rwm.c rwmconf.c rwmdn.c rwmmap.c seqmod.c sssvlv.c syncprov.c translucent.c unique.c valsort.c
# command: cc -M -I.  -I../../../include -I../../../include -I.. -I./.. overlays.c accesslog.c auditlog.c constraint.c dds.c deref.c dyngroup.c dynlist.c memberof.c pcache.c collect.c ppolicy.c refint.c retcode.c rwm.c rwmconf.c rwmdn.c rwmmap.c seqmod.c sssvlv.c syncprov.c translucent.c unique.c valsort.c
#

overlays.o: overlays.c
overlays.o: /usr/include/stdc-predef.h
overlays.o: ../../../include/portable.h
overlays.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
overlays.o: ../../../include/ac/fdset.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/types.h
overlays.o: /usr/include/features.h
overlays.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
overlays.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
overlays.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
overlays.o: ../../../include/ldap_cdefs.h
overlays.o: ../../../include/ldap_features.h
overlays.o: ../../../include/ac/assert.h
overlays.o: /usr/include/assert.h
overlays.o: ../../../include/ac/localize.h
overlays.o: ../slap.h
overlays.o: ../../../include/ldap_defaults.h
overlays.o: ../../../include/ldap_config.h
overlays.o: /usr/include/stdio.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/libio.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/_G_config.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
overlays.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
overlays.o: ../../../include/ac/stdlib.h
overlays.o: /usr/include/stdlib.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/waitflags.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/floatn.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
overlays.o: /usr/include/x86_64-linux-gnu/sys/types.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
overlays.o: /usr/include/endian.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/endian.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/byteswap.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
overlays.o: /usr/include/x86_64-linux-gnu/sys/select.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/select.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
overlays.o: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
overlays.o: /usr/include/alloca.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
overlays.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
overlays.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
overlays.o: /usr/include/limits.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/local_lim.h
overlays.o: /usr/include/linux/limits.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
overlays.o: ../../../include/ac/syslog.h
overlays.o: /usr/include/syslog.h
overlays.o: /usr/include/x86_64-linux-gnu/sys/syslog.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/syslog-path.h
overlays.o: ../../../include/ac/regex.h
overlays.o: /usr/include/regex.h
overlays.o: ../../../include/ac/signal.h
overlays.o: /usr/include/signal.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/signum.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/sigaction.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
overlays.o: /usr/include/x86_64-linux-gnu/sys/ucontext.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/sigstack.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/sigthread.h
overlays.o: ../../../include/ac/socket.h
overlays.o: /usr/include/poll.h
overlays.o: /usr/include/x86_64-linux-gnu/sys/poll.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/poll.h
overlays.o: /usr/include/x86_64-linux-gnu/sys/socket.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/socket.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/socket_type.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
overlays.o: /usr/include/x86_64-linux-gnu/asm/socket.h
overlays.o: /usr/include/asm-generic/socket.h
overlays.o: /usr/include/x86_64-linux-gnu/asm/sockios.h
overlays.o: /usr/include/asm-generic/sockios.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
overlays.o: /usr/include/x86_64-linux-gnu/sys/un.h
overlays.o: /usr/include/string.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
overlays.o: /usr/include/strings.h
overlays.o: /usr/include/netinet/in.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/in.h
overlays.o: /usr/include/netinet/tcp.h
overlays.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
overlays.o: /usr/include/stdint.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/wchar.h
overlays.o: /usr/include/arpa/inet.h
overlays.o: /usr/include/arpa/nameser.h
overlays.o: /usr/include/x86_64-linux-gnu/sys/param.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/param.h
overlays.o: /usr/include/linux/param.h
overlays.o: /usr/include/x86_64-linux-gnu/asm/param.h
overlays.o: /usr/include/asm-generic/param.h
overlays.o: /usr/include/arpa/nameser_compat.h
overlays.o: /usr/include/netdb.h
overlays.o: /usr/include/rpc/netdb.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/netdb.h
overlays.o: /usr/include/resolv.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
overlays.o: ../../../include/ac/time.h
overlays.o: /usr/include/x86_64-linux-gnu/sys/time.h
overlays.o: /usr/include/time.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/time.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/types/struct_tm.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/types/struct_itimerspec.h
overlays.o: ../../../include/ac/param.h
overlays.o: ../../../include/ac/unistd.h
overlays.o: /usr/include/unistd.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/environments.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/confname.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
overlays.o: /usr/include/getopt.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
overlays.o: ../../../include/lutil_lockf.h
overlays.o: ../../../include/avl.h
overlays.o: ../../../include/ldap_cdefs.h
overlays.o: ../../../include/ldap_log.h
overlays.o: ../../../include/ldap.h
overlays.o: ../../../include/lber.h
overlays.o: ../../../include/lber_types.h
overlays.o: ../../../include/ldap_features.h
overlays.o: ../../../include/ldap_schema.h
overlays.o: ../../../include/lber_pvt.h
overlays.o: ../../../include/ldap_pvt.h
overlays.o: ../../../include/openldap.h
overlays.o: ../../../include/ldap_pvt_uc.h
overlays.o: ../../../include/ac/bytes.h
overlays.o: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
overlays.o: ../../../include/ldap_pvt_thread.h
overlays.o: ../../../include/ldap_int_thread.h
overlays.o: /usr/include/pthread.h
overlays.o: /usr/include/sched.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/sched.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/cpu-set.h
overlays.o: /usr/include/x86_64-linux-gnu/bits/setjmp.h
overlays.o: ../../../include/ldap_queue.h
overlays.o: ../proto-slap.h
accesslog.lo: accesslog.c
accesslog.lo: /usr/include/stdc-predef.h
accesslog.lo: ../../../include/portable.h
accesslog.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
accesslog.lo: ../../../include/ac/fdset.h
accesslog.lo: /usr/include/x86_64-linux-gnu/bits/types.h
accesslog.lo: /usr/include/features.h
accesslog.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
accesslog.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
accesslog.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
accesslog.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
accesslog.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
accesslog.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
accesslog.lo: ../../../include/ldap_cdefs.h
accesslog.lo: ../../../include/ldap_features.h
accesslog.lo: ../../../include/ac/assert.h
accesslog.lo: /usr/include/assert.h
accesslog.lo: ../../../include/ac/localize.h
auditlog.lo: auditlog.c
auditlog.lo: /usr/include/stdc-predef.h
auditlog.lo: ../../../include/portable.h
auditlog.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
auditlog.lo: ../../../include/ac/fdset.h
auditlog.lo: /usr/include/x86_64-linux-gnu/bits/types.h
auditlog.lo: /usr/include/features.h
auditlog.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
auditlog.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
auditlog.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
auditlog.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
auditlog.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
auditlog.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
auditlog.lo: ../../../include/ldap_cdefs.h
auditlog.lo: ../../../include/ldap_features.h
auditlog.lo: ../../../include/ac/assert.h
auditlog.lo: /usr/include/assert.h
auditlog.lo: ../../../include/ac/localize.h
constraint.lo: constraint.c
constraint.lo: /usr/include/stdc-predef.h
constraint.lo: ../../../include/portable.h
constraint.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
constraint.lo: ../../../include/ac/fdset.h
constraint.lo: /usr/include/x86_64-linux-gnu/bits/types.h
constraint.lo: /usr/include/features.h
constraint.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
constraint.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
constraint.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
constraint.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
constraint.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
constraint.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
constraint.lo: ../../../include/ldap_cdefs.h
constraint.lo: ../../../include/ldap_features.h
constraint.lo: ../../../include/ac/assert.h
constraint.lo: /usr/include/assert.h
constraint.lo: ../../../include/ac/localize.h
dds.lo: dds.c
dds.lo: /usr/include/stdc-predef.h
dds.lo: ../../../include/portable.h
dds.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
dds.lo: ../../../include/ac/fdset.h
dds.lo: /usr/include/x86_64-linux-gnu/bits/types.h
dds.lo: /usr/include/features.h
dds.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
dds.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
dds.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
dds.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
dds.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
dds.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
dds.lo: ../../../include/ldap_cdefs.h
dds.lo: ../../../include/ldap_features.h
dds.lo: ../../../include/ac/assert.h
dds.lo: /usr/include/assert.h
dds.lo: ../../../include/ac/localize.h
deref.lo: deref.c
deref.lo: /usr/include/stdc-predef.h
deref.lo: ../../../include/portable.h
deref.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
deref.lo: ../../../include/ac/fdset.h
deref.lo: /usr/include/x86_64-linux-gnu/bits/types.h
deref.lo: /usr/include/features.h
deref.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
deref.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
deref.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
deref.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
deref.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
deref.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
deref.lo: ../../../include/ldap_cdefs.h
deref.lo: ../../../include/ldap_features.h
deref.lo: ../../../include/ac/assert.h
deref.lo: /usr/include/assert.h
deref.lo: ../../../include/ac/localize.h
dyngroup.lo: dyngroup.c
dyngroup.lo: /usr/include/stdc-predef.h
dyngroup.lo: ../../../include/portable.h
dyngroup.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
dyngroup.lo: ../../../include/ac/fdset.h
dyngroup.lo: /usr/include/x86_64-linux-gnu/bits/types.h
dyngroup.lo: /usr/include/features.h
dyngroup.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
dyngroup.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
dyngroup.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
dyngroup.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
dyngroup.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
dyngroup.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
dyngroup.lo: ../../../include/ldap_cdefs.h
dyngroup.lo: ../../../include/ldap_features.h
dyngroup.lo: ../../../include/ac/assert.h
dyngroup.lo: /usr/include/assert.h
dyngroup.lo: ../../../include/ac/localize.h
dynlist.lo: dynlist.c
dynlist.lo: /usr/include/stdc-predef.h
dynlist.lo: ../../../include/portable.h
dynlist.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
dynlist.lo: ../../../include/ac/fdset.h
dynlist.lo: /usr/include/x86_64-linux-gnu/bits/types.h
dynlist.lo: /usr/include/features.h
dynlist.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
dynlist.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
dynlist.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
dynlist.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
dynlist.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
dynlist.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
dynlist.lo: ../../../include/ldap_cdefs.h
dynlist.lo: ../../../include/ldap_features.h
dynlist.lo: ../../../include/ac/assert.h
dynlist.lo: /usr/include/assert.h
dynlist.lo: ../../../include/ac/localize.h
memberof.lo: memberof.c
memberof.lo: /usr/include/stdc-predef.h
memberof.lo: ../../../include/portable.h
memberof.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
memberof.lo: ../../../include/ac/fdset.h
memberof.lo: /usr/include/x86_64-linux-gnu/bits/types.h
memberof.lo: /usr/include/features.h
memberof.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
memberof.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
memberof.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
memberof.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
memberof.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
memberof.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
memberof.lo: ../../../include/ldap_cdefs.h
memberof.lo: ../../../include/ldap_features.h
memberof.lo: ../../../include/ac/assert.h
memberof.lo: /usr/include/assert.h
memberof.lo: ../../../include/ac/localize.h
pcache.lo: pcache.c
pcache.lo: /usr/include/stdc-predef.h
pcache.lo: ../../../include/portable.h
pcache.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
pcache.lo: ../../../include/ac/fdset.h
pcache.lo: /usr/include/x86_64-linux-gnu/bits/types.h
pcache.lo: /usr/include/features.h
pcache.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
pcache.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
pcache.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
pcache.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
pcache.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
pcache.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
pcache.lo: ../../../include/ldap_cdefs.h
pcache.lo: ../../../include/ldap_features.h
pcache.lo: ../../../include/ac/assert.h
pcache.lo: /usr/include/assert.h
pcache.lo: ../../../include/ac/localize.h
collect.lo: collect.c
collect.lo: /usr/include/stdc-predef.h
collect.lo: ../../../include/portable.h
collect.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
collect.lo: ../../../include/ac/fdset.h
collect.lo: /usr/include/x86_64-linux-gnu/bits/types.h
collect.lo: /usr/include/features.h
collect.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
collect.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
collect.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
collect.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
collect.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
collect.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
collect.lo: ../../../include/ldap_cdefs.h
collect.lo: ../../../include/ldap_features.h
collect.lo: ../../../include/ac/assert.h
collect.lo: /usr/include/assert.h
collect.lo: ../../../include/ac/localize.h
ppolicy.lo: ppolicy.c
ppolicy.lo: /usr/include/stdc-predef.h
ppolicy.lo: ../../../include/portable.h
ppolicy.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
ppolicy.lo: ../../../include/ac/fdset.h
ppolicy.lo: /usr/include/x86_64-linux-gnu/bits/types.h
ppolicy.lo: /usr/include/features.h
ppolicy.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
ppolicy.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
ppolicy.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
ppolicy.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
ppolicy.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
ppolicy.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
ppolicy.lo: ../../../include/ldap_cdefs.h
ppolicy.lo: ../../../include/ldap_features.h
ppolicy.lo: ../../../include/ac/assert.h
ppolicy.lo: /usr/include/assert.h
ppolicy.lo: ../../../include/ac/localize.h
refint.lo: refint.c
refint.lo: /usr/include/stdc-predef.h
refint.lo: ../../../include/portable.h
refint.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
refint.lo: ../../../include/ac/fdset.h
refint.lo: /usr/include/x86_64-linux-gnu/bits/types.h
refint.lo: /usr/include/features.h
refint.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
refint.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
refint.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
refint.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
refint.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
refint.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
refint.lo: ../../../include/ldap_cdefs.h
refint.lo: ../../../include/ldap_features.h
refint.lo: ../../../include/ac/assert.h
refint.lo: /usr/include/assert.h
refint.lo: ../../../include/ac/localize.h
retcode.lo: retcode.c
retcode.lo: /usr/include/stdc-predef.h
retcode.lo: ../../../include/portable.h
retcode.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
retcode.lo: ../../../include/ac/fdset.h
retcode.lo: /usr/include/x86_64-linux-gnu/bits/types.h
retcode.lo: /usr/include/features.h
retcode.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
retcode.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
retcode.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
retcode.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
retcode.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
retcode.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
retcode.lo: ../../../include/ldap_cdefs.h
retcode.lo: ../../../include/ldap_features.h
retcode.lo: ../../../include/ac/assert.h
retcode.lo: /usr/include/assert.h
retcode.lo: ../../../include/ac/localize.h
rwm.o: rwm.c
rwm.o: /usr/include/stdc-predef.h
rwm.o: ../../../include/portable.h
rwm.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
rwm.o: ../../../include/ac/fdset.h
rwm.o: /usr/include/x86_64-linux-gnu/bits/types.h
rwm.o: /usr/include/features.h
rwm.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
rwm.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
rwm.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
rwm.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
rwm.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
rwm.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
rwm.o: ../../../include/ldap_cdefs.h
rwm.o: ../../../include/ldap_features.h
rwm.o: ../../../include/ac/assert.h
rwm.o: /usr/include/assert.h
rwm.o: ../../../include/ac/localize.h
rwmconf.o: rwmconf.c
rwmconf.o: /usr/include/stdc-predef.h
rwmconf.o: ../../../include/portable.h
rwmconf.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
rwmconf.o: ../../../include/ac/fdset.h
rwmconf.o: /usr/include/x86_64-linux-gnu/bits/types.h
rwmconf.o: /usr/include/features.h
rwmconf.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
rwmconf.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
rwmconf.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
rwmconf.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
rwmconf.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
rwmconf.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
rwmconf.o: ../../../include/ldap_cdefs.h
rwmconf.o: ../../../include/ldap_features.h
rwmconf.o: ../../../include/ac/assert.h
rwmconf.o: /usr/include/assert.h
rwmconf.o: ../../../include/ac/localize.h
rwmdn.o: rwmdn.c
rwmdn.o: /usr/include/stdc-predef.h
rwmdn.o: ../../../include/portable.h
rwmdn.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
rwmdn.o: ../../../include/ac/fdset.h
rwmdn.o: /usr/include/x86_64-linux-gnu/bits/types.h
rwmdn.o: /usr/include/features.h
rwmdn.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
rwmdn.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
rwmdn.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
rwmdn.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
rwmdn.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
rwmdn.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
rwmdn.o: ../../../include/ldap_cdefs.h
rwmdn.o: ../../../include/ldap_features.h
rwmdn.o: ../../../include/ac/assert.h
rwmdn.o: /usr/include/assert.h
rwmdn.o: ../../../include/ac/localize.h
rwmmap.o: rwmmap.c
rwmmap.o: /usr/include/stdc-predef.h
rwmmap.o: ../../../include/portable.h
rwmmap.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
rwmmap.o: ../../../include/ac/fdset.h
rwmmap.o: /usr/include/x86_64-linux-gnu/bits/types.h
rwmmap.o: /usr/include/features.h
rwmmap.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
rwmmap.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
rwmmap.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
rwmmap.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
rwmmap.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
rwmmap.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
rwmmap.o: ../../../include/ldap_cdefs.h
rwmmap.o: ../../../include/ldap_features.h
rwmmap.o: ../../../include/ac/assert.h
rwmmap.o: /usr/include/assert.h
rwmmap.o: ../../../include/ac/localize.h
seqmod.lo: seqmod.c
seqmod.lo: /usr/include/stdc-predef.h
seqmod.lo: ../../../include/portable.h
seqmod.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
seqmod.lo: ../../../include/ac/fdset.h
seqmod.lo: /usr/include/x86_64-linux-gnu/bits/types.h
seqmod.lo: /usr/include/features.h
seqmod.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
seqmod.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
seqmod.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
seqmod.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
seqmod.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
seqmod.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
seqmod.lo: ../../../include/ldap_cdefs.h
seqmod.lo: ../../../include/ldap_features.h
seqmod.lo: ../../../include/ac/assert.h
seqmod.lo: /usr/include/assert.h
seqmod.lo: ../../../include/ac/localize.h
sssvlv.lo: sssvlv.c
sssvlv.lo: /usr/include/stdc-predef.h
sssvlv.lo: ../../../include/portable.h
sssvlv.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
sssvlv.lo: ../../../include/ac/fdset.h
sssvlv.lo: /usr/include/x86_64-linux-gnu/bits/types.h
sssvlv.lo: /usr/include/features.h
sssvlv.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
sssvlv.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
sssvlv.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
sssvlv.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
sssvlv.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
sssvlv.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
sssvlv.lo: ../../../include/ldap_cdefs.h
sssvlv.lo: ../../../include/ldap_features.h
sssvlv.lo: ../../../include/ac/assert.h
sssvlv.lo: /usr/include/assert.h
sssvlv.lo: ../../../include/ac/localize.h
syncprov.o: syncprov.c
syncprov.o: /usr/include/stdc-predef.h
syncprov.o: ../../../include/portable.h
syncprov.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
syncprov.o: ../../../include/ac/fdset.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/types.h
syncprov.o: /usr/include/features.h
syncprov.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
syncprov.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
syncprov.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
syncprov.o: ../../../include/ldap_cdefs.h
syncprov.o: ../../../include/ldap_features.h
syncprov.o: ../../../include/ac/assert.h
syncprov.o: /usr/include/assert.h
syncprov.o: ../../../include/ac/localize.h
syncprov.o: ../../../include/ac/string.h
syncprov.o: /usr/include/string.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
syncprov.o: /usr/include/strings.h
syncprov.o: ../../../include/lutil.h
syncprov.o: ../../../include/ldap_cdefs.h
syncprov.o: ../../../include/lber_types.h
syncprov.o: ../slap.h
syncprov.o: ../../../include/ldap_defaults.h
syncprov.o: ../../../include/ldap_config.h
syncprov.o: /usr/include/stdio.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/libio.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/_G_config.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
syncprov.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
syncprov.o: ../../../include/ac/stdlib.h
syncprov.o: /usr/include/stdlib.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/waitflags.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/floatn.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
syncprov.o: /usr/include/x86_64-linux-gnu/sys/types.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
syncprov.o: /usr/include/endian.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/endian.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/byteswap.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
syncprov.o: /usr/include/x86_64-linux-gnu/sys/select.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/select.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
syncprov.o: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
syncprov.o: /usr/include/alloca.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
syncprov.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
syncprov.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
syncprov.o: /usr/include/limits.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/local_lim.h
syncprov.o: /usr/include/linux/limits.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
syncprov.o: ../../../include/ac/syslog.h
syncprov.o: /usr/include/syslog.h
syncprov.o: /usr/include/x86_64-linux-gnu/sys/syslog.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/syslog-path.h
syncprov.o: ../../../include/ac/regex.h
syncprov.o: /usr/include/regex.h
syncprov.o: ../../../include/ac/signal.h
syncprov.o: /usr/include/signal.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/signum.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/sigaction.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
syncprov.o: /usr/include/x86_64-linux-gnu/sys/ucontext.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/sigstack.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/sigthread.h
syncprov.o: ../../../include/ac/socket.h
syncprov.o: /usr/include/poll.h
syncprov.o: /usr/include/x86_64-linux-gnu/sys/poll.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/poll.h
syncprov.o: /usr/include/x86_64-linux-gnu/sys/socket.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/socket.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/socket_type.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
syncprov.o: /usr/include/x86_64-linux-gnu/asm/socket.h
syncprov.o: /usr/include/asm-generic/socket.h
syncprov.o: /usr/include/x86_64-linux-gnu/asm/sockios.h
syncprov.o: /usr/include/asm-generic/sockios.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
syncprov.o: /usr/include/x86_64-linux-gnu/sys/un.h
syncprov.o: /usr/include/netinet/in.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/in.h
syncprov.o: /usr/include/netinet/tcp.h
syncprov.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
syncprov.o: /usr/include/stdint.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/wchar.h
syncprov.o: /usr/include/arpa/inet.h
syncprov.o: /usr/include/arpa/nameser.h
syncprov.o: /usr/include/x86_64-linux-gnu/sys/param.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/param.h
syncprov.o: /usr/include/linux/param.h
syncprov.o: /usr/include/x86_64-linux-gnu/asm/param.h
syncprov.o: /usr/include/asm-generic/param.h
syncprov.o: /usr/include/arpa/nameser_compat.h
syncprov.o: /usr/include/netdb.h
syncprov.o: /usr/include/rpc/netdb.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/netdb.h
syncprov.o: /usr/include/resolv.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
syncprov.o: ../../../include/ac/time.h
syncprov.o: /usr/include/x86_64-linux-gnu/sys/time.h
syncprov.o: /usr/include/time.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/time.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/types/struct_tm.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/types/struct_itimerspec.h
syncprov.o: ../../../include/ac/param.h
syncprov.o: ../../../include/ac/unistd.h
syncprov.o: /usr/include/unistd.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/environments.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/confname.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
syncprov.o: /usr/include/getopt.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
syncprov.o: ../../../include/lutil_lockf.h
syncprov.o: ../../../include/avl.h
syncprov.o: ../../../include/ldap_log.h
syncprov.o: ../../../include/ldap.h
syncprov.o: ../../../include/lber.h
syncprov.o: ../../../include/ldap_features.h
syncprov.o: ../../../include/ldap_schema.h
syncprov.o: ../../../include/lber_pvt.h
syncprov.o: ../../../include/ldap_pvt.h
syncprov.o: ../../../include/openldap.h
syncprov.o: ../../../include/ldap_pvt_uc.h
syncprov.o: ../../../include/ac/bytes.h
syncprov.o: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
syncprov.o: ../../../include/ldap_pvt_thread.h
syncprov.o: ../../../include/ldap_int_thread.h
syncprov.o: /usr/include/pthread.h
syncprov.o: /usr/include/sched.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/sched.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/cpu-set.h
syncprov.o: /usr/include/x86_64-linux-gnu/bits/setjmp.h
syncprov.o: ../../../include/ldap_queue.h
syncprov.o: ../proto-slap.h
syncprov.o: ../config.h
syncprov.o: ../../../include/ldap_rq.h
translucent.lo: translucent.c
translucent.lo: /usr/include/stdc-predef.h
translucent.lo: ../../../include/portable.h
translucent.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
translucent.lo: ../../../include/ac/fdset.h
translucent.lo: /usr/include/x86_64-linux-gnu/bits/types.h
translucent.lo: /usr/include/features.h
translucent.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
translucent.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
translucent.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
translucent.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
translucent.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
translucent.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
translucent.lo: ../../../include/ldap_cdefs.h
translucent.lo: ../../../include/ldap_features.h
translucent.lo: ../../../include/ac/assert.h
translucent.lo: /usr/include/assert.h
translucent.lo: ../../../include/ac/localize.h
unique.lo: unique.c
unique.lo: /usr/include/stdc-predef.h
unique.lo: ../../../include/portable.h
unique.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
unique.lo: ../../../include/ac/fdset.h
unique.lo: /usr/include/x86_64-linux-gnu/bits/types.h
unique.lo: /usr/include/features.h
unique.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
unique.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
unique.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
unique.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
unique.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
unique.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
unique.lo: ../../../include/ldap_cdefs.h
unique.lo: ../../../include/ldap_features.h
unique.lo: ../../../include/ac/assert.h
unique.lo: /usr/include/assert.h
unique.lo: ../../../include/ac/localize.h
valsort.lo: valsort.c
valsort.lo: /usr/include/stdc-predef.h
valsort.lo: ../../../include/portable.h
valsort.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
valsort.lo: ../../../include/ac/fdset.h
valsort.lo: /usr/include/x86_64-linux-gnu/bits/types.h
valsort.lo: /usr/include/features.h
valsort.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
valsort.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
valsort.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
valsort.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
valsort.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
valsort.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
valsort.lo: ../../../include/ldap_cdefs.h
valsort.lo: ../../../include/ldap_features.h
valsort.lo: ../../../include/ac/assert.h
valsort.lo: /usr/include/assert.h
valsort.lo: ../../../include/ac/localize.h

# IF YOU PUT ANYTHING HERE IT WILL GO AWAY

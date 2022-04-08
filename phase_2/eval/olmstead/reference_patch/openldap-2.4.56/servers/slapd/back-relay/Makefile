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
VERSION= 2.4.56
RELEASEDATE= 2020/11/10


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
LIBVERSION = 13:4:11
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
MAKEINFO = ${SHELL} /vagrant/chess/olmstead/challenge/openldap-2.4.56/build/missing --run makeinfo
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
LTHREAD_LIBS = 

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

# Makefile.in for back-relay
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

SRCS	= init.c op.c
OBJS	= init.lo op.lo

LDAP_INCDIR= ../../../include       
LDAP_LIBDIR= ../../../libraries

BUILD_OPT = "--enable-relay"
BUILD_MOD = yes

mod_DEFS = -DSLAPD_IMPORT
MOD_DEFS = $(yes_DEFS)

shared_LDAP_LIBS = $(LDAP_LIBLDAP_R_LA) $(LDAP_LIBLBER_LA)
NT_LINK_LIBS = -L.. -lslapd $(static_LDAP_LIBS) $(REWRITE)
UNIX_LINK_LIBS = $(static_LDAP_LIBS) $(REWRITE)

LIBBASE = back_relay

XINCPATH = -I.. -I$(srcdir)/..
XDEFS = $(MODULES_CPPFLAGS)

all-local-lib:	../.backend

../.backend: lib$(LIBBASE).a
	@touch $@

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
# Makefile Template for Server Modules
#

LIBRARY = $(LIBBASE).la
LIBSTAT = lib$(LIBBASE).a

MKDEPFLAG = -l

.SUFFIXES: .c .o .lo

.c.lo:
	$(LTCOMPILE_MOD) $<

all-no lint-no 5lint-no depend-no install-no: FORCE
	@echo "run configure with $(BUILD_OPT) to make $(LIBBASE)"

all-common: all-$(BUILD_MOD)

version.c: Makefile
	$(RM) $@
	$(MKVERSION) $(LIBBASE) > $@

version.lo: version.c $(OBJS)

$(LIBRARY): version.lo
	$(LTLINK_MOD) -module -o $@ $(OBJS) version.lo $(LINK_LIBS)

$(LIBSTAT): version.lo
	$(AR) ruv $@ `echo $(OBJS) | sed 's/\.lo/.o/g'` version.o
	@$(RANLIB) $@

clean-common: clean-lib FORCE
veryclean-common: veryclean-lib FORCE


lint-common: lint-$(BUILD_MOD)

5lint-common: 5lint-$(BUILD_MOD)

depend-common: depend-$(BUILD_MOD)

install-common: install-$(BUILD_MOD)

all-local-mod:
all-mod: $(LIBRARY) all-local-mod FORCE

all-local-lib:
all-yes: $(LIBSTAT) all-local-lib FORCE

install-mod: $(LIBRARY)
	@-$(MKDIR) $(DESTDIR)$(moduledir)
	$(LTINSTALL) $(INSTALLFLAGS) -m 755 $(LIBRARY) $(DESTDIR)$(moduledir)

install-local-lib:
install-yes: install-local-lib FORCE

lint-local-lib:
lint-yes lint-mod: lint-local-lib FORCE
	$(LINT) $(DEFS) $(DEFINES) $(SRCS)

5lint-local-lib:
5lint-yes 5lint-mod: 5lint-local-lib FORCE
	$(5LINT) $(DEFS) $(DEFINES) $(SRCS)

clean-local-lib:
clean-lib: 	clean-local-lib FORCE
	$(RM) $(LIBRARY) $(LIBSTAT) version.c *.o *.lo a.out core .libs/*

depend-local-lib:
depend-yes depend-mod: depend-local-lib FORCE
	$(MKDEP) $(DEFS) $(DEFINES) $(SRCS)

veryclean-local-lib:
veryclean-lib: 	clean-lib veryclean-local-lib

Makefile: $(top_srcdir)/build/mod.mk

# DO NOT DELETE THIS LINE -- mkdep uses it.
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.


#
# files: -I../../../include -I../../../include -I.. -I./.. init.c op.c
# command: cc -M -I.  -I../../../include -I../../../include -I.. -I./.. init.c op.c
#

init.lo: init.c
init.lo: /usr/include/stdc-predef.h
init.lo: ../../../include/portable.h
init.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
init.lo: ../../../include/ac/fdset.h
init.lo: /usr/include/x86_64-linux-gnu/bits/types.h
init.lo: /usr/include/features.h
init.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
init.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
init.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
init.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
init.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
init.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
init.lo: ../../../include/ldap_cdefs.h
init.lo: ../../../include/ldap_features.h
init.lo: ../../../include/ac/assert.h
init.lo: /usr/include/assert.h
init.lo: ../../../include/ac/localize.h
init.lo: /usr/include/stdio.h
init.lo: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
init.lo: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
init.lo: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
init.lo: /usr/include/x86_64-linux-gnu/bits/libio.h
init.lo: /usr/include/x86_64-linux-gnu/bits/_G_config.h
init.lo: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
init.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
init.lo: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
init.lo: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
init.lo: ../../../include/ac/string.h
init.lo: /usr/include/string.h
init.lo: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
init.lo: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
init.lo: /usr/include/strings.h
init.lo: ../slap.h
init.lo: ../../../include/ldap_defaults.h
init.lo: ../../../include/ldap_config.h
init.lo: ../../../include/ac/stdlib.h
init.lo: /usr/include/stdlib.h
init.lo: /usr/include/x86_64-linux-gnu/bits/waitflags.h
init.lo: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
init.lo: /usr/include/x86_64-linux-gnu/bits/floatn.h
init.lo: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
init.lo: /usr/include/x86_64-linux-gnu/sys/types.h
init.lo: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
init.lo: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
init.lo: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
init.lo: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
init.lo: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
init.lo: /usr/include/endian.h
init.lo: /usr/include/x86_64-linux-gnu/bits/endian.h
init.lo: /usr/include/x86_64-linux-gnu/bits/byteswap.h
init.lo: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
init.lo: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
init.lo: /usr/include/x86_64-linux-gnu/sys/select.h
init.lo: /usr/include/x86_64-linux-gnu/bits/select.h
init.lo: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
init.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
init.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
init.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
init.lo: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
init.lo: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
init.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
init.lo: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
init.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
init.lo: /usr/include/alloca.h
init.lo: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
init.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
init.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
init.lo: /usr/include/limits.h
init.lo: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
init.lo: /usr/include/x86_64-linux-gnu/bits/local_lim.h
init.lo: /usr/include/linux/limits.h
init.lo: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
init.lo: ../../../include/ac/syslog.h
init.lo: /usr/include/syslog.h
init.lo: /usr/include/x86_64-linux-gnu/sys/syslog.h
init.lo: /usr/include/x86_64-linux-gnu/bits/syslog-path.h
init.lo: ../../../include/ac/regex.h
init.lo: /usr/include/regex.h
init.lo: ../../../include/ac/signal.h
init.lo: /usr/include/signal.h
init.lo: /usr/include/x86_64-linux-gnu/bits/signum.h
init.lo: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
init.lo: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
init.lo: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
init.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
init.lo: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
init.lo: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
init.lo: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
init.lo: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
init.lo: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
init.lo: /usr/include/x86_64-linux-gnu/bits/sigaction.h
init.lo: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
init.lo: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
init.lo: /usr/include/x86_64-linux-gnu/sys/ucontext.h
init.lo: /usr/include/x86_64-linux-gnu/bits/sigstack.h
init.lo: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
init.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
init.lo: /usr/include/x86_64-linux-gnu/bits/sigthread.h
init.lo: ../../../include/ac/socket.h
init.lo: /usr/include/poll.h
init.lo: /usr/include/x86_64-linux-gnu/sys/poll.h
init.lo: /usr/include/x86_64-linux-gnu/bits/poll.h
init.lo: /usr/include/x86_64-linux-gnu/sys/socket.h
init.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
init.lo: /usr/include/x86_64-linux-gnu/bits/socket.h
init.lo: /usr/include/x86_64-linux-gnu/bits/socket_type.h
init.lo: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
init.lo: /usr/include/x86_64-linux-gnu/asm/socket.h
init.lo: /usr/include/asm-generic/socket.h
init.lo: /usr/include/x86_64-linux-gnu/asm/sockios.h
init.lo: /usr/include/asm-generic/sockios.h
init.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
init.lo: /usr/include/x86_64-linux-gnu/sys/un.h
init.lo: /usr/include/netinet/in.h
init.lo: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
init.lo: /usr/include/x86_64-linux-gnu/bits/in.h
init.lo: /usr/include/netinet/tcp.h
init.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
init.lo: /usr/include/stdint.h
init.lo: /usr/include/x86_64-linux-gnu/bits/wchar.h
init.lo: /usr/include/arpa/inet.h
init.lo: /usr/include/arpa/nameser.h
init.lo: /usr/include/x86_64-linux-gnu/sys/param.h
init.lo: /usr/include/x86_64-linux-gnu/bits/param.h
init.lo: /usr/include/linux/param.h
init.lo: /usr/include/x86_64-linux-gnu/asm/param.h
init.lo: /usr/include/asm-generic/param.h
init.lo: /usr/include/arpa/nameser_compat.h
init.lo: /usr/include/netdb.h
init.lo: /usr/include/rpc/netdb.h
init.lo: /usr/include/x86_64-linux-gnu/bits/netdb.h
init.lo: /usr/include/resolv.h
init.lo: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
init.lo: ../../../include/ac/time.h
init.lo: /usr/include/x86_64-linux-gnu/sys/time.h
init.lo: /usr/include/time.h
init.lo: /usr/include/x86_64-linux-gnu/bits/time.h
init.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_tm.h
init.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_itimerspec.h
init.lo: ../../../include/ac/param.h
init.lo: ../../../include/ac/unistd.h
init.lo: /usr/include/unistd.h
init.lo: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
init.lo: /usr/include/x86_64-linux-gnu/bits/environments.h
init.lo: /usr/include/x86_64-linux-gnu/bits/confname.h
init.lo: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
init.lo: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
init.lo: /usr/include/getopt.h
init.lo: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
init.lo: ../../../include/lutil_lockf.h
init.lo: ../../../include/avl.h
init.lo: ../../../include/ldap_cdefs.h
init.lo: ../../../include/ldap_log.h
init.lo: ../../../include/ldap.h
init.lo: ../../../include/lber.h
init.lo: ../../../include/lber_types.h
init.lo: ../../../include/ldap_features.h
init.lo: ../../../include/ldap_schema.h
init.lo: ../../../include/lber_pvt.h
init.lo: ../../../include/ldap_pvt.h
init.lo: ../../../include/openldap.h
init.lo: ../../../include/ldap_pvt_uc.h
init.lo: ../../../include/ac/bytes.h
init.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
init.lo: ../../../include/ldap_pvt_thread.h
init.lo: ../../../include/ldap_int_thread.h
init.lo: ../../../include/ldap_queue.h
init.lo: ../proto-slap.h
init.lo: ../config.h
init.lo: back-relay.h
init.lo: proto-back-relay.h
op.lo: op.c
op.lo: /usr/include/stdc-predef.h
op.lo: ../../../include/portable.h
op.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
op.lo: ../../../include/ac/fdset.h
op.lo: /usr/include/x86_64-linux-gnu/bits/types.h
op.lo: /usr/include/features.h
op.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
op.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
op.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
op.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
op.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
op.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
op.lo: ../../../include/ldap_cdefs.h
op.lo: ../../../include/ldap_features.h
op.lo: ../../../include/ac/assert.h
op.lo: /usr/include/assert.h
op.lo: ../../../include/ac/localize.h
op.lo: /usr/include/stdio.h
op.lo: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
op.lo: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
op.lo: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
op.lo: /usr/include/x86_64-linux-gnu/bits/libio.h
op.lo: /usr/include/x86_64-linux-gnu/bits/_G_config.h
op.lo: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
op.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
op.lo: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
op.lo: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
op.lo: ../slap.h
op.lo: ../../../include/ldap_defaults.h
op.lo: ../../../include/ldap_config.h
op.lo: ../../../include/ac/stdlib.h
op.lo: /usr/include/stdlib.h
op.lo: /usr/include/x86_64-linux-gnu/bits/waitflags.h
op.lo: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
op.lo: /usr/include/x86_64-linux-gnu/bits/floatn.h
op.lo: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
op.lo: /usr/include/x86_64-linux-gnu/sys/types.h
op.lo: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
op.lo: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
op.lo: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
op.lo: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
op.lo: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
op.lo: /usr/include/endian.h
op.lo: /usr/include/x86_64-linux-gnu/bits/endian.h
op.lo: /usr/include/x86_64-linux-gnu/bits/byteswap.h
op.lo: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
op.lo: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
op.lo: /usr/include/x86_64-linux-gnu/sys/select.h
op.lo: /usr/include/x86_64-linux-gnu/bits/select.h
op.lo: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
op.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
op.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
op.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
op.lo: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
op.lo: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
op.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
op.lo: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
op.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
op.lo: /usr/include/alloca.h
op.lo: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
op.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
op.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
op.lo: /usr/include/limits.h
op.lo: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
op.lo: /usr/include/x86_64-linux-gnu/bits/local_lim.h
op.lo: /usr/include/linux/limits.h
op.lo: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
op.lo: ../../../include/ac/syslog.h
op.lo: /usr/include/syslog.h
op.lo: /usr/include/x86_64-linux-gnu/sys/syslog.h
op.lo: /usr/include/x86_64-linux-gnu/bits/syslog-path.h
op.lo: ../../../include/ac/regex.h
op.lo: /usr/include/regex.h
op.lo: ../../../include/ac/signal.h
op.lo: /usr/include/signal.h
op.lo: /usr/include/x86_64-linux-gnu/bits/signum.h
op.lo: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
op.lo: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
op.lo: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
op.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
op.lo: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
op.lo: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
op.lo: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
op.lo: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
op.lo: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
op.lo: /usr/include/x86_64-linux-gnu/bits/sigaction.h
op.lo: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
op.lo: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
op.lo: /usr/include/x86_64-linux-gnu/sys/ucontext.h
op.lo: /usr/include/x86_64-linux-gnu/bits/sigstack.h
op.lo: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
op.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
op.lo: /usr/include/x86_64-linux-gnu/bits/sigthread.h
op.lo: ../../../include/ac/socket.h
op.lo: /usr/include/poll.h
op.lo: /usr/include/x86_64-linux-gnu/sys/poll.h
op.lo: /usr/include/x86_64-linux-gnu/bits/poll.h
op.lo: /usr/include/x86_64-linux-gnu/sys/socket.h
op.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
op.lo: /usr/include/x86_64-linux-gnu/bits/socket.h
op.lo: /usr/include/x86_64-linux-gnu/bits/socket_type.h
op.lo: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
op.lo: /usr/include/x86_64-linux-gnu/asm/socket.h
op.lo: /usr/include/asm-generic/socket.h
op.lo: /usr/include/x86_64-linux-gnu/asm/sockios.h
op.lo: /usr/include/asm-generic/sockios.h
op.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
op.lo: /usr/include/x86_64-linux-gnu/sys/un.h
op.lo: /usr/include/string.h
op.lo: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
op.lo: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
op.lo: /usr/include/strings.h
op.lo: /usr/include/netinet/in.h
op.lo: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
op.lo: /usr/include/x86_64-linux-gnu/bits/in.h
op.lo: /usr/include/netinet/tcp.h
op.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
op.lo: /usr/include/stdint.h
op.lo: /usr/include/x86_64-linux-gnu/bits/wchar.h
op.lo: /usr/include/arpa/inet.h
op.lo: /usr/include/arpa/nameser.h
op.lo: /usr/include/x86_64-linux-gnu/sys/param.h
op.lo: /usr/include/x86_64-linux-gnu/bits/param.h
op.lo: /usr/include/linux/param.h
op.lo: /usr/include/x86_64-linux-gnu/asm/param.h
op.lo: /usr/include/asm-generic/param.h
op.lo: /usr/include/arpa/nameser_compat.h
op.lo: /usr/include/netdb.h
op.lo: /usr/include/rpc/netdb.h
op.lo: /usr/include/x86_64-linux-gnu/bits/netdb.h
op.lo: /usr/include/resolv.h
op.lo: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
op.lo: ../../../include/ac/time.h
op.lo: /usr/include/x86_64-linux-gnu/sys/time.h
op.lo: /usr/include/time.h
op.lo: /usr/include/x86_64-linux-gnu/bits/time.h
op.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_tm.h
op.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_itimerspec.h
op.lo: ../../../include/ac/param.h
op.lo: ../../../include/ac/unistd.h
op.lo: /usr/include/unistd.h
op.lo: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
op.lo: /usr/include/x86_64-linux-gnu/bits/environments.h
op.lo: /usr/include/x86_64-linux-gnu/bits/confname.h
op.lo: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
op.lo: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
op.lo: /usr/include/getopt.h
op.lo: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
op.lo: ../../../include/lutil_lockf.h
op.lo: ../../../include/avl.h
op.lo: ../../../include/ldap_cdefs.h
op.lo: ../../../include/ldap_log.h
op.lo: ../../../include/ldap.h
op.lo: ../../../include/lber.h
op.lo: ../../../include/lber_types.h
op.lo: ../../../include/ldap_features.h
op.lo: ../../../include/ldap_schema.h
op.lo: ../../../include/lber_pvt.h
op.lo: ../../../include/ldap_pvt.h
op.lo: ../../../include/openldap.h
op.lo: ../../../include/ldap_pvt_uc.h
op.lo: ../../../include/ac/bytes.h
op.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
op.lo: ../../../include/ldap_pvt_thread.h
op.lo: ../../../include/ldap_int_thread.h
op.lo: ../../../include/ldap_queue.h
op.lo: ../proto-slap.h
op.lo: back-relay.h
op.lo: proto-back-relay.h

# IF YOU PUT ANYTHING HERE IT WILL GO AWAY

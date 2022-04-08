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

# Makefile.in for back-ldif
# $OpenLDAP$
## This work is part of OpenLDAP Software <http://www.openldap.org/>.
##
## Copyright 2005-2020 The OpenLDAP Foundation.
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted only as authorized by the OpenLDAP
## Public License.
##
## A copy of this license is available in the file LICENSE in the
## top-level directory of the distribution or, alternatively, at
## <http://www.OpenLDAP.org/license.html>.

SRCS = ldif.c
OBJS = ldif.lo

LDAP_INCDIR= ../../../include       
LDAP_LIBDIR= ../../../libraries

BUILD_OPT = "--enable-ldif"
BUILD_MOD = yes

mod_DEFS = -DSLAPD_IMPORT
MOD_DEFS = $(yes_DEFS)

shared_LDAP_LIBS = $(LDAP_LIBLDAP_R_LA) $(LDAP_LIBLBER_LA)
NT_LINK_LIBS = -L.. -lslapd $(static_LDAP_LIBS)
UNIX_LINK_LIBS = $(static_LDAP_LIBS)

LIBBASE = back_ldif

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
# files: -I../../../include -I../../../include -I.. -I./.. ldif.c
# command: cc -M -I.  -I../../../include -I../../../include -I.. -I./.. ldif.c
#

ldif.lo: ldif.c
ldif.lo: /usr/include/stdc-predef.h
ldif.lo: ../../../include/portable.h
ldif.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
ldif.lo: ../../../include/ac/fdset.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/types.h
ldif.lo: /usr/include/features.h
ldif.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
ldif.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
ldif.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
ldif.lo: ../../../include/ldap_cdefs.h
ldif.lo: ../../../include/ldap_features.h
ldif.lo: ../../../include/ac/assert.h
ldif.lo: /usr/include/assert.h
ldif.lo: ../../../include/ac/localize.h
ldif.lo: /usr/include/stdio.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/libio.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/_G_config.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
ldif.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
ldif.lo: ../../../include/ac/string.h
ldif.lo: /usr/include/string.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
ldif.lo: /usr/include/strings.h
ldif.lo: /usr/include/x86_64-linux-gnu/sys/types.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
ldif.lo: /usr/include/endian.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/endian.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/byteswap.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
ldif.lo: /usr/include/x86_64-linux-gnu/sys/select.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/select.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
ldif.lo: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
ldif.lo: /usr/include/x86_64-linux-gnu/sys/stat.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/stat.h
ldif.lo: ../../../include/ac/dirent.h
ldif.lo: /usr/include/dirent.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/dirent.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/local_lim.h
ldif.lo: /usr/include/linux/limits.h
ldif.lo: /usr/include/fcntl.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/fcntl.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/fcntl-linux.h
ldif.lo: ../../../include/ac/errno.h
ldif.lo: /usr/include/errno.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/errno.h
ldif.lo: /usr/include/linux/errno.h
ldif.lo: /usr/include/x86_64-linux-gnu/asm/errno.h
ldif.lo: /usr/include/asm-generic/errno.h
ldif.lo: /usr/include/asm-generic/errno-base.h
ldif.lo: ../../../include/ac/unistd.h
ldif.lo: /usr/include/unistd.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/environments.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/confname.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
ldif.lo: /usr/include/getopt.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
ldif.lo: ../../../include/lutil_lockf.h
ldif.lo: ../slap.h
ldif.lo: ../../../include/ldap_defaults.h
ldif.lo: ../../../include/ldap_config.h
ldif.lo: ../../../include/ac/stdlib.h
ldif.lo: /usr/include/stdlib.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/waitflags.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/floatn.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
ldif.lo: /usr/include/alloca.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
ldif.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
ldif.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
ldif.lo: /usr/include/limits.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
ldif.lo: ../../../include/ac/syslog.h
ldif.lo: /usr/include/syslog.h
ldif.lo: /usr/include/x86_64-linux-gnu/sys/syslog.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/syslog-path.h
ldif.lo: ../../../include/ac/regex.h
ldif.lo: /usr/include/regex.h
ldif.lo: ../../../include/ac/signal.h
ldif.lo: /usr/include/signal.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/signum.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/sigaction.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
ldif.lo: /usr/include/x86_64-linux-gnu/sys/ucontext.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/sigstack.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/sigthread.h
ldif.lo: ../../../include/ac/socket.h
ldif.lo: /usr/include/poll.h
ldif.lo: /usr/include/x86_64-linux-gnu/sys/poll.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/poll.h
ldif.lo: /usr/include/x86_64-linux-gnu/sys/socket.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/socket.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/socket_type.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
ldif.lo: /usr/include/x86_64-linux-gnu/asm/socket.h
ldif.lo: /usr/include/asm-generic/socket.h
ldif.lo: /usr/include/x86_64-linux-gnu/asm/sockios.h
ldif.lo: /usr/include/asm-generic/sockios.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
ldif.lo: /usr/include/x86_64-linux-gnu/sys/un.h
ldif.lo: /usr/include/netinet/in.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/in.h
ldif.lo: /usr/include/netinet/tcp.h
ldif.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
ldif.lo: /usr/include/stdint.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/wchar.h
ldif.lo: /usr/include/arpa/inet.h
ldif.lo: /usr/include/arpa/nameser.h
ldif.lo: /usr/include/x86_64-linux-gnu/sys/param.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/param.h
ldif.lo: /usr/include/linux/param.h
ldif.lo: /usr/include/x86_64-linux-gnu/asm/param.h
ldif.lo: /usr/include/asm-generic/param.h
ldif.lo: /usr/include/arpa/nameser_compat.h
ldif.lo: /usr/include/netdb.h
ldif.lo: /usr/include/rpc/netdb.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/netdb.h
ldif.lo: /usr/include/resolv.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
ldif.lo: ../../../include/ac/time.h
ldif.lo: /usr/include/x86_64-linux-gnu/sys/time.h
ldif.lo: /usr/include/time.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/time.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_tm.h
ldif.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_itimerspec.h
ldif.lo: ../../../include/ac/param.h
ldif.lo: ../../../include/avl.h
ldif.lo: ../../../include/ldap_cdefs.h
ldif.lo: ../../../include/ldap_log.h
ldif.lo: ../../../include/ldap.h
ldif.lo: ../../../include/lber.h
ldif.lo: ../../../include/lber_types.h
ldif.lo: ../../../include/ldap_features.h
ldif.lo: ../../../include/ldap_schema.h
ldif.lo: ../../../include/lber_pvt.h
ldif.lo: ../../../include/ldap_pvt.h
ldif.lo: ../../../include/openldap.h
ldif.lo: ../../../include/ldap_pvt_uc.h
ldif.lo: ../../../include/ac/bytes.h
ldif.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
ldif.lo: ../../../include/ldap_pvt_thread.h
ldif.lo: ../../../include/ldap_int_thread.h
ldif.lo: ../../../include/ldap_queue.h
ldif.lo: ../proto-slap.h
ldif.lo: ../../../include/lutil.h
ldif.lo: ../config.h

# IF YOU PUT ANYTHING HERE IT WILL GO AWAY

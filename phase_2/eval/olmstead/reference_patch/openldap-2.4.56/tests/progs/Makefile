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

top_builddir = ../..

srcdir = .
top_srcdir = ../..

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

## Makefile.in for test programs
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

PROGRAMS = slapd-tester slapd-search slapd-read slapd-addel slapd-modrdn \
		slapd-modify slapd-bind slapd-mtread ldif-filter

SRCS     = slapd-common.c \
		slapd-tester.c slapd-search.c slapd-read.c slapd-addel.c \
		slapd-modrdn.c slapd-modify.c slapd-bind.c slapd-mtread.c \
		ldif-filter.c

LDAP_INCDIR= ../../include
LDAP_LIBDIR= ../../libraries

XLIBS    = $(LDAP_LIBLDAP_LA) $(LDAP_LIBLUTIL_A) $(LDAP_LIBLBER_LA)
XRLIBS    = $(LDAP_LIBLDAP_R_LA) $(LDAP_LIBLUTIL_A) $(LDAP_LIBLBER_LA)
XXLIBS	 = $(SECURITY_LIBS) $(LUTIL_LIBS)
RLIBS = $(XRLIBS) $(XXLIBS) $(AC_LIBS) $(XXXLIBS)


OBJS     = slapd-common.o

# build-tools: FORCE
# $(MAKE) $(MFLAGS) load-tools

# load-tools: $(PROGRAMS)

slapd-tester: slapd-tester.o $(OBJS) $(XLIBS)
	$(LTLINK) -o $@ slapd-tester.o $(OBJS) $(LIBS)

slapd-search: slapd-search.o $(OBJS) $(XLIBS)
	$(LTLINK) -o $@ slapd-search.o $(OBJS) $(LIBS)

slapd-read: slapd-read.o $(OBJS) $(XLIBS)
	$(LTLINK) -o $@ slapd-read.o $(OBJS) $(LIBS)

slapd-addel: slapd-addel.o $(OBJS) $(XLIBS)
	$(LTLINK) -o $@ slapd-addel.o $(OBJS) $(LIBS)

slapd-modrdn: slapd-modrdn.o $(OBJS) $(XLIBS)
	$(LTLINK) -o $@ slapd-modrdn.o $(OBJS) $(LIBS)

slapd-modify: slapd-modify.o $(OBJS) $(XLIBS)
	$(LTLINK) -o $@ slapd-modify.o $(OBJS) $(LIBS)

slapd-bind: slapd-bind.o $(OBJS) $(XLIBS)
	$(LTLINK) -o $@ slapd-bind.o $(OBJS) $(LIBS)

ldif-filter: ldif-filter.o $(XLIBS)
	$(LTLINK) -o $@ ldif-filter.o $(LIBS)

slapd-mtread: slapd-mtread.o $(OBJS) $(XRLIBS)
	$(LTLINK) -o $@ slapd-mtread.o $(OBJS) $(RLIBS)

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
# Makefile Template for Programs
#

all-common: $(PROGRAMS) FORCE

clean-common: 	FORCE
	$(RM) $(PROGRAMS) $(XPROGRAMS) $(XSRCS) *.o *.lo a.out core *.core \
		    .libs/* *.exe

depend-common: FORCE
	$(MKDEP) $(DEFS) $(DEFINES) $(SRCS)

lint: FORCE
	$(LINT) $(DEFS) $(DEFINES) $(SRCS)

lint5: FORCE
	$(5LINT) $(DEFS) $(DEFINES) $(SRCS)

Makefile: $(top_srcdir)/build/rules.mk

# DO NOT DELETE THIS LINE -- mkdep uses it.
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.


#
# files: -I../../include -I../../include slapd-common.c slapd-tester.c slapd-search.c slapd-read.c slapd-addel.c slapd-modrdn.c slapd-modify.c slapd-bind.c slapd-mtread.c ldif-filter.c
# command: cc -M -I.  -I../../include -I../../include slapd-common.c slapd-tester.c slapd-search.c slapd-read.c slapd-addel.c slapd-modrdn.c slapd-modify.c slapd-bind.c slapd-mtread.c ldif-filter.c
#

slapd-common.o: slapd-common.c
slapd-common.o: /usr/include/stdc-predef.h
slapd-common.o: ../../include/portable.h
slapd-common.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
slapd-common.o: ../../include/ac/fdset.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/types.h
slapd-common.o: /usr/include/features.h
slapd-common.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
slapd-common.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
slapd-common.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
slapd-common.o: ../../include/ldap_cdefs.h
slapd-common.o: ../../include/ldap_features.h
slapd-common.o: ../../include/ac/assert.h
slapd-common.o: /usr/include/assert.h
slapd-common.o: ../../include/ac/localize.h
slapd-common.o: /usr/include/stdio.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/libio.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/_G_config.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
slapd-common.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
slapd-common.o: ../../include/ac/stdlib.h
slapd-common.o: /usr/include/stdlib.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/waitflags.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/floatn.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
slapd-common.o: /usr/include/x86_64-linux-gnu/sys/types.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
slapd-common.o: /usr/include/endian.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/endian.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/byteswap.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
slapd-common.o: /usr/include/x86_64-linux-gnu/sys/select.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/select.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
slapd-common.o: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
slapd-common.o: /usr/include/alloca.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
slapd-common.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
slapd-common.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
slapd-common.o: /usr/include/limits.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/local_lim.h
slapd-common.o: /usr/include/linux/limits.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
slapd-common.o: ../../include/ac/unistd.h
slapd-common.o: /usr/include/unistd.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/environments.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/confname.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
slapd-common.o: /usr/include/getopt.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
slapd-common.o: ../../include/lutil_lockf.h
slapd-common.o: ../../include/ac/string.h
slapd-common.o: /usr/include/string.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
slapd-common.o: /usr/include/strings.h
slapd-common.o: ../../include/ac/errno.h
slapd-common.o: /usr/include/errno.h
slapd-common.o: /usr/include/x86_64-linux-gnu/bits/errno.h
slapd-common.o: /usr/include/linux/errno.h
slapd-common.o: /usr/include/x86_64-linux-gnu/asm/errno.h
slapd-common.o: /usr/include/asm-generic/errno.h
slapd-common.o: /usr/include/asm-generic/errno-base.h
slapd-common.o: ../../include/ldap.h
slapd-common.o: ../../include/lber.h
slapd-common.o: ../../include/lber_types.h
slapd-common.o: ../../include/ldap_cdefs.h
slapd-common.o: ../../include/ldap_features.h
slapd-common.o: ../../include/ldap_pvt.h
slapd-common.o: ../../include/openldap.h
slapd-common.o: ../../include/lber_pvt.h
slapd-common.o: ../../include/ldap_pvt_uc.h
slapd-common.o: ../../include/ac/bytes.h
slapd-common.o: ../../include/../libraries/liblunicode/ucdata/ucdata.h
slapd-common.o: slapd-common.h
slapd-tester.o: slapd-tester.c
slapd-tester.o: /usr/include/stdc-predef.h
slapd-tester.o: ../../include/portable.h
slapd-tester.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
slapd-tester.o: ../../include/ac/fdset.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/types.h
slapd-tester.o: /usr/include/features.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
slapd-tester.o: ../../include/ldap_cdefs.h
slapd-tester.o: ../../include/ldap_features.h
slapd-tester.o: ../../include/ac/assert.h
slapd-tester.o: /usr/include/assert.h
slapd-tester.o: ../../include/ac/localize.h
slapd-tester.o: /usr/include/stdio.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/libio.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/_G_config.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
slapd-tester.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
slapd-tester.o: ../../include/ac/stdlib.h
slapd-tester.o: /usr/include/stdlib.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/waitflags.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/floatn.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/sys/types.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
slapd-tester.o: /usr/include/endian.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/endian.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/byteswap.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/sys/select.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/select.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
slapd-tester.o: /usr/include/alloca.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
slapd-tester.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
slapd-tester.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
slapd-tester.o: /usr/include/limits.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/local_lim.h
slapd-tester.o: /usr/include/linux/limits.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
slapd-tester.o: ../../include/ac/ctype.h
slapd-tester.o: /usr/include/ctype.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
slapd-tester.o: ../../include/ac/dirent.h
slapd-tester.o: /usr/include/dirent.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/dirent.h
slapd-tester.o: ../../include/ac/param.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/sys/param.h
slapd-tester.o: /usr/include/signal.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/signum.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/sigaction.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/sys/ucontext.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/sigstack.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/sigthread.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/param.h
slapd-tester.o: /usr/include/linux/param.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/asm/param.h
slapd-tester.o: /usr/include/asm-generic/param.h
slapd-tester.o: ../../include/ac/unistd.h
slapd-tester.o: /usr/include/unistd.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/environments.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/confname.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
slapd-tester.o: /usr/include/getopt.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
slapd-tester.o: ../../include/lutil_lockf.h
slapd-tester.o: ../../include/ac/socket.h
slapd-tester.o: /usr/include/poll.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/sys/poll.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/poll.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/sys/socket.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/socket.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/socket_type.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/asm/socket.h
slapd-tester.o: /usr/include/asm-generic/socket.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/asm/sockios.h
slapd-tester.o: /usr/include/asm-generic/sockios.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/sys/un.h
slapd-tester.o: /usr/include/string.h
slapd-tester.o: /usr/include/strings.h
slapd-tester.o: /usr/include/netinet/in.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/in.h
slapd-tester.o: /usr/include/netinet/tcp.h
slapd-tester.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
slapd-tester.o: /usr/include/stdint.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/wchar.h
slapd-tester.o: /usr/include/arpa/inet.h
slapd-tester.o: /usr/include/arpa/nameser.h
slapd-tester.o: /usr/include/arpa/nameser_compat.h
slapd-tester.o: /usr/include/netdb.h
slapd-tester.o: /usr/include/rpc/netdb.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/netdb.h
slapd-tester.o: /usr/include/resolv.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
slapd-tester.o: ../../include/ac/string.h
slapd-tester.o: ../../include/ac/wait.h
slapd-tester.o: /usr/include/x86_64-linux-gnu/sys/wait.h
slapd-tester.o: ../../include/ldap_defaults.h
slapd-tester.o: ../../include/ldap_config.h
slapd-tester.o: ../../include/lutil.h
slapd-tester.o: ../../include/ldap_cdefs.h
slapd-tester.o: ../../include/lber_types.h
slapd-tester.o: ../../include/ldap.h
slapd-tester.o: ../../include/lber.h
slapd-tester.o: ../../include/ldap_features.h
slapd-tester.o: ../../include/ldap_pvt.h
slapd-tester.o: ../../include/openldap.h
slapd-tester.o: ../../include/lber_pvt.h
slapd-tester.o: ../../include/ldap_pvt_uc.h
slapd-tester.o: ../../include/ac/bytes.h
slapd-tester.o: ../../include/../libraries/liblunicode/ucdata/ucdata.h
slapd-tester.o: slapd-common.h
slapd-search.o: slapd-search.c
slapd-search.o: /usr/include/stdc-predef.h
slapd-search.o: ../../include/portable.h
slapd-search.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
slapd-search.o: ../../include/ac/fdset.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/types.h
slapd-search.o: /usr/include/features.h
slapd-search.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
slapd-search.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
slapd-search.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
slapd-search.o: ../../include/ldap_cdefs.h
slapd-search.o: ../../include/ldap_features.h
slapd-search.o: ../../include/ac/assert.h
slapd-search.o: /usr/include/assert.h
slapd-search.o: ../../include/ac/localize.h
slapd-search.o: /usr/include/stdio.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/libio.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/_G_config.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
slapd-search.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
slapd-search.o: ../../include/ac/stdlib.h
slapd-search.o: /usr/include/stdlib.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/waitflags.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/floatn.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
slapd-search.o: /usr/include/x86_64-linux-gnu/sys/types.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
slapd-search.o: /usr/include/endian.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/endian.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/byteswap.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
slapd-search.o: /usr/include/x86_64-linux-gnu/sys/select.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/select.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
slapd-search.o: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
slapd-search.o: /usr/include/alloca.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
slapd-search.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
slapd-search.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
slapd-search.o: /usr/include/limits.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/local_lim.h
slapd-search.o: /usr/include/linux/limits.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
slapd-search.o: ../../include/ac/ctype.h
slapd-search.o: /usr/include/ctype.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
slapd-search.o: ../../include/ac/param.h
slapd-search.o: /usr/include/x86_64-linux-gnu/sys/param.h
slapd-search.o: /usr/include/signal.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/signum.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/sigaction.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
slapd-search.o: /usr/include/x86_64-linux-gnu/sys/ucontext.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/sigstack.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/sigthread.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/param.h
slapd-search.o: /usr/include/linux/param.h
slapd-search.o: /usr/include/x86_64-linux-gnu/asm/param.h
slapd-search.o: /usr/include/asm-generic/param.h
slapd-search.o: ../../include/ac/unistd.h
slapd-search.o: /usr/include/unistd.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/environments.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/confname.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
slapd-search.o: /usr/include/getopt.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
slapd-search.o: ../../include/lutil_lockf.h
slapd-search.o: ../../include/ac/socket.h
slapd-search.o: /usr/include/poll.h
slapd-search.o: /usr/include/x86_64-linux-gnu/sys/poll.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/poll.h
slapd-search.o: /usr/include/x86_64-linux-gnu/sys/socket.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/socket.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/socket_type.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
slapd-search.o: /usr/include/x86_64-linux-gnu/asm/socket.h
slapd-search.o: /usr/include/asm-generic/socket.h
slapd-search.o: /usr/include/x86_64-linux-gnu/asm/sockios.h
slapd-search.o: /usr/include/asm-generic/sockios.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
slapd-search.o: /usr/include/x86_64-linux-gnu/sys/un.h
slapd-search.o: /usr/include/string.h
slapd-search.o: /usr/include/strings.h
slapd-search.o: /usr/include/netinet/in.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/in.h
slapd-search.o: /usr/include/netinet/tcp.h
slapd-search.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
slapd-search.o: /usr/include/stdint.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/wchar.h
slapd-search.o: /usr/include/arpa/inet.h
slapd-search.o: /usr/include/arpa/nameser.h
slapd-search.o: /usr/include/arpa/nameser_compat.h
slapd-search.o: /usr/include/netdb.h
slapd-search.o: /usr/include/rpc/netdb.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/netdb.h
slapd-search.o: /usr/include/resolv.h
slapd-search.o: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
slapd-search.o: ../../include/ac/string.h
slapd-search.o: ../../include/ac/wait.h
slapd-search.o: /usr/include/x86_64-linux-gnu/sys/wait.h
slapd-search.o: ../../include/ldap.h
slapd-search.o: ../../include/lber.h
slapd-search.o: ../../include/lber_types.h
slapd-search.o: ../../include/ldap_cdefs.h
slapd-search.o: ../../include/ldap_features.h
slapd-search.o: ../../include/lutil.h
slapd-search.o: ../../include/ldap_pvt.h
slapd-search.o: ../../include/openldap.h
slapd-search.o: ../../include/lber_pvt.h
slapd-search.o: ../../include/ldap_pvt_uc.h
slapd-search.o: ../../include/ac/bytes.h
slapd-search.o: ../../include/../libraries/liblunicode/ucdata/ucdata.h
slapd-search.o: slapd-common.h
slapd-read.o: slapd-read.c
slapd-read.o: /usr/include/stdc-predef.h
slapd-read.o: ../../include/portable.h
slapd-read.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
slapd-read.o: ../../include/ac/fdset.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/types.h
slapd-read.o: /usr/include/features.h
slapd-read.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
slapd-read.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
slapd-read.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
slapd-read.o: ../../include/ldap_cdefs.h
slapd-read.o: ../../include/ldap_features.h
slapd-read.o: ../../include/ac/assert.h
slapd-read.o: /usr/include/assert.h
slapd-read.o: ../../include/ac/localize.h
slapd-read.o: /usr/include/stdio.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/libio.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/_G_config.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
slapd-read.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
slapd-read.o: ../../include/ac/stdlib.h
slapd-read.o: /usr/include/stdlib.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/waitflags.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/floatn.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
slapd-read.o: /usr/include/x86_64-linux-gnu/sys/types.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
slapd-read.o: /usr/include/endian.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/endian.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/byteswap.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
slapd-read.o: /usr/include/x86_64-linux-gnu/sys/select.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/select.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
slapd-read.o: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
slapd-read.o: /usr/include/alloca.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
slapd-read.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
slapd-read.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
slapd-read.o: /usr/include/limits.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/local_lim.h
slapd-read.o: /usr/include/linux/limits.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
slapd-read.o: ../../include/ac/ctype.h
slapd-read.o: /usr/include/ctype.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
slapd-read.o: ../../include/ac/param.h
slapd-read.o: /usr/include/x86_64-linux-gnu/sys/param.h
slapd-read.o: /usr/include/signal.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/signum.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/sigaction.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
slapd-read.o: /usr/include/x86_64-linux-gnu/sys/ucontext.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/sigstack.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/sigthread.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/param.h
slapd-read.o: /usr/include/linux/param.h
slapd-read.o: /usr/include/x86_64-linux-gnu/asm/param.h
slapd-read.o: /usr/include/asm-generic/param.h
slapd-read.o: ../../include/ac/unistd.h
slapd-read.o: /usr/include/unistd.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/environments.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/confname.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
slapd-read.o: /usr/include/getopt.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
slapd-read.o: ../../include/lutil_lockf.h
slapd-read.o: ../../include/ac/socket.h
slapd-read.o: /usr/include/poll.h
slapd-read.o: /usr/include/x86_64-linux-gnu/sys/poll.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/poll.h
slapd-read.o: /usr/include/x86_64-linux-gnu/sys/socket.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/socket.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/socket_type.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
slapd-read.o: /usr/include/x86_64-linux-gnu/asm/socket.h
slapd-read.o: /usr/include/asm-generic/socket.h
slapd-read.o: /usr/include/x86_64-linux-gnu/asm/sockios.h
slapd-read.o: /usr/include/asm-generic/sockios.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
slapd-read.o: /usr/include/x86_64-linux-gnu/sys/un.h
slapd-read.o: /usr/include/string.h
slapd-read.o: /usr/include/strings.h
slapd-read.o: /usr/include/netinet/in.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/in.h
slapd-read.o: /usr/include/netinet/tcp.h
slapd-read.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
slapd-read.o: /usr/include/stdint.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/wchar.h
slapd-read.o: /usr/include/arpa/inet.h
slapd-read.o: /usr/include/arpa/nameser.h
slapd-read.o: /usr/include/arpa/nameser_compat.h
slapd-read.o: /usr/include/netdb.h
slapd-read.o: /usr/include/rpc/netdb.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/netdb.h
slapd-read.o: /usr/include/resolv.h
slapd-read.o: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
slapd-read.o: ../../include/ac/string.h
slapd-read.o: ../../include/ac/wait.h
slapd-read.o: /usr/include/x86_64-linux-gnu/sys/wait.h
slapd-read.o: ../../include/ldap.h
slapd-read.o: ../../include/lber.h
slapd-read.o: ../../include/lber_types.h
slapd-read.o: ../../include/ldap_cdefs.h
slapd-read.o: ../../include/ldap_features.h
slapd-read.o: ../../include/lutil.h
slapd-read.o: ../../include/ldap_pvt.h
slapd-read.o: ../../include/openldap.h
slapd-read.o: ../../include/lber_pvt.h
slapd-read.o: ../../include/ldap_pvt_uc.h
slapd-read.o: ../../include/ac/bytes.h
slapd-read.o: ../../include/../libraries/liblunicode/ucdata/ucdata.h
slapd-read.o: slapd-common.h
slapd-addel.o: slapd-addel.c
slapd-addel.o: /usr/include/stdc-predef.h
slapd-addel.o: ../../include/portable.h
slapd-addel.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
slapd-addel.o: ../../include/ac/fdset.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/types.h
slapd-addel.o: /usr/include/features.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
slapd-addel.o: ../../include/ldap_cdefs.h
slapd-addel.o: ../../include/ldap_features.h
slapd-addel.o: ../../include/ac/assert.h
slapd-addel.o: /usr/include/assert.h
slapd-addel.o: ../../include/ac/localize.h
slapd-addel.o: /usr/include/stdio.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/libio.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/_G_config.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
slapd-addel.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
slapd-addel.o: ../../include/ac/stdlib.h
slapd-addel.o: /usr/include/stdlib.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/waitflags.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/floatn.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/sys/types.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
slapd-addel.o: /usr/include/endian.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/endian.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/byteswap.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/sys/select.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/select.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
slapd-addel.o: /usr/include/alloca.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
slapd-addel.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
slapd-addel.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
slapd-addel.o: /usr/include/limits.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/local_lim.h
slapd-addel.o: /usr/include/linux/limits.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
slapd-addel.o: ../../include/ac/ctype.h
slapd-addel.o: /usr/include/ctype.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
slapd-addel.o: ../../include/ac/param.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/sys/param.h
slapd-addel.o: /usr/include/signal.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/signum.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/sigaction.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/sys/ucontext.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/sigstack.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/sigthread.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/param.h
slapd-addel.o: /usr/include/linux/param.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/asm/param.h
slapd-addel.o: /usr/include/asm-generic/param.h
slapd-addel.o: ../../include/ac/unistd.h
slapd-addel.o: /usr/include/unistd.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/environments.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/confname.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
slapd-addel.o: /usr/include/getopt.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
slapd-addel.o: ../../include/lutil_lockf.h
slapd-addel.o: ../../include/ac/socket.h
slapd-addel.o: /usr/include/poll.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/sys/poll.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/poll.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/sys/socket.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/socket.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/socket_type.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/asm/socket.h
slapd-addel.o: /usr/include/asm-generic/socket.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/asm/sockios.h
slapd-addel.o: /usr/include/asm-generic/sockios.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/sys/un.h
slapd-addel.o: /usr/include/string.h
slapd-addel.o: /usr/include/strings.h
slapd-addel.o: /usr/include/netinet/in.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/in.h
slapd-addel.o: /usr/include/netinet/tcp.h
slapd-addel.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
slapd-addel.o: /usr/include/stdint.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/wchar.h
slapd-addel.o: /usr/include/arpa/inet.h
slapd-addel.o: /usr/include/arpa/nameser.h
slapd-addel.o: /usr/include/arpa/nameser_compat.h
slapd-addel.o: /usr/include/netdb.h
slapd-addel.o: /usr/include/rpc/netdb.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/netdb.h
slapd-addel.o: /usr/include/resolv.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
slapd-addel.o: ../../include/ac/string.h
slapd-addel.o: ../../include/ac/wait.h
slapd-addel.o: /usr/include/x86_64-linux-gnu/sys/wait.h
slapd-addel.o: ../../include/ldap.h
slapd-addel.o: ../../include/lber.h
slapd-addel.o: ../../include/lber_types.h
slapd-addel.o: ../../include/ldap_cdefs.h
slapd-addel.o: ../../include/ldap_features.h
slapd-addel.o: ../../include/lutil.h
slapd-addel.o: slapd-common.h
slapd-modrdn.o: slapd-modrdn.c
slapd-modrdn.o: /usr/include/stdc-predef.h
slapd-modrdn.o: ../../include/portable.h
slapd-modrdn.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
slapd-modrdn.o: ../../include/ac/fdset.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/types.h
slapd-modrdn.o: /usr/include/features.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
slapd-modrdn.o: ../../include/ldap_cdefs.h
slapd-modrdn.o: ../../include/ldap_features.h
slapd-modrdn.o: ../../include/ac/assert.h
slapd-modrdn.o: /usr/include/assert.h
slapd-modrdn.o: ../../include/ac/localize.h
slapd-modrdn.o: /usr/include/stdio.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/libio.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/_G_config.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
slapd-modrdn.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
slapd-modrdn.o: ../../include/ac/stdlib.h
slapd-modrdn.o: /usr/include/stdlib.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/waitflags.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/floatn.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/sys/types.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
slapd-modrdn.o: /usr/include/endian.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/endian.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/byteswap.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/sys/select.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/select.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
slapd-modrdn.o: /usr/include/alloca.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
slapd-modrdn.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
slapd-modrdn.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
slapd-modrdn.o: /usr/include/limits.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/local_lim.h
slapd-modrdn.o: /usr/include/linux/limits.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
slapd-modrdn.o: ../../include/ac/ctype.h
slapd-modrdn.o: /usr/include/ctype.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
slapd-modrdn.o: ../../include/ac/param.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/sys/param.h
slapd-modrdn.o: /usr/include/signal.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/signum.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/sigaction.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/sys/ucontext.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/sigstack.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/sigthread.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/param.h
slapd-modrdn.o: /usr/include/linux/param.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/asm/param.h
slapd-modrdn.o: /usr/include/asm-generic/param.h
slapd-modrdn.o: ../../include/ac/unistd.h
slapd-modrdn.o: /usr/include/unistd.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/environments.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/confname.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
slapd-modrdn.o: /usr/include/getopt.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
slapd-modrdn.o: ../../include/lutil_lockf.h
slapd-modrdn.o: ../../include/ac/socket.h
slapd-modrdn.o: /usr/include/poll.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/sys/poll.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/poll.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/sys/socket.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/socket.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/socket_type.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/asm/socket.h
slapd-modrdn.o: /usr/include/asm-generic/socket.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/asm/sockios.h
slapd-modrdn.o: /usr/include/asm-generic/sockios.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/sys/un.h
slapd-modrdn.o: /usr/include/string.h
slapd-modrdn.o: /usr/include/strings.h
slapd-modrdn.o: /usr/include/netinet/in.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/in.h
slapd-modrdn.o: /usr/include/netinet/tcp.h
slapd-modrdn.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
slapd-modrdn.o: /usr/include/stdint.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/wchar.h
slapd-modrdn.o: /usr/include/arpa/inet.h
slapd-modrdn.o: /usr/include/arpa/nameser.h
slapd-modrdn.o: /usr/include/arpa/nameser_compat.h
slapd-modrdn.o: /usr/include/netdb.h
slapd-modrdn.o: /usr/include/rpc/netdb.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/netdb.h
slapd-modrdn.o: /usr/include/resolv.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
slapd-modrdn.o: ../../include/ac/string.h
slapd-modrdn.o: ../../include/ac/wait.h
slapd-modrdn.o: /usr/include/x86_64-linux-gnu/sys/wait.h
slapd-modrdn.o: ../../include/ldap.h
slapd-modrdn.o: ../../include/lber.h
slapd-modrdn.o: ../../include/lber_types.h
slapd-modrdn.o: ../../include/ldap_cdefs.h
slapd-modrdn.o: ../../include/ldap_features.h
slapd-modrdn.o: ../../include/lutil.h
slapd-modrdn.o: slapd-common.h
slapd-modify.o: slapd-modify.c
slapd-modify.o: /usr/include/stdc-predef.h
slapd-modify.o: ../../include/portable.h
slapd-modify.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
slapd-modify.o: ../../include/ac/fdset.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/types.h
slapd-modify.o: /usr/include/features.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
slapd-modify.o: ../../include/ldap_cdefs.h
slapd-modify.o: ../../include/ldap_features.h
slapd-modify.o: ../../include/ac/assert.h
slapd-modify.o: /usr/include/assert.h
slapd-modify.o: ../../include/ac/localize.h
slapd-modify.o: /usr/include/stdio.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/libio.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/_G_config.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
slapd-modify.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
slapd-modify.o: ../../include/ac/stdlib.h
slapd-modify.o: /usr/include/stdlib.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/waitflags.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/floatn.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/sys/types.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
slapd-modify.o: /usr/include/endian.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/endian.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/byteswap.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/sys/select.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/select.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
slapd-modify.o: /usr/include/alloca.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
slapd-modify.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
slapd-modify.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
slapd-modify.o: /usr/include/limits.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/local_lim.h
slapd-modify.o: /usr/include/linux/limits.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
slapd-modify.o: ../../include/ac/ctype.h
slapd-modify.o: /usr/include/ctype.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
slapd-modify.o: ../../include/ac/param.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/sys/param.h
slapd-modify.o: /usr/include/signal.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/signum.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/sigaction.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/sys/ucontext.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/sigstack.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/sigthread.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/param.h
slapd-modify.o: /usr/include/linux/param.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/asm/param.h
slapd-modify.o: /usr/include/asm-generic/param.h
slapd-modify.o: ../../include/ac/unistd.h
slapd-modify.o: /usr/include/unistd.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/environments.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/confname.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
slapd-modify.o: /usr/include/getopt.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
slapd-modify.o: ../../include/lutil_lockf.h
slapd-modify.o: ../../include/ac/socket.h
slapd-modify.o: /usr/include/poll.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/sys/poll.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/poll.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/sys/socket.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/socket.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/socket_type.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/asm/socket.h
slapd-modify.o: /usr/include/asm-generic/socket.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/asm/sockios.h
slapd-modify.o: /usr/include/asm-generic/sockios.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/sys/un.h
slapd-modify.o: /usr/include/string.h
slapd-modify.o: /usr/include/strings.h
slapd-modify.o: /usr/include/netinet/in.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/in.h
slapd-modify.o: /usr/include/netinet/tcp.h
slapd-modify.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
slapd-modify.o: /usr/include/stdint.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/wchar.h
slapd-modify.o: /usr/include/arpa/inet.h
slapd-modify.o: /usr/include/arpa/nameser.h
slapd-modify.o: /usr/include/arpa/nameser_compat.h
slapd-modify.o: /usr/include/netdb.h
slapd-modify.o: /usr/include/rpc/netdb.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/netdb.h
slapd-modify.o: /usr/include/resolv.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
slapd-modify.o: ../../include/ac/string.h
slapd-modify.o: ../../include/ac/wait.h
slapd-modify.o: /usr/include/x86_64-linux-gnu/sys/wait.h
slapd-modify.o: ../../include/ldap.h
slapd-modify.o: ../../include/lber.h
slapd-modify.o: ../../include/lber_types.h
slapd-modify.o: ../../include/ldap_cdefs.h
slapd-modify.o: ../../include/ldap_features.h
slapd-modify.o: ../../include/lutil.h
slapd-modify.o: slapd-common.h
slapd-bind.o: slapd-bind.c
slapd-bind.o: /usr/include/stdc-predef.h
slapd-bind.o: ../../include/portable.h
slapd-bind.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
slapd-bind.o: ../../include/ac/fdset.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/types.h
slapd-bind.o: /usr/include/features.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
slapd-bind.o: ../../include/ldap_cdefs.h
slapd-bind.o: ../../include/ldap_features.h
slapd-bind.o: ../../include/ac/assert.h
slapd-bind.o: /usr/include/assert.h
slapd-bind.o: ../../include/ac/localize.h
slapd-bind.o: /usr/include/stdio.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/libio.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/_G_config.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
slapd-bind.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
slapd-bind.o: ../../include/ac/stdlib.h
slapd-bind.o: /usr/include/stdlib.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/waitflags.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/floatn.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/sys/types.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
slapd-bind.o: /usr/include/endian.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/endian.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/byteswap.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/sys/select.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/select.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
slapd-bind.o: /usr/include/alloca.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
slapd-bind.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
slapd-bind.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
slapd-bind.o: /usr/include/limits.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/local_lim.h
slapd-bind.o: /usr/include/linux/limits.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
slapd-bind.o: ../../include/ac/time.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/sys/time.h
slapd-bind.o: /usr/include/time.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/time.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/types/struct_tm.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/types/struct_itimerspec.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
slapd-bind.o: ../../include/ac/ctype.h
slapd-bind.o: /usr/include/ctype.h
slapd-bind.o: ../../include/ac/param.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/sys/param.h
slapd-bind.o: /usr/include/signal.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/signum.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/sigaction.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/sys/ucontext.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/sigstack.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/sigthread.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/param.h
slapd-bind.o: /usr/include/linux/param.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/asm/param.h
slapd-bind.o: /usr/include/asm-generic/param.h
slapd-bind.o: ../../include/ac/unistd.h
slapd-bind.o: /usr/include/unistd.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/environments.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/confname.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
slapd-bind.o: /usr/include/getopt.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
slapd-bind.o: ../../include/lutil_lockf.h
slapd-bind.o: ../../include/ac/socket.h
slapd-bind.o: /usr/include/poll.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/sys/poll.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/poll.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/sys/socket.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/socket.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/socket_type.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/asm/socket.h
slapd-bind.o: /usr/include/asm-generic/socket.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/asm/sockios.h
slapd-bind.o: /usr/include/asm-generic/sockios.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/sys/un.h
slapd-bind.o: /usr/include/string.h
slapd-bind.o: /usr/include/strings.h
slapd-bind.o: /usr/include/netinet/in.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/in.h
slapd-bind.o: /usr/include/netinet/tcp.h
slapd-bind.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
slapd-bind.o: /usr/include/stdint.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/wchar.h
slapd-bind.o: /usr/include/arpa/inet.h
slapd-bind.o: /usr/include/arpa/nameser.h
slapd-bind.o: /usr/include/arpa/nameser_compat.h
slapd-bind.o: /usr/include/netdb.h
slapd-bind.o: /usr/include/rpc/netdb.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/netdb.h
slapd-bind.o: /usr/include/resolv.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
slapd-bind.o: ../../include/ac/string.h
slapd-bind.o: ../../include/ac/wait.h
slapd-bind.o: /usr/include/x86_64-linux-gnu/sys/wait.h
slapd-bind.o: ../../include/ldap.h
slapd-bind.o: ../../include/lber.h
slapd-bind.o: ../../include/lber_types.h
slapd-bind.o: ../../include/ldap_cdefs.h
slapd-bind.o: ../../include/ldap_features.h
slapd-bind.o: ../../include/lutil.h
slapd-bind.o: ../../include/lber_pvt.h
slapd-bind.o: ../../include/ldap_pvt.h
slapd-bind.o: ../../include/openldap.h
slapd-bind.o: ../../include/ldap_pvt_uc.h
slapd-bind.o: ../../include/ac/bytes.h
slapd-bind.o: ../../include/../libraries/liblunicode/ucdata/ucdata.h
slapd-bind.o: slapd-common.h
slapd-mtread.o: slapd-mtread.c
slapd-mtread.o: /usr/include/stdc-predef.h
slapd-mtread.o: ../../include/portable.h
slapd-mtread.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
slapd-mtread.o: ../../include/ac/fdset.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/types.h
slapd-mtread.o: /usr/include/features.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
slapd-mtread.o: ../../include/ldap_cdefs.h
slapd-mtread.o: ../../include/ldap_features.h
slapd-mtread.o: ../../include/ac/assert.h
slapd-mtread.o: /usr/include/assert.h
slapd-mtread.o: ../../include/ac/localize.h
slapd-mtread.o: /usr/include/stdio.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/libio.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/_G_config.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
slapd-mtread.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
slapd-mtread.o: ../../include/ldap_pvt_thread.h
slapd-mtread.o: ../../include/ldap_int_thread.h
slapd-mtread.o: ../../include/ac/stdlib.h
slapd-mtread.o: /usr/include/stdlib.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/waitflags.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/floatn.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/sys/types.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
slapd-mtread.o: /usr/include/endian.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/endian.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/byteswap.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/sys/select.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/select.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
slapd-mtread.o: /usr/include/alloca.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
slapd-mtread.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
slapd-mtread.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
slapd-mtread.o: /usr/include/limits.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/local_lim.h
slapd-mtread.o: /usr/include/linux/limits.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
slapd-mtread.o: ../../include/ac/ctype.h
slapd-mtread.o: /usr/include/ctype.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
slapd-mtread.o: ../../include/ac/param.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/sys/param.h
slapd-mtread.o: /usr/include/signal.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/signum.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/sigaction.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/sys/ucontext.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/sigstack.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/sigthread.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/param.h
slapd-mtread.o: /usr/include/linux/param.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/asm/param.h
slapd-mtread.o: /usr/include/asm-generic/param.h
slapd-mtread.o: ../../include/ac/unistd.h
slapd-mtread.o: /usr/include/unistd.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/environments.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/confname.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
slapd-mtread.o: /usr/include/getopt.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
slapd-mtread.o: ../../include/lutil_lockf.h
slapd-mtread.o: ../../include/ac/socket.h
slapd-mtread.o: /usr/include/poll.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/sys/poll.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/poll.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/sys/socket.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/socket.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/socket_type.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/asm/socket.h
slapd-mtread.o: /usr/include/asm-generic/socket.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/asm/sockios.h
slapd-mtread.o: /usr/include/asm-generic/sockios.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/sys/un.h
slapd-mtread.o: /usr/include/string.h
slapd-mtread.o: /usr/include/strings.h
slapd-mtread.o: /usr/include/netinet/in.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/in.h
slapd-mtread.o: /usr/include/netinet/tcp.h
slapd-mtread.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
slapd-mtread.o: /usr/include/stdint.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/wchar.h
slapd-mtread.o: /usr/include/arpa/inet.h
slapd-mtread.o: /usr/include/arpa/nameser.h
slapd-mtread.o: /usr/include/arpa/nameser_compat.h
slapd-mtread.o: /usr/include/netdb.h
slapd-mtread.o: /usr/include/rpc/netdb.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/netdb.h
slapd-mtread.o: /usr/include/resolv.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
slapd-mtread.o: ../../include/ac/string.h
slapd-mtread.o: ../../include/ac/wait.h
slapd-mtread.o: /usr/include/x86_64-linux-gnu/sys/wait.h
slapd-mtread.o: ../../include/ldap.h
slapd-mtread.o: ../../include/lber.h
slapd-mtread.o: ../../include/lber_types.h
slapd-mtread.o: ../../include/ldap_cdefs.h
slapd-mtread.o: ../../include/ldap_features.h
slapd-mtread.o: ../../include/lutil.h
slapd-mtread.o: ../../include/ldap_pvt.h
slapd-mtread.o: ../../include/openldap.h
slapd-mtread.o: ../../include/lber_pvt.h
slapd-mtread.o: ../../include/ldap_pvt_uc.h
slapd-mtread.o: ../../include/ac/bytes.h
slapd-mtread.o: ../../include/../libraries/liblunicode/ucdata/ucdata.h
slapd-mtread.o: slapd-common.h
ldif-filter.o: ldif-filter.c
ldif-filter.o: /usr/include/stdc-predef.h
ldif-filter.o: ../../include/portable.h
ldif-filter.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
ldif-filter.o: ../../include/ac/fdset.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/types.h
ldif-filter.o: /usr/include/features.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
ldif-filter.o: ../../include/ldap_cdefs.h
ldif-filter.o: ../../include/ldap_features.h
ldif-filter.o: ../../include/ac/assert.h
ldif-filter.o: /usr/include/assert.h
ldif-filter.o: ../../include/ac/localize.h
ldif-filter.o: /usr/include/stdio.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/libio.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/_G_config.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
ldif-filter.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
ldif-filter.o: ../../include/ac/ctype.h
ldif-filter.o: /usr/include/ctype.h
ldif-filter.o: /usr/include/endian.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/endian.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/byteswap.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
ldif-filter.o: ../../include/ac/stdlib.h
ldif-filter.o: /usr/include/stdlib.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/waitflags.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/floatn.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/sys/types.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/sys/select.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/select.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
ldif-filter.o: /usr/include/alloca.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
ldif-filter.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
ldif-filter.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
ldif-filter.o: /usr/include/limits.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/local_lim.h
ldif-filter.o: /usr/include/linux/limits.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
ldif-filter.o: ../../include/ac/string.h
ldif-filter.o: /usr/include/string.h
ldif-filter.o: /usr/include/strings.h
ldif-filter.o: ../../include/ac/unistd.h
ldif-filter.o: /usr/include/unistd.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/environments.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/confname.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
ldif-filter.o: /usr/include/getopt.h
ldif-filter.o: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
ldif-filter.o: ../../include/lutil_lockf.h

# IF YOU PUT ANYTHING HERE IT WILL GO AWAY

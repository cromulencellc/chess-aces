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

# LIBREWRITE
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
##
## Copyright 2000-2001 Pierangelo Masarati <ando@sys-net.it>
##

SRCS = config.c context.c info.c ldapmap.c map.c params.c rule.c \
	session.c subst.c var.c xmap.c \
	parse.c rewrite.c
XSRCS = version.c
OBJS = config.o context.o info.o ldapmap.o map.o params.o rule.o \
	session.o subst.o var.o xmap.o

LDAP_INCDIR= ../../include       
LDAP_LIBDIR= ../../libraries

LIBRARY = librewrite.a
PROGRAMS	= rewrite
XLIBS = $(LIBRARY) $(LDAP_LIBLUTIL_A) \
	$(LDAP_LIBLDAP_R_LA) $(LDAP_LIBLBER_LA)
XXLIBS  = $(SECURITY_LIBS) $(LUTIL_LIBS)
XXXLIBS = $(LTHREAD_LIBS)

rewrite:	$(XLIBS) rewrite.o parse.o
	$(LTLINK) -o $@ rewrite.o parse.o $(LIBS)
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
# Makefile Template for Static Libraries
#

$(LIBRARY): version.o
	$(AR) ru $@ $(OBJS) version.o
	@$(RANLIB) $@

Makefile: $(top_srcdir)/build/lib-static.mk
# DO NOT DELETE THIS LINE -- mkdep uses it.
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.


#
# files: -I../../include -I../../include config.c context.c info.c ldapmap.c map.c params.c rule.c session.c subst.c var.c xmap.c parse.c rewrite.c
# command: cc -M -I.  -I../../include -I../../include config.c context.c info.c ldapmap.c map.c params.c rule.c session.c subst.c var.c xmap.c parse.c rewrite.c
#

config.o: config.c
config.o: /usr/include/stdc-predef.h
config.o: ../../include/portable.h
config.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
config.o: ../../include/ac/fdset.h
config.o: /usr/include/x86_64-linux-gnu/bits/types.h
config.o: /usr/include/features.h
config.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
config.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
config.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
config.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
config.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
config.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
config.o: ../../include/ldap_cdefs.h
config.o: ../../include/ldap_features.h
config.o: ../../include/ac/assert.h
config.o: /usr/include/assert.h
config.o: ../../include/ac/localize.h
config.o: rewrite-int.h
config.o: ../../include/ac/stdlib.h
config.o: /usr/include/stdlib.h
config.o: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
config.o: /usr/include/x86_64-linux-gnu/bits/waitflags.h
config.o: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
config.o: /usr/include/x86_64-linux-gnu/bits/floatn.h
config.o: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
config.o: /usr/include/x86_64-linux-gnu/sys/types.h
config.o: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
config.o: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
config.o: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
config.o: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
config.o: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
config.o: /usr/include/endian.h
config.o: /usr/include/x86_64-linux-gnu/bits/endian.h
config.o: /usr/include/x86_64-linux-gnu/bits/byteswap.h
config.o: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
config.o: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
config.o: /usr/include/x86_64-linux-gnu/sys/select.h
config.o: /usr/include/x86_64-linux-gnu/bits/select.h
config.o: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
config.o: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
config.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
config.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
config.o: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
config.o: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
config.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
config.o: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
config.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
config.o: /usr/include/alloca.h
config.o: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
config.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
config.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
config.o: /usr/include/limits.h
config.o: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
config.o: /usr/include/x86_64-linux-gnu/bits/local_lim.h
config.o: /usr/include/linux/limits.h
config.o: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
config.o: ../../include/ac/string.h
config.o: /usr/include/string.h
config.o: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
config.o: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
config.o: /usr/include/strings.h
config.o: ../../include/ac/syslog.h
config.o: /usr/include/syslog.h
config.o: /usr/include/x86_64-linux-gnu/sys/syslog.h
config.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
config.o: /usr/include/x86_64-linux-gnu/bits/syslog-path.h
config.o: ../../include/ac/regex.h
config.o: /usr/include/regex.h
config.o: ../../include/ac/socket.h
config.o: /usr/include/poll.h
config.o: /usr/include/x86_64-linux-gnu/sys/poll.h
config.o: /usr/include/x86_64-linux-gnu/bits/poll.h
config.o: /usr/include/x86_64-linux-gnu/sys/socket.h
config.o: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
config.o: /usr/include/x86_64-linux-gnu/bits/socket.h
config.o: /usr/include/x86_64-linux-gnu/bits/socket_type.h
config.o: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
config.o: /usr/include/x86_64-linux-gnu/asm/socket.h
config.o: /usr/include/asm-generic/socket.h
config.o: /usr/include/x86_64-linux-gnu/asm/sockios.h
config.o: /usr/include/asm-generic/sockios.h
config.o: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
config.o: /usr/include/x86_64-linux-gnu/sys/un.h
config.o: /usr/include/netinet/in.h
config.o: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
config.o: /usr/include/x86_64-linux-gnu/bits/in.h
config.o: /usr/include/netinet/tcp.h
config.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
config.o: /usr/include/stdint.h
config.o: /usr/include/x86_64-linux-gnu/bits/wchar.h
config.o: /usr/include/arpa/inet.h
config.o: /usr/include/arpa/nameser.h
config.o: /usr/include/x86_64-linux-gnu/sys/param.h
config.o: /usr/include/signal.h
config.o: /usr/include/x86_64-linux-gnu/bits/signum.h
config.o: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
config.o: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
config.o: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
config.o: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
config.o: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
config.o: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
config.o: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
config.o: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
config.o: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
config.o: /usr/include/x86_64-linux-gnu/bits/sigaction.h
config.o: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
config.o: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
config.o: /usr/include/x86_64-linux-gnu/sys/ucontext.h
config.o: /usr/include/x86_64-linux-gnu/bits/sigstack.h
config.o: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
config.o: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
config.o: /usr/include/x86_64-linux-gnu/bits/sigthread.h
config.o: /usr/include/x86_64-linux-gnu/bits/param.h
config.o: /usr/include/linux/param.h
config.o: /usr/include/x86_64-linux-gnu/asm/param.h
config.o: /usr/include/asm-generic/param.h
config.o: /usr/include/arpa/nameser_compat.h
config.o: /usr/include/netdb.h
config.o: /usr/include/rpc/netdb.h
config.o: /usr/include/x86_64-linux-gnu/bits/netdb.h
config.o: /usr/include/resolv.h
config.o: /usr/include/stdio.h
config.o: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
config.o: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
config.o: /usr/include/x86_64-linux-gnu/bits/libio.h
config.o: /usr/include/x86_64-linux-gnu/bits/_G_config.h
config.o: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
config.o: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
config.o: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
config.o: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
config.o: ../../include/ac/unistd.h
config.o: /usr/include/unistd.h
config.o: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
config.o: /usr/include/x86_64-linux-gnu/bits/environments.h
config.o: /usr/include/x86_64-linux-gnu/bits/confname.h
config.o: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
config.o: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
config.o: /usr/include/getopt.h
config.o: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
config.o: ../../include/lutil_lockf.h
config.o: ../../include/ac/ctype.h
config.o: /usr/include/ctype.h
config.o: ../../include/lber.h
config.o: ../../include/lber_types.h
config.o: ../../include/ldap_cdefs.h
config.o: ../../include/ldap.h
config.o: ../../include/ldap_features.h
config.o: ../../include/ldap_log.h
config.o: ../../include/lutil.h
config.o: ../../include/avl.h
config.o: ../../include/rewrite.h
config.o: ../../include/ldap_pvt_thread.h
config.o: ../../include/ldap_int_thread.h
config.o: rewrite-map.h
context.o: context.c
context.o: /usr/include/stdc-predef.h
context.o: ../../include/portable.h
context.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
context.o: ../../include/ac/fdset.h
context.o: /usr/include/x86_64-linux-gnu/bits/types.h
context.o: /usr/include/features.h
context.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
context.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
context.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
context.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
context.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
context.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
context.o: ../../include/ldap_cdefs.h
context.o: ../../include/ldap_features.h
context.o: ../../include/ac/assert.h
context.o: /usr/include/assert.h
context.o: ../../include/ac/localize.h
context.o: rewrite-int.h
context.o: ../../include/ac/stdlib.h
context.o: /usr/include/stdlib.h
context.o: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
context.o: /usr/include/x86_64-linux-gnu/bits/waitflags.h
context.o: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
context.o: /usr/include/x86_64-linux-gnu/bits/floatn.h
context.o: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
context.o: /usr/include/x86_64-linux-gnu/sys/types.h
context.o: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
context.o: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
context.o: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
context.o: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
context.o: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
context.o: /usr/include/endian.h
context.o: /usr/include/x86_64-linux-gnu/bits/endian.h
context.o: /usr/include/x86_64-linux-gnu/bits/byteswap.h
context.o: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
context.o: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
context.o: /usr/include/x86_64-linux-gnu/sys/select.h
context.o: /usr/include/x86_64-linux-gnu/bits/select.h
context.o: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
context.o: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
context.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
context.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
context.o: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
context.o: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
context.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
context.o: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
context.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
context.o: /usr/include/alloca.h
context.o: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
context.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
context.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
context.o: /usr/include/limits.h
context.o: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
context.o: /usr/include/x86_64-linux-gnu/bits/local_lim.h
context.o: /usr/include/linux/limits.h
context.o: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
context.o: ../../include/ac/string.h
context.o: /usr/include/string.h
context.o: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
context.o: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
context.o: /usr/include/strings.h
context.o: ../../include/ac/syslog.h
context.o: /usr/include/syslog.h
context.o: /usr/include/x86_64-linux-gnu/sys/syslog.h
context.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
context.o: /usr/include/x86_64-linux-gnu/bits/syslog-path.h
context.o: ../../include/ac/regex.h
context.o: /usr/include/regex.h
context.o: ../../include/ac/socket.h
context.o: /usr/include/poll.h
context.o: /usr/include/x86_64-linux-gnu/sys/poll.h
context.o: /usr/include/x86_64-linux-gnu/bits/poll.h
context.o: /usr/include/x86_64-linux-gnu/sys/socket.h
context.o: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
context.o: /usr/include/x86_64-linux-gnu/bits/socket.h
context.o: /usr/include/x86_64-linux-gnu/bits/socket_type.h
context.o: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
context.o: /usr/include/x86_64-linux-gnu/asm/socket.h
context.o: /usr/include/asm-generic/socket.h
context.o: /usr/include/x86_64-linux-gnu/asm/sockios.h
context.o: /usr/include/asm-generic/sockios.h
context.o: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
context.o: /usr/include/x86_64-linux-gnu/sys/un.h
context.o: /usr/include/netinet/in.h
context.o: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
context.o: /usr/include/x86_64-linux-gnu/bits/in.h
context.o: /usr/include/netinet/tcp.h
context.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
context.o: /usr/include/stdint.h
context.o: /usr/include/x86_64-linux-gnu/bits/wchar.h
context.o: /usr/include/arpa/inet.h
context.o: /usr/include/arpa/nameser.h
context.o: /usr/include/x86_64-linux-gnu/sys/param.h
context.o: /usr/include/signal.h
context.o: /usr/include/x86_64-linux-gnu/bits/signum.h
context.o: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
context.o: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
context.o: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
context.o: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
context.o: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
context.o: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
context.o: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
context.o: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
context.o: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
context.o: /usr/include/x86_64-linux-gnu/bits/sigaction.h
context.o: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
context.o: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
context.o: /usr/include/x86_64-linux-gnu/sys/ucontext.h
context.o: /usr/include/x86_64-linux-gnu/bits/sigstack.h
context.o: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
context.o: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
context.o: /usr/include/x86_64-linux-gnu/bits/sigthread.h
context.o: /usr/include/x86_64-linux-gnu/bits/param.h
context.o: /usr/include/linux/param.h
context.o: /usr/include/x86_64-linux-gnu/asm/param.h
context.o: /usr/include/asm-generic/param.h
context.o: /usr/include/arpa/nameser_compat.h
context.o: /usr/include/netdb.h
context.o: /usr/include/rpc/netdb.h
context.o: /usr/include/x86_64-linux-gnu/bits/netdb.h
context.o: /usr/include/resolv.h
context.o: /usr/include/stdio.h
context.o: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
context.o: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
context.o: /usr/include/x86_64-linux-gnu/bits/libio.h
context.o: /usr/include/x86_64-linux-gnu/bits/_G_config.h
context.o: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
context.o: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
context.o: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
context.o: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
context.o: ../../include/ac/unistd.h
context.o: /usr/include/unistd.h
context.o: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
context.o: /usr/include/x86_64-linux-gnu/bits/environments.h
context.o: /usr/include/x86_64-linux-gnu/bits/confname.h
context.o: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
context.o: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
context.o: /usr/include/getopt.h
context.o: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
context.o: ../../include/lutil_lockf.h
context.o: ../../include/ac/ctype.h
context.o: /usr/include/ctype.h
context.o: ../../include/lber.h
context.o: ../../include/lber_types.h
context.o: ../../include/ldap_cdefs.h
context.o: ../../include/ldap.h
context.o: ../../include/ldap_features.h
context.o: ../../include/ldap_log.h
context.o: ../../include/lutil.h
context.o: ../../include/avl.h
context.o: ../../include/rewrite.h
context.o: ../../include/ldap_pvt_thread.h
context.o: ../../include/ldap_int_thread.h
info.o: info.c
info.o: /usr/include/stdc-predef.h
info.o: ../../include/portable.h
info.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
info.o: ../../include/ac/fdset.h
info.o: /usr/include/x86_64-linux-gnu/bits/types.h
info.o: /usr/include/features.h
info.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
info.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
info.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
info.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
info.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
info.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
info.o: ../../include/ldap_cdefs.h
info.o: ../../include/ldap_features.h
info.o: ../../include/ac/assert.h
info.o: /usr/include/assert.h
info.o: ../../include/ac/localize.h
info.o: rewrite-int.h
info.o: ../../include/ac/stdlib.h
info.o: /usr/include/stdlib.h
info.o: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
info.o: /usr/include/x86_64-linux-gnu/bits/waitflags.h
info.o: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
info.o: /usr/include/x86_64-linux-gnu/bits/floatn.h
info.o: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
info.o: /usr/include/x86_64-linux-gnu/sys/types.h
info.o: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
info.o: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
info.o: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
info.o: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
info.o: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
info.o: /usr/include/endian.h
info.o: /usr/include/x86_64-linux-gnu/bits/endian.h
info.o: /usr/include/x86_64-linux-gnu/bits/byteswap.h
info.o: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
info.o: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
info.o: /usr/include/x86_64-linux-gnu/sys/select.h
info.o: /usr/include/x86_64-linux-gnu/bits/select.h
info.o: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
info.o: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
info.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
info.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
info.o: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
info.o: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
info.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
info.o: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
info.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
info.o: /usr/include/alloca.h
info.o: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
info.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
info.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
info.o: /usr/include/limits.h
info.o: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
info.o: /usr/include/x86_64-linux-gnu/bits/local_lim.h
info.o: /usr/include/linux/limits.h
info.o: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
info.o: ../../include/ac/string.h
info.o: /usr/include/string.h
info.o: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
info.o: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
info.o: /usr/include/strings.h
info.o: ../../include/ac/syslog.h
info.o: /usr/include/syslog.h
info.o: /usr/include/x86_64-linux-gnu/sys/syslog.h
info.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
info.o: /usr/include/x86_64-linux-gnu/bits/syslog-path.h
info.o: ../../include/ac/regex.h
info.o: /usr/include/regex.h
info.o: ../../include/ac/socket.h
info.o: /usr/include/poll.h
info.o: /usr/include/x86_64-linux-gnu/sys/poll.h
info.o: /usr/include/x86_64-linux-gnu/bits/poll.h
info.o: /usr/include/x86_64-linux-gnu/sys/socket.h
info.o: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
info.o: /usr/include/x86_64-linux-gnu/bits/socket.h
info.o: /usr/include/x86_64-linux-gnu/bits/socket_type.h
info.o: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
info.o: /usr/include/x86_64-linux-gnu/asm/socket.h
info.o: /usr/include/asm-generic/socket.h
info.o: /usr/include/x86_64-linux-gnu/asm/sockios.h
info.o: /usr/include/asm-generic/sockios.h
info.o: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
info.o: /usr/include/x86_64-linux-gnu/sys/un.h
info.o: /usr/include/netinet/in.h
info.o: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
info.o: /usr/include/x86_64-linux-gnu/bits/in.h
info.o: /usr/include/netinet/tcp.h
info.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
info.o: /usr/include/stdint.h
info.o: /usr/include/x86_64-linux-gnu/bits/wchar.h
info.o: /usr/include/arpa/inet.h
info.o: /usr/include/arpa/nameser.h
info.o: /usr/include/x86_64-linux-gnu/sys/param.h
info.o: /usr/include/signal.h
info.o: /usr/include/x86_64-linux-gnu/bits/signum.h
info.o: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
info.o: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
info.o: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
info.o: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
info.o: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
info.o: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
info.o: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
info.o: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
info.o: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
info.o: /usr/include/x86_64-linux-gnu/bits/sigaction.h
info.o: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
info.o: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
info.o: /usr/include/x86_64-linux-gnu/sys/ucontext.h
info.o: /usr/include/x86_64-linux-gnu/bits/sigstack.h
info.o: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
info.o: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
info.o: /usr/include/x86_64-linux-gnu/bits/sigthread.h
info.o: /usr/include/x86_64-linux-gnu/bits/param.h
info.o: /usr/include/linux/param.h
info.o: /usr/include/x86_64-linux-gnu/asm/param.h
info.o: /usr/include/asm-generic/param.h
info.o: /usr/include/arpa/nameser_compat.h
info.o: /usr/include/netdb.h
info.o: /usr/include/rpc/netdb.h
info.o: /usr/include/x86_64-linux-gnu/bits/netdb.h
info.o: /usr/include/resolv.h
info.o: /usr/include/stdio.h
info.o: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
info.o: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
info.o: /usr/include/x86_64-linux-gnu/bits/libio.h
info.o: /usr/include/x86_64-linux-gnu/bits/_G_config.h
info.o: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
info.o: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
info.o: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
info.o: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
info.o: ../../include/ac/unistd.h
info.o: /usr/include/unistd.h
info.o: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
info.o: /usr/include/x86_64-linux-gnu/bits/environments.h
info.o: /usr/include/x86_64-linux-gnu/bits/confname.h
info.o: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
info.o: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
info.o: /usr/include/getopt.h
info.o: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
info.o: ../../include/lutil_lockf.h
info.o: ../../include/ac/ctype.h
info.o: /usr/include/ctype.h
info.o: ../../include/lber.h
info.o: ../../include/lber_types.h
info.o: ../../include/ldap_cdefs.h
info.o: ../../include/ldap.h
info.o: ../../include/ldap_features.h
info.o: ../../include/ldap_log.h
info.o: ../../include/lutil.h
info.o: ../../include/avl.h
info.o: ../../include/rewrite.h
info.o: ../../include/ldap_pvt_thread.h
info.o: ../../include/ldap_int_thread.h
ldapmap.o: ldapmap.c
ldapmap.o: /usr/include/stdc-predef.h
ldapmap.o: ../../include/portable.h
ldapmap.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
ldapmap.o: ../../include/ac/fdset.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/types.h
ldapmap.o: /usr/include/features.h
ldapmap.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
ldapmap.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
ldapmap.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
ldapmap.o: ../../include/ldap_cdefs.h
ldapmap.o: ../../include/ldap_features.h
ldapmap.o: ../../include/ac/assert.h
ldapmap.o: /usr/include/assert.h
ldapmap.o: ../../include/ac/localize.h
ldapmap.o: rewrite-int.h
ldapmap.o: ../../include/ac/stdlib.h
ldapmap.o: /usr/include/stdlib.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/waitflags.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/floatn.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
ldapmap.o: /usr/include/x86_64-linux-gnu/sys/types.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
ldapmap.o: /usr/include/endian.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/endian.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/byteswap.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
ldapmap.o: /usr/include/x86_64-linux-gnu/sys/select.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/select.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
ldapmap.o: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
ldapmap.o: /usr/include/alloca.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
ldapmap.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
ldapmap.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
ldapmap.o: /usr/include/limits.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/local_lim.h
ldapmap.o: /usr/include/linux/limits.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
ldapmap.o: ../../include/ac/string.h
ldapmap.o: /usr/include/string.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
ldapmap.o: /usr/include/strings.h
ldapmap.o: ../../include/ac/syslog.h
ldapmap.o: /usr/include/syslog.h
ldapmap.o: /usr/include/x86_64-linux-gnu/sys/syslog.h
ldapmap.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/syslog-path.h
ldapmap.o: ../../include/ac/regex.h
ldapmap.o: /usr/include/regex.h
ldapmap.o: ../../include/ac/socket.h
ldapmap.o: /usr/include/poll.h
ldapmap.o: /usr/include/x86_64-linux-gnu/sys/poll.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/poll.h
ldapmap.o: /usr/include/x86_64-linux-gnu/sys/socket.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/socket.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/socket_type.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
ldapmap.o: /usr/include/x86_64-linux-gnu/asm/socket.h
ldapmap.o: /usr/include/asm-generic/socket.h
ldapmap.o: /usr/include/x86_64-linux-gnu/asm/sockios.h
ldapmap.o: /usr/include/asm-generic/sockios.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
ldapmap.o: /usr/include/x86_64-linux-gnu/sys/un.h
ldapmap.o: /usr/include/netinet/in.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/in.h
ldapmap.o: /usr/include/netinet/tcp.h
ldapmap.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
ldapmap.o: /usr/include/stdint.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/wchar.h
ldapmap.o: /usr/include/arpa/inet.h
ldapmap.o: /usr/include/arpa/nameser.h
ldapmap.o: /usr/include/x86_64-linux-gnu/sys/param.h
ldapmap.o: /usr/include/signal.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/signum.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/sigaction.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
ldapmap.o: /usr/include/x86_64-linux-gnu/sys/ucontext.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/sigstack.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/sigthread.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/param.h
ldapmap.o: /usr/include/linux/param.h
ldapmap.o: /usr/include/x86_64-linux-gnu/asm/param.h
ldapmap.o: /usr/include/asm-generic/param.h
ldapmap.o: /usr/include/arpa/nameser_compat.h
ldapmap.o: /usr/include/netdb.h
ldapmap.o: /usr/include/rpc/netdb.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/netdb.h
ldapmap.o: /usr/include/resolv.h
ldapmap.o: /usr/include/stdio.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/libio.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/_G_config.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
ldapmap.o: ../../include/ac/unistd.h
ldapmap.o: /usr/include/unistd.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/environments.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/confname.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
ldapmap.o: /usr/include/getopt.h
ldapmap.o: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
ldapmap.o: ../../include/lutil_lockf.h
ldapmap.o: ../../include/ac/ctype.h
ldapmap.o: /usr/include/ctype.h
ldapmap.o: ../../include/lber.h
ldapmap.o: ../../include/lber_types.h
ldapmap.o: ../../include/ldap_cdefs.h
ldapmap.o: ../../include/ldap.h
ldapmap.o: ../../include/ldap_features.h
ldapmap.o: ../../include/ldap_log.h
ldapmap.o: ../../include/lutil.h
ldapmap.o: ../../include/avl.h
ldapmap.o: ../../include/rewrite.h
ldapmap.o: ../../include/ldap_pvt_thread.h
ldapmap.o: ../../include/ldap_int_thread.h
ldapmap.o: rewrite-map.h
map.o: map.c
map.o: /usr/include/stdc-predef.h
map.o: ../../include/portable.h
map.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
map.o: ../../include/ac/fdset.h
map.o: /usr/include/x86_64-linux-gnu/bits/types.h
map.o: /usr/include/features.h
map.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
map.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
map.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
map.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
map.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
map.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
map.o: ../../include/ldap_cdefs.h
map.o: ../../include/ldap_features.h
map.o: ../../include/ac/assert.h
map.o: /usr/include/assert.h
map.o: ../../include/ac/localize.h
map.o: /usr/include/stdio.h
map.o: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
map.o: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
map.o: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
map.o: /usr/include/x86_64-linux-gnu/bits/libio.h
map.o: /usr/include/x86_64-linux-gnu/bits/_G_config.h
map.o: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
map.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
map.o: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
map.o: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
map.o: /usr/include/pwd.h
map.o: rewrite-int.h
map.o: ../../include/ac/stdlib.h
map.o: /usr/include/stdlib.h
map.o: /usr/include/x86_64-linux-gnu/bits/waitflags.h
map.o: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
map.o: /usr/include/x86_64-linux-gnu/bits/floatn.h
map.o: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
map.o: /usr/include/x86_64-linux-gnu/sys/types.h
map.o: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
map.o: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
map.o: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
map.o: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
map.o: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
map.o: /usr/include/endian.h
map.o: /usr/include/x86_64-linux-gnu/bits/endian.h
map.o: /usr/include/x86_64-linux-gnu/bits/byteswap.h
map.o: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
map.o: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
map.o: /usr/include/x86_64-linux-gnu/sys/select.h
map.o: /usr/include/x86_64-linux-gnu/bits/select.h
map.o: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
map.o: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
map.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
map.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
map.o: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
map.o: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
map.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
map.o: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
map.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
map.o: /usr/include/alloca.h
map.o: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
map.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
map.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
map.o: /usr/include/limits.h
map.o: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
map.o: /usr/include/x86_64-linux-gnu/bits/local_lim.h
map.o: /usr/include/linux/limits.h
map.o: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
map.o: ../../include/ac/string.h
map.o: /usr/include/string.h
map.o: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
map.o: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
map.o: /usr/include/strings.h
map.o: ../../include/ac/syslog.h
map.o: /usr/include/syslog.h
map.o: /usr/include/x86_64-linux-gnu/sys/syslog.h
map.o: /usr/include/x86_64-linux-gnu/bits/syslog-path.h
map.o: ../../include/ac/regex.h
map.o: /usr/include/regex.h
map.o: ../../include/ac/socket.h
map.o: /usr/include/poll.h
map.o: /usr/include/x86_64-linux-gnu/sys/poll.h
map.o: /usr/include/x86_64-linux-gnu/bits/poll.h
map.o: /usr/include/x86_64-linux-gnu/sys/socket.h
map.o: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
map.o: /usr/include/x86_64-linux-gnu/bits/socket.h
map.o: /usr/include/x86_64-linux-gnu/bits/socket_type.h
map.o: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
map.o: /usr/include/x86_64-linux-gnu/asm/socket.h
map.o: /usr/include/asm-generic/socket.h
map.o: /usr/include/x86_64-linux-gnu/asm/sockios.h
map.o: /usr/include/asm-generic/sockios.h
map.o: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
map.o: /usr/include/x86_64-linux-gnu/sys/un.h
map.o: /usr/include/netinet/in.h
map.o: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
map.o: /usr/include/x86_64-linux-gnu/bits/in.h
map.o: /usr/include/netinet/tcp.h
map.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
map.o: /usr/include/stdint.h
map.o: /usr/include/x86_64-linux-gnu/bits/wchar.h
map.o: /usr/include/arpa/inet.h
map.o: /usr/include/arpa/nameser.h
map.o: /usr/include/x86_64-linux-gnu/sys/param.h
map.o: /usr/include/signal.h
map.o: /usr/include/x86_64-linux-gnu/bits/signum.h
map.o: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
map.o: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
map.o: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
map.o: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
map.o: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
map.o: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
map.o: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
map.o: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
map.o: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
map.o: /usr/include/x86_64-linux-gnu/bits/sigaction.h
map.o: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
map.o: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
map.o: /usr/include/x86_64-linux-gnu/sys/ucontext.h
map.o: /usr/include/x86_64-linux-gnu/bits/sigstack.h
map.o: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
map.o: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
map.o: /usr/include/x86_64-linux-gnu/bits/sigthread.h
map.o: /usr/include/x86_64-linux-gnu/bits/param.h
map.o: /usr/include/linux/param.h
map.o: /usr/include/x86_64-linux-gnu/asm/param.h
map.o: /usr/include/asm-generic/param.h
map.o: /usr/include/arpa/nameser_compat.h
map.o: /usr/include/netdb.h
map.o: /usr/include/rpc/netdb.h
map.o: /usr/include/x86_64-linux-gnu/bits/netdb.h
map.o: /usr/include/resolv.h
map.o: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
map.o: ../../include/ac/unistd.h
map.o: /usr/include/unistd.h
map.o: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
map.o: /usr/include/x86_64-linux-gnu/bits/environments.h
map.o: /usr/include/x86_64-linux-gnu/bits/confname.h
map.o: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
map.o: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
map.o: /usr/include/getopt.h
map.o: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
map.o: ../../include/lutil_lockf.h
map.o: ../../include/ac/ctype.h
map.o: /usr/include/ctype.h
map.o: ../../include/lber.h
map.o: ../../include/lber_types.h
map.o: ../../include/ldap_cdefs.h
map.o: ../../include/ldap.h
map.o: ../../include/ldap_features.h
map.o: ../../include/ldap_log.h
map.o: ../../include/lutil.h
map.o: ../../include/avl.h
map.o: ../../include/rewrite.h
map.o: ../../include/ldap_pvt_thread.h
map.o: ../../include/ldap_int_thread.h
map.o: rewrite-map.h
params.o: params.c
params.o: /usr/include/stdc-predef.h
params.o: ../../include/portable.h
params.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
params.o: ../../include/ac/fdset.h
params.o: /usr/include/x86_64-linux-gnu/bits/types.h
params.o: /usr/include/features.h
params.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
params.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
params.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
params.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
params.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
params.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
params.o: ../../include/ldap_cdefs.h
params.o: ../../include/ldap_features.h
params.o: ../../include/ac/assert.h
params.o: /usr/include/assert.h
params.o: ../../include/ac/localize.h
params.o: rewrite-int.h
params.o: ../../include/ac/stdlib.h
params.o: /usr/include/stdlib.h
params.o: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
params.o: /usr/include/x86_64-linux-gnu/bits/waitflags.h
params.o: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
params.o: /usr/include/x86_64-linux-gnu/bits/floatn.h
params.o: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
params.o: /usr/include/x86_64-linux-gnu/sys/types.h
params.o: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
params.o: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
params.o: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
params.o: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
params.o: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
params.o: /usr/include/endian.h
params.o: /usr/include/x86_64-linux-gnu/bits/endian.h
params.o: /usr/include/x86_64-linux-gnu/bits/byteswap.h
params.o: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
params.o: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
params.o: /usr/include/x86_64-linux-gnu/sys/select.h
params.o: /usr/include/x86_64-linux-gnu/bits/select.h
params.o: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
params.o: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
params.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
params.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
params.o: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
params.o: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
params.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
params.o: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
params.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
params.o: /usr/include/alloca.h
params.o: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
params.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
params.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
params.o: /usr/include/limits.h
params.o: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
params.o: /usr/include/x86_64-linux-gnu/bits/local_lim.h
params.o: /usr/include/linux/limits.h
params.o: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
params.o: ../../include/ac/string.h
params.o: /usr/include/string.h
params.o: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
params.o: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
params.o: /usr/include/strings.h
params.o: ../../include/ac/syslog.h
params.o: /usr/include/syslog.h
params.o: /usr/include/x86_64-linux-gnu/sys/syslog.h
params.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
params.o: /usr/include/x86_64-linux-gnu/bits/syslog-path.h
params.o: ../../include/ac/regex.h
params.o: /usr/include/regex.h
params.o: ../../include/ac/socket.h
params.o: /usr/include/poll.h
params.o: /usr/include/x86_64-linux-gnu/sys/poll.h
params.o: /usr/include/x86_64-linux-gnu/bits/poll.h
params.o: /usr/include/x86_64-linux-gnu/sys/socket.h
params.o: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
params.o: /usr/include/x86_64-linux-gnu/bits/socket.h
params.o: /usr/include/x86_64-linux-gnu/bits/socket_type.h
params.o: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
params.o: /usr/include/x86_64-linux-gnu/asm/socket.h
params.o: /usr/include/asm-generic/socket.h
params.o: /usr/include/x86_64-linux-gnu/asm/sockios.h
params.o: /usr/include/asm-generic/sockios.h
params.o: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
params.o: /usr/include/x86_64-linux-gnu/sys/un.h
params.o: /usr/include/netinet/in.h
params.o: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
params.o: /usr/include/x86_64-linux-gnu/bits/in.h
params.o: /usr/include/netinet/tcp.h
params.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
params.o: /usr/include/stdint.h
params.o: /usr/include/x86_64-linux-gnu/bits/wchar.h
params.o: /usr/include/arpa/inet.h
params.o: /usr/include/arpa/nameser.h
params.o: /usr/include/x86_64-linux-gnu/sys/param.h
params.o: /usr/include/signal.h
params.o: /usr/include/x86_64-linux-gnu/bits/signum.h
params.o: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
params.o: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
params.o: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
params.o: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
params.o: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
params.o: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
params.o: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
params.o: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
params.o: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
params.o: /usr/include/x86_64-linux-gnu/bits/sigaction.h
params.o: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
params.o: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
params.o: /usr/include/x86_64-linux-gnu/sys/ucontext.h
params.o: /usr/include/x86_64-linux-gnu/bits/sigstack.h
params.o: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
params.o: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
params.o: /usr/include/x86_64-linux-gnu/bits/sigthread.h
params.o: /usr/include/x86_64-linux-gnu/bits/param.h
params.o: /usr/include/linux/param.h
params.o: /usr/include/x86_64-linux-gnu/asm/param.h
params.o: /usr/include/asm-generic/param.h
params.o: /usr/include/arpa/nameser_compat.h
params.o: /usr/include/netdb.h
params.o: /usr/include/rpc/netdb.h
params.o: /usr/include/x86_64-linux-gnu/bits/netdb.h
params.o: /usr/include/resolv.h
params.o: /usr/include/stdio.h
params.o: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
params.o: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
params.o: /usr/include/x86_64-linux-gnu/bits/libio.h
params.o: /usr/include/x86_64-linux-gnu/bits/_G_config.h
params.o: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
params.o: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
params.o: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
params.o: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
params.o: ../../include/ac/unistd.h
params.o: /usr/include/unistd.h
params.o: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
params.o: /usr/include/x86_64-linux-gnu/bits/environments.h
params.o: /usr/include/x86_64-linux-gnu/bits/confname.h
params.o: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
params.o: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
params.o: /usr/include/getopt.h
params.o: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
params.o: ../../include/lutil_lockf.h
params.o: ../../include/ac/ctype.h
params.o: /usr/include/ctype.h
params.o: ../../include/lber.h
params.o: ../../include/lber_types.h
params.o: ../../include/ldap_cdefs.h
params.o: ../../include/ldap.h
params.o: ../../include/ldap_features.h
params.o: ../../include/ldap_log.h
params.o: ../../include/lutil.h
params.o: ../../include/avl.h
params.o: ../../include/rewrite.h
params.o: ../../include/ldap_pvt_thread.h
params.o: ../../include/ldap_int_thread.h
rule.o: rule.c
rule.o: /usr/include/stdc-predef.h
rule.o: ../../include/portable.h
rule.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
rule.o: ../../include/ac/fdset.h
rule.o: /usr/include/x86_64-linux-gnu/bits/types.h
rule.o: /usr/include/features.h
rule.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
rule.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
rule.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
rule.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
rule.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
rule.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
rule.o: ../../include/ldap_cdefs.h
rule.o: ../../include/ldap_features.h
rule.o: ../../include/ac/assert.h
rule.o: /usr/include/assert.h
rule.o: ../../include/ac/localize.h
rule.o: rewrite-int.h
rule.o: ../../include/ac/stdlib.h
rule.o: /usr/include/stdlib.h
rule.o: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
rule.o: /usr/include/x86_64-linux-gnu/bits/waitflags.h
rule.o: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
rule.o: /usr/include/x86_64-linux-gnu/bits/floatn.h
rule.o: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
rule.o: /usr/include/x86_64-linux-gnu/sys/types.h
rule.o: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
rule.o: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
rule.o: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
rule.o: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
rule.o: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
rule.o: /usr/include/endian.h
rule.o: /usr/include/x86_64-linux-gnu/bits/endian.h
rule.o: /usr/include/x86_64-linux-gnu/bits/byteswap.h
rule.o: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
rule.o: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
rule.o: /usr/include/x86_64-linux-gnu/sys/select.h
rule.o: /usr/include/x86_64-linux-gnu/bits/select.h
rule.o: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
rule.o: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
rule.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
rule.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
rule.o: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
rule.o: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
rule.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
rule.o: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
rule.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
rule.o: /usr/include/alloca.h
rule.o: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
rule.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
rule.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
rule.o: /usr/include/limits.h
rule.o: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
rule.o: /usr/include/x86_64-linux-gnu/bits/local_lim.h
rule.o: /usr/include/linux/limits.h
rule.o: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
rule.o: ../../include/ac/string.h
rule.o: /usr/include/string.h
rule.o: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
rule.o: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
rule.o: /usr/include/strings.h
rule.o: ../../include/ac/syslog.h
rule.o: /usr/include/syslog.h
rule.o: /usr/include/x86_64-linux-gnu/sys/syslog.h
rule.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
rule.o: /usr/include/x86_64-linux-gnu/bits/syslog-path.h
rule.o: ../../include/ac/regex.h
rule.o: /usr/include/regex.h
rule.o: ../../include/ac/socket.h
rule.o: /usr/include/poll.h
rule.o: /usr/include/x86_64-linux-gnu/sys/poll.h
rule.o: /usr/include/x86_64-linux-gnu/bits/poll.h
rule.o: /usr/include/x86_64-linux-gnu/sys/socket.h
rule.o: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
rule.o: /usr/include/x86_64-linux-gnu/bits/socket.h
rule.o: /usr/include/x86_64-linux-gnu/bits/socket_type.h
rule.o: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
rule.o: /usr/include/x86_64-linux-gnu/asm/socket.h
rule.o: /usr/include/asm-generic/socket.h
rule.o: /usr/include/x86_64-linux-gnu/asm/sockios.h
rule.o: /usr/include/asm-generic/sockios.h
rule.o: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
rule.o: /usr/include/x86_64-linux-gnu/sys/un.h
rule.o: /usr/include/netinet/in.h
rule.o: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
rule.o: /usr/include/x86_64-linux-gnu/bits/in.h
rule.o: /usr/include/netinet/tcp.h
rule.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
rule.o: /usr/include/stdint.h
rule.o: /usr/include/x86_64-linux-gnu/bits/wchar.h
rule.o: /usr/include/arpa/inet.h
rule.o: /usr/include/arpa/nameser.h
rule.o: /usr/include/x86_64-linux-gnu/sys/param.h
rule.o: /usr/include/signal.h
rule.o: /usr/include/x86_64-linux-gnu/bits/signum.h
rule.o: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
rule.o: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
rule.o: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
rule.o: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
rule.o: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
rule.o: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
rule.o: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
rule.o: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
rule.o: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
rule.o: /usr/include/x86_64-linux-gnu/bits/sigaction.h
rule.o: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
rule.o: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
rule.o: /usr/include/x86_64-linux-gnu/sys/ucontext.h
rule.o: /usr/include/x86_64-linux-gnu/bits/sigstack.h
rule.o: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
rule.o: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
rule.o: /usr/include/x86_64-linux-gnu/bits/sigthread.h
rule.o: /usr/include/x86_64-linux-gnu/bits/param.h
rule.o: /usr/include/linux/param.h
rule.o: /usr/include/x86_64-linux-gnu/asm/param.h
rule.o: /usr/include/asm-generic/param.h
rule.o: /usr/include/arpa/nameser_compat.h
rule.o: /usr/include/netdb.h
rule.o: /usr/include/rpc/netdb.h
rule.o: /usr/include/x86_64-linux-gnu/bits/netdb.h
rule.o: /usr/include/resolv.h
rule.o: /usr/include/stdio.h
rule.o: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
rule.o: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
rule.o: /usr/include/x86_64-linux-gnu/bits/libio.h
rule.o: /usr/include/x86_64-linux-gnu/bits/_G_config.h
rule.o: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
rule.o: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
rule.o: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
rule.o: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
rule.o: ../../include/ac/unistd.h
rule.o: /usr/include/unistd.h
rule.o: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
rule.o: /usr/include/x86_64-linux-gnu/bits/environments.h
rule.o: /usr/include/x86_64-linux-gnu/bits/confname.h
rule.o: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
rule.o: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
rule.o: /usr/include/getopt.h
rule.o: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
rule.o: ../../include/lutil_lockf.h
rule.o: ../../include/ac/ctype.h
rule.o: /usr/include/ctype.h
rule.o: ../../include/lber.h
rule.o: ../../include/lber_types.h
rule.o: ../../include/ldap_cdefs.h
rule.o: ../../include/ldap.h
rule.o: ../../include/ldap_features.h
rule.o: ../../include/ldap_log.h
rule.o: ../../include/lutil.h
rule.o: ../../include/avl.h
rule.o: ../../include/rewrite.h
rule.o: ../../include/ldap_pvt_thread.h
rule.o: ../../include/ldap_int_thread.h
session.o: session.c
session.o: /usr/include/stdc-predef.h
session.o: ../../include/portable.h
session.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
session.o: ../../include/ac/fdset.h
session.o: /usr/include/x86_64-linux-gnu/bits/types.h
session.o: /usr/include/features.h
session.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
session.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
session.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
session.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
session.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
session.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
session.o: ../../include/ldap_cdefs.h
session.o: ../../include/ldap_features.h
session.o: ../../include/ac/assert.h
session.o: /usr/include/assert.h
session.o: ../../include/ac/localize.h
session.o: rewrite-int.h
session.o: ../../include/ac/stdlib.h
session.o: /usr/include/stdlib.h
session.o: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
session.o: /usr/include/x86_64-linux-gnu/bits/waitflags.h
session.o: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
session.o: /usr/include/x86_64-linux-gnu/bits/floatn.h
session.o: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
session.o: /usr/include/x86_64-linux-gnu/sys/types.h
session.o: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
session.o: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
session.o: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
session.o: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
session.o: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
session.o: /usr/include/endian.h
session.o: /usr/include/x86_64-linux-gnu/bits/endian.h
session.o: /usr/include/x86_64-linux-gnu/bits/byteswap.h
session.o: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
session.o: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
session.o: /usr/include/x86_64-linux-gnu/sys/select.h
session.o: /usr/include/x86_64-linux-gnu/bits/select.h
session.o: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
session.o: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
session.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
session.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
session.o: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
session.o: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
session.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
session.o: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
session.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
session.o: /usr/include/alloca.h
session.o: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
session.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
session.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
session.o: /usr/include/limits.h
session.o: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
session.o: /usr/include/x86_64-linux-gnu/bits/local_lim.h
session.o: /usr/include/linux/limits.h
session.o: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
session.o: ../../include/ac/string.h
session.o: /usr/include/string.h
session.o: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
session.o: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
session.o: /usr/include/strings.h
session.o: ../../include/ac/syslog.h
session.o: /usr/include/syslog.h
session.o: /usr/include/x86_64-linux-gnu/sys/syslog.h
session.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
session.o: /usr/include/x86_64-linux-gnu/bits/syslog-path.h
session.o: ../../include/ac/regex.h
session.o: /usr/include/regex.h
session.o: ../../include/ac/socket.h
session.o: /usr/include/poll.h
session.o: /usr/include/x86_64-linux-gnu/sys/poll.h
session.o: /usr/include/x86_64-linux-gnu/bits/poll.h
session.o: /usr/include/x86_64-linux-gnu/sys/socket.h
session.o: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
session.o: /usr/include/x86_64-linux-gnu/bits/socket.h
session.o: /usr/include/x86_64-linux-gnu/bits/socket_type.h
session.o: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
session.o: /usr/include/x86_64-linux-gnu/asm/socket.h
session.o: /usr/include/asm-generic/socket.h
session.o: /usr/include/x86_64-linux-gnu/asm/sockios.h
session.o: /usr/include/asm-generic/sockios.h
session.o: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
session.o: /usr/include/x86_64-linux-gnu/sys/un.h
session.o: /usr/include/netinet/in.h
session.o: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
session.o: /usr/include/x86_64-linux-gnu/bits/in.h
session.o: /usr/include/netinet/tcp.h
session.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
session.o: /usr/include/stdint.h
session.o: /usr/include/x86_64-linux-gnu/bits/wchar.h
session.o: /usr/include/arpa/inet.h
session.o: /usr/include/arpa/nameser.h
session.o: /usr/include/x86_64-linux-gnu/sys/param.h
session.o: /usr/include/signal.h
session.o: /usr/include/x86_64-linux-gnu/bits/signum.h
session.o: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
session.o: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
session.o: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
session.o: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
session.o: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
session.o: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
session.o: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
session.o: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
session.o: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
session.o: /usr/include/x86_64-linux-gnu/bits/sigaction.h
session.o: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
session.o: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
session.o: /usr/include/x86_64-linux-gnu/sys/ucontext.h
session.o: /usr/include/x86_64-linux-gnu/bits/sigstack.h
session.o: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
session.o: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
session.o: /usr/include/x86_64-linux-gnu/bits/sigthread.h
session.o: /usr/include/x86_64-linux-gnu/bits/param.h
session.o: /usr/include/linux/param.h
session.o: /usr/include/x86_64-linux-gnu/asm/param.h
session.o: /usr/include/asm-generic/param.h
session.o: /usr/include/arpa/nameser_compat.h
session.o: /usr/include/netdb.h
session.o: /usr/include/rpc/netdb.h
session.o: /usr/include/x86_64-linux-gnu/bits/netdb.h
session.o: /usr/include/resolv.h
session.o: /usr/include/stdio.h
session.o: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
session.o: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
session.o: /usr/include/x86_64-linux-gnu/bits/libio.h
session.o: /usr/include/x86_64-linux-gnu/bits/_G_config.h
session.o: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
session.o: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
session.o: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
session.o: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
session.o: ../../include/ac/unistd.h
session.o: /usr/include/unistd.h
session.o: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
session.o: /usr/include/x86_64-linux-gnu/bits/environments.h
session.o: /usr/include/x86_64-linux-gnu/bits/confname.h
session.o: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
session.o: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
session.o: /usr/include/getopt.h
session.o: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
session.o: ../../include/lutil_lockf.h
session.o: ../../include/ac/ctype.h
session.o: /usr/include/ctype.h
session.o: ../../include/lber.h
session.o: ../../include/lber_types.h
session.o: ../../include/ldap_cdefs.h
session.o: ../../include/ldap.h
session.o: ../../include/ldap_features.h
session.o: ../../include/ldap_log.h
session.o: ../../include/lutil.h
session.o: ../../include/avl.h
session.o: ../../include/rewrite.h
session.o: ../../include/ldap_pvt_thread.h
session.o: ../../include/ldap_int_thread.h
subst.o: subst.c
subst.o: /usr/include/stdc-predef.h
subst.o: ../../include/portable.h
subst.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
subst.o: ../../include/ac/fdset.h
subst.o: /usr/include/x86_64-linux-gnu/bits/types.h
subst.o: /usr/include/features.h
subst.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
subst.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
subst.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
subst.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
subst.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
subst.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
subst.o: ../../include/ldap_cdefs.h
subst.o: ../../include/ldap_features.h
subst.o: ../../include/ac/assert.h
subst.o: /usr/include/assert.h
subst.o: ../../include/ac/localize.h
subst.o: rewrite-int.h
subst.o: ../../include/ac/stdlib.h
subst.o: /usr/include/stdlib.h
subst.o: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
subst.o: /usr/include/x86_64-linux-gnu/bits/waitflags.h
subst.o: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
subst.o: /usr/include/x86_64-linux-gnu/bits/floatn.h
subst.o: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
subst.o: /usr/include/x86_64-linux-gnu/sys/types.h
subst.o: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
subst.o: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
subst.o: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
subst.o: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
subst.o: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
subst.o: /usr/include/endian.h
subst.o: /usr/include/x86_64-linux-gnu/bits/endian.h
subst.o: /usr/include/x86_64-linux-gnu/bits/byteswap.h
subst.o: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
subst.o: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
subst.o: /usr/include/x86_64-linux-gnu/sys/select.h
subst.o: /usr/include/x86_64-linux-gnu/bits/select.h
subst.o: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
subst.o: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
subst.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
subst.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
subst.o: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
subst.o: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
subst.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
subst.o: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
subst.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
subst.o: /usr/include/alloca.h
subst.o: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
subst.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
subst.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
subst.o: /usr/include/limits.h
subst.o: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
subst.o: /usr/include/x86_64-linux-gnu/bits/local_lim.h
subst.o: /usr/include/linux/limits.h
subst.o: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
subst.o: ../../include/ac/string.h
subst.o: /usr/include/string.h
subst.o: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
subst.o: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
subst.o: /usr/include/strings.h
subst.o: ../../include/ac/syslog.h
subst.o: /usr/include/syslog.h
subst.o: /usr/include/x86_64-linux-gnu/sys/syslog.h
subst.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
subst.o: /usr/include/x86_64-linux-gnu/bits/syslog-path.h
subst.o: ../../include/ac/regex.h
subst.o: /usr/include/regex.h
subst.o: ../../include/ac/socket.h
subst.o: /usr/include/poll.h
subst.o: /usr/include/x86_64-linux-gnu/sys/poll.h
subst.o: /usr/include/x86_64-linux-gnu/bits/poll.h
subst.o: /usr/include/x86_64-linux-gnu/sys/socket.h
subst.o: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
subst.o: /usr/include/x86_64-linux-gnu/bits/socket.h
subst.o: /usr/include/x86_64-linux-gnu/bits/socket_type.h
subst.o: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
subst.o: /usr/include/x86_64-linux-gnu/asm/socket.h
subst.o: /usr/include/asm-generic/socket.h
subst.o: /usr/include/x86_64-linux-gnu/asm/sockios.h
subst.o: /usr/include/asm-generic/sockios.h
subst.o: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
subst.o: /usr/include/x86_64-linux-gnu/sys/un.h
subst.o: /usr/include/netinet/in.h
subst.o: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
subst.o: /usr/include/x86_64-linux-gnu/bits/in.h
subst.o: /usr/include/netinet/tcp.h
subst.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
subst.o: /usr/include/stdint.h
subst.o: /usr/include/x86_64-linux-gnu/bits/wchar.h
subst.o: /usr/include/arpa/inet.h
subst.o: /usr/include/arpa/nameser.h
subst.o: /usr/include/x86_64-linux-gnu/sys/param.h
subst.o: /usr/include/signal.h
subst.o: /usr/include/x86_64-linux-gnu/bits/signum.h
subst.o: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
subst.o: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
subst.o: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
subst.o: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
subst.o: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
subst.o: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
subst.o: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
subst.o: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
subst.o: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
subst.o: /usr/include/x86_64-linux-gnu/bits/sigaction.h
subst.o: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
subst.o: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
subst.o: /usr/include/x86_64-linux-gnu/sys/ucontext.h
subst.o: /usr/include/x86_64-linux-gnu/bits/sigstack.h
subst.o: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
subst.o: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
subst.o: /usr/include/x86_64-linux-gnu/bits/sigthread.h
subst.o: /usr/include/x86_64-linux-gnu/bits/param.h
subst.o: /usr/include/linux/param.h
subst.o: /usr/include/x86_64-linux-gnu/asm/param.h
subst.o: /usr/include/asm-generic/param.h
subst.o: /usr/include/arpa/nameser_compat.h
subst.o: /usr/include/netdb.h
subst.o: /usr/include/rpc/netdb.h
subst.o: /usr/include/x86_64-linux-gnu/bits/netdb.h
subst.o: /usr/include/resolv.h
subst.o: /usr/include/stdio.h
subst.o: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
subst.o: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
subst.o: /usr/include/x86_64-linux-gnu/bits/libio.h
subst.o: /usr/include/x86_64-linux-gnu/bits/_G_config.h
subst.o: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
subst.o: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
subst.o: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
subst.o: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
subst.o: ../../include/ac/unistd.h
subst.o: /usr/include/unistd.h
subst.o: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
subst.o: /usr/include/x86_64-linux-gnu/bits/environments.h
subst.o: /usr/include/x86_64-linux-gnu/bits/confname.h
subst.o: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
subst.o: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
subst.o: /usr/include/getopt.h
subst.o: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
subst.o: ../../include/lutil_lockf.h
subst.o: ../../include/ac/ctype.h
subst.o: /usr/include/ctype.h
subst.o: ../../include/lber.h
subst.o: ../../include/lber_types.h
subst.o: ../../include/ldap_cdefs.h
subst.o: ../../include/ldap.h
subst.o: ../../include/ldap_features.h
subst.o: ../../include/ldap_log.h
subst.o: ../../include/lutil.h
subst.o: ../../include/avl.h
subst.o: ../../include/rewrite.h
subst.o: ../../include/ldap_pvt_thread.h
subst.o: ../../include/ldap_int_thread.h
var.o: var.c
var.o: /usr/include/stdc-predef.h
var.o: ../../include/portable.h
var.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
var.o: ../../include/ac/fdset.h
var.o: /usr/include/x86_64-linux-gnu/bits/types.h
var.o: /usr/include/features.h
var.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
var.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
var.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
var.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
var.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
var.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
var.o: ../../include/ldap_cdefs.h
var.o: ../../include/ldap_features.h
var.o: ../../include/ac/assert.h
var.o: /usr/include/assert.h
var.o: ../../include/ac/localize.h
var.o: rewrite-int.h
var.o: ../../include/ac/stdlib.h
var.o: /usr/include/stdlib.h
var.o: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
var.o: /usr/include/x86_64-linux-gnu/bits/waitflags.h
var.o: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
var.o: /usr/include/x86_64-linux-gnu/bits/floatn.h
var.o: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
var.o: /usr/include/x86_64-linux-gnu/sys/types.h
var.o: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
var.o: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
var.o: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
var.o: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
var.o: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
var.o: /usr/include/endian.h
var.o: /usr/include/x86_64-linux-gnu/bits/endian.h
var.o: /usr/include/x86_64-linux-gnu/bits/byteswap.h
var.o: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
var.o: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
var.o: /usr/include/x86_64-linux-gnu/sys/select.h
var.o: /usr/include/x86_64-linux-gnu/bits/select.h
var.o: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
var.o: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
var.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
var.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
var.o: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
var.o: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
var.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
var.o: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
var.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
var.o: /usr/include/alloca.h
var.o: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
var.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
var.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
var.o: /usr/include/limits.h
var.o: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
var.o: /usr/include/x86_64-linux-gnu/bits/local_lim.h
var.o: /usr/include/linux/limits.h
var.o: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
var.o: ../../include/ac/string.h
var.o: /usr/include/string.h
var.o: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
var.o: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
var.o: /usr/include/strings.h
var.o: ../../include/ac/syslog.h
var.o: /usr/include/syslog.h
var.o: /usr/include/x86_64-linux-gnu/sys/syslog.h
var.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
var.o: /usr/include/x86_64-linux-gnu/bits/syslog-path.h
var.o: ../../include/ac/regex.h
var.o: /usr/include/regex.h
var.o: ../../include/ac/socket.h
var.o: /usr/include/poll.h
var.o: /usr/include/x86_64-linux-gnu/sys/poll.h
var.o: /usr/include/x86_64-linux-gnu/bits/poll.h
var.o: /usr/include/x86_64-linux-gnu/sys/socket.h
var.o: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
var.o: /usr/include/x86_64-linux-gnu/bits/socket.h
var.o: /usr/include/x86_64-linux-gnu/bits/socket_type.h
var.o: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
var.o: /usr/include/x86_64-linux-gnu/asm/socket.h
var.o: /usr/include/asm-generic/socket.h
var.o: /usr/include/x86_64-linux-gnu/asm/sockios.h
var.o: /usr/include/asm-generic/sockios.h
var.o: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
var.o: /usr/include/x86_64-linux-gnu/sys/un.h
var.o: /usr/include/netinet/in.h
var.o: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
var.o: /usr/include/x86_64-linux-gnu/bits/in.h
var.o: /usr/include/netinet/tcp.h
var.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
var.o: /usr/include/stdint.h
var.o: /usr/include/x86_64-linux-gnu/bits/wchar.h
var.o: /usr/include/arpa/inet.h
var.o: /usr/include/arpa/nameser.h
var.o: /usr/include/x86_64-linux-gnu/sys/param.h
var.o: /usr/include/signal.h
var.o: /usr/include/x86_64-linux-gnu/bits/signum.h
var.o: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
var.o: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
var.o: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
var.o: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
var.o: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
var.o: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
var.o: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
var.o: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
var.o: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
var.o: /usr/include/x86_64-linux-gnu/bits/sigaction.h
var.o: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
var.o: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
var.o: /usr/include/x86_64-linux-gnu/sys/ucontext.h
var.o: /usr/include/x86_64-linux-gnu/bits/sigstack.h
var.o: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
var.o: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
var.o: /usr/include/x86_64-linux-gnu/bits/sigthread.h
var.o: /usr/include/x86_64-linux-gnu/bits/param.h
var.o: /usr/include/linux/param.h
var.o: /usr/include/x86_64-linux-gnu/asm/param.h
var.o: /usr/include/asm-generic/param.h
var.o: /usr/include/arpa/nameser_compat.h
var.o: /usr/include/netdb.h
var.o: /usr/include/rpc/netdb.h
var.o: /usr/include/x86_64-linux-gnu/bits/netdb.h
var.o: /usr/include/resolv.h
var.o: /usr/include/stdio.h
var.o: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
var.o: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
var.o: /usr/include/x86_64-linux-gnu/bits/libio.h
var.o: /usr/include/x86_64-linux-gnu/bits/_G_config.h
var.o: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
var.o: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
var.o: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
var.o: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
var.o: ../../include/ac/unistd.h
var.o: /usr/include/unistd.h
var.o: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
var.o: /usr/include/x86_64-linux-gnu/bits/environments.h
var.o: /usr/include/x86_64-linux-gnu/bits/confname.h
var.o: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
var.o: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
var.o: /usr/include/getopt.h
var.o: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
var.o: ../../include/lutil_lockf.h
var.o: ../../include/ac/ctype.h
var.o: /usr/include/ctype.h
var.o: ../../include/lber.h
var.o: ../../include/lber_types.h
var.o: ../../include/ldap_cdefs.h
var.o: ../../include/ldap.h
var.o: ../../include/ldap_features.h
var.o: ../../include/ldap_log.h
var.o: ../../include/lutil.h
var.o: ../../include/avl.h
var.o: ../../include/rewrite.h
var.o: ../../include/ldap_pvt_thread.h
var.o: ../../include/ldap_int_thread.h
xmap.o: xmap.c
xmap.o: /usr/include/stdc-predef.h
xmap.o: ../../include/portable.h
xmap.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
xmap.o: ../../include/ac/fdset.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/types.h
xmap.o: /usr/include/features.h
xmap.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
xmap.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
xmap.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
xmap.o: ../../include/ldap_cdefs.h
xmap.o: ../../include/ldap_features.h
xmap.o: ../../include/ac/assert.h
xmap.o: /usr/include/assert.h
xmap.o: ../../include/ac/localize.h
xmap.o: /usr/include/stdio.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/libio.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/_G_config.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
xmap.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
xmap.o: /usr/include/pwd.h
xmap.o: rewrite-int.h
xmap.o: ../../include/ac/stdlib.h
xmap.o: /usr/include/stdlib.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/waitflags.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/floatn.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
xmap.o: /usr/include/x86_64-linux-gnu/sys/types.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
xmap.o: /usr/include/endian.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/endian.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/byteswap.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
xmap.o: /usr/include/x86_64-linux-gnu/sys/select.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/select.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
xmap.o: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
xmap.o: /usr/include/alloca.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
xmap.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
xmap.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
xmap.o: /usr/include/limits.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/local_lim.h
xmap.o: /usr/include/linux/limits.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
xmap.o: ../../include/ac/string.h
xmap.o: /usr/include/string.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
xmap.o: /usr/include/strings.h
xmap.o: ../../include/ac/syslog.h
xmap.o: /usr/include/syslog.h
xmap.o: /usr/include/x86_64-linux-gnu/sys/syslog.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/syslog-path.h
xmap.o: ../../include/ac/regex.h
xmap.o: /usr/include/regex.h
xmap.o: ../../include/ac/socket.h
xmap.o: /usr/include/poll.h
xmap.o: /usr/include/x86_64-linux-gnu/sys/poll.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/poll.h
xmap.o: /usr/include/x86_64-linux-gnu/sys/socket.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/socket.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/socket_type.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
xmap.o: /usr/include/x86_64-linux-gnu/asm/socket.h
xmap.o: /usr/include/asm-generic/socket.h
xmap.o: /usr/include/x86_64-linux-gnu/asm/sockios.h
xmap.o: /usr/include/asm-generic/sockios.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
xmap.o: /usr/include/x86_64-linux-gnu/sys/un.h
xmap.o: /usr/include/netinet/in.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/in.h
xmap.o: /usr/include/netinet/tcp.h
xmap.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
xmap.o: /usr/include/stdint.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/wchar.h
xmap.o: /usr/include/arpa/inet.h
xmap.o: /usr/include/arpa/nameser.h
xmap.o: /usr/include/x86_64-linux-gnu/sys/param.h
xmap.o: /usr/include/signal.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/signum.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/sigaction.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
xmap.o: /usr/include/x86_64-linux-gnu/sys/ucontext.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/sigstack.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/sigthread.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/param.h
xmap.o: /usr/include/linux/param.h
xmap.o: /usr/include/x86_64-linux-gnu/asm/param.h
xmap.o: /usr/include/asm-generic/param.h
xmap.o: /usr/include/arpa/nameser_compat.h
xmap.o: /usr/include/netdb.h
xmap.o: /usr/include/rpc/netdb.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/netdb.h
xmap.o: /usr/include/resolv.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
xmap.o: ../../include/ac/unistd.h
xmap.o: /usr/include/unistd.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/environments.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/confname.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
xmap.o: /usr/include/getopt.h
xmap.o: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
xmap.o: ../../include/lutil_lockf.h
xmap.o: ../../include/ac/ctype.h
xmap.o: /usr/include/ctype.h
xmap.o: ../../include/lber.h
xmap.o: ../../include/lber_types.h
xmap.o: ../../include/ldap_cdefs.h
xmap.o: ../../include/ldap.h
xmap.o: ../../include/ldap_features.h
xmap.o: ../../include/ldap_log.h
xmap.o: ../../include/lutil.h
xmap.o: ../../include/avl.h
xmap.o: ../../include/rewrite.h
xmap.o: ../../include/ldap_pvt_thread.h
xmap.o: ../../include/ldap_int_thread.h
xmap.o: rewrite-map.h
parse.o: parse.c
parse.o: /usr/include/stdc-predef.h
parse.o: ../../include/portable.h
parse.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
parse.o: ../../include/ac/fdset.h
parse.o: /usr/include/x86_64-linux-gnu/bits/types.h
parse.o: /usr/include/features.h
parse.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
parse.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
parse.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
parse.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
parse.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
parse.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
parse.o: ../../include/ldap_cdefs.h
parse.o: ../../include/ldap_features.h
parse.o: ../../include/ac/assert.h
parse.o: /usr/include/assert.h
parse.o: ../../include/ac/localize.h
parse.o: /usr/include/stdio.h
parse.o: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
parse.o: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
parse.o: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
parse.o: /usr/include/x86_64-linux-gnu/bits/libio.h
parse.o: /usr/include/x86_64-linux-gnu/bits/_G_config.h
parse.o: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
parse.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
parse.o: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
parse.o: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
parse.o: rewrite-int.h
parse.o: ../../include/ac/stdlib.h
parse.o: /usr/include/stdlib.h
parse.o: /usr/include/x86_64-linux-gnu/bits/waitflags.h
parse.o: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
parse.o: /usr/include/x86_64-linux-gnu/bits/floatn.h
parse.o: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
parse.o: /usr/include/x86_64-linux-gnu/sys/types.h
parse.o: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
parse.o: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
parse.o: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
parse.o: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
parse.o: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
parse.o: /usr/include/endian.h
parse.o: /usr/include/x86_64-linux-gnu/bits/endian.h
parse.o: /usr/include/x86_64-linux-gnu/bits/byteswap.h
parse.o: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
parse.o: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
parse.o: /usr/include/x86_64-linux-gnu/sys/select.h
parse.o: /usr/include/x86_64-linux-gnu/bits/select.h
parse.o: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
parse.o: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
parse.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
parse.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
parse.o: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
parse.o: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
parse.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
parse.o: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
parse.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
parse.o: /usr/include/alloca.h
parse.o: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
parse.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
parse.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
parse.o: /usr/include/limits.h
parse.o: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
parse.o: /usr/include/x86_64-linux-gnu/bits/local_lim.h
parse.o: /usr/include/linux/limits.h
parse.o: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
parse.o: ../../include/ac/string.h
parse.o: /usr/include/string.h
parse.o: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
parse.o: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
parse.o: /usr/include/strings.h
parse.o: ../../include/ac/syslog.h
parse.o: /usr/include/syslog.h
parse.o: /usr/include/x86_64-linux-gnu/sys/syslog.h
parse.o: /usr/include/x86_64-linux-gnu/bits/syslog-path.h
parse.o: ../../include/ac/regex.h
parse.o: /usr/include/regex.h
parse.o: ../../include/ac/socket.h
parse.o: /usr/include/poll.h
parse.o: /usr/include/x86_64-linux-gnu/sys/poll.h
parse.o: /usr/include/x86_64-linux-gnu/bits/poll.h
parse.o: /usr/include/x86_64-linux-gnu/sys/socket.h
parse.o: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
parse.o: /usr/include/x86_64-linux-gnu/bits/socket.h
parse.o: /usr/include/x86_64-linux-gnu/bits/socket_type.h
parse.o: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
parse.o: /usr/include/x86_64-linux-gnu/asm/socket.h
parse.o: /usr/include/asm-generic/socket.h
parse.o: /usr/include/x86_64-linux-gnu/asm/sockios.h
parse.o: /usr/include/asm-generic/sockios.h
parse.o: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
parse.o: /usr/include/x86_64-linux-gnu/sys/un.h
parse.o: /usr/include/netinet/in.h
parse.o: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
parse.o: /usr/include/x86_64-linux-gnu/bits/in.h
parse.o: /usr/include/netinet/tcp.h
parse.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
parse.o: /usr/include/stdint.h
parse.o: /usr/include/x86_64-linux-gnu/bits/wchar.h
parse.o: /usr/include/arpa/inet.h
parse.o: /usr/include/arpa/nameser.h
parse.o: /usr/include/x86_64-linux-gnu/sys/param.h
parse.o: /usr/include/signal.h
parse.o: /usr/include/x86_64-linux-gnu/bits/signum.h
parse.o: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
parse.o: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
parse.o: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
parse.o: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
parse.o: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
parse.o: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
parse.o: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
parse.o: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
parse.o: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
parse.o: /usr/include/x86_64-linux-gnu/bits/sigaction.h
parse.o: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
parse.o: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
parse.o: /usr/include/x86_64-linux-gnu/sys/ucontext.h
parse.o: /usr/include/x86_64-linux-gnu/bits/sigstack.h
parse.o: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
parse.o: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
parse.o: /usr/include/x86_64-linux-gnu/bits/sigthread.h
parse.o: /usr/include/x86_64-linux-gnu/bits/param.h
parse.o: /usr/include/linux/param.h
parse.o: /usr/include/x86_64-linux-gnu/asm/param.h
parse.o: /usr/include/asm-generic/param.h
parse.o: /usr/include/arpa/nameser_compat.h
parse.o: /usr/include/netdb.h
parse.o: /usr/include/rpc/netdb.h
parse.o: /usr/include/x86_64-linux-gnu/bits/netdb.h
parse.o: /usr/include/resolv.h
parse.o: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
parse.o: ../../include/ac/unistd.h
parse.o: /usr/include/unistd.h
parse.o: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
parse.o: /usr/include/x86_64-linux-gnu/bits/environments.h
parse.o: /usr/include/x86_64-linux-gnu/bits/confname.h
parse.o: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
parse.o: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
parse.o: /usr/include/getopt.h
parse.o: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
parse.o: ../../include/lutil_lockf.h
parse.o: ../../include/ac/ctype.h
parse.o: /usr/include/ctype.h
parse.o: ../../include/lber.h
parse.o: ../../include/lber_types.h
parse.o: ../../include/ldap_cdefs.h
parse.o: ../../include/ldap.h
parse.o: ../../include/ldap_features.h
parse.o: ../../include/ldap_log.h
parse.o: ../../include/lutil.h
parse.o: ../../include/avl.h
parse.o: ../../include/rewrite.h
parse.o: ../../include/ldap_pvt_thread.h
parse.o: ../../include/ldap_int_thread.h
rewrite.o: rewrite.c
rewrite.o: /usr/include/stdc-predef.h
rewrite.o: ../../include/portable.h
rewrite.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
rewrite.o: ../../include/ac/fdset.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/types.h
rewrite.o: /usr/include/features.h
rewrite.o: /usr/include/x86_64-linux-gnu/sys/cdefs.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/wordsize.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/long-double.h
rewrite.o: /usr/include/x86_64-linux-gnu/gnu/stubs.h
rewrite.o: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/typesizes.h
rewrite.o: ../../include/ldap_cdefs.h
rewrite.o: ../../include/ldap_features.h
rewrite.o: ../../include/ac/assert.h
rewrite.o: /usr/include/assert.h
rewrite.o: ../../include/ac/localize.h
rewrite.o: ../../include/ac/stdlib.h
rewrite.o: /usr/include/stdlib.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/waitflags.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/floatn.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
rewrite.o: /usr/include/x86_64-linux-gnu/sys/types.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
rewrite.o: /usr/include/endian.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/endian.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/byteswap.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
rewrite.o: /usr/include/x86_64-linux-gnu/sys/select.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/select.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
rewrite.o: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
rewrite.o: /usr/include/alloca.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
rewrite.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
rewrite.o: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
rewrite.o: /usr/include/limits.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/local_lim.h
rewrite.o: /usr/include/linux/limits.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
rewrite.o: ../../include/ac/string.h
rewrite.o: /usr/include/string.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
rewrite.o: /usr/include/strings.h
rewrite.o: ../../include/ac/syslog.h
rewrite.o: /usr/include/syslog.h
rewrite.o: /usr/include/x86_64-linux-gnu/sys/syslog.h
rewrite.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/syslog-path.h
rewrite.o: ../../include/ac/regex.h
rewrite.o: /usr/include/regex.h
rewrite.o: ../../include/ac/socket.h
rewrite.o: /usr/include/poll.h
rewrite.o: /usr/include/x86_64-linux-gnu/sys/poll.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/poll.h
rewrite.o: /usr/include/x86_64-linux-gnu/sys/socket.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/socket.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/socket_type.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
rewrite.o: /usr/include/x86_64-linux-gnu/asm/socket.h
rewrite.o: /usr/include/asm-generic/socket.h
rewrite.o: /usr/include/x86_64-linux-gnu/asm/sockios.h
rewrite.o: /usr/include/asm-generic/sockios.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
rewrite.o: /usr/include/x86_64-linux-gnu/sys/un.h
rewrite.o: /usr/include/netinet/in.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/in.h
rewrite.o: /usr/include/netinet/tcp.h
rewrite.o: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
rewrite.o: /usr/include/stdint.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/wchar.h
rewrite.o: /usr/include/arpa/inet.h
rewrite.o: /usr/include/arpa/nameser.h
rewrite.o: /usr/include/x86_64-linux-gnu/sys/param.h
rewrite.o: /usr/include/signal.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/signum.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/sigaction.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
rewrite.o: /usr/include/x86_64-linux-gnu/sys/ucontext.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/sigstack.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/sigthread.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/param.h
rewrite.o: /usr/include/linux/param.h
rewrite.o: /usr/include/x86_64-linux-gnu/asm/param.h
rewrite.o: /usr/include/asm-generic/param.h
rewrite.o: /usr/include/arpa/nameser_compat.h
rewrite.o: /usr/include/netdb.h
rewrite.o: /usr/include/rpc/netdb.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/netdb.h
rewrite.o: /usr/include/resolv.h
rewrite.o: /usr/include/stdio.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/libio.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/_G_config.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
rewrite.o: ../../include/ac/unistd.h
rewrite.o: /usr/include/unistd.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/environments.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/confname.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
rewrite.o: /usr/include/getopt.h
rewrite.o: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
rewrite.o: ../../include/lutil_lockf.h
rewrite.o: ../../include/ac/ctype.h
rewrite.o: /usr/include/ctype.h
rewrite.o: ../../include/rewrite.h
rewrite.o: ../../include/lutil.h
rewrite.o: ../../include/ldap_cdefs.h
rewrite.o: ../../include/lber_types.h
rewrite.o: ../../include/ldap.h
rewrite.o: ../../include/lber.h
rewrite.o: ../../include/ldap_features.h

# IF YOU PUT ANYTHING HERE IT WILL GO AWAY

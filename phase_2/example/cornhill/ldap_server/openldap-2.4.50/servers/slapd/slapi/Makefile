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

# Makefile.in for SLAPI
# $OpenLDAP$
## This work is part of OpenLDAP Software <http://www.openldap.org/>.
##
## Copyright 1998-2020 The OpenLDAP Foundation.
## Portions Copyright IBM Corp. 1997,2002,2003
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted only as authorized by the OpenLDAP
## Public License.
##
## A copy of this license is available in the file LICENSE in the
## top-level directory of the distribution or, alternatively, at
## <http://www.OpenLDAP.org/license.html>.

LIBRARY = libslapi.la

#all-common: $(LIBRARY) $(PROGRAMS)
#	@touch plugin.c slapi_pblock.c slapi_utils.c slapi_ops.c slapi_ext.c

NT_SRCS = nt_err.c
NT_OBJS = nt_err.lo

LIB_DEFS = -DSLAPI_LIBRARY

SRCS=  plugin.c slapi_pblock.c slapi_utils.c printmsg.c slapi_ops.c slapi_dn.c slapi_ext.c slapi_overlay.c \
	$(UNIX_SRCS)
OBJS=  plugin.lo slapi_pblock.lo slapi_utils.lo printmsg.lo slapi_ops.lo slapi_dn.lo slapi_ext.lo slapi_overlay.lo \
	$(UNIX_SRCS)

XSRCS= version.c

LDAP_INCDIR= ../../../include -I.. -I.       
LDAP_LIBDIR= ../../../libraries       

XLIBS = $(LIBRARY)
XXLIBS = 
NT_LINK_LIBS = $(AC_LIBS)

XINCPATH = -I$(srcdir)/.. -I$(srcdir)
XDEFS = $(MODULES_CPPFLAGS)

BUILD_MOD = no

install-local: FORCE
	if test "$(BUILD_MOD)" = "yes"; then \
		$(MKDIR) $(DESTDIR)$(libdir); \
		$(LTINSTALL) $(INSTALLFLAGS) -m 644 $(LIBRARY) $(DESTDIR)$(libdir); \
	fi

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
# Makefile Template for Shared Libraries
#

MKDEPFLAG = -l

.SUFFIXES: .c .o .lo

.c.lo:
	$(LTCOMPILE_LIB) $<

$(LIBRARY): version.lo
	$(LTLINK_LIB) -o $@ $(OBJS) version.lo $(LINK_LIBS)

Makefile: $(top_srcdir)/build/lib-shared.mk

# DO NOT DELETE THIS LINE -- mkdep uses it.
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.


#
# files: -I../../../include -I.. -I. -I../../../include -I./.. -I. plugin.c slapi_pblock.c slapi_utils.c printmsg.c slapi_ops.c slapi_dn.c slapi_ext.c slapi_overlay.c
# command: cc -M -I.  -I../../../include -I.. -I. -I../../../include -I./.. -I. plugin.c slapi_pblock.c slapi_utils.c printmsg.c slapi_ops.c slapi_dn.c slapi_ext.c slapi_overlay.c
#

plugin.lo: plugin.c
plugin.lo: /usr/include/stdc-predef.h
plugin.lo: ../../../include/portable.h
plugin.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
plugin.lo: ../../../include/ac/fdset.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/types.h
plugin.lo: /usr/include/features.h
plugin.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
plugin.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
plugin.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
plugin.lo: ../../../include/ldap_cdefs.h
plugin.lo: ../../../include/ldap_features.h
plugin.lo: ../../../include/ac/assert.h
plugin.lo: /usr/include/assert.h
plugin.lo: ../../../include/ac/localize.h
plugin.lo: ../../../include/ldap_pvt_thread.h
plugin.lo: ../../../include/ldap_int_thread.h
plugin.lo: /usr/include/pthread.h
plugin.lo: /usr/include/endian.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/endian.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/byteswap.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
plugin.lo: /usr/include/sched.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/sched.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/cpu-set.h
plugin.lo: /usr/include/time.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/time.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_tm.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_itimerspec.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/setjmp.h
plugin.lo: ../slap.h
plugin.lo: ../../../include/ldap_defaults.h
plugin.lo: ../../../include/ldap_config.h
plugin.lo: /usr/include/stdio.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/libio.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/_G_config.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
plugin.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
plugin.lo: ../../../include/ac/stdlib.h
plugin.lo: /usr/include/stdlib.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/waitflags.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/floatn.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
plugin.lo: /usr/include/x86_64-linux-gnu/sys/types.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
plugin.lo: /usr/include/x86_64-linux-gnu/sys/select.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/select.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
plugin.lo: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
plugin.lo: /usr/include/alloca.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
plugin.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
plugin.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
plugin.lo: /usr/include/limits.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/local_lim.h
plugin.lo: /usr/include/linux/limits.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
plugin.lo: ../../../include/ac/syslog.h
plugin.lo: /usr/include/syslog.h
plugin.lo: /usr/include/x86_64-linux-gnu/sys/syslog.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/syslog-path.h
plugin.lo: ../../../include/ac/regex.h
plugin.lo: /usr/include/regex.h
plugin.lo: ../../../include/ac/signal.h
plugin.lo: /usr/include/signal.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/signum.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/sigaction.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
plugin.lo: /usr/include/x86_64-linux-gnu/sys/ucontext.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/sigstack.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/sigthread.h
plugin.lo: ../../../include/ac/socket.h
plugin.lo: /usr/include/poll.h
plugin.lo: /usr/include/x86_64-linux-gnu/sys/poll.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/poll.h
plugin.lo: /usr/include/x86_64-linux-gnu/sys/socket.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/socket.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/socket_type.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
plugin.lo: /usr/include/x86_64-linux-gnu/asm/socket.h
plugin.lo: /usr/include/asm-generic/socket.h
plugin.lo: /usr/include/x86_64-linux-gnu/asm/sockios.h
plugin.lo: /usr/include/asm-generic/sockios.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
plugin.lo: /usr/include/x86_64-linux-gnu/sys/un.h
plugin.lo: /usr/include/string.h
plugin.lo: /usr/include/strings.h
plugin.lo: /usr/include/netinet/in.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/in.h
plugin.lo: /usr/include/netinet/tcp.h
plugin.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
plugin.lo: /usr/include/stdint.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/wchar.h
plugin.lo: /usr/include/arpa/inet.h
plugin.lo: /usr/include/arpa/nameser.h
plugin.lo: /usr/include/x86_64-linux-gnu/sys/param.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/param.h
plugin.lo: /usr/include/linux/param.h
plugin.lo: /usr/include/x86_64-linux-gnu/asm/param.h
plugin.lo: /usr/include/asm-generic/param.h
plugin.lo: /usr/include/arpa/nameser_compat.h
plugin.lo: /usr/include/netdb.h
plugin.lo: /usr/include/rpc/netdb.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/netdb.h
plugin.lo: /usr/include/resolv.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
plugin.lo: ../../../include/ac/time.h
plugin.lo: /usr/include/x86_64-linux-gnu/sys/time.h
plugin.lo: ../../../include/ac/param.h
plugin.lo: ../../../include/ac/unistd.h
plugin.lo: /usr/include/unistd.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/environments.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/confname.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
plugin.lo: /usr/include/getopt.h
plugin.lo: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
plugin.lo: ../../../include/lutil_lockf.h
plugin.lo: ../../../include/avl.h
plugin.lo: ../../../include/ldap_cdefs.h
plugin.lo: ../../../include/ldap_log.h
plugin.lo: ../../../include/ldap.h
plugin.lo: ../../../include/lber.h
plugin.lo: ../../../include/lber_types.h
plugin.lo: ../../../include/ldap_features.h
plugin.lo: ../../../include/ldap_schema.h
plugin.lo: ../../../include/lber_pvt.h
plugin.lo: ../../../include/ldap_pvt.h
plugin.lo: ../../../include/openldap.h
plugin.lo: ../../../include/ldap_pvt_uc.h
plugin.lo: ../../../include/ac/bytes.h
plugin.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
plugin.lo: ../../../include/ldap_queue.h
plugin.lo: ../proto-slap.h
plugin.lo: ../config.h
plugin.lo: ../../../include/ac/string.h
plugin.lo: slapi.h
plugin.lo: ../../../include/lutil.h
plugin.lo: /usr/include/ltdl.h
plugin.lo: /usr/include/libltdl/lt_system.h
plugin.lo: /usr/include/libltdl/lt_error.h
plugin.lo: /usr/include/libltdl/lt_dlloader.h
slapi_pblock.lo: slapi_pblock.c
slapi_pblock.lo: /usr/include/stdc-predef.h
slapi_pblock.lo: ../../../include/portable.h
slapi_pblock.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
slapi_pblock.lo: ../../../include/ac/fdset.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/types.h
slapi_pblock.lo: /usr/include/features.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
slapi_pblock.lo: ../../../include/ldap_cdefs.h
slapi_pblock.lo: ../../../include/ldap_features.h
slapi_pblock.lo: ../../../include/ac/assert.h
slapi_pblock.lo: /usr/include/assert.h
slapi_pblock.lo: ../../../include/ac/localize.h
slapi_pblock.lo: ../slap.h
slapi_pblock.lo: ../../../include/ldap_defaults.h
slapi_pblock.lo: ../../../include/ldap_config.h
slapi_pblock.lo: /usr/include/stdio.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/libio.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/_G_config.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
slapi_pblock.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
slapi_pblock.lo: ../../../include/ac/stdlib.h
slapi_pblock.lo: /usr/include/stdlib.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/waitflags.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/floatn.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/sys/types.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
slapi_pblock.lo: /usr/include/endian.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/endian.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/byteswap.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/sys/select.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/select.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
slapi_pblock.lo: /usr/include/alloca.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
slapi_pblock.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
slapi_pblock.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
slapi_pblock.lo: /usr/include/limits.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/local_lim.h
slapi_pblock.lo: /usr/include/linux/limits.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
slapi_pblock.lo: ../../../include/ac/syslog.h
slapi_pblock.lo: /usr/include/syslog.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/sys/syslog.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/syslog-path.h
slapi_pblock.lo: ../../../include/ac/regex.h
slapi_pblock.lo: /usr/include/regex.h
slapi_pblock.lo: ../../../include/ac/signal.h
slapi_pblock.lo: /usr/include/signal.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/signum.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/sigaction.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/sys/ucontext.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/sigstack.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/sigthread.h
slapi_pblock.lo: ../../../include/ac/socket.h
slapi_pblock.lo: /usr/include/poll.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/sys/poll.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/poll.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/sys/socket.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/socket.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/socket_type.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/asm/socket.h
slapi_pblock.lo: /usr/include/asm-generic/socket.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/asm/sockios.h
slapi_pblock.lo: /usr/include/asm-generic/sockios.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/sys/un.h
slapi_pblock.lo: /usr/include/string.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
slapi_pblock.lo: /usr/include/strings.h
slapi_pblock.lo: /usr/include/netinet/in.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/in.h
slapi_pblock.lo: /usr/include/netinet/tcp.h
slapi_pblock.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
slapi_pblock.lo: /usr/include/stdint.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/wchar.h
slapi_pblock.lo: /usr/include/arpa/inet.h
slapi_pblock.lo: /usr/include/arpa/nameser.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/sys/param.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/param.h
slapi_pblock.lo: /usr/include/linux/param.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/asm/param.h
slapi_pblock.lo: /usr/include/asm-generic/param.h
slapi_pblock.lo: /usr/include/arpa/nameser_compat.h
slapi_pblock.lo: /usr/include/netdb.h
slapi_pblock.lo: /usr/include/rpc/netdb.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/netdb.h
slapi_pblock.lo: /usr/include/resolv.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
slapi_pblock.lo: ../../../include/ac/time.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/sys/time.h
slapi_pblock.lo: /usr/include/time.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/time.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_tm.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_itimerspec.h
slapi_pblock.lo: ../../../include/ac/param.h
slapi_pblock.lo: ../../../include/ac/unistd.h
slapi_pblock.lo: /usr/include/unistd.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/environments.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/confname.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
slapi_pblock.lo: /usr/include/getopt.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
slapi_pblock.lo: ../../../include/lutil_lockf.h
slapi_pblock.lo: ../../../include/avl.h
slapi_pblock.lo: ../../../include/ldap_cdefs.h
slapi_pblock.lo: ../../../include/ldap_log.h
slapi_pblock.lo: ../../../include/ldap.h
slapi_pblock.lo: ../../../include/lber.h
slapi_pblock.lo: ../../../include/lber_types.h
slapi_pblock.lo: ../../../include/ldap_features.h
slapi_pblock.lo: ../../../include/ldap_schema.h
slapi_pblock.lo: ../../../include/lber_pvt.h
slapi_pblock.lo: ../../../include/ldap_pvt.h
slapi_pblock.lo: ../../../include/openldap.h
slapi_pblock.lo: ../../../include/ldap_pvt_uc.h
slapi_pblock.lo: ../../../include/ac/bytes.h
slapi_pblock.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
slapi_pblock.lo: ../../../include/ldap_pvt_thread.h
slapi_pblock.lo: ../../../include/ldap_int_thread.h
slapi_pblock.lo: /usr/include/pthread.h
slapi_pblock.lo: /usr/include/sched.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/sched.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/cpu-set.h
slapi_pblock.lo: /usr/include/x86_64-linux-gnu/bits/setjmp.h
slapi_pblock.lo: ../../../include/ldap_queue.h
slapi_pblock.lo: ../proto-slap.h
slapi_pblock.lo: slapi.h
slapi_utils.lo: slapi_utils.c
slapi_utils.lo: /usr/include/stdc-predef.h
slapi_utils.lo: ../../../include/portable.h
slapi_utils.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
slapi_utils.lo: ../../../include/ac/fdset.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/types.h
slapi_utils.lo: /usr/include/features.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
slapi_utils.lo: ../../../include/ldap_cdefs.h
slapi_utils.lo: ../../../include/ldap_features.h
slapi_utils.lo: ../../../include/ac/assert.h
slapi_utils.lo: /usr/include/assert.h
slapi_utils.lo: ../../../include/ac/localize.h
slapi_utils.lo: ../../../include/ac/string.h
slapi_utils.lo: /usr/include/string.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
slapi_utils.lo: /usr/include/strings.h
slapi_utils.lo: ../../../include/ac/stdarg.h
slapi_utils.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
slapi_utils.lo: ../../../include/ac/ctype.h
slapi_utils.lo: /usr/include/ctype.h
slapi_utils.lo: /usr/include/endian.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/endian.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/byteswap.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
slapi_utils.lo: ../../../include/ac/unistd.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/sys/types.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/sys/select.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/select.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
slapi_utils.lo: /usr/include/unistd.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/environments.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/confname.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
slapi_utils.lo: /usr/include/getopt.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
slapi_utils.lo: ../../../include/lutil_lockf.h
slapi_utils.lo: ../../../include/lutil.h
slapi_utils.lo: ../../../include/ldap_cdefs.h
slapi_utils.lo: ../../../include/lber_types.h
slapi_utils.lo: ../slap.h
slapi_utils.lo: ../../../include/ldap_defaults.h
slapi_utils.lo: ../../../include/ldap_config.h
slapi_utils.lo: /usr/include/stdio.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/libio.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/_G_config.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
slapi_utils.lo: ../../../include/ac/stdlib.h
slapi_utils.lo: /usr/include/stdlib.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/waitflags.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/floatn.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
slapi_utils.lo: /usr/include/alloca.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
slapi_utils.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
slapi_utils.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
slapi_utils.lo: /usr/include/limits.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/local_lim.h
slapi_utils.lo: /usr/include/linux/limits.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
slapi_utils.lo: ../../../include/ac/syslog.h
slapi_utils.lo: /usr/include/syslog.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/sys/syslog.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/syslog-path.h
slapi_utils.lo: ../../../include/ac/regex.h
slapi_utils.lo: /usr/include/regex.h
slapi_utils.lo: ../../../include/ac/signal.h
slapi_utils.lo: /usr/include/signal.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/signum.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/sigaction.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/sys/ucontext.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/sigstack.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/sigthread.h
slapi_utils.lo: ../../../include/ac/socket.h
slapi_utils.lo: /usr/include/poll.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/sys/poll.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/poll.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/sys/socket.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/socket.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/socket_type.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/asm/socket.h
slapi_utils.lo: /usr/include/asm-generic/socket.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/asm/sockios.h
slapi_utils.lo: /usr/include/asm-generic/sockios.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/sys/un.h
slapi_utils.lo: /usr/include/netinet/in.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/in.h
slapi_utils.lo: /usr/include/netinet/tcp.h
slapi_utils.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
slapi_utils.lo: /usr/include/stdint.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/wchar.h
slapi_utils.lo: /usr/include/arpa/inet.h
slapi_utils.lo: /usr/include/arpa/nameser.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/sys/param.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/param.h
slapi_utils.lo: /usr/include/linux/param.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/asm/param.h
slapi_utils.lo: /usr/include/asm-generic/param.h
slapi_utils.lo: /usr/include/arpa/nameser_compat.h
slapi_utils.lo: /usr/include/netdb.h
slapi_utils.lo: /usr/include/rpc/netdb.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/netdb.h
slapi_utils.lo: /usr/include/resolv.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
slapi_utils.lo: ../../../include/ac/time.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/sys/time.h
slapi_utils.lo: /usr/include/time.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/time.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_tm.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_itimerspec.h
slapi_utils.lo: ../../../include/ac/param.h
slapi_utils.lo: ../../../include/avl.h
slapi_utils.lo: ../../../include/ldap_log.h
slapi_utils.lo: ../../../include/ldap.h
slapi_utils.lo: ../../../include/lber.h
slapi_utils.lo: ../../../include/ldap_features.h
slapi_utils.lo: ../../../include/ldap_schema.h
slapi_utils.lo: ../../../include/lber_pvt.h
slapi_utils.lo: ../../../include/ldap_pvt.h
slapi_utils.lo: ../../../include/openldap.h
slapi_utils.lo: ../../../include/ldap_pvt_uc.h
slapi_utils.lo: ../../../include/ac/bytes.h
slapi_utils.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
slapi_utils.lo: ../../../include/ldap_pvt_thread.h
slapi_utils.lo: ../../../include/ldap_int_thread.h
slapi_utils.lo: /usr/include/pthread.h
slapi_utils.lo: /usr/include/sched.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/sched.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/cpu-set.h
slapi_utils.lo: /usr/include/x86_64-linux-gnu/bits/setjmp.h
slapi_utils.lo: ../../../include/ldap_queue.h
slapi_utils.lo: ../proto-slap.h
slapi_utils.lo: slapi.h
printmsg.lo: printmsg.c
printmsg.lo: /usr/include/stdc-predef.h
printmsg.lo: ../../../include/portable.h
printmsg.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
printmsg.lo: ../../../include/ac/fdset.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/types.h
printmsg.lo: /usr/include/features.h
printmsg.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
printmsg.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
printmsg.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
printmsg.lo: ../../../include/ldap_cdefs.h
printmsg.lo: ../../../include/ldap_features.h
printmsg.lo: ../../../include/ac/assert.h
printmsg.lo: /usr/include/assert.h
printmsg.lo: ../../../include/ac/localize.h
printmsg.lo: /usr/include/stdio.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/libio.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/_G_config.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
printmsg.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
printmsg.lo: ../../../include/ac/string.h
printmsg.lo: /usr/include/string.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
printmsg.lo: /usr/include/strings.h
printmsg.lo: ../../../include/ac/stdarg.h
printmsg.lo: ../../../include/ac/unistd.h
printmsg.lo: /usr/include/x86_64-linux-gnu/sys/types.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
printmsg.lo: /usr/include/endian.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/endian.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/byteswap.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
printmsg.lo: /usr/include/x86_64-linux-gnu/sys/select.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/select.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
printmsg.lo: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
printmsg.lo: /usr/include/unistd.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/environments.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/confname.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
printmsg.lo: /usr/include/getopt.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
printmsg.lo: ../../../include/lutil_lockf.h
printmsg.lo: /usr/include/fcntl.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/fcntl.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/fcntl-linux.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/stat.h
printmsg.lo: ../../../include/ac/errno.h
printmsg.lo: /usr/include/errno.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/errno.h
printmsg.lo: /usr/include/linux/errno.h
printmsg.lo: /usr/include/x86_64-linux-gnu/asm/errno.h
printmsg.lo: /usr/include/asm-generic/errno.h
printmsg.lo: /usr/include/asm-generic/errno-base.h
printmsg.lo: ../../../include/ldap.h
printmsg.lo: ../../../include/lber.h
printmsg.lo: ../../../include/lber_types.h
printmsg.lo: ../../../include/ldap_cdefs.h
printmsg.lo: ../../../include/ldap_features.h
printmsg.lo: ../../../include/ldap_config.h
printmsg.lo: ../slap.h
printmsg.lo: ../../../include/ldap_defaults.h
printmsg.lo: ../../../include/ac/stdlib.h
printmsg.lo: /usr/include/stdlib.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/waitflags.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/floatn.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
printmsg.lo: /usr/include/alloca.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
printmsg.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
printmsg.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
printmsg.lo: /usr/include/limits.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/local_lim.h
printmsg.lo: /usr/include/linux/limits.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
printmsg.lo: ../../../include/ac/syslog.h
printmsg.lo: /usr/include/syslog.h
printmsg.lo: /usr/include/x86_64-linux-gnu/sys/syslog.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/syslog-path.h
printmsg.lo: ../../../include/ac/regex.h
printmsg.lo: /usr/include/regex.h
printmsg.lo: ../../../include/ac/signal.h
printmsg.lo: /usr/include/signal.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/signum.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/sigaction.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
printmsg.lo: /usr/include/x86_64-linux-gnu/sys/ucontext.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/sigstack.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/sigthread.h
printmsg.lo: ../../../include/ac/socket.h
printmsg.lo: /usr/include/poll.h
printmsg.lo: /usr/include/x86_64-linux-gnu/sys/poll.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/poll.h
printmsg.lo: /usr/include/x86_64-linux-gnu/sys/socket.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/socket.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/socket_type.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
printmsg.lo: /usr/include/x86_64-linux-gnu/asm/socket.h
printmsg.lo: /usr/include/asm-generic/socket.h
printmsg.lo: /usr/include/x86_64-linux-gnu/asm/sockios.h
printmsg.lo: /usr/include/asm-generic/sockios.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
printmsg.lo: /usr/include/x86_64-linux-gnu/sys/un.h
printmsg.lo: /usr/include/netinet/in.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/in.h
printmsg.lo: /usr/include/netinet/tcp.h
printmsg.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
printmsg.lo: /usr/include/stdint.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/wchar.h
printmsg.lo: /usr/include/arpa/inet.h
printmsg.lo: /usr/include/arpa/nameser.h
printmsg.lo: /usr/include/x86_64-linux-gnu/sys/param.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/param.h
printmsg.lo: /usr/include/linux/param.h
printmsg.lo: /usr/include/x86_64-linux-gnu/asm/param.h
printmsg.lo: /usr/include/asm-generic/param.h
printmsg.lo: /usr/include/arpa/nameser_compat.h
printmsg.lo: /usr/include/netdb.h
printmsg.lo: /usr/include/rpc/netdb.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/netdb.h
printmsg.lo: /usr/include/resolv.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
printmsg.lo: ../../../include/ac/time.h
printmsg.lo: /usr/include/x86_64-linux-gnu/sys/time.h
printmsg.lo: /usr/include/time.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/time.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_tm.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_itimerspec.h
printmsg.lo: ../../../include/ac/param.h
printmsg.lo: ../../../include/avl.h
printmsg.lo: ../../../include/ldap_log.h
printmsg.lo: ../../../include/ldap_schema.h
printmsg.lo: ../../../include/lber_pvt.h
printmsg.lo: ../../../include/ldap_pvt.h
printmsg.lo: ../../../include/openldap.h
printmsg.lo: ../../../include/ldap_pvt_uc.h
printmsg.lo: ../../../include/ac/bytes.h
printmsg.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
printmsg.lo: ../../../include/ldap_pvt_thread.h
printmsg.lo: ../../../include/ldap_int_thread.h
printmsg.lo: /usr/include/pthread.h
printmsg.lo: /usr/include/sched.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/sched.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/cpu-set.h
printmsg.lo: /usr/include/x86_64-linux-gnu/bits/setjmp.h
printmsg.lo: ../../../include/ldap_queue.h
printmsg.lo: ../proto-slap.h
printmsg.lo: slapi.h
slapi_ops.lo: slapi_ops.c
slapi_ops.lo: /usr/include/stdc-predef.h
slapi_ops.lo: ../../../include/portable.h
slapi_ops.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
slapi_ops.lo: ../../../include/ac/fdset.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/types.h
slapi_ops.lo: /usr/include/features.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
slapi_ops.lo: ../../../include/ldap_cdefs.h
slapi_ops.lo: ../../../include/ldap_features.h
slapi_ops.lo: ../../../include/ac/assert.h
slapi_ops.lo: /usr/include/assert.h
slapi_ops.lo: ../../../include/ac/localize.h
slapi_ops.lo: ../../../include/ac/string.h
slapi_ops.lo: /usr/include/string.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
slapi_ops.lo: /usr/include/strings.h
slapi_ops.lo: ../../../include/ac/stdarg.h
slapi_ops.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
slapi_ops.lo: ../../../include/ac/ctype.h
slapi_ops.lo: /usr/include/ctype.h
slapi_ops.lo: /usr/include/endian.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/endian.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/byteswap.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
slapi_ops.lo: ../../../include/ac/unistd.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/sys/types.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/sys/select.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/select.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
slapi_ops.lo: /usr/include/unistd.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/environments.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/confname.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
slapi_ops.lo: /usr/include/getopt.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
slapi_ops.lo: ../../../include/lutil_lockf.h
slapi_ops.lo: ../slap.h
slapi_ops.lo: ../../../include/ldap_defaults.h
slapi_ops.lo: ../../../include/ldap_config.h
slapi_ops.lo: /usr/include/stdio.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/libio.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/_G_config.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
slapi_ops.lo: ../../../include/ac/stdlib.h
slapi_ops.lo: /usr/include/stdlib.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/waitflags.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/floatn.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
slapi_ops.lo: /usr/include/alloca.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
slapi_ops.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
slapi_ops.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
slapi_ops.lo: /usr/include/limits.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/local_lim.h
slapi_ops.lo: /usr/include/linux/limits.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
slapi_ops.lo: ../../../include/ac/syslog.h
slapi_ops.lo: /usr/include/syslog.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/sys/syslog.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/syslog-path.h
slapi_ops.lo: ../../../include/ac/regex.h
slapi_ops.lo: /usr/include/regex.h
slapi_ops.lo: ../../../include/ac/signal.h
slapi_ops.lo: /usr/include/signal.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/signum.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/sigaction.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/sys/ucontext.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/sigstack.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/sigthread.h
slapi_ops.lo: ../../../include/ac/socket.h
slapi_ops.lo: /usr/include/poll.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/sys/poll.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/poll.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/sys/socket.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/socket.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/socket_type.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/asm/socket.h
slapi_ops.lo: /usr/include/asm-generic/socket.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/asm/sockios.h
slapi_ops.lo: /usr/include/asm-generic/sockios.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/sys/un.h
slapi_ops.lo: /usr/include/netinet/in.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/in.h
slapi_ops.lo: /usr/include/netinet/tcp.h
slapi_ops.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
slapi_ops.lo: /usr/include/stdint.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/wchar.h
slapi_ops.lo: /usr/include/arpa/inet.h
slapi_ops.lo: /usr/include/arpa/nameser.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/sys/param.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/param.h
slapi_ops.lo: /usr/include/linux/param.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/asm/param.h
slapi_ops.lo: /usr/include/asm-generic/param.h
slapi_ops.lo: /usr/include/arpa/nameser_compat.h
slapi_ops.lo: /usr/include/netdb.h
slapi_ops.lo: /usr/include/rpc/netdb.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/netdb.h
slapi_ops.lo: /usr/include/resolv.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
slapi_ops.lo: ../../../include/ac/time.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/sys/time.h
slapi_ops.lo: /usr/include/time.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/time.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_tm.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_itimerspec.h
slapi_ops.lo: ../../../include/ac/param.h
slapi_ops.lo: ../../../include/avl.h
slapi_ops.lo: ../../../include/ldap_cdefs.h
slapi_ops.lo: ../../../include/ldap_log.h
slapi_ops.lo: ../../../include/ldap.h
slapi_ops.lo: ../../../include/lber.h
slapi_ops.lo: ../../../include/lber_types.h
slapi_ops.lo: ../../../include/ldap_features.h
slapi_ops.lo: ../../../include/ldap_schema.h
slapi_ops.lo: ../../../include/lber_pvt.h
slapi_ops.lo: ../../../include/ldap_pvt.h
slapi_ops.lo: ../../../include/openldap.h
slapi_ops.lo: ../../../include/ldap_pvt_uc.h
slapi_ops.lo: ../../../include/ac/bytes.h
slapi_ops.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
slapi_ops.lo: ../../../include/ldap_pvt_thread.h
slapi_ops.lo: ../../../include/ldap_int_thread.h
slapi_ops.lo: /usr/include/pthread.h
slapi_ops.lo: /usr/include/sched.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/sched.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/cpu-set.h
slapi_ops.lo: /usr/include/x86_64-linux-gnu/bits/setjmp.h
slapi_ops.lo: ../../../include/ldap_queue.h
slapi_ops.lo: ../proto-slap.h
slapi_ops.lo: slapi.h
slapi_dn.lo: slapi_dn.c
slapi_dn.lo: /usr/include/stdc-predef.h
slapi_dn.lo: ../../../include/portable.h
slapi_dn.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
slapi_dn.lo: ../../../include/ac/fdset.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/types.h
slapi_dn.lo: /usr/include/features.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
slapi_dn.lo: ../../../include/ldap_cdefs.h
slapi_dn.lo: ../../../include/ldap_features.h
slapi_dn.lo: ../../../include/ac/assert.h
slapi_dn.lo: /usr/include/assert.h
slapi_dn.lo: ../../../include/ac/localize.h
slapi_dn.lo: ../../../include/ac/string.h
slapi_dn.lo: /usr/include/string.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
slapi_dn.lo: /usr/include/strings.h
slapi_dn.lo: ../../../include/ac/stdarg.h
slapi_dn.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
slapi_dn.lo: ../../../include/ac/ctype.h
slapi_dn.lo: /usr/include/ctype.h
slapi_dn.lo: /usr/include/endian.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/endian.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/byteswap.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
slapi_dn.lo: ../../../include/ac/unistd.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/sys/types.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/sys/select.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/select.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
slapi_dn.lo: /usr/include/unistd.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/environments.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/confname.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
slapi_dn.lo: /usr/include/getopt.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
slapi_dn.lo: ../../../include/lutil_lockf.h
slapi_dn.lo: ../../../include/ldap_pvt.h
slapi_dn.lo: ../../../include/openldap.h
slapi_dn.lo: ../../../include/ldap.h
slapi_dn.lo: ../../../include/lber.h
slapi_dn.lo: ../../../include/lber_types.h
slapi_dn.lo: ../../../include/ldap_cdefs.h
slapi_dn.lo: ../../../include/ldap_features.h
slapi_dn.lo: ../../../include/lber_pvt.h
slapi_dn.lo: ../../../include/ldap_pvt_uc.h
slapi_dn.lo: ../../../include/ac/bytes.h
slapi_dn.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
slapi_dn.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
slapi_dn.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
slapi_dn.lo: /usr/include/limits.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/local_lim.h
slapi_dn.lo: /usr/include/linux/limits.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
slapi_dn.lo: ../slap.h
slapi_dn.lo: ../../../include/ldap_defaults.h
slapi_dn.lo: ../../../include/ldap_config.h
slapi_dn.lo: /usr/include/stdio.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/libio.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/_G_config.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
slapi_dn.lo: ../../../include/ac/stdlib.h
slapi_dn.lo: /usr/include/stdlib.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/waitflags.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/floatn.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
slapi_dn.lo: /usr/include/alloca.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
slapi_dn.lo: ../../../include/ac/syslog.h
slapi_dn.lo: /usr/include/syslog.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/sys/syslog.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/syslog-path.h
slapi_dn.lo: ../../../include/ac/regex.h
slapi_dn.lo: /usr/include/regex.h
slapi_dn.lo: ../../../include/ac/signal.h
slapi_dn.lo: /usr/include/signal.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/signum.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/sigaction.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/sys/ucontext.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/sigstack.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/sigthread.h
slapi_dn.lo: ../../../include/ac/socket.h
slapi_dn.lo: /usr/include/poll.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/sys/poll.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/poll.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/sys/socket.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/socket.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/socket_type.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/asm/socket.h
slapi_dn.lo: /usr/include/asm-generic/socket.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/asm/sockios.h
slapi_dn.lo: /usr/include/asm-generic/sockios.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/sys/un.h
slapi_dn.lo: /usr/include/netinet/in.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/in.h
slapi_dn.lo: /usr/include/netinet/tcp.h
slapi_dn.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
slapi_dn.lo: /usr/include/stdint.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/wchar.h
slapi_dn.lo: /usr/include/arpa/inet.h
slapi_dn.lo: /usr/include/arpa/nameser.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/sys/param.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/param.h
slapi_dn.lo: /usr/include/linux/param.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/asm/param.h
slapi_dn.lo: /usr/include/asm-generic/param.h
slapi_dn.lo: /usr/include/arpa/nameser_compat.h
slapi_dn.lo: /usr/include/netdb.h
slapi_dn.lo: /usr/include/rpc/netdb.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/netdb.h
slapi_dn.lo: /usr/include/resolv.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
slapi_dn.lo: ../../../include/ac/time.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/sys/time.h
slapi_dn.lo: /usr/include/time.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/time.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_tm.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_itimerspec.h
slapi_dn.lo: ../../../include/ac/param.h
slapi_dn.lo: ../../../include/avl.h
slapi_dn.lo: ../../../include/ldap_log.h
slapi_dn.lo: ../../../include/ldap_schema.h
slapi_dn.lo: ../../../include/ldap_pvt_thread.h
slapi_dn.lo: ../../../include/ldap_int_thread.h
slapi_dn.lo: /usr/include/pthread.h
slapi_dn.lo: /usr/include/sched.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/sched.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/cpu-set.h
slapi_dn.lo: /usr/include/x86_64-linux-gnu/bits/setjmp.h
slapi_dn.lo: ../../../include/ldap_queue.h
slapi_dn.lo: ../proto-slap.h
slapi_dn.lo: slapi.h
slapi_ext.lo: slapi_ext.c
slapi_ext.lo: /usr/include/stdc-predef.h
slapi_ext.lo: ../../../include/portable.h
slapi_ext.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
slapi_ext.lo: ../../../include/ac/fdset.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/types.h
slapi_ext.lo: /usr/include/features.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
slapi_ext.lo: ../../../include/ldap_cdefs.h
slapi_ext.lo: ../../../include/ldap_features.h
slapi_ext.lo: ../../../include/ac/assert.h
slapi_ext.lo: /usr/include/assert.h
slapi_ext.lo: ../../../include/ac/localize.h
slapi_ext.lo: ../../../include/ac/string.h
slapi_ext.lo: /usr/include/string.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
slapi_ext.lo: /usr/include/strings.h
slapi_ext.lo: ../../../include/ac/stdarg.h
slapi_ext.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
slapi_ext.lo: ../../../include/ac/ctype.h
slapi_ext.lo: /usr/include/ctype.h
slapi_ext.lo: /usr/include/endian.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/endian.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/byteswap.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
slapi_ext.lo: ../../../include/ac/unistd.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/sys/types.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/sys/select.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/select.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
slapi_ext.lo: /usr/include/unistd.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/environments.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/confname.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
slapi_ext.lo: /usr/include/getopt.h
slapi_ext.lo: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
slapi_ext.lo: ../../../include/lutil_lockf.h
slapi_overlay.lo: slapi_overlay.c
slapi_overlay.lo: /usr/include/stdc-predef.h
slapi_overlay.lo: ../../../include/portable.h
slapi_overlay.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
slapi_overlay.lo: ../../../include/ac/fdset.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/types.h
slapi_overlay.lo: /usr/include/features.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
slapi_overlay.lo: ../../../include/ldap_cdefs.h
slapi_overlay.lo: ../../../include/ldap_features.h
slapi_overlay.lo: ../../../include/ac/assert.h
slapi_overlay.lo: /usr/include/assert.h
slapi_overlay.lo: ../../../include/ac/localize.h
slapi_overlay.lo: /usr/include/stdio.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/libio.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/_G_config.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
slapi_overlay.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
slapi_overlay.lo: ../../../include/ac/string.h
slapi_overlay.lo: /usr/include/string.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
slapi_overlay.lo: /usr/include/strings.h
slapi_overlay.lo: ../../../include/ac/socket.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/sys/types.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
slapi_overlay.lo: /usr/include/endian.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/endian.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/byteswap.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/sys/select.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/select.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
slapi_overlay.lo: /usr/include/poll.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/sys/poll.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/poll.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/sys/socket.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/socket.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/socket_type.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/asm/socket.h
slapi_overlay.lo: /usr/include/asm-generic/socket.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/asm/sockios.h
slapi_overlay.lo: /usr/include/asm-generic/sockios.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/sys/un.h
slapi_overlay.lo: /usr/include/netinet/in.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/in.h
slapi_overlay.lo: /usr/include/netinet/tcp.h
slapi_overlay.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
slapi_overlay.lo: /usr/include/stdint.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/wchar.h
slapi_overlay.lo: /usr/include/arpa/inet.h
slapi_overlay.lo: /usr/include/arpa/nameser.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/sys/param.h
slapi_overlay.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
slapi_overlay.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
slapi_overlay.lo: /usr/include/limits.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/local_lim.h
slapi_overlay.lo: /usr/include/linux/limits.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
slapi_overlay.lo: /usr/include/signal.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/signum.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/sigaction.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/sys/ucontext.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/sigstack.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/sigthread.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/param.h
slapi_overlay.lo: /usr/include/linux/param.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/asm/param.h
slapi_overlay.lo: /usr/include/asm-generic/param.h
slapi_overlay.lo: /usr/include/arpa/nameser_compat.h
slapi_overlay.lo: /usr/include/netdb.h
slapi_overlay.lo: /usr/include/rpc/netdb.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/netdb.h
slapi_overlay.lo: /usr/include/resolv.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
slapi_overlay.lo: ../slap.h
slapi_overlay.lo: ../../../include/ldap_defaults.h
slapi_overlay.lo: ../../../include/ldap_config.h
slapi_overlay.lo: ../../../include/ac/stdlib.h
slapi_overlay.lo: /usr/include/stdlib.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/waitflags.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/floatn.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
slapi_overlay.lo: /usr/include/alloca.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
slapi_overlay.lo: ../../../include/ac/syslog.h
slapi_overlay.lo: /usr/include/syslog.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/sys/syslog.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/syslog-path.h
slapi_overlay.lo: ../../../include/ac/regex.h
slapi_overlay.lo: /usr/include/regex.h
slapi_overlay.lo: ../../../include/ac/signal.h
slapi_overlay.lo: ../../../include/ac/time.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/sys/time.h
slapi_overlay.lo: /usr/include/time.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/time.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_tm.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_itimerspec.h
slapi_overlay.lo: ../../../include/ac/param.h
slapi_overlay.lo: ../../../include/ac/unistd.h
slapi_overlay.lo: /usr/include/unistd.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/environments.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/confname.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
slapi_overlay.lo: /usr/include/getopt.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
slapi_overlay.lo: ../../../include/lutil_lockf.h
slapi_overlay.lo: ../../../include/avl.h
slapi_overlay.lo: ../../../include/ldap_cdefs.h
slapi_overlay.lo: ../../../include/ldap_log.h
slapi_overlay.lo: ../../../include/ldap.h
slapi_overlay.lo: ../../../include/lber.h
slapi_overlay.lo: ../../../include/lber_types.h
slapi_overlay.lo: ../../../include/ldap_features.h
slapi_overlay.lo: ../../../include/ldap_schema.h
slapi_overlay.lo: ../../../include/lber_pvt.h
slapi_overlay.lo: ../../../include/ldap_pvt.h
slapi_overlay.lo: ../../../include/openldap.h
slapi_overlay.lo: ../../../include/ldap_pvt_uc.h
slapi_overlay.lo: ../../../include/ac/bytes.h
slapi_overlay.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
slapi_overlay.lo: ../../../include/ldap_pvt_thread.h
slapi_overlay.lo: ../../../include/ldap_int_thread.h
slapi_overlay.lo: /usr/include/pthread.h
slapi_overlay.lo: /usr/include/sched.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/sched.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/cpu-set.h
slapi_overlay.lo: /usr/include/x86_64-linux-gnu/bits/setjmp.h
slapi_overlay.lo: ../../../include/ldap_queue.h
slapi_overlay.lo: ../proto-slap.h
slapi_overlay.lo: slapi.h
slapi_overlay.lo: ../config.h

# IF YOU PUT ANYTHING HERE IT WILL GO AWAY

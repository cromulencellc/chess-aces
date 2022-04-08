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

BUILD_LIBS_DYNAMIC = shared

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

LTSTATIC = 

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
MAKEINFO = ${SHELL} /vagrant/openldap-2.4.50/build/missing --run makeinfo
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

LUTIL_LIBS =  -lcrypt
LTHREAD_LIBS = 

BDB_LIBS = 
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

SLAPD_LIBS =      

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

# LIBLBER
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

LIBRARY = liblber.la

NT_SRCS = nt_err.c
NT_OBJS = nt_err.lo

UNIX_SRCS = stdio.c
UNIX_OBJS = stdio.lo

LIB_DEFS = -DLBER_LIBRARY

SRCS= assert.c decode.c encode.c io.c bprint.c debug.c \
	memory.c options.c sockbuf.c $(UNIX_SRCS)
OBJS= assert.lo decode.lo encode.lo io.lo bprint.lo debug.lo \
	memory.lo options.lo sockbuf.lo $(UNIX_OBJS)
XSRCS= version.c

PROGRAMS= dtest etest idtest

LDAP_INCDIR= ../../include
LDAP_LIBDIR= ../../libraries

XLIBS = $(LIBRARY) $(LDAP_LIBLUTIL_A)
XXLIBS = 
NT_LINK_LIBS = $(AC_LIBS)
UNIX_LINK_LIBS = $(AC_LIBS)

dtest:    $(XLIBS) dtest.o
	$(LTLINK) -o $@ dtest.o $(LIBS)
etest:  $(XLIBS) etest.o
	$(LTLINK) -o $@ etest.o $(LIBS)
idtest:  $(XLIBS) idtest.o
	$(LTLINK) -o $@ idtest.o $(LIBS)

install-local: FORCE
	-$(MKDIR) $(DESTDIR)$(libdir)
	$(LTINSTALL) $(INSTALLFLAGS) -m 644 $(LIBRARY) $(DESTDIR)$(libdir)
	$(LTFINISH) $(DESTDIR)$(libdir)

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
# files: -I../../include -I../../include assert.c decode.c encode.c io.c bprint.c debug.c memory.c options.c sockbuf.c stdio.c
# command: cc -M -I.  -I../../include -I../../include assert.c decode.c encode.c io.c bprint.c debug.c memory.c options.c sockbuf.c stdio.c
#

assert.lo: assert.c
assert.lo: /usr/include/stdc-predef.h
assert.lo: ../../include/portable.h
assert.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
assert.lo: ../../include/ac/fdset.h
assert.lo: /usr/include/x86_64-linux-gnu/bits/types.h
assert.lo: /usr/include/features.h
assert.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
assert.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
assert.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
assert.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
assert.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
assert.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
assert.lo: ../../include/ldap_cdefs.h
assert.lo: ../../include/ldap_features.h
assert.lo: ../../include/ac/assert.h
assert.lo: /usr/include/assert.h
assert.lo: ../../include/ac/localize.h
decode.lo: decode.c
decode.lo: /usr/include/stdc-predef.h
decode.lo: ../../include/portable.h
decode.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
decode.lo: ../../include/ac/fdset.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/types.h
decode.lo: /usr/include/features.h
decode.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
decode.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
decode.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
decode.lo: ../../include/ldap_cdefs.h
decode.lo: ../../include/ldap_features.h
decode.lo: ../../include/ac/assert.h
decode.lo: /usr/include/assert.h
decode.lo: ../../include/ac/localize.h
decode.lo: /usr/include/stdio.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/libio.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/_G_config.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
decode.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
decode.lo: ../../include/ac/stdlib.h
decode.lo: /usr/include/stdlib.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/waitflags.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/floatn.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
decode.lo: /usr/include/x86_64-linux-gnu/sys/types.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
decode.lo: /usr/include/endian.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/endian.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/byteswap.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
decode.lo: /usr/include/x86_64-linux-gnu/sys/select.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/select.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
decode.lo: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
decode.lo: /usr/include/alloca.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
decode.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
decode.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
decode.lo: /usr/include/limits.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/local_lim.h
decode.lo: /usr/include/linux/limits.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
decode.lo: ../../include/ac/stdarg.h
decode.lo: ../../include/ac/string.h
decode.lo: /usr/include/string.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
decode.lo: /usr/include/strings.h
decode.lo: ../../include/ac/socket.h
decode.lo: /usr/include/poll.h
decode.lo: /usr/include/x86_64-linux-gnu/sys/poll.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/poll.h
decode.lo: /usr/include/x86_64-linux-gnu/sys/socket.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/socket.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/socket_type.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
decode.lo: /usr/include/x86_64-linux-gnu/asm/socket.h
decode.lo: /usr/include/asm-generic/socket.h
decode.lo: /usr/include/x86_64-linux-gnu/asm/sockios.h
decode.lo: /usr/include/asm-generic/sockios.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
decode.lo: /usr/include/x86_64-linux-gnu/sys/un.h
decode.lo: /usr/include/netinet/in.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/in.h
decode.lo: /usr/include/netinet/tcp.h
decode.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
decode.lo: /usr/include/stdint.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/wchar.h
decode.lo: /usr/include/arpa/inet.h
decode.lo: /usr/include/arpa/nameser.h
decode.lo: /usr/include/x86_64-linux-gnu/sys/param.h
decode.lo: /usr/include/signal.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/signum.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/sigaction.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
decode.lo: /usr/include/x86_64-linux-gnu/sys/ucontext.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/sigstack.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/sigthread.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/param.h
decode.lo: /usr/include/linux/param.h
decode.lo: /usr/include/x86_64-linux-gnu/asm/param.h
decode.lo: /usr/include/asm-generic/param.h
decode.lo: /usr/include/arpa/nameser_compat.h
decode.lo: /usr/include/netdb.h
decode.lo: /usr/include/rpc/netdb.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/netdb.h
decode.lo: /usr/include/resolv.h
decode.lo: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
decode.lo: lber-int.h
decode.lo: ../../include/lber.h
decode.lo: ../../include/lber_types.h
decode.lo: ../../include/ldap_cdefs.h
decode.lo: ../../include/ldap_log.h
decode.lo: ../../include/lber_pvt.h
decode.lo: ../../include/ldap_queue.h
encode.lo: encode.c
encode.lo: /usr/include/stdc-predef.h
encode.lo: ../../include/portable.h
encode.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
encode.lo: ../../include/ac/fdset.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/types.h
encode.lo: /usr/include/features.h
encode.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
encode.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
encode.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
encode.lo: ../../include/ldap_cdefs.h
encode.lo: ../../include/ldap_features.h
encode.lo: ../../include/ac/assert.h
encode.lo: /usr/include/assert.h
encode.lo: ../../include/ac/localize.h
encode.lo: /usr/include/ctype.h
encode.lo: /usr/include/endian.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/endian.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/byteswap.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
encode.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
encode.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
encode.lo: /usr/include/limits.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/local_lim.h
encode.lo: /usr/include/linux/limits.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
encode.lo: /usr/include/stdio.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/libio.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/_G_config.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
encode.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
encode.lo: ../../include/ac/stdlib.h
encode.lo: /usr/include/stdlib.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/waitflags.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/floatn.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
encode.lo: /usr/include/x86_64-linux-gnu/sys/types.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
encode.lo: /usr/include/x86_64-linux-gnu/sys/select.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/select.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
encode.lo: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
encode.lo: /usr/include/alloca.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
encode.lo: ../../include/ac/stdarg.h
encode.lo: ../../include/ac/socket.h
encode.lo: /usr/include/poll.h
encode.lo: /usr/include/x86_64-linux-gnu/sys/poll.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/poll.h
encode.lo: /usr/include/x86_64-linux-gnu/sys/socket.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/socket.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/socket_type.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
encode.lo: /usr/include/x86_64-linux-gnu/asm/socket.h
encode.lo: /usr/include/asm-generic/socket.h
encode.lo: /usr/include/x86_64-linux-gnu/asm/sockios.h
encode.lo: /usr/include/asm-generic/sockios.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
encode.lo: /usr/include/x86_64-linux-gnu/sys/un.h
encode.lo: /usr/include/string.h
encode.lo: /usr/include/strings.h
encode.lo: /usr/include/netinet/in.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/in.h
encode.lo: /usr/include/netinet/tcp.h
encode.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
encode.lo: /usr/include/stdint.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/wchar.h
encode.lo: /usr/include/arpa/inet.h
encode.lo: /usr/include/arpa/nameser.h
encode.lo: /usr/include/x86_64-linux-gnu/sys/param.h
encode.lo: /usr/include/signal.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/signum.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/sigaction.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
encode.lo: /usr/include/x86_64-linux-gnu/sys/ucontext.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/sigstack.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/sigthread.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/param.h
encode.lo: /usr/include/linux/param.h
encode.lo: /usr/include/x86_64-linux-gnu/asm/param.h
encode.lo: /usr/include/asm-generic/param.h
encode.lo: /usr/include/arpa/nameser_compat.h
encode.lo: /usr/include/netdb.h
encode.lo: /usr/include/rpc/netdb.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/netdb.h
encode.lo: /usr/include/resolv.h
encode.lo: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
encode.lo: ../../include/ac/string.h
encode.lo: lber-int.h
encode.lo: ../../include/lber.h
encode.lo: ../../include/lber_types.h
encode.lo: ../../include/ldap_cdefs.h
encode.lo: ../../include/ldap_log.h
encode.lo: ../../include/lber_pvt.h
encode.lo: ../../include/ldap_queue.h
io.lo: io.c
io.lo: /usr/include/stdc-predef.h
io.lo: ../../include/portable.h
io.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
io.lo: ../../include/ac/fdset.h
io.lo: /usr/include/x86_64-linux-gnu/bits/types.h
io.lo: /usr/include/features.h
io.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
io.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
io.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
io.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
io.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
io.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
io.lo: ../../include/ldap_cdefs.h
io.lo: ../../include/ldap_features.h
io.lo: ../../include/ac/assert.h
io.lo: /usr/include/assert.h
io.lo: ../../include/ac/localize.h
io.lo: /usr/include/stdio.h
io.lo: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
io.lo: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
io.lo: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
io.lo: /usr/include/x86_64-linux-gnu/bits/libio.h
io.lo: /usr/include/x86_64-linux-gnu/bits/_G_config.h
io.lo: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
io.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
io.lo: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
io.lo: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
io.lo: ../../include/ac/stdlib.h
io.lo: /usr/include/stdlib.h
io.lo: /usr/include/x86_64-linux-gnu/bits/waitflags.h
io.lo: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
io.lo: /usr/include/x86_64-linux-gnu/bits/floatn.h
io.lo: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
io.lo: /usr/include/x86_64-linux-gnu/sys/types.h
io.lo: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
io.lo: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
io.lo: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
io.lo: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
io.lo: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
io.lo: /usr/include/endian.h
io.lo: /usr/include/x86_64-linux-gnu/bits/endian.h
io.lo: /usr/include/x86_64-linux-gnu/bits/byteswap.h
io.lo: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
io.lo: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
io.lo: /usr/include/x86_64-linux-gnu/sys/select.h
io.lo: /usr/include/x86_64-linux-gnu/bits/select.h
io.lo: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
io.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
io.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
io.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
io.lo: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
io.lo: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
io.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
io.lo: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
io.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
io.lo: /usr/include/alloca.h
io.lo: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
io.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
io.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
io.lo: /usr/include/limits.h
io.lo: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
io.lo: /usr/include/x86_64-linux-gnu/bits/local_lim.h
io.lo: /usr/include/linux/limits.h
io.lo: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
io.lo: ../../include/ac/ctype.h
io.lo: /usr/include/ctype.h
io.lo: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
io.lo: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
io.lo: ../../include/ac/errno.h
io.lo: /usr/include/errno.h
io.lo: /usr/include/x86_64-linux-gnu/bits/errno.h
io.lo: /usr/include/linux/errno.h
io.lo: /usr/include/x86_64-linux-gnu/asm/errno.h
io.lo: /usr/include/asm-generic/errno.h
io.lo: /usr/include/asm-generic/errno-base.h
io.lo: ../../include/ac/socket.h
io.lo: /usr/include/poll.h
io.lo: /usr/include/x86_64-linux-gnu/sys/poll.h
io.lo: /usr/include/x86_64-linux-gnu/bits/poll.h
io.lo: /usr/include/x86_64-linux-gnu/sys/socket.h
io.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
io.lo: /usr/include/x86_64-linux-gnu/bits/socket.h
io.lo: /usr/include/x86_64-linux-gnu/bits/socket_type.h
io.lo: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
io.lo: /usr/include/x86_64-linux-gnu/asm/socket.h
io.lo: /usr/include/asm-generic/socket.h
io.lo: /usr/include/x86_64-linux-gnu/asm/sockios.h
io.lo: /usr/include/asm-generic/sockios.h
io.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
io.lo: /usr/include/x86_64-linux-gnu/sys/un.h
io.lo: /usr/include/string.h
io.lo: /usr/include/strings.h
io.lo: /usr/include/netinet/in.h
io.lo: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
io.lo: /usr/include/x86_64-linux-gnu/bits/in.h
io.lo: /usr/include/netinet/tcp.h
io.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
io.lo: /usr/include/stdint.h
io.lo: /usr/include/x86_64-linux-gnu/bits/wchar.h
io.lo: /usr/include/arpa/inet.h
io.lo: /usr/include/arpa/nameser.h
io.lo: /usr/include/x86_64-linux-gnu/sys/param.h
io.lo: /usr/include/signal.h
io.lo: /usr/include/x86_64-linux-gnu/bits/signum.h
io.lo: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
io.lo: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
io.lo: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
io.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
io.lo: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
io.lo: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
io.lo: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
io.lo: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
io.lo: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
io.lo: /usr/include/x86_64-linux-gnu/bits/sigaction.h
io.lo: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
io.lo: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
io.lo: /usr/include/x86_64-linux-gnu/sys/ucontext.h
io.lo: /usr/include/x86_64-linux-gnu/bits/sigstack.h
io.lo: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
io.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
io.lo: /usr/include/x86_64-linux-gnu/bits/sigthread.h
io.lo: /usr/include/x86_64-linux-gnu/bits/param.h
io.lo: /usr/include/linux/param.h
io.lo: /usr/include/x86_64-linux-gnu/asm/param.h
io.lo: /usr/include/asm-generic/param.h
io.lo: /usr/include/arpa/nameser_compat.h
io.lo: /usr/include/netdb.h
io.lo: /usr/include/rpc/netdb.h
io.lo: /usr/include/x86_64-linux-gnu/bits/netdb.h
io.lo: /usr/include/resolv.h
io.lo: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
io.lo: ../../include/ac/string.h
io.lo: ../../include/ac/unistd.h
io.lo: /usr/include/unistd.h
io.lo: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
io.lo: /usr/include/x86_64-linux-gnu/bits/environments.h
io.lo: /usr/include/x86_64-linux-gnu/bits/confname.h
io.lo: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
io.lo: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
io.lo: /usr/include/getopt.h
io.lo: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
io.lo: ../../include/lutil_lockf.h
io.lo: lber-int.h
io.lo: ../../include/lber.h
io.lo: ../../include/lber_types.h
io.lo: ../../include/ldap_cdefs.h
io.lo: ../../include/ldap_log.h
io.lo: ../../include/lber_pvt.h
io.lo: ../../include/ldap_queue.h
bprint.lo: bprint.c
bprint.lo: /usr/include/stdc-predef.h
bprint.lo: ../../include/portable.h
bprint.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
bprint.lo: ../../include/ac/fdset.h
bprint.lo: /usr/include/x86_64-linux-gnu/bits/types.h
bprint.lo: /usr/include/features.h
bprint.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
bprint.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
bprint.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
bprint.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
bprint.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
bprint.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
bprint.lo: ../../include/ldap_cdefs.h
bprint.lo: ../../include/ldap_features.h
bprint.lo: ../../include/ac/assert.h
bprint.lo: /usr/include/assert.h
bprint.lo: ../../include/ac/localize.h
bprint.lo: /usr/include/stdio.h
bprint.lo: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
bprint.lo: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
bprint.lo: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
bprint.lo: /usr/include/x86_64-linux-gnu/bits/libio.h
bprint.lo: /usr/include/x86_64-linux-gnu/bits/_G_config.h
bprint.lo: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
bprint.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
bprint.lo: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
bprint.lo: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
bprint.lo: ../../include/ac/ctype.h
bprint.lo: /usr/include/ctype.h
bprint.lo: /usr/include/endian.h
bprint.lo: /usr/include/x86_64-linux-gnu/bits/endian.h
bprint.lo: /usr/include/x86_64-linux-gnu/bits/byteswap.h
bprint.lo: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
bprint.lo: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
bprint.lo: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
bprint.lo: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
bprint.lo: ../../include/ac/stdarg.h
bprint.lo: ../../include/ac/string.h
bprint.lo: /usr/include/string.h
bprint.lo: /usr/include/strings.h
bprint.lo: lber-int.h
bprint.lo: ../../include/lber.h
bprint.lo: ../../include/lber_types.h
bprint.lo: ../../include/ldap_cdefs.h
bprint.lo: ../../include/ldap_log.h
bprint.lo: ../../include/lber_pvt.h
bprint.lo: ../../include/ldap_queue.h
debug.lo: debug.c
debug.lo: /usr/include/stdc-predef.h
debug.lo: ../../include/portable.h
debug.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
debug.lo: ../../include/ac/fdset.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/types.h
debug.lo: /usr/include/features.h
debug.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
debug.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
debug.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
debug.lo: ../../include/ldap_cdefs.h
debug.lo: ../../include/ldap_features.h
debug.lo: ../../include/ac/assert.h
debug.lo: /usr/include/assert.h
debug.lo: ../../include/ac/localize.h
debug.lo: /usr/include/stdio.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/libio.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/_G_config.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
debug.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
debug.lo: ../../include/ac/stdarg.h
debug.lo: ../../include/ac/stdlib.h
debug.lo: /usr/include/stdlib.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/waitflags.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/floatn.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
debug.lo: /usr/include/x86_64-linux-gnu/sys/types.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
debug.lo: /usr/include/endian.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/endian.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/byteswap.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
debug.lo: /usr/include/x86_64-linux-gnu/sys/select.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/select.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
debug.lo: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
debug.lo: /usr/include/alloca.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
debug.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
debug.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
debug.lo: /usr/include/limits.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/local_lim.h
debug.lo: /usr/include/linux/limits.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
debug.lo: ../../include/ac/string.h
debug.lo: /usr/include/string.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
debug.lo: /usr/include/strings.h
debug.lo: ../../include/ac/time.h
debug.lo: /usr/include/x86_64-linux-gnu/sys/time.h
debug.lo: /usr/include/time.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/time.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_tm.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_itimerspec.h
debug.lo: ../../include/ac/ctype.h
debug.lo: /usr/include/ctype.h
debug.lo: ../../include/ac/syslog.h
debug.lo: /usr/include/syslog.h
debug.lo: /usr/include/x86_64-linux-gnu/sys/syslog.h
debug.lo: /usr/include/x86_64-linux-gnu/bits/syslog-path.h
debug.lo: ../../include/ldap_log.h
debug.lo: ../../include/ldap_cdefs.h
debug.lo: ../../include/ldap_defaults.h
debug.lo: ../../include/ldap_config.h
debug.lo: ../../include/lber.h
debug.lo: ../../include/lber_types.h
debug.lo: ../../include/ldap_pvt.h
debug.lo: ../../include/openldap.h
debug.lo: ../../include/ldap.h
debug.lo: ../../include/ldap_features.h
debug.lo: ../../include/lber_pvt.h
debug.lo: ../../include/ldap_pvt_uc.h
debug.lo: ../../include/ac/bytes.h
debug.lo: ../../include/../libraries/liblunicode/ucdata/ucdata.h
memory.lo: memory.c
memory.lo: /usr/include/stdc-predef.h
memory.lo: ../../include/portable.h
memory.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
memory.lo: ../../include/ac/fdset.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/types.h
memory.lo: /usr/include/features.h
memory.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
memory.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
memory.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
memory.lo: ../../include/ldap_cdefs.h
memory.lo: ../../include/ldap_features.h
memory.lo: ../../include/ac/assert.h
memory.lo: /usr/include/assert.h
memory.lo: ../../include/ac/localize.h
memory.lo: ../../include/ac/stdlib.h
memory.lo: /usr/include/stdlib.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/waitflags.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/floatn.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
memory.lo: /usr/include/x86_64-linux-gnu/sys/types.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
memory.lo: /usr/include/endian.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/endian.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/byteswap.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
memory.lo: /usr/include/x86_64-linux-gnu/sys/select.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/select.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
memory.lo: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
memory.lo: /usr/include/alloca.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
memory.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
memory.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
memory.lo: /usr/include/limits.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/local_lim.h
memory.lo: /usr/include/linux/limits.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
memory.lo: ../../include/ac/string.h
memory.lo: /usr/include/string.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
memory.lo: /usr/include/strings.h
memory.lo: lber-int.h
memory.lo: ../../include/lber.h
memory.lo: ../../include/lber_types.h
memory.lo: ../../include/ldap_cdefs.h
memory.lo: ../../include/ldap_log.h
memory.lo: /usr/include/stdio.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/libio.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/_G_config.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
memory.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
memory.lo: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
memory.lo: ../../include/lber_pvt.h
memory.lo: ../../include/ldap_queue.h
options.lo: options.c
options.lo: /usr/include/stdc-predef.h
options.lo: ../../include/portable.h
options.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
options.lo: ../../include/ac/fdset.h
options.lo: /usr/include/x86_64-linux-gnu/bits/types.h
options.lo: /usr/include/features.h
options.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
options.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
options.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
options.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
options.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
options.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
options.lo: ../../include/ldap_cdefs.h
options.lo: ../../include/ldap_features.h
options.lo: ../../include/ac/assert.h
options.lo: /usr/include/assert.h
options.lo: ../../include/ac/localize.h
options.lo: ../../include/ac/stdlib.h
options.lo: /usr/include/stdlib.h
options.lo: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
options.lo: /usr/include/x86_64-linux-gnu/bits/waitflags.h
options.lo: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
options.lo: /usr/include/x86_64-linux-gnu/bits/floatn.h
options.lo: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
options.lo: /usr/include/x86_64-linux-gnu/sys/types.h
options.lo: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
options.lo: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
options.lo: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
options.lo: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
options.lo: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
options.lo: /usr/include/endian.h
options.lo: /usr/include/x86_64-linux-gnu/bits/endian.h
options.lo: /usr/include/x86_64-linux-gnu/bits/byteswap.h
options.lo: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
options.lo: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
options.lo: /usr/include/x86_64-linux-gnu/sys/select.h
options.lo: /usr/include/x86_64-linux-gnu/bits/select.h
options.lo: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
options.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
options.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
options.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
options.lo: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
options.lo: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
options.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
options.lo: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
options.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
options.lo: /usr/include/alloca.h
options.lo: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
options.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
options.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
options.lo: /usr/include/limits.h
options.lo: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
options.lo: /usr/include/x86_64-linux-gnu/bits/local_lim.h
options.lo: /usr/include/linux/limits.h
options.lo: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
options.lo: ../../include/ac/string.h
options.lo: /usr/include/string.h
options.lo: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
options.lo: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
options.lo: /usr/include/strings.h
options.lo: ../../include/ac/stdarg.h
options.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
options.lo: lber-int.h
options.lo: ../../include/lber.h
options.lo: ../../include/lber_types.h
options.lo: ../../include/ldap_cdefs.h
options.lo: ../../include/ldap_log.h
options.lo: /usr/include/stdio.h
options.lo: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
options.lo: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
options.lo: /usr/include/x86_64-linux-gnu/bits/libio.h
options.lo: /usr/include/x86_64-linux-gnu/bits/_G_config.h
options.lo: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
options.lo: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
options.lo: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
options.lo: ../../include/lber_pvt.h
options.lo: ../../include/ldap_queue.h
sockbuf.lo: sockbuf.c
sockbuf.lo: /usr/include/stdc-predef.h
sockbuf.lo: ../../include/portable.h
sockbuf.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
sockbuf.lo: ../../include/ac/fdset.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/types.h
sockbuf.lo: /usr/include/features.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
sockbuf.lo: ../../include/ldap_cdefs.h
sockbuf.lo: ../../include/ldap_features.h
sockbuf.lo: ../../include/ac/assert.h
sockbuf.lo: /usr/include/assert.h
sockbuf.lo: ../../include/ac/localize.h
sockbuf.lo: /usr/include/stdio.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/libio.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/_G_config.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
sockbuf.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
sockbuf.lo: ../../include/ac/stdlib.h
sockbuf.lo: /usr/include/stdlib.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/waitflags.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/waitstatus.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/floatn.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/floatn-common.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/sys/types.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/types/clock_t.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/types/time_t.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/types/timer_t.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/stdint-intn.h
sockbuf.lo: /usr/include/endian.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/endian.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/byteswap.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/sys/select.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/select.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/sys/sysmacros.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/sysmacros.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h
sockbuf.lo: /usr/include/alloca.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/stdlib-float.h
sockbuf.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/limits.h
sockbuf.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include-fixed/syslimits.h
sockbuf.lo: /usr/include/limits.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/posix1_lim.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/local_lim.h
sockbuf.lo: /usr/include/linux/limits.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/posix2_lim.h
sockbuf.lo: ../../include/ac/ctype.h
sockbuf.lo: /usr/include/ctype.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
sockbuf.lo: ../../include/ac/errno.h
sockbuf.lo: /usr/include/errno.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/errno.h
sockbuf.lo: /usr/include/linux/errno.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/asm/errno.h
sockbuf.lo: /usr/include/asm-generic/errno.h
sockbuf.lo: /usr/include/asm-generic/errno-base.h
sockbuf.lo: ../../include/ac/socket.h
sockbuf.lo: /usr/include/poll.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/sys/poll.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/poll.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/sys/socket.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_iovec.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/socket.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/socket_type.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/sockaddr.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/asm/socket.h
sockbuf.lo: /usr/include/asm-generic/socket.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/asm/sockios.h
sockbuf.lo: /usr/include/asm-generic/sockios.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_osockaddr.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/sys/un.h
sockbuf.lo: /usr/include/string.h
sockbuf.lo: /usr/include/strings.h
sockbuf.lo: /usr/include/netinet/in.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/in.h
sockbuf.lo: /usr/include/netinet/tcp.h
sockbuf.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdint.h
sockbuf.lo: /usr/include/stdint.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/wchar.h
sockbuf.lo: /usr/include/arpa/inet.h
sockbuf.lo: /usr/include/arpa/nameser.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/sys/param.h
sockbuf.lo: /usr/include/signal.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/signum.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/signum-generic.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/types/siginfo_t.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/types/__sigval_t.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/siginfo-arch.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/siginfo-consts.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/types/sigval_t.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/types/sigevent_t.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/sigevent-consts.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/sigaction.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/sigcontext.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/types/stack_t.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/sys/ucontext.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/sigstack.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/ss_flags.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/types/struct_sigstack.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/sigthread.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/param.h
sockbuf.lo: /usr/include/linux/param.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/asm/param.h
sockbuf.lo: /usr/include/asm-generic/param.h
sockbuf.lo: /usr/include/arpa/nameser_compat.h
sockbuf.lo: /usr/include/netdb.h
sockbuf.lo: /usr/include/rpc/netdb.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/netdb.h
sockbuf.lo: /usr/include/resolv.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/types/res_state.h
sockbuf.lo: ../../include/ac/string.h
sockbuf.lo: ../../include/ac/unistd.h
sockbuf.lo: /usr/include/unistd.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/posix_opt.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/environments.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/confname.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/getopt_posix.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/getopt_core.h
sockbuf.lo: /usr/include/getopt.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/getopt_ext.h
sockbuf.lo: ../../include/lutil_lockf.h
sockbuf.lo: /usr/include/fcntl.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/fcntl.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/fcntl-linux.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/stat.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/sys/ioctl.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/ioctls.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/asm/ioctls.h
sockbuf.lo: /usr/include/asm-generic/ioctls.h
sockbuf.lo: /usr/include/linux/ioctl.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/asm/ioctl.h
sockbuf.lo: /usr/include/asm-generic/ioctl.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/bits/ioctl-types.h
sockbuf.lo: /usr/include/x86_64-linux-gnu/sys/ttydefaults.h
sockbuf.lo: lber-int.h
sockbuf.lo: ../../include/lber.h
sockbuf.lo: ../../include/lber_types.h
sockbuf.lo: ../../include/ldap_cdefs.h
sockbuf.lo: ../../include/ldap_log.h
sockbuf.lo: ../../include/lber_pvt.h
sockbuf.lo: ../../include/ldap_queue.h
stdio.lo: stdio.c
stdio.lo: /usr/include/stdc-predef.h
stdio.lo: ../../include/portable.h
stdio.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stddef.h
stdio.lo: ../../include/ac/fdset.h
stdio.lo: /usr/include/x86_64-linux-gnu/bits/types.h
stdio.lo: /usr/include/features.h
stdio.lo: /usr/include/x86_64-linux-gnu/sys/cdefs.h
stdio.lo: /usr/include/x86_64-linux-gnu/bits/wordsize.h
stdio.lo: /usr/include/x86_64-linux-gnu/bits/long-double.h
stdio.lo: /usr/include/x86_64-linux-gnu/gnu/stubs.h
stdio.lo: /usr/include/x86_64-linux-gnu/gnu/stubs-64.h
stdio.lo: /usr/include/x86_64-linux-gnu/bits/typesizes.h
stdio.lo: ../../include/ldap_cdefs.h
stdio.lo: ../../include/ldap_features.h
stdio.lo: ../../include/ac/assert.h
stdio.lo: /usr/include/assert.h
stdio.lo: ../../include/ac/localize.h
stdio.lo: /usr/include/stdio.h
stdio.lo: /usr/include/x86_64-linux-gnu/bits/libc-header-start.h
stdio.lo: /usr/include/x86_64-linux-gnu/bits/types/__FILE.h
stdio.lo: /usr/include/x86_64-linux-gnu/bits/types/FILE.h
stdio.lo: /usr/include/x86_64-linux-gnu/bits/libio.h
stdio.lo: /usr/include/x86_64-linux-gnu/bits/_G_config.h
stdio.lo: /usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h
stdio.lo: /usr/lib/gcc/x86_64-linux-gnu/7/include/stdarg.h
stdio.lo: /usr/include/x86_64-linux-gnu/bits/stdio_lim.h
stdio.lo: /usr/include/x86_64-linux-gnu/bits/sys_errlist.h
stdio.lo: ../../include/ac/stdarg.h
stdio.lo: ../../include/ac/string.h
stdio.lo: /usr/include/string.h
stdio.lo: /usr/include/x86_64-linux-gnu/bits/types/locale_t.h
stdio.lo: /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h
stdio.lo: /usr/include/strings.h
stdio.lo: ../../include/ac/ctype.h
stdio.lo: /usr/include/ctype.h
stdio.lo: /usr/include/endian.h
stdio.lo: /usr/include/x86_64-linux-gnu/bits/endian.h
stdio.lo: /usr/include/x86_64-linux-gnu/bits/byteswap.h
stdio.lo: /usr/include/x86_64-linux-gnu/bits/byteswap-16.h
stdio.lo: /usr/include/x86_64-linux-gnu/bits/uintn-identity.h
stdio.lo: ../../include/lutil.h
stdio.lo: ../../include/ldap_cdefs.h
stdio.lo: ../../include/lber_types.h

# IF YOU PUT ANYTHING HERE IT WILL GO AWAY

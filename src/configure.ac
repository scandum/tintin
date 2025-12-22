###############################################################################
#   This file is part of TinTin++                                             #
#                                                                             #
#   Copyright 2004-2019 Igor van den Hoven                                    #
#                                                                             #
#   TinTin++ is free software; you can redistribute it and/or modify          #
#   it under the terms of the GNU General Public License as published by      #
#   the Free Software Foundation; either version 3 of the License, or         #
#   (at your option) any later version.                                       #
#                                                                             #
#   This program is distributed in the hope that it will be useful,           #
#   but WITHOUT ANY WARRANTY; without even the implied warranty of            #
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                      #
#                                                                             #
#   You should have received a copy of the GNU General Public License         #
#   along with TinTin++.  If not, see https://www.gnu.org/licenses.           #
###############################################################################

AC_INIT([TinTin++], [2.02.00], [https://tintin.mudhalla.net/bugs])
AC_CONFIG_SRCDIR([tintin.h])
AC_CONFIG_HEADERS([config.h])

AC_MSG_RESULT([Configuring TinTin++])

AC_SUBST(MYINCLUDE)
AC_SUBST(MYLIB)
AC_SUBST(mybindir)
AC_SUBST(BIG5)

# Set up paths
if test "$bindir" != "\${exec_prefix}/bin"; then
    mybindir="$bindir"
else
    mybindir="."
fi

if test "$libdir" != "\${exec_prefix}/lib"; then
    MYLIB="-L$libdir"
else
    MYLIB=""
fi

if test "$includedir" != "\${prefix}/include"; then
    MYINCLUDE="-I$includedir"
else
    MYINCLUDE=""
fi

# Enable optional features
AC_ARG_ENABLE([big5],
    [AS_HELP_STRING([--enable-big5], [enable big5 support])],
    [BIG5="-DBIG5"]
)

# Checks for programs
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for header files
AC_CHECK_HEADERS([
    arpa/inet.h
    ctype.h
    fcntl.h
    net/errno.h
    netdb.h
    netinet/in.h
    param.h
    pthread.h
    socks.h
    stdlib.h
    string.h
    strings.h
    sys/ioctl.h
    sys/param.h
    sys/ptem.h
    sys/socket.h
    sys/termio.h
    sys/time.h
    time.h
    unistd.h
    util.h
    pty.h
    stropts.h
])

# Set 8-bit mode for PCRE2
CPPFLAGS="$CPPFLAGS -DPCRE2_CODE_UNIT_WIDTH=8"
AC_CHECK_HEADERS([pcre2.h],,
    [AC_MSG_ERROR([pcre2 header file not found, is the development part present?])])

# Zlib
AC_CHECK_HEADERS([zlib.h],,
    [AC_MSG_ERROR([zlib header file not found, is the development part present?])])

# GnuTLS (optional)
AC_CHECK_HEADERS([gnutls/gnutls.h],
    [AC_DEFINE([HAVE_GNUTLS_H],[1],[Define to 1 if you have <gnutls/gnutls.h>.])],
    [AC_MSG_WARN([gnutls header file not found, is the development part present?])]
)

# Typedefs, structures, compiler characteristics
AC_C_CONST
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_STRUCT_TM

# Library functions
AC_FUNC_MEMCMP
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_UTIME_NULL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([
    gethostbyname
    gethostname
    gettimeofday
    inet_ntoa
    memset
    select
    socket
    strcasecmp
    strchr
    strdup
    strerror
    strftime
    strncasecmp
    strstr
    utime
    getaddrinfo
    forkpty
    popen
])

# Libraries
AC_CHECK_LIB([z], [inflate],,
    [AC_MSG_ERROR([zlib runtime library not working, is it and the header installed?])])
AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([nsl], [gethostbyname])
AC_CHECK_LIB([socket], [rresvport])
AC_CHECK_LIB([util], [forkpty])
AC_CHECK_LIB([pcre2-8], [pcre2_compile_8],,
    [AC_MSG_ERROR([pcre2 runtime library not working, is it and the header installed?])])
#AC_CHECK_LIB([pcre2], [pcre2_compile],,
#    [AC_MSG_ERROR([pcre2 runtime library not working, is it and the header installed?])])
AC_CHECK_LIB([gnutls], [gnutls_init])
AC_CHECK_LIB([m], [sqrt])

# Programs
AC_CHECK_PROGS([MAKE], [gmake make])
AC_CHECK_FILES([/dev/ptmx])
AC_FUNC_GETMNTENT
AC_HEADER_DIRENT

# Configure output
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

echo
echo "Configure has finished. To compile, type: make"
echo


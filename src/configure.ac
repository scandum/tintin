###############################################################################
#   This file is part of TinTin++                                             #
#                                                                             #
#   Copyright 2004-2026 Igor van den Hoven                                    #
#                                                                             #
#   SPDX-License-Identifier: LGPL-2.1-or-later                                #
###############################################################################

AC_INIT([tintin], [2.02.62])

AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([tintin.h])

AC_CONFIG_HEADERS([config.h])

AC_MSG_RESULT([Configuring TinTin++])

AC_SUBST(MYINCLUDE)
AC_SUBST(MYLIB)    
AC_SUBST(mybindir)
AC_SUBST(BIG5)

if [ test $bindir != "\${exec_prefix}/bin" ]
then
	mybindir="$bindir"
else
	mybindir="."
fi

if [ test $libdir != "\${exec_prefix}/lib" ]
then
	MYLIB="-L$libdir"
else
	MYLIB=""
fi

if [ test $includedir != "\${prefix}/include" ]
then
	MYINCLUDE="-I$includedir"
else
	MYINCLUDE=""
fi

AC_ARG_ENABLE(big5, [  --enable-big5           enable big5 support], BIG5="-DBIG5")


# Checks for programs.

AC_PROG_CC
AC_PROG_MAKE_SET


# Checks for header files.

dnl	AC_CHECK_INCLUDES_DEFAULT
AC_PROG_EGREP


AC_CHECK_HEADERS([arpa/inet.h ctype.h fcntl.h net/errno.h netdb.h netinet/in.h param.h pthread.h socks.h stdlib.h string.h strings.h sys/ioctl.h sys/param.h sys/ptem.h sys/socket.h sys/termio.h sys/time.h time.h unistd.h util.h pty.h stropts.h]) 
AC_CHECK_HEADERS([zlib.h],,
	[AC_MSG_ERROR([zlib header file not found, is the development part present])])
AC_CHECK_HEADERS([gnutls/gnutls.h],
	[AC_DEFINE([HAVE_GNUTLS_H],[1],[Define to 1 if you have <gnutls/gnutls.h>.])],
	[AC_MSG_WARN([gnutls header file not found, is the develoment part present])])
AC_CHECK_HEADERS([pcre2.h],,
	[AC_MSG_ERROR([pcre2 header file not found, is the development part present])],
	[#define PCRE2_CODE_UNIT_WIDTH 8])
AC_CHECK_HEADERS([zstd.h],,
	[AC_MSG_WARN([zstd header file not found, is the develoment part present])])


# Checks for typedefs, structures, and compiler characteristics.

AC_C_CONST
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_STRUCT_TM


# Checks for library functions.

AC_FUNC_MEMCMP
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_UTIME_NULL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([gethostbyname gethostname gettimeofday inet_ntoa memset select socket strcasecmp strchr strdup strerror strftime strncasecmp strstr utime getaddrinfo forkpty popen])

# Checks libraries

AC_CHECK_LIB(z, inflate,,
	[AC_MSG_ERROR([zlib runtime library not working, is it AND the corresponding development (header) file installed])])
AC_CHECK_LIB([zstd], [ZSTD_compress])

AC_CHECK_LIB(pthread, pthread_create)
dnl	AC_CHECK_LIB(nsl, gethostbyname)
AC_CHECK_LIB(socket, rresvport)
AC_CHECK_LIB(util, forkpty)
AC_CHECK_LIB(pcre2-8, pcre2_compile_8)
dnl	AC_CHECK_LIB(pcre2-8, pcre2_compile_8,,
dnl	[AC_MSG_ERROR([pcre2 runtime library not working, is it AND the corresponding development (header) file installed])])
AC_CHECK_LIB(gnutls, gnutls_init)
AC_CHECK_LIB(m, sqrt)

AC_CHECK_PROGS(MAKE, gmake make)

AC_CHECK_FILES([/dev/ptmx])

AC_FUNC_GETMNTENT
AC_HEADER_DIRENT 

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

echo
echo "Configure has finished.  To compile, type: make"
echo

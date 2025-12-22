AC_INIT([TinTin++], [2.02.00], [https://tintin.mudhalla.net/bugs])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_PROG_CC
AC_PROG_MAKE_SET

# Check libraries
AC_CHECK_LIB([z], [inflate])
AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([pcre2-8], [pcre2_compile_8])
AC_CHECK_LIB([gnutls], [gnutls_init])
AC_CHECK_LIB([m], [sqrt])

# Check headers
AC_CHECK_HEADERS([arpa/inet.h ctype.h fcntl.h netdb.h netinet/in.h pthread.h pty.h stdlib.h string.h strings.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h time.h unistd.h zlib.h pcre2.h gnutls/gnutls.h])

# Generate Makefile
AC_CONFIG_FILES([Makefile])
AC_OUTPUT


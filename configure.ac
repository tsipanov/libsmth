# Fire up the whole enchilada
AC_INIT([libsmth], [0.0.1])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
LT_INIT

# Check headers and libs
AC_STDC_HEADERS
AC_CHECK_HEADER(expat.h, [],
	[AC_MSG_ERROR(missing required header <expat.h>)])
AC_CHECK_LIB(expat, [XML_ParserCreate], [],
	[AC_MSG_ERROR(missing required library <libexpat>)])
AC_CHECK_HEADER(curl/curl.h, [],
	[AC_MSG_ERROR(missing required header <curl.h>)])
AC_CHECK_LIB(curl, [curl_easy_init], [],
	[AC_MSG_ERROR(missing required library <libcurl>)])

# Check compiler environment
AC_PROG_CC
AM_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_LIBTOOL
#PKG_PROG_PKG_CONFIG

# Generate makefiles
AC_CONFIG_MACRO_DIR([m4])
#AC_CONFIG_SRCDIR([src/foo.c])
#AC_CONFIG_HEADERS([expat.h])
AC_CONFIG_FILES([Makefile src/Makefile src/utils/Makefile])
AC_OUTPUT

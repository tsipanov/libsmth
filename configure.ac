AC_INIT([libsmth], [0.0.1])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

LT_INIT

AC_CHECK_HEADER(expat.h, [],
	 [AC_MSG_WARN(missing required header file <expat.h>)])

AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_INSTALL

AM_PROG_LIBTOOL
AC_PROG_LIBTOOL

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_FILES([Makefile src/Makefile src/utils/Makefile])
AC_OUTPUT

#AC_CONFIG_SRCDIR([src/foo.c])
#AC_CONFIG_HEADERS([expat.h])

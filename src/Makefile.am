# Build a libtool library for installation in libdir.

lib_LTLIBRARIES    = libsmth.la

include_HEADERS    = smth.h

libsmth_la_SOURCES = smth.c  \
					 smth-dump.c  \
                     smth-http.c \
					 smth-fragment-parser.c \
					 smth-manifest-parser.c \
                     smth-dynlist.c \
					 smth-base64.c \
                     smth-error.c

noinst_HEADERS     = smth-defs.h smth-common-defs.h \
                     smth-dump.h \
                     smth-fragment-parser.h smth-fragment-defs.h \
                     smth-http.h smth-http-defs.h \
                     smth-manifest-defs.h smth-manifest-parser.h \
					 smth-dynlist.h

libsmth_la_LIBADD  = -lexpat -lcurl
libsmth_la_LDFLAGS = -version-info 0:0:0

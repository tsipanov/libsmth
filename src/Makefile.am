# Build a libtool library for installation in libdir.

lib_LTLIBRARIES    = libsmth.la
include_HEADERS    = smth.h
libsmth_la_SOURCES = smth.c smth-defs.h \
					 smth-dump.c smth-dump.h \
					 smth-fragment-parser.c smth-fragment-parser.h \
						smth-fragment-defs.h \
					 smth-http.c smth-http.h smth-http-defs.h \
					 smth-manifest-parser.c smth-manifest-parser.h \
						smth-manifest-defs.h \
					 smth-dynlist.c smth-dynlist.h \
					 smth-common-defs.h \
					 smth-base64.c \
                     smth-error.c

libsmth_la_LIBADD  = -lexpat -lcurl
libsmth_la_LDFLAGS = -version-info 0:0:0

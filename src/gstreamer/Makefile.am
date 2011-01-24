#FIXME plugin

AM_CFLAGS = `pkg-config --cflags gstreamer-0.10 glib-2.0 libxml-2.0 2> /dev/null`

lib_LTLIBRARIES       = libgstsmth.la 
libgstsmth_la_SOURCES = smth-gst.c
noinst_HEADERS        = smth-gst.h

libgstsmth_la_LIBTOOLFLAGS = --tag=disable-static

#GST_ALL_LDFLAGS="-no-undefined"
#GST_PLUGIN_LDFLAGS="-module -avoid-version -export-symbols-regex '^[_]*gst_plugin_desc\$\$' $GST_ALL_LDFLAGS"
#LIBMMS_CFLAGS = $PKG_CONFIG --cflags "libmms >= 0.4"

#libgstsmth_la_CFLAGS      = $(GST_CFLAGS) $(LIBMMS_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS)
#libgstsmth_la_LIBADD      = $(GST_LIBS) $(LIBMMS_LIBS) $(GST_PLUGINS_BASE_LIBS) $(GST_BASE_LIBS)
#libgstsmth_la_LDFLAGS     = $(GST_PLUGIN_LDFLAGS)

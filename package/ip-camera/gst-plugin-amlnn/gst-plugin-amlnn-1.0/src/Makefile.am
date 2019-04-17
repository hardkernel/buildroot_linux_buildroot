# Note: plugindir is set in configure

plugin_LTLIBRARIES = libgstamlnn.la

# sources used to compile this plug-in
libgstamlnn_la_SOURCES = gstamlnn.c \
						 frameresize.c

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstamlnn_la_CFLAGS = $(GST_CFLAGS) \
						$(DIRECTFB_CFLAGS)

libgstamlnn_la_LIBADD = $(GST_LIBS) \
						$(DIRECTFB_LIBS) \
						-lnn_detect

libgstamlnn_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstamlnn_la_LIBTOOLFLAGS = --tag=disable-static

# headers we need but don't want installed
noinst_HEADERS = gstamlnn.h

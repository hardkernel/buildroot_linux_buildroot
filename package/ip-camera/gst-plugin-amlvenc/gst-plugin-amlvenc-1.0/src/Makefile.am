# Note: plugindir is set in configure

plugin_LTLIBRARIES = libgstamlvenc.la

# sources used to compile this plug-in
libgstamlvenc_la_SOURCES = gstamlvenc.c

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstamlvenc_la_CFLAGS = $(GST_CFLAGS)
libgstamlvenc_la_LIBADD = $(GST_LIBS)
libgstamlvenc_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstamlvenc_la_LIBTOOLFLAGS = --tag=disable-static

# headers we need but don't want installed
noinst_HEADERS = gstamlvenc.h

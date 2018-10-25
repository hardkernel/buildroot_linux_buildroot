# Note: plugindir is set in configure

plugin_LTLIBRARIES = libgstamlyoloface.la

# sources used to compile this plug-in
libgstamlyoloface_la_SOURCES = gstamlyoloface.c

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstamlyoloface_la_CFLAGS = $(GST_CFLAGS)
libgstamlyoloface_la_LIBADD = $(GST_LIBS)
libgstamlyoloface_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstamlyoloface_la_LIBTOOLFLAGS = --tag=disable-static

# headers we need but don't want installed
noinst_HEADERS = gstamlyoloface.h

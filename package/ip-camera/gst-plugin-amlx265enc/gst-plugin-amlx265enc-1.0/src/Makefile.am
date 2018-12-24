# Note: plugindir is set in configure

plugin_LTLIBRARIES = libgstamlx265enc.la

# sources used to compile this plug-in
libgstamlx265enc_la_SOURCES = gstamlx265enc.c

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstamlx265enc_la_CFLAGS = $(GST_CFLAGS)
libgstamlx265enc_la_LIBADD = $(GST_LIBS)
libgstamlx265enc_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstamlx265enc_la_LIBTOOLFLAGS = --tag=disable-static

# headers we need but don't want installed
noinst_HEADERS = gstamlx265enc.h

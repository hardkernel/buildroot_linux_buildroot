# Note: plugindir is set in configure

plugin_LTLIBRARIES = libgstamlx264enc.la

# sources used to compile this plug-in
libgstamlx264enc_la_SOURCES = gstamlx264enc.c

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstamlx264enc_la_CFLAGS = $(GST_CFLAGS)
libgstamlx264enc_la_LIBADD = $(GST_LIBS)
libgstamlx264enc_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstamlx264enc_la_LIBTOOLFLAGS = --tag=disable-static

# headers we need but don't want installed
noinst_HEADERS = gstamlx264enc.h

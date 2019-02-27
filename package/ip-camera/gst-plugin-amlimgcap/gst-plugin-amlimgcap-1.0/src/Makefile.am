# Note: plugindir is set in configure

plugin_LTLIBRARIES = libgstamlimgcap.la

# sources used to compile this plug-in
libgstamlimgcap_la_SOURCES = gstamlimgcap.c
libgstamlimgcap_la_SOURCES += jpeg.c

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstamlimgcap_la_CFLAGS = $(GST_CFLAGS)
libgstamlimgcap_la_LIBADD = $(GST_LIBS) \
							-ljpeg
libgstamlimgcap_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstamlimgcap_la_LIBTOOLFLAGS = --tag=disable-static

# headers we need but don't want installed
noinst_HEADERS = gstamlimgcap.h

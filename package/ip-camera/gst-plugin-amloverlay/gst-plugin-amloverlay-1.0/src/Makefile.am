# Note: plugindir is set in configure

plugin_LTLIBRARIES = libgstamloverlay.la

# sources used to compile this plug-in
libgstamloverlay_la_SOURCES = gstamloverlay.c
libgstamloverlay_la_SOURCES += dfboverlay.c

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstamloverlay_la_CFLAGS = $(GST_CFLAGS) \
							 $(DIRECTFB_CFLAGS)
libgstamloverlay_la_LIBADD = $(GST_LIBS) \
							 $(DIRECTFB_LIBS)
libgstamloverlay_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstamloverlay_la_LIBTOOLFLAGS = --tag=disable-static

# headers we need but don't want installed
noinst_HEADERS = gstamloverlay.h

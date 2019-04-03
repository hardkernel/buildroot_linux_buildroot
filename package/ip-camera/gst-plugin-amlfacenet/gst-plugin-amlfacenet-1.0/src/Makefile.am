# Note: plugindir is set in configure

plugin_LTLIBRARIES = libgstamlfacenet.la

# sources used to compile this plug-in
libgstamlfacenet_la_SOURCES = gstamlfacenet.c \
						 framecrop.c \
						 facenetdb.c \
						 jpeg.c

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstamlfacenet_la_CFLAGS = $(GST_CFLAGS) \
						$(DIRECTFB_CFLAGS) \
						$(SQLITE3_CFLAGS)

libgstamlfacenet_la_LIBADD = $(GST_LIBS) \
						$(DIRECTFB_LIBS) \
						$(SQLITE3_LIBS) \
						-ljpeg \
						-lnn_detect

libgstamlfacenet_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstamlfacenet_la_LIBTOOLFLAGS = --tag=disable-static

# headers we need but don't want installed
noinst_HEADERS = gstamlfacenet.h

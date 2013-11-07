# Note: plugindir is set in configure

##############################################################################
# TODO: change libgstplugin.la to something else, e.g. libmysomething.la     #
##############################################################################
lib_LIBRARIES = libgstamlsysctl.a

##############################################################################
# TODO: for the next set of variables, name the prefix if you named the .la, #
#  e.g. libmysomething.la => libmysomething_la_SOURCES                       #
#                            libmysomething_la_CFLAGS                        #
#                            libmysomething_la_LIBADD                        #
#                            libmysomething_la_LDFLAGS                       #
##############################################################################

# sources used to compile this plug-in
libgstamlsysctl_a_SOURCES = amlsysctl/gstamlsysctl.c amlsysctl/gstamlsysctl.h

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstamlsysctl_a_CFLAGS = $(GST_CFLAGS)
#libgstamlsysctl_a_LIBADD = $(GST_LIBS)
#libgstamlsysctl_a_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
#libgstamlsysctl_a_LDFLAGS = -avoid-version  -no-undefined $(LDFLAGS_NOUNDEFINED)
#libgstamlsysctl_a_LIBTOOLFLAGS = --tag=disable-static
#libgstamlsysctl_include_HEADERS = amlsysctl/gstamlsysctl.h
# headers we need but don't want installed
noinst_HEADERS = amlsysctl/gstamlsysctl.h

################################################################################
#
# glmark2
#
################################################################################
GLMARK2_VERSION = 2014.03
GLMARK2_SITE = https://launchpad.net/glmark2/trunk/$(GLMARK2_VERSION)/+download/
GLMARK2_LICENSE = GPLv2+
GLMARK2_LICENSE_FILES = COPYING

GLMARK2_DEPENDENCIES = host-python host-libpng

define GLMARK2_CONFIGURE_CMDS
	(cd $(@D); \
		$(TARGET_CONFIGURE_OPTS)	\
		./waf configure			\
		--with-flavors=x11-glesv2\
		--prefix=/usr\
		--data-path=/usr/lib/share/glmark2\
       )
endef

define GLMARK2_BUILD_CMDS
       (cd $(@D); ./waf build -j $(PARALLEL_JOBS))
endef

define GLMARK2_INSTALL_TARGET_CMDS
       (cd $(@D); ./waf --destdir=$(TARGET_DIR) install)
endef

$(eval $(generic-package))

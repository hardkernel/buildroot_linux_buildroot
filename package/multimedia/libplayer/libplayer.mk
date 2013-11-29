#############################################################
#
# libplayer
#
#############################################################
LIBPLAYER_VERSION:=2.1.0
LIBPLAYER_SITE=$(TOPDIR)/package/multimedia/libplayer/src
LIBPLAYER_SITE_METHOD=local
LIBPLAYER_BUILD_DIR = $(BUILD_DIR)
LIBPLAYER_INSTALL_STAGING = YES
LIBPLAYER_DEPENDENCIES = zlib alsa-lib

export LIBPLAYER_STAGING_DIR = $(STAGING_DIR)
export LIBPLAYER_TARGET_DIR = $(TARGET_DIR)

#define LIBPLAYER_CONFIGURE_CMDS
#	$(MAKE) CC=$(TARGET_CC) -C $(@D) configure
#endef

define LIBPLAYER_BUILD_CMDS
	$(MAKE) CC=$(TARGET_CC) -C $(@D) all
	$(MAKE) CC=$(TARGET_CC) -C $(@D) install
endef

define LIBPLAYER_CLEAN_CMDS
	$(MAKE) -C $(@D) clean
endef

$(eval $(generic-package))

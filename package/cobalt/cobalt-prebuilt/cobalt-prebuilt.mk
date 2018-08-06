#############################################################
#
# Cobalt-prebuilt
#
#############################################################
ifeq ($(BR2_PACKAGE_COBALT_PREBUILT),y)

COBALT_PREBUILT_VERSION = 19.lts.1.183273

COBALT_PREBUILT_DEPENDENCIES = libxkbcommon gconf libexif libnss libdrm pulseaudio libplayer

#prebuilt defines.
COBALT_PREBUILT_SITE = $(TOPDIR)/../vendor/amlogic/cobalt
COBALT_PREBUILT_SITE_METHOD = local

ifeq ($(BR2_aarch64),y)
COBALT_PREBUILT_DIRECTORY = $(COBALT_PREBUILT_SITE)/cobalt-$(COBALT_PREBUILT_VERSION)/arm64
else
COBALT_PREBUILT_DIRECTORY = $(COBALT_PREBUILT_SITE)/cobalt-$(COBALT_PREBUILT_VERSION)/arm
endif

COBALT_INSTALL_DIR = $(TARGET_DIR)/usr/bin/cobalt

define COBALT_PREBUILT_INSTALL_TARGET_CMDS
	mkdir -p $(COBALT_INSTALL_DIR)
	cp -a $(COBALT_PREBUILT_DIRECTORY)/cobalt            $(COBALT_INSTALL_DIR)
	cp -a $(COBALT_PREBUILT_DIRECTORY)/../content        $(COBALT_INSTALL_DIR)
endef

$(eval $(generic-package))

endif

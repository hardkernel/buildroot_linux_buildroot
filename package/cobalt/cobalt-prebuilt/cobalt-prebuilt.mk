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

ifeq ($(BR2_PACKAGE_LAUNCHER_USE_COBALT), y)
define COBALT_PREBUILT_INSTALL_INIT_SYSV
	rm -rf $(TARGET_DIR)/etc/init.d/S90*
	$(INSTALL) -D -m 755 package/cobalt/S90cobalt \
		$(TARGET_DIR)/etc/init.d/S90cobalt
	cp -af package/cobalt/launcher \
		$(TARGET_DIR)/var/www/
endef
endif

$(eval $(generic-package))

endif

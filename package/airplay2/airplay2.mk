#############################################################
#
# AirPlay 2
#
#############################################################

AIRPLAY2_VERSION = master
AIRPLAY2_DEPENDENCIES = mdnsresponder
ifeq ($(BR2_PACKAGE_GPTP),y)
	AIRPLAY2_DEPENDENCIES += gptp
endif
AIRPLAY2_SITE_METHOD = git
AIRPLAY2_SITE = $(call qstrip,$(BR2_PACKAGE_AIRPLAY2_GIT_URL))

define AIRPLAY2_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CROSSPREFIX=$(TARGET_CROSS) -C $(@D)/AirPlaySDK/PlatformPOSIX
endef

define AIRPLAY2_INSTALL_TARGET_CMDS
	$(INSTALL) -m 755 -D $(@D)/AirPlaySDK/build/Release-linux/airplaydemo $(TARGET_DIR)/usr/bin/
endef

$(eval $(generic-package))


include package/airplay2/gptp/gptp.mk

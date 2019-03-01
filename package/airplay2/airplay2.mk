#############################################################
#
# AirPlay 2
#
#############################################################

AIRPLAY2_VERSION = master
AIRPLAY2_DEPENDENCIES = mdnsresponder
AIRPLAY2_PTP_OPTS := sdkptp=1
ifeq ($(BR2_PACKAGE_GPTP),y)
	AIRPLAY2_DEPENDENCIES += gptp
	AIRPLAY2_PTP_OPTS = sdkptp=0
endif
#AIRPLAY2_SITE_METHOD = local
#AIRPLAY2_SITE = $(AIRPLAY2_PKGDIR)/airplayv2
#AIRPLAY2_PTP_OPTS += debug=1
AIRPLAY2_SITE_METHOD = git
AIRPLAY2_SITE = $(call qstrip,$(BR2_PACKAGE_AIRPLAY2_GIT_URL))
define AIRPLAY2_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CROSSPREFIX=$(TARGET_CROSS) $(AIRPLAY2_PTP_OPTS) -C $(@D)/AirPlaySDK/PlatformPOSIX
endef

define AIRPLAY2_INSTALL_TARGET_CMDS
	$(INSTALL) -m 755 -D $(@D)/AirPlaySDK/build/Release-linux/airplaydemo $(TARGET_DIR)/usr/bin/
	$(INSTALL) -m 755 -D $(AIRPLAY2_PKGDIR)/S82airplay2 $(TARGET_DIR)/etc/init.d/
endef

$(eval $(generic-package))


include package/airplay2/gptp/gptp.mk

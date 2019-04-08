#############################################################
#
# AirPlay 2
#
#############################################################

AIRPLAY2_VERSION = master
AIRPLAY2_DEPENDENCIES = mdnsresponder alsa-lib fdk-aac
AIRPLAY2_MAKE_OPTS := sdkptp=1
ifeq ($(BR2_PACKAGE_GPTP),y)
	AIRPLAY2_DEPENDENCIES += gptp
	AIRPLAY2_MAKE_OPTS = sdkptp=0
endif
#AIRPLAY2_SITE_METHOD = local
#AIRPLAY2_SITE = $(AIRPLAY2_PKGDIR)/airplayv2
#AIRPLAY2_ENABLE_DEBUG := y
AIRPLAY2_BUILD_DIR := Release-linux
ifeq ($(AIRPLAY2_ENABLE_DEBUG),y)
	AIRPLAY2_MAKE_OPTS += debug=1
	AIRPLAY2_BUILD_DIR := Debug-linux
endif
AIRPLAY2_SITE_METHOD = git
AIRPLAY2_SITE = $(call qstrip,$(BR2_PACKAGE_AIRPLAY2_GIT_URL))
AIRPLAY2_MAKE_OPTS += EXT_INCLUDES="-I$(STAGING_DIR)/usr/include/alsa -I$(STAGING_DIR)/usr/include/fdk-aac" EXT_LINKFLAGS="-ldns_sd -lasound -lfdk-aac"
define AIRPLAY2_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CROSSPREFIX=$(TARGET_CROSS) $(AIRPLAY2_MAKE_OPTS) -C $(@D)/AirPlaySDK/PlatformPOSIX
	$(TARGET_MAKE_ENV) $(MAKE) CC=$(TARGET_CC) LD=$(TARGET_LD) STRIP=$(TARGET_STRIP) debug=1 platform_makefile=Platform/Platform.include.mk -C $(@D)/WAC
endef

define AIRPLAY2_INSTALL_TARGET_CMDS
	$(INSTALL) -m 755 -D $(@D)/AirPlaySDK/build/$(AIRPLAY2_BUILD_DIR)/airplaydemo $(TARGET_DIR)/usr/bin/
	$(INSTALL) -m 755 -D $(AIRPLAY2_PKGDIR)S82airplay2 $(TARGET_DIR)/etc/init.d/
	$(INSTALL) -m 755 -D $(@D)/WAC/WACServer $(TARGET_DIR)/usr/bin/
	$(INSTALL) -m 755 -D $(@D)/WAC/wac.sh $(TARGET_DIR)/usr/bin/
endef

$(eval $(generic-package))


include package/airplay2/gptp/gptp.mk

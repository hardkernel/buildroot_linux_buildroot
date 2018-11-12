#############################################################
#
# Onvif Server
#
#############################################################

ONVIF_SRVD_VERSION = 2af96b38026e603617dc3fb5b6547a0191aa586c

ONVIF_SRVD_LICENSE = BSD 3-Clause
#ONVIF_SRVD_LICENSE_FILES = COPYING
ONVIF_SRVD_DEPENDENCIES = openssl zlib

ONVIF_SRVD_SITE = $(call github,jemyzhang,onvif_srvd,$(ONVIF_SRVD_VERSION))

ONVIF_SRVD_SDK_VERSION = 2.8.65
ONVIF_SRVD_EXTRA_DOWNLOADS = https://sourceforge.net/projects/gsoap2/files/gsoap-2.8/gsoap_$(ONVIF_SRVD_SDK_VERSION).zip


define ONVIF_SRVD_COPY_SDK
	mkdir -p $(@D)/SDK
	cp -af $(DL_DIR)/gsoap_$(ONVIF_SRVD_SDK_VERSION).zip $(@D)/SDK/gsoap.zip
endef

ONVIF_SRVD_POST_EXTRACT_HOOKS += ONVIF_SRVD_COPY_SDK

ONVIF_SRVD_MAKE_OPTS = WSSE_ON=1
ONVIF_TARGET_CFLAGS = "--sysroot=$(STAGING_DIR)"
define ONVIF_SRVD_BUILD_CMDS
	$(TARGET_MAKE_ENV) GCC=$(TARGET_CXX) CFLAGS=$(ONVIF_TARGET_CFLAGS) $(MAKE) $(ONVIF_SRVD_MAKE_OPTS) -C $(@D) all
endef

ONVIF_SRVD_INSTALL_DIR = $(TARGET_DIR)/usr/bin/
ONVIF_SRVD_SCRIPTS_INSTALL_DIR = $(TARGET_DIR)/etc/init.d

define ONVIF_SRVD_INSTALL_TARGET_CMDS
	mkdir -p $(ONVIF_SRVD_INSTALL_DIR)
	cp -a $(ONVIF_SRVD_DIR)/onvif_srvd            $(ONVIF_SRVD_INSTALL_DIR)
	cp -a $(ONVIF_SRVD_DIR)/start_scripts/S90onvif_srvd           $(ONVIF_SRVD_SCRIPTS_INSTALL_DIR)/S91onvif_srvd
endef

$(eval $(generic-package))

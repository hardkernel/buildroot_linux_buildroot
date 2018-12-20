#############################################################
#
# Onvif Server
#
#############################################################

ONVIF_SRVD_VERSION = 4c61623b6d3388ae1506957e62c943c71cac8545

ONVIF_SRVD_LICENSE = BSD 3-Clause
#ONVIF_SRVD_LICENSE_FILES = COPYING
ONVIF_SRVD_DEPENDENCIES = host-openssl openssl zlib

ONVIF_SRVD_SITE = $(call github,KoynovStas,onvif_srvd,$(ONVIF_SRVD_VERSION))

ONVIF_SRVD_SDK_VERSION = 2.8.65
ONVIF_SRVD_EXTRA_DOWNLOADS = https://sourceforge.net/projects/gsoap2/files/gsoap-2.8/gsoap_$(ONVIF_SRVD_SDK_VERSION).zip


define ONVIF_SRVD_COPY_SDK
	mkdir -p $(@D)/SDK
	cp -af $(DL_DIR)/gsoap_$(ONVIF_SRVD_SDK_VERSION).zip $(@D)/SDK/gsoap.zip
endef

ONVIF_SRVD_POST_EXTRACT_HOOKS += ONVIF_SRVD_COPY_SDK

ONVIF_SRVD_MAKE_OPTS = WSSE_ON=1
ONVIF_SRVD_GSOAP_OPENSSL = $(HOST_DIR)/usr
define ONVIF_SRVD_BUILD_CMDS
	$(TARGET_MAKE_ENV) \
	  GCC=$(TARGET_CXX) CFLAGS="$(TARGET_CFLAGS)" \
	  OPENSSL=$(ONVIF_SRVD_GSOAP_OPENSSL) \
	  $(MAKE) $(ONVIF_SRVD_MAKE_OPTS) -C $(@D) all
endef

ONVIF_SRVD_INSTALL_DIR = $(TARGET_DIR)/usr/bin/
ONVIF_SRVD_SCRIPTS_INSTALL_DIR = $(TARGET_DIR)/etc/init.d

define ONVIF_SRVD_INSTALL_TARGET_CMDS
	mkdir -p $(ONVIF_SRVD_INSTALL_DIR)
	cp -a $(ONVIF_SRVD_DIR)/onvif_srvd            $(ONVIF_SRVD_INSTALL_DIR)
	cp -a $(ONVIF_SRVD_DIR)/start_scripts/S90onvif_srvd           $(ONVIF_SRVD_SCRIPTS_INSTALL_DIR)/S91onvif_srvd
endef

$(eval $(generic-package))

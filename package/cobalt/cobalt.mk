#############################################################
#
# Cobalt
#
#############################################################
ifeq ($(BR2_PACKAGE_COBALT_PREBUILT),y)
include package/cobalt/cobalt-prebuilt/cobalt-prebuilt.mk
endif

ifeq ($(BR2_PACKAGE_COBALT_COMPILE_ALL),y)

COBALT_VERSION = 19.lts.1.186281

#COBALT_LICENSE = Apache License
#COBALT_LICENSE_FILES = COPYING
COBALT_DEPENDENCIES = libxkbcommon gconf libexif libnss libdrm pulseaudio libplayer browser_toolchain_depot_tools

COBALT_SOURCE = cobalt-$(COBALT_VERSION).tar.gz
COBALT_SITE = http://openlinux.amlogic.com:8000/download/GPL_code_release/ThirdParty

ifeq ($(BR2_aarch64), y)
COBALT_DEPENDENCIES += browser_toolchain_gcc-linaro-aarch64
COBALT_REL = amlogic-wayland-arm64
COBALT_TOOLCHAIN_DIR = $(BROWSER_TOOLCHAIN_GCC_LINARO_AARCH64_INSTALL_DIR)/bin
else
COBALT_DEPENDENCIES += browser_toolchain_gcc-linaro-armeabihf
COBALT_REL = amlogic-wayland-armv7l
COBALT_TOOLCHAIN_DIR = $(BROWSER_TOOLCHAIN_GCC_LINARO_ARMEABIHF_INSTALL_DIR)/bin
endif

COBALT_MODE = qa
COBALT_OUT_DIR = $(COBALT_DIR)/src/out/$(COBALT_REL)_$(COBALT_MODE)
COBALT_DEPOT_TOOL_DIR = $(BROWSER_DEPOT_TOOL_PATH)

define COBALT_BUILD_CMDS
	touch $(COBALT_DIR)/src/third_party/__init__.py
	cp -af $(TOPDIR)/package/cobalt/starboard $(COBALT_DIR)/src/third_party
	export SYS_ROOT=$(STAGING_DIR); \
	export PATH=$(COBALT_TOOLCHAIN_DIR):$(COBALT_DEPOT_TOOL_DIR):$(PATH); \
	cd $(COBALT_DIR)/src; \
	cobalt/build/gyp_cobalt -C $(COBALT_MODE) $(COBALT_REL); \
	ninja -C $(COBALT_OUT_DIR) cobalt &&  \
	if [ -e third_party/starboard/amlogic/shared/ce_cdm/cdm/include/cdm.h ]; then ninja -C $(COBALT_OUT_DIR) widevine_cmd_cobalt; fi
endef

define COBALT_INSTALL_STAGING_CMDS
endef

COBALT_INSTALL_DIR = $(TARGET_DIR)/usr/bin/cobalt

define COBALT_INSTALL_TARGET_CMDS
	mkdir -p $(COBALT_INSTALL_DIR)
	cp -a $(COBALT_OUT_DIR)/cobalt            $(COBALT_INSTALL_DIR)
	cp -a $(COBALT_OUT_DIR)/content           $(COBALT_INSTALL_DIR)
	if [ -e $(COBALT_DIR)/src/third_party/starboard/amlogic/shared/ce_cdm/cdm/include/cdm.h ]; then \
	   cp -a $(COBALT_OUT_DIR)/lib/libwidevine_cmd_cobalt.so $(TARGET_DIR)/usr/lib; \
	fi
endef

ifeq ($(BR2_PACKAGE_LAUNCHER_USE_COBALT), y)
define COBALT_INSTALL_INIT_SYSV
	rm -rf $(TARGET_DIR)/etc/init.d/S90*
	$(INSTALL) -D -m 755 package/cobalt/S90cobalt \
		$(TARGET_DIR)/etc/init.d/S90cobalt
	cp -af package/cobalt/launcher \
		$(TARGET_DIR)/var/www/
endef
endif

$(eval $(generic-package))

endif

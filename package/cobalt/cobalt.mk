#############################################################
#
# Cobalt
#
#############################################################
ifeq ($(BR2_PACKAGE_COBALT_PREBUILT),y)
include package/cobalt/cobalt-prebuilt/cobalt-prebuilt.mk
endif

ifeq ($(BR2_PACKAGE_COBALT_COMPILE_ALL),y)

COBALT_VERSION = 19.lts.1.183273

#COBALT_LICENSE = Apache License
#COBALT_LICENSE_FILES = COPYING
COBALT_DEPENDENCIES = libxkbcommon gconf libexif libnss libdrm pulseaudio libplayer browser_toolchain_depot_tools browser_toolchain_bison

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
COBALT_BISON_TOOLS = $(BROWSER_BISON_TOOLS_PATH)
COBALT_BISON_PKGDATADIR = $(COBALT_BISON_TOOLS)/share/bison
COBALT_BISON_BIN_DIR = $(COBALT_BISON_TOOLS)/bin

define COBALT_BUILD_CMDS
	touch $(COBALT_DIR)/src/third_party/__init__.py
	cp -af $(TOPDIR)/package/cobalt/starboard $(COBALT_DIR)/src/third_party
	export BISON_TOOLS=$(COBALT_BISON_TOOLS); \
	export BISON_PKGDATADIR=$(COBALT_BISON_PKGDATADIR); \
	export SYS_ROOT=$(STAGING_DIR); \
	export PATH=$(COBALT_TOOLCHAIN_DIR):$(COBALT_DEPOT_TOOL_DIR):$(COBALT_BISON_BIN_DIR):$(PATH); \
	cd $(COBALT_DIR)/src; \
	cobalt/build/gyp_cobalt -C $(COBALT_MODE) $(COBALT_REL); \
	ninja -C $(COBALT_OUT_DIR) cobalt
endef

define COBALT_INSTALL_STAGING_CMDS
endef

COBALT_INSTALL_DIR = $(TARGET_DIR)/usr/bin/cobalt

define COBALT_INSTALL_TARGET_CMDS
	mkdir -p $(COBALT_INSTALL_DIR)
	cp -a $(COBALT_OUT_DIR)/cobalt            $(COBALT_INSTALL_DIR)
	cp -a $(COBALT_OUT_DIR)/content           $(COBALT_INSTALL_DIR)
endef

$(eval $(generic-package))

endif

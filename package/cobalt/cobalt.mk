#############################################################
#
# Cobalt
#
#############################################################
COBALT_VERSION = 19.lts.5.205289

ifeq ($(BR2_aarch64), y)
COBALT_TOOLCHAIN_DEPENDENCIES = browser_toolchain_gcc-linaro-aarch64
COBALT_TOOLCHAIN_DIR = $(BROWSER_TOOLCHAIN_GCC_LINARO_AARCH64_INSTALL_DIR)/bin
COBALT_ARCH=arm64
COBALT_CROSS=aarch64-linux-gnu-
else
COBALT_TOOLCHAIN_DEPENDENCIES = browser_toolchain_gcc-linaro-armeabihf
COBALT_TOOLCHAIN_DIR = $(BROWSER_TOOLCHAIN_GCC_LINARO_ARMEABIHF_INSTALL_DIR)/bin
COBALT_ARCH=arm
COBALT_CROSS=arm-linux-gnueabihf-
endif

COBALT_INSTALL_DIR = $(TARGET_DIR)/usr/bin/cobalt
COBALT_PREBUILT_SITE = $(TOPDIR)/../vendor/amlogic/cobalt
COBALT_PREBUILT_DIRECTORY=$(COBALT_PREBUILT_SITE)/cobalt-$(COBALT_VERSION)/$(COBALT_ARCH)

ifeq ($(BR2_PACKAGE_COBALT_PREBUILT),y)
COBALT_DEPENDENCIES = libxkbcommon gconf libexif libnss libdrm pulseaudio libplayer
COBALT_SITE = $(COBALT_PREBUILT_SITE)
COBALT_SITE_METHOD = local
define COBALT_INSTALL_TARGET_CMDS
	mkdir -p $(COBALT_INSTALL_DIR)
	cp -a $(COBALT_PREBUILT_DIRECTORY)/cobalt            $(COBALT_INSTALL_DIR)
	cp -a $(COBALT_PREBUILT_DIRECTORY)/../content        $(COBALT_INSTALL_DIR)
endef
endif

ifeq ($(BR2_PACKAGE_COBALT_COMPILE_ALL), y)
COBALT_DEPENDENCIES = libxkbcommon gconf libexif libnss libdrm pulseaudio libplayer browser_toolchain_depot_tools $(COBALT_TOOLCHAIN_DEPENDENCIES)
COBALT_SOURCE = cobalt-$(COBALT_VERSION).tar.gz
COBALT_SITE = http://openlinux.amlogic.com:8000/download/GPL_code_release/ThirdParty
COBALT_STRIP_COMPONENTS=0

ifeq ($(BR2_PACKAGE_COBALT_GLES), y)
COBALT_REL = amlogic-wayland
else
COBALT_REL = amlogic-directfb
COBALT_DEPENDENCIES += directfb
endif
COBALT_MODE = qa
COBALT_OUT_DIR = $(COBALT_DIR)/src/out/$(COBALT_REL)_$(COBALT_MODE)
COBALT_DEPOT_TOOL_DIR = $(BROWSER_DEPOT_TOOL_PATH)

ifeq ($(BR2_PACKAGE_COBALT_WIDEVINE), y)
	COBALT_DEPENDENCIES += widevine-bin
	COBALT_ENABLE_WIDEVINE_CE_CDM="export WIDEVINE_CE_CDM_INC=$(STAGING_DIR)/usr/include/widevine/"
else
	COBALT_ENABLE_WIDEVINE_CE_CDM="echo compile without widevine support"
endif

ifeq ($(BR2_PACKAGE_COBALT_UPDATE_PREBUILD), y)
	COBALT_UPDATE_PREBUILD_CMDS = $(TARGET_STRIP) -s $(COBALT_OUT_DIR)/cobalt -o $(COBALT_PREBUILT_DIRECTORY)/cobalt;
	COBALT_UPDATE_PREBUILD_CMDS += cp -afT $(COBALT_OUT_DIR)/content $(COBALT_PREBUILT_DIRECTORY)/../content;
endif

COBALT_CFLAGS="$(TOOLCHAIN_EXTERNAL_CFLAGS)"
#COBALT_CFLAGS="-mcpu=cortex-a9 -mabi=aapcs-linux -mfloat-abi=softfp -marm  -msoft-float "

define COBALT_BUILD_CMDS
	touch $(COBALT_DIR)/src/third_party/__init__.py
	rsync -a $(TOPDIR)/package/cobalt/starboard $(COBALT_DIR)/src/third_party
	$(call qstrip, $(COBALT_ENABLE_WIDEVINE_CE_CDM)); \
	export SYS_ROOT=$(STAGING_DIR); \
	export COBALT_CFLAGS=$(COBALT_CFLAGS); \
	export COBALT_ARCH=$(COBALT_ARCH); \
	export COBALT_CROSS=$(COBALT_CROSS); \
	export PATH=$(COBALT_TOOLCHAIN_DIR):$(COBALT_DEPOT_TOOL_DIR):$(PATH); \
	cd $(COBALT_DIR)/src; \
	cobalt/build/gyp_cobalt -C $(COBALT_MODE) $(COBALT_REL); \
	ninja -C $(COBALT_OUT_DIR) cobalt
endef

define COBALT_INSTALL_TARGET_CMDS
	mkdir -p $(COBALT_INSTALL_DIR)
	cp -a $(COBALT_OUT_DIR)/cobalt            $(COBALT_INSTALL_DIR)
	cp -a $(COBALT_OUT_DIR)/content           $(COBALT_INSTALL_DIR)
	$(COBALT_UPDATE_PREBUILD_CMDS)
endef

endif # BR2_PACKAGE_COBALT_COMPILE_ALL

ifeq ($(BR2_PACKAGE_LAUNCHER_USE_COBALT), y)
define COBALT_INSTALL_INIT_SYSV
	$(INSTALL) -D -m 755 package/cobalt/S90cobalt $(TARGET_DIR)/etc/init.d/S90cobalt
	mkdir -p $(TARGET_DIR)/var/www/
	cp -af package/cobalt/launcher $(TARGET_DIR)/var/www/
endef
endif # BR2_PACKAGE_LAUNCHER_USE_COBALT

$(eval $(generic-package))

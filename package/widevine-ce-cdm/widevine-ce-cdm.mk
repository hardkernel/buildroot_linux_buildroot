#############################################################
#
# widevine-ce-cdm
#
#############################################################
WIDEVINE_CE_CDM_VERSION = master
WIDEVINE_CE_CDM_DEPENDENCIES = widevine-bin browser_toolchain_depot_tools

WIDEVINE_CE_CDM_LIB_NAME = $(call qstrip, libwidevine_ce_cdm_shared.so.$(BR2_ARCH).$(CC_TARGET_ABI_).$(CC_TARGET_FLOAT_ABI_))
ifeq ($(BR2_PACKAGE_WIDEVINE_CE_CDM_PREBUILD),y)
WIDEVINE_CE_CDM_SITE = $(TOPDIR)/package/widevine-ce-cdm/prebuild
WIDEVINE_CE_CDM_SITE_METHOD = local
WIDEVINE_CE_CDM_INSTALL_STAGING = YES
define WIDEVINE_CE_CDM_INSTALL_STAGING_CMDS
	$(INSTALL) -DT $(@D)/$(WIDEVINE_CE_CDM_LIB_NAME) $(STAGING_DIR)/usr/lib/libwidevine_ce_cdm_shared.so
	$(INSTALL) -Dt $(STAGING_DIR)/usr/include/widevine $(@D)/include/cdm.h $(@D)/include/OEMCryptoCENC.h
endef

define WIDEVINE_CE_CDM_INSTALL_TARGET_CMDS
	$(INSTALL) -DT $(@D)/$(WIDEVINE_CE_CDM_LIB_NAME) $(TARGET_DIR)/usr/lib/libwidevine_ce_cdm_shared.so
endef
else #BR2_PACKAGE_WIDEVINE_CE_CDM_PREBUILD
WIDEVINE_CE_CDM_SITE = $(call qstrip,$(BR2_PACKAGE_WIDEVINE_CE_CDM_REPO))
WIDEVINE_CE_CDM_SITE_METHOD = git

WIDEVINE_CE_CDM_INSTALL_STAGING = YES

WIDEVINE_CE_CDM_ENVS = PATH=$(BROWSER_DEPOT_TOOL_PATH):$(PATH)
WIDEVINE_CE_CDM_ENVS += WV_CE_CDM_CROSS=$(TARGET_CROSS)
WIDEVINE_CE_CDM_ENVS += WV_CE_CDM_CFLAGS="$(TOOLCHAIN_EXTERNAL_CFLAGS)"
#WIDEVINE_CE_CDM_ENVS += WV_CE_CDM_CFLAGS="-mcpu=cortex-a9 -mabi=aapcs-linux -mfloat-abi=softfp -marm  -msoft-float "

ifeq ($(BR2_PACKAGE_WIDEVINE_CE_CDM_UPDATE_PREBUILD),y)
	WIDEVINE_CE_CDM_UPDATE_PREBUILD_CMDS = $(INSTALL) -m 0644 -Dt $(TOPDIR)/package/widevine-ce-cdm/prebuild/include $(@D)/cdm/include/cdm.h $(@D)/oemcrypto/include/OEMCryptoCENC.h;
	WIDEVINE_CE_CDM_UPDATE_PREBUILD_CMDS += $(TARGET_STRIP) -s $(@D)/out/amlogic/Release/lib/libwidevine_ce_cdm_shared.so -o $(TOPDIR)/package/widevine-ce-cdm/prebuild/$(WIDEVINE_CE_CDM_LIB_NAME);
endif

define WIDEVINE_CE_CDM_APPLY_PATCHES
	$(APPLY_PATCHES) $(@D) $(TOPDIR)/package/widevine-ce-cdm/patches \*.patch;
endef

WIDEVINE_CE_CDM_POST_PATCH_HOOKS = WIDEVINE_CE_CDM_APPLY_PATCHES

define WIDEVINE_CE_CDM_BUILD_CMDS
	cd $(WIDEVINE_CE_CDM_DIR) && \
	$(WIDEVINE_CE_CDM_ENVS) ./build.py amlogic -r -j32
endef

define WIDEVINE_CE_CDM_INSTALL_STAGING_CMDS
	$(INSTALL) -Dt $(STAGING_DIR)/usr/include/widevine $(@D)/cdm/include/cdm.h $(@D)/oemcrypto/include/OEMCryptoCENC.h
	$(INSTALL) -Dt $(STAGING_DIR)/usr/lib $(@D)/out/amlogic/Release/lib/libwidevine_ce_cdm_shared.so
endef

define WIDEVINE_CE_CDM_INSTALL_TARGET_CMDS
	$(INSTALL) -Dt $(TARGET_DIR)/usr/lib $(@D)/out/amlogic/Release/lib/libwidevine_ce_cdm_shared.so
	$(WIDEVINE_CE_CDM_UPDATE_PREBUILD_CMDS)
endef
endif #BR2_PACKAGE_WIDEVINE_CE_CDM_PREBUILD

$(eval $(generic-package))

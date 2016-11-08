#############################################################
#
# TDK driver
#
#############################################################
TDK_VERSION = $(call qstrip,$(BR2_PACKAGE_TDK_GIT_VERSION))
TDK_SITE = $(call qstrip,$(BR2_PACKAGE_TDK_GIT_REPO_URL))
TDK_SITE_METHOD = git
TDK_INSTALL_STAGING = YES
TDK_DEPENDENCIES = linux

define TDK_BUILD_CMDS
	export KERNEL_SRC_DIR=$(LINUX_DIR); \
	export KERNEL_OUT_DIR=$(LINUX_DIR); \
	export KERNEL_CONFIG=$(BR2_LINUX_KERNEL_DEFCONFIG)_defconfig; \
	if [ "$(TARGET_KERNEL_CROSS)" != "" ]; then \
		export CROSS_COMPILE=$(TARGET_KERNEL_CROSS); \
	else \
		export CROSS_COMPILE=$(TARGET_CROSS); \
	fi; \
	$(TARGET_CONFIGURE_OPTS) $(MAKE1) -C $(@D) driver
	$(TARGET_CONFIGURE_OPTS) $(MAKE1) -C $(@D)/demos/hello_world
endef

define TDK_INSTALL_STAGING_CMDS
	$(INSTALL) -D -m 0644 $(@D)/ca_export_arm/include/*.h $(STAGING_DIR)/usr/include
	$(INSTALL) -D -m 0644 $(@D)/ca_export_arm/lib/* $(STAGING_DIR)/usr/lib/
endef

define TDK_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0644 $(@D)/ca_export_arm/lib/*.so $(TARGET_DIR)/usr/lib/
	$(INSTALL) -D -m 0644 $(@D)/ca_export_arm/lib/*.so.* $(TARGET_DIR)/usr/lib/
	$(INSTALL) -D -m 0755 $(@D)/ca_export_arm/bin/tee-supplicant $(TARGET_DIR)/usr/bin/

	$(INSTALL) -D -m 0755 $(@D)/demos/hello_world/ca/tee_helloworld_32 $(TARGET_DIR)/usr/bin/
	mkdir -p $(TARGET_DIR)/lib/teetz/
	$(INSTALL) -D -m 0644 $(@D)/demos/hello_world/ta/8aaaf200-2450-11e4-abe20002a5d5c51b.ta $(TARGET_DIR)/lib/teetz/

	mkdir -p $(TARGET_DIR)/lib/modules/$(LINUX_VERSION_PROBED)/kernel/drivers/trustzone/
	$(INSTALL) -D -m 0644 $(@D)/linuxdriver/core/*.ko $(TARGET_DIR)/lib/modules/$(LINUX_VERSION_PROBED)/kernel/drivers/trustzone/
	$(INSTALL) -D -m 0644 $(@D)/linuxdriver/armtz/*.ko $(TARGET_DIR)/lib/modules/$(LINUX_VERSION_PROBED)/kernel/drivers/trustzone/
	echo kernel/drivers/trustzone/optee_armtz.ko: kernel/drivers/trustzone/optee.ko >> $(TARGET_DIR)/lib/modules/$(LINUX_VERSION_PROBED)/modules.dep
	echo kernel/drivers/trustzone/optee.ko: >> $(TARGET_DIR)/lib/modules/$(LINUX_VERSION_PROBED)/modules.dep
endef

#define TDK_CLEAN_CMDS
	#echo clean
#endef

$(eval $(generic-package))

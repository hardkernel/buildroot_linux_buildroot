#############################################################
#
# TDK driver
#
#############################################################
TDK_VERSION = $(call qstrip,$(BR2_PACKAGE_TDK_GIT_VERSION))
ifneq ($(BR2_PACKAGE_TDK_GIT_REPO_URL),"")
TDK_SITE = $(call qstrip,$(BR2_PACKAGE_TDK_GIT_REPO_URL))
TDK_SITE_METHOD = git
else
TDK_SITE = $(TOPDIR)/../tdk
TDK_SITE_METHOD = local
endif
TDK_INSTALL_STAGING = YES
TDK_DEPENDENCIES = linux

ifeq ($(BR2_aarch64), y)
_ARCH = arm64
_CROSS_COMPILE = aarch64-linux-gnu-
else
_ARCH = arm
_CROSS_COMPILE = arm-linux-gnueabihf-
endif

define XTEST
	$(TARGET_CONFIGURE_OPTS) $(MAKE1) ARCH=$(_ARCH) CROSS_COMPILE=$(_CROSS_COMPILE) \
					-C $(@D)/demos/optee_test
endef


define TDK_BUILD_CMDS
	$(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(LINUX_DIR) M=$(@D)/linuxdriver ARCH=$(KERNEL_ARCH) \
		CROSS_COMPILE=$(TARGET_KERNEL_CROSS) modules
	$(XTEST)
	$(TARGET_CONFIGURE_OPTS) $(MAKE1) ARCH=$(_ARCH) CROSS_COMPILE=$(_CROSS_COMPILE) -C $(@D)/demos/hello_world
endef

ifeq ($(BR2_aarch64), y)
define TDK_INSTALL_STAGING_CMDS
	$(INSTALL) -D -m 0644 $(@D)/ca_export_arm64/include/*.h $(STAGING_DIR)/usr/include
	$(INSTALL) -D -m 0644 $(@D)/ca_export_arm64/lib/* $(STAGING_DIR)/usr/lib/
endef
define TDK_INSTALL_LIBS
	$(INSTALL) -D -m 0644 $(@D)/ca_export_arm64/lib/*.so $(TARGET_DIR)/usr/lib/
	$(INSTALL) -D -m 0644 $(@D)/ca_export_arm64/lib/*.so.* $(TARGET_DIR)/usr/lib/
	$(INSTALL) -D -m 0755 $(@D)/ca_export_arm64/bin/tee-supplicant $(TARGET_DIR)/usr/bin/
endef
else
define TDK_INSTALL_STAGING_CMDS
	$(INSTALL) -D -m 0644 $(@D)/ca_export_arm/include/*.h $(STAGING_DIR)/usr/include
	$(INSTALL) -D -m 0644 $(@D)/ca_export_arm/lib/* $(STAGING_DIR)/usr/lib/
endef
define TDK_INSTALL_LIBS
	$(INSTALL) -D -m 0644 $(@D)/ca_export_arm/lib/*.so $(TARGET_DIR)/usr/lib/
	$(INSTALL) -D -m 0644 $(@D)/ca_export_arm/lib/*.so.* $(TARGET_DIR)/usr/lib/
	$(INSTALL) -D -m 0755 $(@D)/ca_export_arm/bin/tee-supplicant $(TARGET_DIR)/usr/bin/
endef
endif
define TDK_INSTALL_TARGET_CMDS
	$(TDK_INSTALL_LIBS)
	cd $(@D); find . -name *_32 | xargs -i $(INSTALL) -m 0755 {} $(TARGET_DIR)/usr/bin
	cd $(@D); find . -name *_64 | xargs -i $(INSTALL) -m 0755 {} $(TARGET_DIR)/usr/bin

	mkdir -p $(TARGET_DIR)/lib/teetz/
	cd $(@D); find . -name *.ta | xargs -i $(INSTALL) -m 0644 {} $(TARGET_DIR)/lib/teetz

	mkdir -p $(TARGET_DIR)/lib/modules/$(LINUX_VERSION_PROBED)/kernel/drivers/trustzone/
	$(INSTALL) -D -m 0644 $(@D)/linuxdriver/optee/optee_armtz.ko $(TARGET_DIR)/lib/modules/$(LINUX_VERSION_PROBED)/kernel/drivers/trustzone/
	$(INSTALL) -D -m 0644 $(@D)/linuxdriver/optee.ko $(TARGET_DIR)/lib/modules/$(LINUX_VERSION_PROBED)/kernel/drivers/trustzone/
	echo kernel/drivers/trustzone/optee_armtz.ko: kernel/drivers/trustzone/optee.ko >> $(TARGET_DIR)/lib/modules/$(LINUX_VERSION_PROBED)/modules.dep
	echo kernel/drivers/trustzone/optee.ko: >> $(TARGET_DIR)/lib/modules/$(LINUX_VERSION_PROBED)/modules.dep

	$(INSTALL) -D -m 0755 $(@D)/demos/hello_world/out/ca/tee_helloworld $(TARGET_DIR)/usr/bin
	$(INSTALL) -D -m 0755 $(@D)/demos/optee_test/out/xtest/tee_xtest $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))

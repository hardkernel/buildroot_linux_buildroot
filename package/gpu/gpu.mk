################################################################################
#
# amlogic gpu driver
#
################################################################################
GPU_VERSION = $(call qstrip,$(BR2_PACKAGE_GPU_VERSION))
ifneq ($(BR2_PACKAGE_GPU_CUSTOM_TARBALL_LOCATION),"")
GPU_TARBALL = $(call qstrip,$(BR2_PACKAGE_GPU_CUSTOM_TARBALL_LOCATION))
GPU_SITE    = $(patsubst %/,%,$(dir $(GPU_TARBALL)))
GPU_SOURCE  = $(notdir $(GPU_TARBALL))
else
GPU_SITE = $(call qstrip,$(BR2_PACKAGE_GPU_GIT_URL))
endif
GPU_MODULE_DIR = kernel/amlogic/gpu
GPU_INSTALL_DIR = $(TARGET_DIR)/lib/modules/$(LINUX_VERSION_PROBED)/$(GPU_MODULE_DIR)

ifeq ($(BR2_PACKAGE_GPU_STANDALONE),y)
define GPU_BUILD_CMDS
	$(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(LINUX_DIR) M=$(@D)/mali ARCH=$(KERNEL_ARCH) \
		CROSS_COMPILE=$(TARGET_CROSS) CONFIG_MALI400=m CONFIG_MALI450=m modules
endef
define GPU_INSTALL_TARGET_CMDS
	mkdir -p $(GPU_INSTALL_DIR)
	$(INSTALL) -m 0666 $(@D)/mali/mali.ko $(GPU_INSTALL_DIR)
	echo $(GPU_MODULE_DIR)/mali.ko: >> $(TARGET_DIR)/lib/modules/$(LINUX_VERSION_PROBED)/modules.dep
endef
endif
$(eval $(generic-package))


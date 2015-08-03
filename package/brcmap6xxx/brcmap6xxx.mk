################################################################################
#
# amlogic broadcom AP6xxx driver
#
################################################################################

BRCMAP6XXX_VERSION = $(call qstrip,$(BR2_PACKAGE_BRCMAP6XXX_GIT_VERSION))
BRCMAP6XXX_SITE = $(call qstrip,$(BR2_PACKAGE_BRCMAP6XXX_GIT_REPO_URL))
BRCMAP6XXX_MODULE_DIR = kernel/broadcom/wifi
BRCMAP6XXX_INSTALL_DIR = $(TARGET_DIR)/lib/modules/$(LINUX_VERSION_PROBED)/$(BRCMAP6XXX_MODULE_DIR)

ifeq ($(BR2_PACKAGE_BRCMAP6XXX_STANDALONE),y)
define BRCMAP6XXX_BUILD_CMDS
	$(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(LINUX_DIR) M=$(@D)/bcmdhd_1_201_59_x ARCH=$(KERNEL_ARCH) \
		CROSS_COMPILE=$(TARGET_CROSS) modules
endef
define BRCMAP6XXX_INSTALL_TARGET_CMDS
	mkdir -p $(BRCMAP6XXX_INSTALL_DIR)
	$(INSTALL) -m 0666 $(@D)/bcmdhd_1_201_59_x/dhd.ko $(BRCMAP6XXX_INSTALL_DIR)
	echo $(BRCMAP6XXX_MODULE_DIR)/dhd.ko: >> $(TARGET_DIR)/lib/modules/$(LINUX_VERSION_PROBED)/modules.dep
endef
endif


$(eval $(generic-package))

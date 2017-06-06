##############################################################################
#
# amlogic gpu driver
#
##############################################################################
GPU_VERSION = $(call qstrip,$(BR2_PACKAGE_GPU_VERSION))
ifneq ($(BR2_PACKAGE_GPU_CUSTOM_TARBALL_LOCATION),"")
GPU_TARBALL = $(call qstrip,$(BR2_PACKAGE_GPU_CUSTOM_TARBALL_LOCATION))
GPU_SITE    = $(patsubst %/,%,$(dir $(GPU_TARBALL)))
GPU_SOURCE  = $(notdir $(GPU_TARBALL))
else ifeq ($(BR2_PACKAGE_GPU_LOCAL),y)
GPU_SITE_METHOD = local
GPU_SITE    = $(call qstrip,$(BR2_PACKAGE_GPU_LOCAL_PATH))
else
GPU_SITE = $(call qstrip,$(BR2_PACKAGE_GPU_GIT_URL))
GPU_SITE_METHOD = git
endif

GPU_MODULE_DIR = kernel/amlogic/gpu
GPU_INSTALL_DIR = $(TARGET_DIR)/lib/modules/$(LINUX_VERSION_PROBED)/$(GPU_MODULE_DIR)
GPU_DEPENDENCIES = linux

MALI_BUILD_CMD = \
	if [ -e $(@D)/utgard/Makefile ]; then \
		cd $(@D)/utgard ;\
		$(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D)/utgard KDIR=$(LINUX_DIR) \
		ARCH=$(KERNEL_ARCH) CROSS_COMPILE=$(TARGET_KERNEL_CROSS) \
		GPU_DRV_VERSION=$(GPU_VERSION); \
	else \
		$(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(LINUX_DIR) M=$(@D)/utgard/$(GPU_VERSION) ARCH=$(KERNEL_ARCH) \
               CROSS_COMPILE=$(TARGET_KERNEL_CROSS) CONFIG_MALI400=m CONFIG_MALI450=m \
                EXTRA_CFLAGS="-DCONFIG_MALI400=m -DCONFIG_MALI450=m" modules; \
	fi;

MALI_INSTALL_TARGETS_CMDS = \
	$(INSTALL) -m 0666 $(@D)/utgard/$(GPU_VERSION)/mali.ko $(GPU_INSTALL_DIR); \
	echo $(GPU_MODULE_DIR)/mali.ko: >> $(TARGET_DIR)/lib/modules/$(LINUX_VERSION_PROBED)/modules.dep;

define GPU_BUILD_CMDS
	$(MALI_BUILD_CMD)
endef
define GPU_INSTALL_TARGET_CMDS
	mkdir -p $(GPU_INSTALL_DIR);
	$(MALI_INSTALL_TARGETS_CMDS)
endef
$(eval $(generic-package))

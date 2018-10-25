################################################################################
#
# libyoloface
#
################################################################################

LIBYOLOFACE_VERSION = 1.0
LIBYOLOFACE_SITE = $(TOPDIR)/package/ip-camera/libyoloface/libyoloface-$(LIBYOLOFACE_VERSION)
LIBYOLOFACE_SITE_METHOD = local
LIBYOLOFACE_DEPENDENCIES = npu directfb
LIBYOLOFACE_LICENSE = LGPL

LIBYOLOFACE_AQROOT=$(LIBYOLOFACE_DIR)/../npu
LIBYOLOFACE_AQARCH=$(AQROOT)/arch/XAQ2
LIBYOLOFACE_SDK_DIR=$(AQROOT)/build/sdk
LIBYOLOFACE_DFB_DIR=$(STAGING_DIR)/usr/include/directfb

ifeq ($(BR2_aarch64), y)
LIBYOLOFACE_ARCH_TYPE=arm64
LIBYOLOFACE_CPU_TYPE=cortex-a53
LIBYOLOFACE_CPU_ARCH=armv8-a
LIBYOLOFACE_FIXED_ARCH_TYPE=arm64
else
LIBYOLOFACE_ARCH_TYPE=arm
LIBYOLOFACE_CPU_TYPE=arm920
LIBYOLOFACE_FIXED_ARCH_TYPE=arm
endif

# This package uses the AML_LIBS_STAGING_DIR variable to construct the
# header and library paths used when compiling
define LIBYOLOFACE_BUILD_CMDS
    ARCH_TYPE=$(LIBYOLOFACE_ARCH_TYPE) \
    CPU_TYPE=$(LIBYOLOFACE_CPU_TYPE) \
	CPU_ARCH=$(LIBYOLOFACE_CPU_ARCH) \
	FIXED_ARCH_TYPE=$(LIBYOLOFACE_FIXED_ARCH_TYPE) \
	CROSS_COMPILE=$(TARGET_CROSS) \
	AQROOT=$(LIBYOLOFACE_AQROOT) \
	SDK_DIR=$(LIBYOLOFACE_SDK_DIR) \
	DFB_DIR=$(LIBYOLOFACE_DFB_DIR) \
	OBJ_DIR=$(LIBYOLOFACE_DIR) \
	$(TARGET_MAKE_ENV) $(MAKE) \
		-C $(@D)/$(d)
endef

define LIBYOLOFACE_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/libyoloface.so $(TARGET_DIR)/usr/lib/libyoloface.so
	$(INSTALL) -D -m 0644 $(LIBYOLOFACE_DIR)/dynamic_fixed_point-8.export.data   $(TARGET_DIR)/etc/yoloface_model.dat
endef

$(eval $(generic-package))

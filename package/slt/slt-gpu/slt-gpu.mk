#
# SLT GPU
#
SLT_GPU_VERSION = 1.0
SLT_GPU_SITE = $(TOPDIR)/../vendor/amlogic/slt/gpu_slt_test/slt
SLT_GPU_SITE_METHOD = local
SLT_GPU_DEPENDENCIES = gpu

CROSS_COMPILE = $(TARGET_CROSS)
INC = $(STAGING_DIR)/usr/include
LIB_PATH = $(STAGING_DIR)/usr/lib

ifeq ($(BR2_aarch64),y)
FILL_BUFFER = fill_buffer/64bit/gpu.fill_buffer
FLATLAND = flatland/64bit/flatland
else ifeq ($(BR2_ARM_EABIHF),y)
FILL_BUFFER = fill_buffer/32bit/gpu.fill_buffer
FLATLAND = flatland/32bit/flatland
endif

define SLT_GPU_BUILD_CMDS
    $(MAKE) CC=$(TARGET_CXX) -C $(@D)
endef

define SLT_GPU_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0755 $(@D)/check_err/gpu.check_err $(TARGET_DIR)/usr/bin/
    $(INSTALL) -D -m 0755 $(@D)/opengles2_basic/gpu.gl2_basic $(TARGET_DIR)/usr/bin/
    $(INSTALL) -D -m 0755 $(@D)/$(FILL_BUFFER) $(TARGET_DIR)/usr/bin/
    $(INSTALL) -D -m 0755 $(@D)/$(FLATLAND) $(TARGET_DIR)/usr/bin/
endef

define SLT_GPU_INSTALL_CLEAN_CMDS
    $(MAKE) CC=$(TARGET_CXX) -C $(@D) clean
endef

$(eval $(generic-package))

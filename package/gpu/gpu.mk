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
$(eval $(generic-package))

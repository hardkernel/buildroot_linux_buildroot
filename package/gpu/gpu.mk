################################################################################
#
# amlogic gpu driver
#
################################################################################

GPU_VERSION = $(call qstrip,$(BR2_PACKAGE_GPU_VERSION))
GPU_SITE = $(call qstrip,$(BR2_PACKAGE_GPU_GIT_URL))
$(eval $(generic-package))

################################################################################
#
# amlogic gpu driver
#
################################################################################

GPU_VERSION = $(call qstrip,$(BR2_PACKAGE_GPU_VERSION))
GPU_SITE = $(call qstrip,$(BR2_PACKAGE_GPU_GIT_URL))
GPU_DEPENDENCIES = linux
$(eval $(generic-package))

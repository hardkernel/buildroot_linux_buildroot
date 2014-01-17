################################################################################
#
# amlogic 8192cu driver
#
################################################################################

RTK8192CU_VERSION = $(call qstrip,$(BR2_PACKAGE_RTK8192CU_GIT_VERSION))
RTK8192CU_SITE = $(call qstrip,$(BR2_PACKAGE_RTK8192CU_GIT_REPO_URL))
$(eval $(generic-package))

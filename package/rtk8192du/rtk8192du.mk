################################################################################
#
# amlogic 8192du driver
#
################################################################################

RTK8192DU_VERSION = $(call qstrip,$(BR2_PACKAGE_RTK8192DU_GIT_VERSION))
RTK8192DU_SITE = $(call qstrip,$(BR2_PACKAGE_RTK8192DU_GIT_REPO_URL))
$(eval $(generic-package))

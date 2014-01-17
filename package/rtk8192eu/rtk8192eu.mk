################################################################################
#
# amlogic 8192eu driver
#
################################################################################

RTK8192EU_VERSION = $(call qstrip,$(BR2_PACKAGE_RTK8192EU_GIT_VERSION))
RTK8192EU_SITE = $(call qstrip,$(BR2_PACKAGE_RTK8192EU_GIT_REPO_URL))
$(eval $(generic-package))

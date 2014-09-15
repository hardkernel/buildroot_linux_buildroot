################################################################################
#
# amlogic 8812au driver
#
################################################################################

RTK8812AU_VERSION = $(call qstrip,$(BR2_PACKAGE_RTK8812AU_GIT_VERSION))
RTK8812AU_SITE = $(call qstrip,$(BR2_PACKAGE_RTK8812AU_GIT_REPO_URL))
$(eval $(generic-package))

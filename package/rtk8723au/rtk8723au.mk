################################################################################
#
# amlogic 8723au driver
#
################################################################################

RTK8723AU_VERSION = $(call qstrip,$(BR2_PACKAGE_RTK8723AU_GIT_VERSION))
RTK8723AU_SITE = $(call qstrip,$(BR2_PACKAGE_RTK8723AU_GIT_REPO_URL))
$(eval $(generic-package))

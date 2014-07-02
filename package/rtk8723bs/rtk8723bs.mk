################################################################################
#
# amlogic 8723bs driver
#
################################################################################

RTK8723BS_VERSION = $(call qstrip,$(BR2_PACKAGE_RTK8723BS_GIT_VERSION))
RTK8723BS_SITE = $(call qstrip,$(BR2_PACKAGE_RTK8723BS_GIT_REPO_URL))
$(eval $(generic-package))

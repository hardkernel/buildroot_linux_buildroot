################################################################################
#
# amlogic 8188eu driver
#
################################################################################

RTK8188EU_VERSION = $(call qstrip,$(BR2_PACKAGE_RTK8188EU_GIT_VERSION))
RTK8188EU_SITE = $(call qstrip,$(BR2_PACKAGE_RTK8188EU_GIT_REPO_URL))
$(eval $(generic-package))

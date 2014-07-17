################################################################################
#
# amlogic 8811au driver
#
################################################################################

RTK8811AU_VERSION = $(call qstrip,$(BR2_PACKAGE_RTK8811AU_GIT_VERSION))
RTK8811AU_SITE = $(call qstrip,$(BR2_PACKAGE_RTK8811AU_GIT_REPO_URL))
$(eval $(generic-package))

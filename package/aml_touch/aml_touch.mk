################################################################################
#
# amlogic touch driver
#
################################################################################

AML_TOUCH_VERSION = $(call qstrip,$(BR2_PACKAGE_AML_TOUCH_GIT_VERSION))
AML_TOUCH_SITE = $(call qstrip,$(BR2_PACKAGE_AML_TOUCH_GIT_REPO_URL))
$(eval $(generic-package))

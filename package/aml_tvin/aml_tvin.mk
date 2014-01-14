################################################################################
#
# amlogic tvin driver
#
################################################################################

AML_TVIN_VERSION = $(call qstrip,$(BR2_PACKAGE_AML_TVIN_GIT_VERSION))
AML_TVIN_SITE = $(call qstrip,$(BR2_PACKAGE_AML_TVIN_GIT_REPO_URL))
$(eval $(generic-package))

################################################################################
#
# broadcom usi driver
#
################################################################################

BRCMUSI_VERSION = $(call qstrip,$(BR2_PACKAGE_BRCMUSI_GIT_VERSION))
BRCMUSI_SITE = $(call qstrip,$(BR2_PACKAGE_BRCMUSI_GIT_REPO_URL))
$(eval $(generic-package))

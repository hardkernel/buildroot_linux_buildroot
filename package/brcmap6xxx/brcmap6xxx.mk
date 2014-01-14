################################################################################
#
# amlogic broadcom AP6xxx driver
#
################################################################################

BRCMAP6XXX_VERSION = $(call qstrip,$(BR2_PACKAGE_BRCMAP6XXX_GIT_VERSION))
BRCMAP6XXX_SITE = $(call qstrip,$(BR2_PACKAGE_BRCMAP6XXX_GIT_REPO_URL))
$(eval $(generic-package))

################################################################################
#
# mali_utility
#
################################################################################

MALI_UTILITY_VERSION = 2.0.0.9444
MALI_UTILITY_SITE = $(TOPDIR)/package/mali_utility/bin
MALI_UTILITY_SITE_METHOD = local

define MALI_UTILITY_INSTALL_TARGET_CMDS
	cp -rf $(@D)/arm/ $(TARGET_DIR)/usr/share/
endef

$(eval $(generic-package))

################################################################################
#
# mali_utility
#
################################################################################

MALI_EXAMPLES_VERSION = 2.0.0.9444
MALI_EXAMPLES_SITE = $(TOPDIR)/package/mali_examples/bin
MALI_EXAMPLES_SITE_METHOD = local

define MALI_EXAMPLES_INSTALL_TARGET_CMDS
	cp -rf $(@D)/arm/ $(TARGET_DIR)/usr/share/
endef

$(eval $(generic-package))

#############################################################
#
# aml_launcher
#
#############################################################
#AML_LAUNCHER_VERSION = 2017
AML_LAUNCHER_SITE = $(TOPDIR)/package/aml_launcher
AML_LAUNCHER_SITE_METHOD = local

ifeq ($(BR2_PACKAGE_LAUNCHER_NONE),y)
define AML_LAUNCHER_INSTALL_TARGET_CMDS
	rm -rf $(TARGET_DIR)/etc/init.d/S90*
endef
endif

$(eval $(generic-package))

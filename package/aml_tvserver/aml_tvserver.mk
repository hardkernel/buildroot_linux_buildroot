################################################################################
#
# aml_tvserver
#
################################################################################
AML_TVSERVER_SITE = $(TOPDIR)/../vendor/amlogic/aml_tvserver
AML_TVSERVER_SITE_METHOD=local
AML_TVSERVER_DEPENDENCIES = dbus

define AML_TVSERVER_BUILD_CMDS
	$(MAKE) CC=$(TARGET_CC) -C $(@D) all
endef

define AML_TVSERVER_CLEAN_CMDS
	$(MAKE) -C $(@D) clean
endef

define AML_TVSERVER_INSTALL_TARGET_CMDS
	$(MAKE) -C $(@D) install
endef

define AML_TVSERVER_UNINSTALL_TARGET_CMDS
        $(MAKE) -C $(@D) uninstall
endef

$(eval $(generic-package))
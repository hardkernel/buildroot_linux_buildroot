#############################################################
#
# PROVISION driver
#
#############################################################
PROVISION_SITE = $(TOPDIR)/../vendor/amlogic/provision
PROVISION_SITE_METHOD = local



define PROVISION_BUILD_CMDS
	@echo "provision comiple"
endef

define PROVISION_INSTALL_TARGET_CMDS
	-mkdir -p $(TARGET_DIR)/lib/teetz/
	$(INSTALL) -D -m 0755 $(@D)/ca/bin/tee_provision $(TARGET_DIR)/usr/bin/
	$(INSTALL) -D -m 0755 $(@D)/ta/e92a43ab-b4c8-4450-aa12b1516259613b.ta $(TARGET_DIR)/lib/teetz/
endef

$(eval $(generic-package))

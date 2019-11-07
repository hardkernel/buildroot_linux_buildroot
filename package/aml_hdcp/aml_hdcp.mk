#############################################################
#
# HDCP driver
#
#############################################################
AML_HDCP_SITE = $(TOPDIR)/../vendor/amlogic/hdcp
AML_HDCP_SITE_METHOD = local



define AML_HDCP_BUILD_CMDS
	@echo "aml hdcp comiple"
endef

define AML_HDCP_INSTALL_TARGET_CMDS
	-mkdir -p $(TARGET_DIR)/lib/teetz/
	$(INSTALL) -D -m 0755 $(@D)/ca/bin/tee_hdcp $(TARGET_DIR)/usr/bin/
	$(INSTALL) -D -m 0755 $(@D)/ta/ff2a4bea-ef6d-11e6-89ccd4ae52a7b3b3.ta $(TARGET_DIR)/lib/teetz/
	install -m 755 package/aml_hdcp/S60hdcp $(TARGET_DIR)/etc/init.d/S60hdcp
endef

$(eval $(generic-package))

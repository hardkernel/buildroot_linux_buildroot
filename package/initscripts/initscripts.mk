################################################################################
#
# initscripts
#
################################################################################

define INITSCRIPTS_INSTALL_TARGET_CMDS
	mkdir -p  $(TARGET_DIR)/etc/init.d
	$(INSTALL) -D -m 0755 package/initscripts/init.d/* $(TARGET_DIR)/etc/init.d/
	$(INSTALL) -D -m 0755 package/initscripts/etc/* $(TARGET_DIR)/etc/
endef

$(eval $(generic-package))

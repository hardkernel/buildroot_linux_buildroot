################################################################################
#
# amlogic image packer
#
################################################################################
AMLOGIC_TEST_VERSION = 2015
AMLOGIC_TEST_SITE = $(TOPDIR)/package/amlogic_test/
AMLOGIC_TEST_SITE_METHOD = local

define AMLOGIC_TEST_CONFIGURE_CMDS
	(cd $(@D)/src/stressapptest; \
	 ./configure \
	 --host=$(BR2_TOOLCHAIN_EXTERNAL_PREFIX) \
	)
endef

define AMLOGIC_TEST_BUILD_CMDS
	$(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D)/src/auto_suspend
	$(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D)/src/stressapptest
endef

define AMLOGIC_TEST_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 $(@D)/src/auto_suspend/auto_suspend $(TARGET_DIR)/usr/bin
	$(INSTALL) -m 0755 $(@D)/amlogic_test.sh $(TARGET_DIR)/usr/bin
	$(INSTALL) -m 0755 $(@D)/auto_freq.sh $(TARGET_DIR)/usr/bin
	$(INSTALL) -m 0755 $(@D)/auto_reboot.sh $(TARGET_DIR)/usr/bin
	$(INSTALL) -m 0755 $(@D)/disk-test.sh $(TARGET_DIR)/usr/bin
	$(INSTALL) -m 0755 $(@D)/wifi_test.sh $(TARGET_DIR)/usr/bin
	$(INSTALL) -m 0755 $(@D)/src/stressapptest/src/stressapptest $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))

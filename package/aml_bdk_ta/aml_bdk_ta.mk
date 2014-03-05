#############################################################
#
# Amlogic Trustzone trusted application
#
#############################################################
AML_BDK_TA_VERSION = 0.5
AML_BDK_TA_SITE = $(TOPDIR)/package/aml_bdk_ta/amlogic_ta_bdk_v0.5
AML_BDK_TA_SITE_METHOD = local

define AML_BDK_TA_BUILD_CMDS
	$(TARGET_CONFIGURE_OPTS) $(MAKE) -j1 -C $(@D) all
endef

define AML_BDK_TA_INSTALL_TARGET_CMDS
	rm $(@D)/out/apps/*.o
	$(INSTALL) -D -m 0644 $(@D)/out/apps/* $(TARGET_DIR)/usr/lib/
endef

define AML_BDK_TA_CLEAN_CMDS
	$(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) clean
endef

$(eval $(generic-package))

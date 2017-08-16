################################################################################
#
# dhd private tools
#
################################################################################
DHD_PRIV_VERSION = 1.1
DHD_PRIV_SITE = $(TOPDIR)/../hardware/aml-4.9/amlogic/wifi/bcm_ampak/tools/dhd_priv
DHD_PRIV_SITE_METHOD = local



define DHD_PRIV_BUILD_CMDS
	$(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) CC=$(TARGET_CC)
endef

define DHD_PRIV_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/dhd_priv $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))

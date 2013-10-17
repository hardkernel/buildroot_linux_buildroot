#############################################################
#
# gat-aml-plugins
#
#############################################################
GST_AML_PLUGINS_VERSION = 0.10.0
GST_AML_PLUGINS_SITE = .

GST_FSL_PLUGINS_INSTALL_STAGING = YES
GST_FSL_PLUGINS_AUTORECONF = YES

	
define GST_AML_PLUGINS_BUILD_CMDS
	$(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) -e
endef

define GST_AML_PLUGINS_INSTALL_TARGET_CMDS
	$(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) -e DESTDIR=$(TARGET_DIR) install
endef

define GST_AML_PLUGINS_UNINSTALL_TARGET_CMDS
	$(RM) $(TARGET_DIR)/usr/lib/gstreamer-0.10/{libgstvout.so,libgstamlaout.so}
endef

GST_AML_PLUGINS_DEPENDENCIES = gstreamer  host-pkgconf libplayer

$(eval $(autotools-package))


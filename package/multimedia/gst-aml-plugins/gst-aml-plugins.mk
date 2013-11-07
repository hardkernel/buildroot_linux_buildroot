#############################################################
#
# gat-aml-plugins
#
#############################################################
GST_AML_PLUGINS_VERSION = 0.11.0
GST_AML_PLUGINS_SITE =file://$(TOPDIR)/package/multimedia/gst-aml-plugins/

GST_FSL_PLUGINS_INSTALL_STAGING = YES
GST_FSL_PLUGINS_AUTORECONF = YES
GST_AML_PLUGINS_DEPENDENCIES = gstreamer host-pkgconf libplayer

$(eval $(autotools-package))


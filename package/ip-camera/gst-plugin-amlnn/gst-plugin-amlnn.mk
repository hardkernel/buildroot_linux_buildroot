################################################################################
#
# gst-plugin-amlnn
#
################################################################################

GST_PLUGIN_AMLNN_VERSION = 1.0
GST_PLUGIN_AMLNN_SITE = $(GST_PLUGIN_AMLNN_PKGDIR)/gst-plugin-amlnn-$(GST_PLUGIN_AMLNN_VERSION)
GST_PLUGIN_AMLNN_SITE_METHOD = local
GST_PLUGIN_AMLNN_LICENSE = LGPL
GST_PLUGIN_AMLNN_INSTALL_STAGING = YES
#GST_PLUGIN_AMLNN_LICENSE_FILES =

define GST_PLUGIN_AMLNN_RUN_AUTOGEN
	cd $(@D) && PATH=$(BR_PATH) ./autogen.sh
endef
GST_PLUGIN_AMLNN_POST_PATCH_HOOKS += GST_PLUGIN_AMLNN_RUN_AUTOGEN
GST_PLUGIN_AMLNN_DEPENDENCIES += host-automake host-autoconf host-libtool gstreamer1 gst1-plugins-base aml_nn_detect

$(eval $(autotools-package))

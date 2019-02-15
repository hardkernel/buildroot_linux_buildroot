################################################################################
#
# gst-plugin-amloverlay
#
################################################################################

GST_PLUGIN_AMLOVERLAY_VERSION = 1.0
GST_PLUGIN_AMLOVERLAY_SITE = $(GST_PLUGIN_AMLOVERLAY_PKGDIR)/gst-plugin-amloverlay-$(GST_PLUGIN_AMLOVERLAY_VERSION)
GST_PLUGIN_AMLOVERLAY_SITE_METHOD = local
GST_PLUGIN_AMLOVERLAY_LICENSE = LGPL
GST_PLUGIN_AMLOVERLAY_INSTALL_STAGING = YES
#GST_PLUGIN_AMLOVERLAY_LICENSE_FILES =

define GST_PLUGIN_AMLOVERLAY_RUN_AUTOGEN
	cd $(@D) && PATH=$(BR_PATH) ./autogen.sh
endef
GST_PLUGIN_AMLOVERLAY_POST_PATCH_HOOKS += GST_PLUGIN_AMLOVERLAY_RUN_AUTOGEN
GST_PLUGIN_AMLOVERLAY_DEPENDENCIES += host-automake host-autoconf host-libtool gstreamer1 gst1-plugins-base

$(eval $(autotools-package))

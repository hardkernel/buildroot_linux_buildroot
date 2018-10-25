################################################################################
#
# gst-plugin-amlyoloface
#
################################################################################

GST_PLUGIN_AMLYOLOFACE_VERSION = 1.0
GST_PLUGIN_AMLYOLOFACE_SITE = $(TOPDIR)/package/ip-camera/gst-plugin-amlyoloface/gst-plugin-amlyoloface-$(GST_PLUGIN_AMLYOLOFACE_VERSION)
GST_PLUGIN_AMLYOLOFACE_SITE_METHOD = local
GST_PLUGIN_AMLYOLOFACE_LICENSE = LGPL
#GST_PLUGIN_AMLYOLOFACE_LICENSE_FILES =

define GST_PLUGIN_AMLYOLOFACE_RUN_AUTOGEN
	cd $(@D) && PATH=$(BR_PATH) ./autogen.sh
endef
GST_PLUGIN_AMLYOLOFACE_POST_PATCH_HOOKS += GST_PLUGIN_AMLYOLOFACE_RUN_AUTOGEN
GST_PLUGIN_AMLYOLOFACE_DEPENDENCIES += host-automake host-autoconf host-libtool gstreamer1 gst1-plugins-base libyoloface

$(eval $(autotools-package))

################################################################################
#
# gst-plugin-amlx265enc
#
################################################################################

GST_PLUGIN_AMLX265ENC_VERSION = 1.0
GST_PLUGIN_AMLX265ENC_SITE = $(TOPDIR)/package/ip-camera/gst-plugin-amlx265enc/gst-plugin-amlx265enc-$(GST_PLUGIN_AMLX265ENC_VERSION)
GST_PLUGIN_AMLX265ENC_SITE_METHOD = local
GST_PLUGIN_AMLX265ENC_LICENSE = LGPL
#GST_PLUGIN_AMLX265ENC_LICENSE_FILES =

define GST_PLUGIN_AMLX265ENC_RUN_AUTOGEN
	cd $(@D) && PATH=$(BR_PATH) ./autogen.sh
endef
GST_PLUGIN_AMLX265ENC_POST_PATCH_HOOKS += GST_PLUGIN_AMLX265ENC_RUN_AUTOGEN
GST_PLUGIN_AMLX265ENC_DEPENDENCIES += host-automake host-autoconf host-libtool gstreamer1 gst1-plugins-base

$(eval $(autotools-package))

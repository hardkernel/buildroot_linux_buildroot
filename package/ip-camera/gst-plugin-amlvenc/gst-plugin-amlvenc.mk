################################################################################
#
# gst-plugin-amlvenc
#
################################################################################

GST_PLUGIN_AMLVENC_VERSION = 1.0
GST_PLUGIN_AMLVENC_SITE = $(GST_PLUGIN_AMLVENC_PKGDIR)/gst-plugin-amlvenc-$(GST_PLUGIN_AMLVENC_VERSION)
GST_PLUGIN_AMLVENC_SITE_METHOD = local
GST_PLUGIN_AMLVENC_LICENSE = LGPL
GST_PLUGIN_AMLVENC_INSTALL_STAGING = YES
#GST_PLUGIN_AMLVENC_LICENSE_FILES =

define GST_PLUGIN_AMLVENC_RUN_AUTOGEN
	cd $(@D) && PATH=$(BR_PATH) ./autogen.sh
endef
GST_PLUGIN_AMLVENC_POST_PATCH_HOOKS += GST_PLUGIN_AMLVENC_RUN_AUTOGEN
GST_PLUGIN_AMLVENC_DEPENDENCIES += host-automake host-autoconf host-libtool gstreamer1 gst1-plugins-base libvpcodec libvphevcodec

$(eval $(autotools-package))

################################################################################
#
# gst-plugin-amlimgcap
#
################################################################################

GST_PLUGIN_AMLIMGCAP_VERSION = 1.0
GST_PLUGIN_AMLIMGCAP_SITE = $(GST_PLUGIN_AMLIMGCAP_PKGDIR)/gst-plugin-amlimgcap-$(GST_PLUGIN_AMLIMGCAP_VERSION)
GST_PLUGIN_AMLIMGCAP_SITE_METHOD = local
GST_PLUGIN_AMLIMGCAP_LICENSE = LGPL
#GST_PLUGIN_AMLIMGCAP_LICENSE_FILES =

define GST_PLUGIN_AMLIMGCAP_RUN_AUTOGEN
	cd $(@D) && PATH=$(BR_PATH) ./autogen.sh
endef
GST_PLUGIN_AMLIMGCAP_POST_PATCH_HOOKS += GST_PLUGIN_AMLIMGCAP_RUN_AUTOGEN
GST_PLUGIN_AMLIMGCAP_DEPENDENCIES += host-automake host-autoconf host-libtool gstreamer1 gst1-plugins-base libjpeg

$(eval $(autotools-package))

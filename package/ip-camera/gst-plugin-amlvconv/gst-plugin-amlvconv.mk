################################################################################
#
# gst-plugin-amlvconv
#
################################################################################

GST_PLUGIN_AMLVCONV_VERSION = 1.0
GST_PLUGIN_AMLVCONV_SITE = $(TOPDIR)/../vendor/amlogic/ipc/ipc_plugins/gst-plugin-amlvconv-$(GST_PLUGIN_AMLVCONV_VERSION)
GST_PLUGIN_AMLVCONV_SITE_METHOD = local
GST_PLUGIN_AMLVCONV_LICENSE = LGPL
GST_PLUGIN_AMLVCONV_INSTALL_STAGING = YES
#GST_PLUGIN_AMLVCONV_LICENSE_FILES =

define GST_PLUGIN_AMLVCONV_RUN_AUTOGEN
	cd $(@D) && PATH=$(BR_PATH) ./autogen.sh
endef
GST_PLUGIN_AMLVCONV_POST_PATCH_HOOKS += GST_PLUGIN_AMLVCONV_RUN_AUTOGEN
GST_PLUGIN_AMLVCONV_DEPENDENCIES += host-automake host-autoconf host-libtool gstreamer1 gst1-plugins-base aml_libge2d

$(eval $(autotools-package))

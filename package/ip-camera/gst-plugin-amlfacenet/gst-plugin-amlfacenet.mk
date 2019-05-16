################################################################################
#
# gst-plugin-amlfacenet
#
################################################################################

GST_PLUGIN_AMLFACENET_VERSION = 1.0
GST_PLUGIN_AMLFACENET_SITE = $(TOPDIR)/../vendor/amlogic/ipc/ipc_plugins/gst-plugin-amlfacenet-$(GST_PLUGIN_AMLFACENET_VERSION)

GST_PLUGIN_AMLFACENET_SITE_METHOD = local
GST_PLUGIN_AMLFACENET_LICENSE = LGPL
GST_PLUGIN_AMLFACENET_INSTALL_STAGING = YES
#GST_PLUGIN_AMLFACENET_LICENSE_FILES =

define GST_PLUGIN_AMLFACENET_RUN_AUTOGEN
	cd $(@D) && PATH=$(BR_PATH) ./autogen.sh
endef
GST_PLUGIN_AMLFACENET_POST_PATCH_HOOKS += GST_PLUGIN_AMLFACENET_RUN_AUTOGEN
GST_PLUGIN_AMLFACENET_DEPENDENCIES += host-automake host-autoconf host-libtool
GST_PLUGIN_AMLFACENET_DEPENDENCIES += gstreamer1 gst1-plugins-base
GST_PLUGIN_AMLFACENET_DEPENDENCIES += sqlite
GST_PLUGIN_AMLFACENET_DEPENDENCIES += aml_nn_detect

$(eval $(autotools-package))

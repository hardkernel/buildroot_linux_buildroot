#############################################################
#
# RTSP All The Things
#
#############################################################

RTSPATT_VERSION = 25c575f266fe224d958a7260fcb9c47bc96fdf1e

RTSPATT_LICENSE = Apache License
#RTSPATT_LICENSE_FILES = COPYING
RTSPATT_DEPENDENCIES = gst1-plugins-base gst1-plugins-good gst1-rtsp-server gstreamer1 gst-plugin-amlx264enc gst-plugin-amlx265enc gst-plugin-amlyoloface

RTSPATT_SITE = $(call github,Ullaakut,RTSPAllTheThings,$(RTSPATT_VERSION))


RTSPATT_INSTALL_DIR = $(TARGET_DIR)/usr/bin/

define RTSPATT_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/rtspatt            $(RTSPATT_INSTALL_DIR)
	$(INSTALL) -D -m 755 package/ip-camera/rtspatt/S91rtspatt $(TARGET_DIR)/etc/init.d/
endef

$(eval $(cmake-package))

#############################################################
#
# ONVIF PREBUILT
#
#############################################################

ONVIF_ARCH=arm64
ONVIF_PREBUILT_SITE = $(TOPDIR)/../vendor/amlogic/ipc/onvif_prebuilt
ONVIF_PREBUILT_SITE_METHOD = local
ONVIF_PREBUILT_DIRECTORY=$(ONVIF_PREBUILT_SITE)/$(ONVIF_ARCH)/

ifeq ($(BR2_PACKAGE_ONVIF_APPLY_PREBUILT),y)
#We will only apply onvif prebuilt packages, so we need to make sure the original package's dependency can meet.
ONVIF_PREBUILT_DEPENDENCIES = openssl zlib  gstreamer1 gst1-plugins-base gst1-plugins-good gst1-rtsp-server libjpeg
define ONVIF_PREBUILT_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)
	cp -av $(ONVIF_PREBUILT_DIRECTORY)/*  $(TARGET_DIR)/
endef

endif

ifeq ($(BR2_PACKAGE_ONVIF_GENERATE_PREBUILT),y)
#We will package the onvif prebuitl pacakge, so we need to wait for onvif libraries/binraries ready, here we list the onvif binraies.
ONVIF_PREBUILT_DEPENDENCIES = onvif_srvd
define ONVIF_PREBUILT_INSTALL_TARGET_CMDS
	rm    -fr $(@D)/$(ONVIF_ARCH)/
	mkdir -p $(@D)/$(ONVIF_ARCH)/

	mkdir -p $(@D)/$(ONVIF_ARCH)/etc/init.d/
	mkdir -p $(@D)/$(ONVIF_ARCH)/usr/bin
	mkdir -p $(@D)/$(ONVIF_ARCH)/usr/lib/gstreamer-1.0/

	cp -a $(TARGET_DIR)/etc/ipc.json                       $(@D)/$(ONVIF_ARCH)/etc/
	cp -a $(TARGET_DIR)/etc/onvif                          $(@D)/$(ONVIF_ARCH)/etc/
	cp -a $(TARGET_DIR)/etc/init.d/S80ipc-property-service $(@D)/$(ONVIF_ARCH)/etc/init.d/
	cp -a $(TARGET_DIR)/etc/init.d/S91onvif_rtsp           $(@D)/$(ONVIF_ARCH)/etc/init.d/
	cp -a $(TARGET_DIR)/etc/init.d/S91onvif_srvd           $(@D)/$(ONVIF_ARCH)/etc/init.d/
	cp -a $(TARGET_DIR)/etc/init.d/S91onvif_wsdd           $(@D)/$(ONVIF_ARCH)/etc/init.d/

	cp -a $(TARGET_DIR)/usr/bin/onvif_rtsp                 $(@D)/$(ONVIF_ARCH)/usr/bin/
	cp -a $(TARGET_DIR)/usr/bin/onvif_srvd                 $(@D)/$(ONVIF_ARCH)/usr/bin/
	cp -a $(TARGET_DIR)/usr/bin/onvif_wsdd                 $(@D)/$(ONVIF_ARCH)/usr/bin/
	cp -a $(TARGET_DIR)/usr/bin/ipc-property               $(@D)/$(ONVIF_ARCH)/usr/bin/
	cp -a $(TARGET_DIR)/usr/bin/ipc-property-service       $(@D)/$(ONVIF_ARCH)/usr/bin/

	cp -a $(TARGET_DIR)/usr/lib/libipc-property.so                   $(@D)/$(ONVIF_ARCH)/usr/lib/
	cp -a $(TARGET_DIR)/usr/lib/gstreamer-1.0/libgstamlimgcap.so     $(@D)/$(ONVIF_ARCH)/usr/lib/gstreamer-1.0/
	cp -a $(TARGET_DIR)/usr/lib/gstreamer-1.0/libgstamlnn.so         $(@D)/$(ONVIF_ARCH)/usr/lib/gstreamer-1.0/
	cp -a $(TARGET_DIR)/usr/lib/gstreamer-1.0/libgstamloverlay.so    $(@D)/$(ONVIF_ARCH)/usr/lib/gstreamer-1.0/
	cp -a $(TARGET_DIR)/usr/lib/gstreamer-1.0/libgstamlvenc.so       $(@D)/$(ONVIF_ARCH)/usr/lib/gstreamer-1.0/
	cp -a $(TARGET_DIR)/usr/lib/gstreamer-1.0/libgstamlfacenet.so    $(@D)/$(ONVIF_ARCH)/usr/lib/gstreamer-1.0/

	tar -zcf $(TARGET_DIR)/../images/onvif-prebuilt-$(ONVIF_ARCH).tgz -C $(@D) $(ONVIF_ARCH)
endef

endif

$(eval $(generic-package))

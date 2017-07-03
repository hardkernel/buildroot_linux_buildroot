#############################################################
#
# Chromium-prebuilt
#
#############################################################
ifeq ($(BR2_PACKAGE_CHROMIUM_PREBUILT),y)

CHROMIUM_PREBUILT_VERSION = 53.0.2785.143

CHROMIUM_PREBUILT_DEPENDENCIES = libxkbcommon gconf libexif cups libnss libdrm pciutils pulseaudio krb5 pango libplayer

#prebuilt defines.
CHROMIUM_PREBUILT_SITE = $(TOPDIR)/../vendor/amlogic/chromium
CHROMIUM_PREBUILT_SITE_METHOD = local

ifeq ($(BR2_aarch64),y)
CHROMIUM_PREBUILT_DIRECTORY = vendor/amlogic/chromium/chromium-$(CHROMIUM_PREBUILT_VERSION)/arm64
else
CHROMIUM_PREBUILT_DIRECTORY = vendor/amlogic/chromium/chromium-$(CHROMIUM_PREBUILT_VERSION)/arm
endif

define CHROMIUM_PREBUILT_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/bin/chromium-browser
	cp -a $(TOPDIR)/../$(CHROMIUM_PREBUILT_DIRECTORY)/chrome            $(TARGET_DIR)/usr/bin/chromium-browser
	cp -a $(TOPDIR)/../$(CHROMIUM_PREBUILT_DIRECTORY)/*.pak             $(TARGET_DIR)/usr/bin/chromium-browser
	cp -a $(TOPDIR)/../$(CHROMIUM_PREBUILT_DIRECTORY)/resources         $(TARGET_DIR)/usr/bin/chromium-browser
	cp -a $(TOPDIR)/../$(CHROMIUM_PREBUILT_DIRECTORY)/locales           $(TARGET_DIR)/usr/bin/chromium-browser
	cp -a $(TOPDIR)/../$(CHROMIUM_PREBUILT_DIRECTORY)/icudtl.dat        $(TARGET_DIR)/usr/bin/chromium-browser
endef

define CHROMIUM_PREBUILT_INSTALL_INIT_SYSV
	$(INSTALL) -D -m 755 package/chromium/S90chrome \
		$(TARGET_DIR)/etc/init.d/S90chrome
	$(INSTALL) -D -m 755 package/chromium/amlogic.html \
		$(TARGET_DIR)/var/www/amlogic.html
endef

$(eval $(generic-package))

endif

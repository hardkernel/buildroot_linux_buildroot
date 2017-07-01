#############################################################
#
# Chromium
#
#############################################################
ifeq ($(BR2_PACKAGE_CHROMIUM_PREBUILT),y)
include package/chromium/chromium-prebuilt/chromium-prebuilt.mk
endif


ifeq ($(BR2_PACKAGE_CHROMIUM_COMPILE_ALL),y)

CHROMIUM_VERSION = 53.0.2785.143

#CHROMIUM_LICENSE = GPLv3+
#CHROMIUM_LICENSE_FILES = COPYING
CHROMIUM_DEPENDENCIES = libxkbcommon gconf libexif cups libnss libdrm pciutils pulseaudio krb5 pango

CHROMIUM_SOURCE = chromium-$(CHROMIUM_VERSION).tar.gz
# This URL will not work directly, it is assumed the chromium tarball is already placed in buildroot/dl
CHROMIUM_SITE = http://openlinux.amlogic.com:8000/download/GPL_code_release/ThirdParty

ifeq ($(BR2_aarch64), y)

CHROMIUM_FLAGS = 'disable_fatal_linker_warnings=1 v8_use_external_startup_data=0 use_allocator=none angle_use_commit_id=0 disable_nacl=1 use_kerberos=0 use_cups=0 use_gnome_keyring=0 linux_link_gnome_keyring=0 linux_link_kerberos=0 v8_use_snapshot="true" use_system_bzip=1 host_clang=0 clang=0 use_sysroot=1 component=static_library linux_use_bundled_gold=0 linux_use_bundled_binutils=0 linux_use_gold_flags=0 use_ozone=1 ozone_auto_platforms=1 ozone_platform_wayland=1 ozone_platform_gbm=0 use_xkbcommon=1 ffmpeg_branding=Chrome media_use_libvpx=0 proprietary_codecs=1 enable_hevc_demuxing=1 enable_ac3_eac3_audio_demuxing=1 remove_webcore_debug_symbols=1 target_os=linux target_arch=arm64 arm_float_abi=hard target_sysroot=$(STAGING_DIR)'

else

CHROMIUM_FLAGS = 'disable_fatal_linker_warnings=1 v8_use_external_startup_data=0 use_allocator=none angle_use_commit_id=0 disable_nacl=1 use_kerberos=0 use_cups=0 use_gnome_keyring=0 linux_link_gnome_keyring=0 linux_link_kerberos=0 v8_use_snapshot="true" use_system_bzip=1 host_clang=0 clang=0 use_sysroot=1 component=static_library linux_use_bundled_gold=0 linux_use_bundled_binutils=0 linux_use_gold_flags=0 use_ozone=1 ozone_auto_platforms=1 ozone_platform_wayland=1 ozone_platform_gbm=0 use_xkbcommon=1 ffmpeg_branding=Chrome media_use_libvpx=0 proprietary_codecs=1 enable_hevc_demuxing=1 enable_ac3_eac3_audio_demuxing=1 remove_webcore_debug_symbols=1 target_os=linux target_arch=arm arm_float_abi=hard target_sysroot=$(STAGING_DIR)'

endif

CHROMIUM_MODE = Release

define CHROMIUM_BUILD_CMDS
    export PATH=$(HOST_DIR)/usr/bin:$(CHROMIUM_DIR)/depot_tools:$(PATH);cd $(CHROMIUM_DIR)/src;export GYP_CROSSCOMPILE=1;export GYP_DEFINES=$(CHROMIUM_FLAGS);./build/gyp_chromium;ninja -C out/$(CHROMIUM_MODE) -j16 chrome
endef

define CHROMIUM_INSTALL_STAGING_CMDS
endef

define CHROMIUM_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/bin/chromium-browser
	cp -a $(CHROMIUM_DIR)/src/out/$(CHROMIUM_MODE)/chrome            $(TARGET_DIR)/usr/bin/chromium-browser
	cp -a $(CHROMIUM_DIR)/src/out/$(CHROMIUM_MODE)/*.pak             $(TARGET_DIR)/usr/bin/chromium-browser
	cp -a $(CHROMIUM_DIR)/src/out/$(CHROMIUM_MODE)/resources         $(TARGET_DIR)/usr/bin/chromium-browser
	cp -a $(CHROMIUM_DIR)/src/out/$(CHROMIUM_MODE)/locales           $(TARGET_DIR)/usr/bin/chromium-browser
	cp -a $(CHROMIUM_DIR)/src/out/$(CHROMIUM_MODE)/icudtl.dat        $(TARGET_DIR)/usr/bin/chromium-browser
endef

define CHROMIUM_INSTALL_INIT_SYSV
	$(INSTALL) -D -m 755 package/chromium/S90chrome \
		$(TARGET_DIR)/etc/init.d/S90chrome
	$(INSTALL) -D -m 755 package/chromium/amlogic.html \
		$(TARGET_DIR)/var/www/amlogic.html
endef

$(eval $(generic-package))

endif

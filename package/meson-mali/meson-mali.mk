#############################################################
#
# meson_mali
#
#############################################################
MESON_MALI_VERSION = 1.0
MESON_MALI_SITE = $(TOPDIR)/package/meson-mali
MESON_MALI_SITE_METHOD = local
MESON_MALI_INSTALL_STAGING = YES
MESON_MALI_PROVIDES = libegl libgles

EGL_PLATFORM_HEADER =

ifneq ($(BR2_PACKAGE_MESON_MALI_VERSION),"")
MESON_MALI_VERSION = $(call qstrip,$(BR2_PACKAGE_MESON_MALI_VERSION))
else
MESON_MALI_VERSION = $(call qstrip,$(BR2_PACKAGE_GPU_VERSION))
endif

API_VERSION = $(call qstrip,$(MESON_MALI_VERSION))
ifneq ($(BR2_PACKAGE_MESON_MALI_MODEL),"")
MALI_VERSION = $(call qstrip,$(BR2_PACKAGE_MESON_MALI_MODEL))
else
MALI_VERSION = $(call qstrip,$(BR2_PACKAGE_OPENGL_MALI_VERSION))
endif

ifeq ($(BR2_PACKAGE_MESON_MALI_WAYLAND_EGL),y)
EGL_PLATFORM_HEADER = EGL_platform/platform_wayland
MALI_LIB_LOC = $(API_VERSION)/$(MALI_VERSION)/wayland
MESON_MALI_DEPENDENCIES += wayland
else
EGL_PLATFORM_HEADER = EGL_platform/platform_fbdev
MALI_LIB_LOC = $(API_VERSION)/$(MALI_VERSION)
endif

ifeq ($(BR2_aarch64),y)
MALI_LIB_DIR = arm64/$(MALI_LIB_LOC)
else ifeq ($(BR2_ARM_EABIHF),y)
MALI_LIB_DIR = eabihf/$(MALI_LIB_LOC)
else
MALI_LIB_DIR = $(MALI_LIB_LOC)
endif


define MESON_MALI_INSTALL_STAGING_CMDS
	cp -arf $(MESON_MALI_DIR)/include/EGL $(STAGING_DIR)/usr/include/
	cp -arf $(MESON_MALI_DIR)/include/GLES $(STAGING_DIR)/usr/include/
	cp -arf $(MESON_MALI_DIR)/include/GLES2 $(STAGING_DIR)/usr/include/
	cp -arf $(MESON_MALI_DIR)/include/KHR $(STAGING_DIR)/usr/include/
	cp -arf $(MESON_MALI_DIR)/include/$(EGL_PLATFORM_HEADER)/*.h $(STAGING_DIR)/usr/include/EGL/
	cp -arf $(MESON_MALI_DIR)/lib/$(MALI_LIB_DIR)/*.so* $(STAGING_DIR)/usr/lib/
	cp -arf $(MESON_MALI_DIR)/lib/*.so* $(STAGING_DIR)/usr/lib/
	mkdir -p $(STAGING_DIR)/usr/lib/pkgconfig/
	cp -arf $(MESON_MALI_DIR)/lib/pkgconfig/*.pc $(STAGING_DIR)/usr/lib/pkgconfig/
endef

define MESON_MALI_INSTALL_TARGET_CMDS
	cp -df $(MESON_MALI_DIR)/lib/*.so* $(TARGET_DIR)/usr/lib
	install -m 755 $(MESON_MALI_DIR)/lib/$(MALI_LIB_DIR)/*.so* $(TARGET_DIR)/usr/lib
	mkdir -p $(TARGET_DIR)/usr/lib/pkgconfig
	install -m 644 $(MESON_MALI_DIR)/lib/pkgconfig/*.pc $(TARGET_DIR)/usr/lib/pkgconfig
endef

$(eval $(generic-package))



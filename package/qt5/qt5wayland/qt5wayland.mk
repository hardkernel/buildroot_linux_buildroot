################################################################################
#
# qt5wayland
#
################################################################################

QT5WAYLAND_VERSION = $(QT5_VERSION)
QT5WAYLAND_SITE = $(QT5_SITE)
QT5WAYLAND_SOURCE = qtwayland-opensource-src-$(QT5WAYLAND_VERSION).tar.xz
QT5WAYLAND_DEPENDENCIES = qt5base wayland libxkbcommon xkeyboard-config
#ifeq ($(BR2_PACKAGE_HAS_LIBEGL),y)
#QT5WAYLAND_DEPENDENCIES += libwayland-egl
#endif
QT5WAYLAND_INSTALL_STAGING = YES


ifeq ($(BR2_PACKAGE_QT5BASE_LICENSE_APPROVED),y)
QT5WAYLAND_LICENSE = GPLv3 or LGPLv2.1 with exception or LGPLv3, GFDLv1.3 (docs)
QT5WAYLAND_LICENSE_FILES = LICENSE.GPLv3 LICENSE.LGPLv21 LGPL_EXCEPTION.txt LICENSE.LGPLv3 LICENSE.FDL
else
QT5WAYLAND_LICENSE = Commercial license
QT5WAYLAND_REDISTRIBUTE = NO
endif

define QT5WAYLAND_CONFIGURE_CMDS
	(cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/usr/bin/qmake)
endef

define QT5WAYLAND_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define QT5WAYLAND_INSTALL_STAGING_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) install
	$(QT5_LA_PRL_FILES_FIXUP)
endef

ifeq ($(BR2_STATIC_LIBS),)
define QT5WAYLAND_INSTALL_TARGET_CMDS
	cp -dpf $(STAGING_DIR)/usr/lib/libQt5WaylandClient.so* $(TARGET_DIR)/usr/lib
	cp -dpfr $(STAGING_DIR)/usr/lib/qt/plugins/wayland-*-client $(TARGET_DIR)/usr/lib/qt/plugins
	cp -dpf $(STAGING_DIR)/usr/lib/qt/plugins/platforms/libqwayland-*.so $(TARGET_DIR)/usr/lib/qt/plugins/platforms
endef
endif

$(eval $(generic-package))

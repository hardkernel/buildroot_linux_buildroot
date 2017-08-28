################################################################################
#
# glib-networking
#
################################################################################

GLIB_NETWORKING_VERSION_MAJOR = 2.50
GLIB_NETWORKING_VERSION = $(GLIB_NETWORKING_VERSION_MAJOR).0
GLIB_NETWORKING_SITE = http://ftp.gnome.org/pub/gnome/sources/glib-networking/$(GLIB_NETWORKING_VERSION_MAJOR)
GLIB_NETWORKING_SOURCE = glib-networking-$(GLIB_NETWORKING_VERSION).tar.xz
GLIB_NETWORKING_INSTALL_STAGING = YES
GLIB_HOST = arm-linux-gnueabihf
GLIB_NETWORKING_DEPENDENCIES = \
	$(if $(BR2_NEEDS_GETTEXT_IF_LOCALE),gettext) \
	host-pkgconf \
	host-intltool \
	libglib2 \
	libsoup
GLIB_NETWORKING_CONF_OPTS = \
	--with-ca-certificates=/etc/ssl/certs/ca-certificates.crt \
	--host=$(GLIB_HOST) \
	--prefix=$(TARGET_DIR)/usr \
#    --with-libgcrypt-prefix=$(TARGET_DIR)
GLIB_NETWORKING_LICENSE = LGPLv2+
GLIB_NETWORKING_LICENSE_FILES = COPYING
GLIB_NETWORKING_MAKE_ENV = \
						   CC="$(TARGET_CC)" \
						   CFLAGS="$(GLIB_NETWORKING_CFLAGS)" \
						   $(TARGET_MAKE_ENV)
ifeq ($(BR2_PACKAGE_GNUTLS),y)
GLIB_NETWORKING_DEPENDENCIES += gnutls
GLIB_NETWORKING_CONF_OPTS += --with-libgcrypt-prefix=$(STAGING_DIR)/usr
else
GLIB_NETWORKING_CONF_OPTS += --without-gnutls
endif

ifeq ($(BR2_aarch64),y)
	$(GLIB_HOST)=aarch64-linux-gnu
endif
define GLIB_NETWORKING_NO_LIBGCRYPT
	$(SED) 's:#include <gcrypt.h>::' $(@D)/tls/gnutls/gtlsbackend-gnutls.c
endef

define GLIB_NETWORKING_CONFIGURE_CMDS
	cd $(@D); \
	$(TARGET_CONFIGURE_ARGS) \
	$(TARGET_CONFIGURE_OPTS) \
	./configure \
    $(GLIB_NETWORKING_CONF_OPTS)
endef

define GLIB_NETWORKING_BUILD_CMDS
	$(GLIB_NETWORKING_MAKE_ENV) $(MAKE) -C $(@D)
endef

define GLIB_NETWORKING_INSTALL_TARGET_CMDS
	#$(GLIB_NETWORKING_MAKE_ENV) $(MAKE)  install -C $(@D)
	mkdir -p $(TARGET_DIR)/usr/lib/gio/modules
    cp $(@D)/tls/gnutls/.libs/libgiognutls.so $(TARGET_DIR)/usr/lib/gio/modules
endef

$(eval $(generic-package))

################################################################################
#
# kexec
#
################################################################################

KEXEC_VERSION = odroid-v2.0.18
KEXEC_SITE = https://github.com/tobetter/kexec-tools.git
KEXEC_SITE_METHOD = git
KEXEC_INSTALL_STAGING = YES
KEXEC_LICENSE = GPLv2
KEXEC_LICENSE_FILES = COPYING

HOST_AUTOMAKE_DEPENDENCIES = host-autoconf

# Makefile expects $STRIP -o to work, so needed for !BR2_STRIP_strip
KEXEC_MAKE_OPTS = STRIP="$(TARGET_CROSS)strip"

ifeq ($(BR2_PACKAGE_KEXEC_ZLIB),y)
KEXEC_CONF_OPTS += --with-zlib
KEXEC_DEPENDENCIES = zlib
else
KEXEC_CONF_OPTS += --without-zlib
endif

ifeq ($(BR2_PACKAGE_XZ),y)
KEXEC_CONF_OPTS += --with-lzma
KEXEC_DEPENDENCIES += xz
else
KEXEC_CONF_OPTS += --without-lzma
endif

define KEXEC_REMOVE_LIB_TOOLS
	rm -rf $(TARGET_DIR)/usr/lib/kexec-tools
endef

KEXEC_POST_INSTALL_TARGET_HOOKS += KEXEC_REMOVE_LIB_TOOLS

define KEXEC_CONFIGURE_CMDS
	(cd $(@D); rm -rf config.cache; \
		$(TARGET_MAKE_ENV) ./bootstrap; \
		$(TARGET_CONFIGURE_ARGS) \
		$(TARGET_CONFIGURE_OPTS) \
		$(TARGET_MAKE_ENV) ./configure \
		--host=$(GNU_TARGET_NAME) \
		--prefix=/usr \
	)
endef

define KEXEC_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE1) -C $(@D)
endef

define KEXEC_INSTALL_STAGING_CMDS
	$(TARGET_MAKE_ENV) $(MAKE1) -C $(@D) DESTDIR=$(STAGING_DIR) install
endef

define KEXEC_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE1) -C $(@D) DESTDIR=$(TARGET_DIR) install
endef

$(eval $(autotools-package))

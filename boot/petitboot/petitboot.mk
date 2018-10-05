################################################################################
#
# libtwin
#
################################################################################

PETITBOOT_VERSION = petitboot-1.7.x
PETITBOOT_SITE = https://github.com/open-power/petitboot.git
PETITBOOT_SITE_METHOD = git
PETITBOOT_INSTALL_STAGING = YES
PETITBOOT_LICENSE = GPLv2
PETITBOOT_LICENSE_FILES = COPYING

PETITBOOT_DEPENDENCIES = \
	host-bison \
	host-flex \
	ncurses \
	zlib

# Makefile expects $STRIP -o to work, so needed for BR2_STRIP_none
PETITBOOT_MAKE_OPTS = STRIP="$(TARGET_CROSS)strip"

define PETITBOOT_CONFIGURE_CMDS
	(cd $(@D); rm -rf config.cache; \
		$(TARGET_MAKE_ENV) ./bootstrap; \
		$(TARGET_CONFIGURE_ARGS) \
		$(TARGET_CONFIGURE_OPTS) \
		$(TARGET_MAKE_ENV) ./configure \
		--host=$(GNU_TARGET_NAME) \
		--prefix=/usr \
		--without-twin-x11 \
		--without-twin-fbdev \
		--without-signed-boot \
		--disable-nls \
	)
endef

define PETITBOOT_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE1) -C $(@D)
endef

define PETITBOOT_INSTALL_STAGING_CMDS
	$(TARGET_MAKE_ENV) $(MAKE1) -C $(@D) DESTDIR=$(STAGING_DIR) install
endef

define PETITBOOT_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE1) -C $(@D) DESTDIR=$(TARGET_DIR) install
endef

$(eval $(autotools-package))

################################################################################
#
# libtwin
#
################################################################################

LIBTWIN_VERSION = master
LIBTWIN_SITE = git://git.kernel.org/pub/scm/linux/kernel/git/geoff/libtwin.git
LIBTWIN_SITE_METHOD = git
LIBTWIN_INSTALL_STAGING = YES
LIBTWIN_LICENSE = GPLv2
LIBTWIN_LICENSE_FILES = COPYING

LIBTWIN_DEPENDENCIES = zlib

# Makefile expects $STRIP -o to work, so needed for BR2_STRIP_none
LIBTWIN_MAKE_OPTS = STRIP="$(TARGET_CROSS)strip"

define LIBTWIN_CONFIGURE_CMDS
	(cd $(@D); rm -rf config.cache; \
		$(TARGET_MAKE_ENV) ./autogen.sh; \
		$(TARGET_CONFIGURE_ARGS) \
		$(TARGET_CONFIGURE_OPTS) \
		$(TARGET_MAKE_ENV) ./configure \
		--host=$(GNU_TARGET_NAME) \
		--prefix=/usr \
	)
endef

define LIBTWIN_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE1) -C $(@D)
endef

define LIBTWIN_INSTALL_STAGING_CMDS
	$(TARGET_MAKE_ENV) $(MAKE1) -C $(@D) DESTDIR=$(STAGING_DIR) install
endef

define LIBTWIN_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE1) -C $(@D) DESTDIR=$(TARGET_DIR) install
endef

$(eval $(autotools-package))

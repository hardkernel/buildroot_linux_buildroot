################################################################################
#
# tslib
#
################################################################################

TSLIB_VERSION = 1.4
TSLIB_SITE = https://github.com/kergoth/tslib/releases/download/$(TSLIB_VERSION)
TSLIB_SOURCE = tslib-$(TSLIB_VERSION).tar.xz
TSLIB_LICENSE = GPL, LGPL
TSLIB_LICENSE_FILES = COPYING

TSLIB_AUTORECONF = YES
TSLIB_INSTALL_STAGING = YES
TSLIB_INSTALL_STAGING_OPTS = DESTDIR=$(STAGING_DIR) LDFLAGS=-L$(STAGING_DIR)/usr/lib install

$(eval $(autotools-package))

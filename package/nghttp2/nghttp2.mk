########################################################################
#
#nghttp2
#
########################################################################

NGHTTP2_VERSION = 1.24.0
NGHTTP2_SITE = https://github.com/nghttp2/nghttp2/releases/download/v$(NGHTTP2_VERSION)
NGHTTP2_SOURCE = nghttp2-$(NGHTTP2_VERSION).tar.bz2
NGHTTP2_LICENSE = MIT_LICENSE
NGHTTP2_LICENSE_FILES = LICENSE
NGHTTP2_INSTALL_STAGING = YES
NGHTTP2_CFLAGS = \

NGHTTP2_CONF_OPTS = \
	--enable-lib-only2221
	-Dccflags="$(NGHTTP2_CFLAGS)"

NGHTTP2_MAKE_ENV = \
	CC="$(TARGET_CC)"
	CFLAGS = "$(NGHTTP2_CFLAGS)" \
	$(TARGET_MAKE_ENV)


#define NGHTTP2_CONFIGURE_CMDS
#	cd $(@D); \
#	$(TARGET_CONFIGURE_ARGS) \
#	$(TARGET_CONFIGURE_OPTS) \
#	./configure
#endef

#define NGHTTP2_BUILD_CMDS
#	$(@D)/configure -h
#	$(NGHTTP2_MAKE_ENV) $(MAKE) -C $(@D)
#endef

#define NGHTTP2_INSTALL_STAGING_CMDS
#	$(NGHTTP2_MAKE_ENV) $(MAKE) -C $(@D) DESTDIR=$(STAGING_DIR) install
#endef

#define NGHTTP2_INSTALL_TARGET_CMDS
#	$(NGHTTP2_MAKE_ENV) $(MAKE)	-C $(@D) DESTDIR=$(TARGET_DIR) install
#endef

#define NGHTTP2_CLEAN_CMDS
#	$(NGHTTP2_MAKE_ENV) $(MAKE) -C $(@D) DESTDIR=$(STAGING_DIR) uninstall
#	$(NGHTTP2_MAKE_ENV) $(MAKE) -C $(@D) clean
#endef

$(eval $(autotools-package))

################################################################################
#
# uboot-parser
#
################################################################################

UBOOT_PARSER_VERSION = master

UBOOT_PARSER_SITE = https://github.com/tobetter/uboot-parser.git
UBOOT_PARSER_SITE_METHOD = git

UBOOT_PARSER_INSTALL_STAGING = YES
UBOOT_PARSER_LICENSE = GPLv2
UBOOT_PARSER_LICENSE_FILES = COPYING

# Makefile expects $STRIP -o to work, so needed for BR2_STRIP_none
UBOOT_PARSER_MAKE_OPTS = STRIP="$(TARGET_CROSS)strip"

define UBOOT_PARSER_CONFIGURE_CMDS
	(cd $(@D); rm -rf config.cache; \
		$(TARGET_MAKE_ENV) ./autogen.sh; \
		$(TARGET_CONFIGURE_ARGS) \
		$(TARGET_CONFIGURE_OPTS) \
		$(TARGET_MAKE_ENV) ./configure \
		--host=$(GNU_TARGET_NAME) \
		--prefix=/usr \
	)
endef

define UBOOT_PARSER_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE1) -C $(@D)
endef

define UBOOT_PARSER_INSTALL_STAGING_CMDS
	$(TARGET_MAKE_ENV) $(MAKE1) -C $(@D) DESTDIR=$(STAGING_DIR) install
endef

define UBOOT_PARSER_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE1) -C $(@D) DESTDIR=$(TARGET_DIR) install
endef

$(eval $(autotools-package))

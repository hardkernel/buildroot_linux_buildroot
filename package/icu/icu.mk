#############################################################
#
# International Components for Unicode
#
#############################################################

ICU_VERSION = 4.2.1
ICU_SOURCE = icu4c-$(subst .,_,$(ICU_VERSION))-src.tgz
ICU_SITE = http://download.icu-project.org/files/icu4c/$(ICU_VERSION)
ICU_DEPENDENCIES = host-icu
ICU_INSTALL_STAGING = YES
ICU_CONFIG_SCRIPTS = icu-config
ICU_CONF_OPT = --with-cross-build=$(HOST_ICU_DIR)/source --enable-samples=no --enable-tests=no --enable-extras=no --enable-renaming=no --enable-static=no
HOST_ICU_CONF_OPT = --enable-samples=no --enable-tests=no --disable-icuio --disable-layout --enable-renaming=no --enable-static=no
ICU_SUBDIR = source
HOST_ICU_SUBDIR = source
ICU_MAKE = $(MAKE1)

define ICU_PREFIX_FIXUP
	$(SED) "s,^default_prefix=.*,default_prefix=\'$(STAGING_DIR)/usr\',g" $(STAGING_DIR)/usr/bin/icu-config
endef

ICU_POST_INSTALL_TARGET_HOOKS += ICU_PREFIX_FIXUP

$(eval $(autotools-package))
$(eval $(host-autotools-package))

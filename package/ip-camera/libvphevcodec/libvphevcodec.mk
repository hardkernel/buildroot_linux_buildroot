################################################################################
#
# libvphevcodec
#
################################################################################

LIBVPHEVCODEC_VERSION = 1.0
LIBVPHEVCODEC_SITE = $(TOPDIR)/package/ip-camera/libvphevcodec/libvphevcodec-$(LIBVPHEVCODEC_VERSION)
LIBVPHEVCODEC_SITE_METHOD = local
LIBVPHEVCODEC_DEPENDENCIES =
LIBVPHEVCODEC_LICENSE = LGPL

# This package uses the AML_LIBS_STAGING_DIR variable to construct the
# header and library paths used when compiling
define LIBVPHEVCODEC_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) \
		-C $(@D)/$(d)
endef

define LIBVPHEVCODEC_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/libvphevcodec.so $(TARGET_DIR)/usr/lib/libvphevcodec.so
endef

$(eval $(generic-package))

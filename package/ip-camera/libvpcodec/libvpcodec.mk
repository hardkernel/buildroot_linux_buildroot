################################################################################
#
# libvpcodec
#
################################################################################

LIBVPCODEC_VERSION = 1.0
LIBVPCODEC_SITE = $(TOPDIR)/package/ip-camera/libvpcodec/libvpcodec-$(LIBVPCODEC_VERSION)
LIBVPCODEC_SITE_METHOD = local
LIBVPCODEC_DEPENDENCIES =
LIBVPCODEC_LICENSE = LGPL

# This package uses the AML_LIBS_STAGING_DIR variable to construct the
# header and library paths used when compiling
define LIBVPCODEC_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) \
		-C $(@D)/$(d)
endef

define LIBVPCODEC_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/libvpcodec.so $(TARGET_DIR)/usr/lib/libvpcodec.so
endef

$(eval $(generic-package))

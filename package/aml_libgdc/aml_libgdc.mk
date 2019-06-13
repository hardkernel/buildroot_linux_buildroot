################################################################################
#
# libgdc
#
################################################################################

AML_LIBGDC_VERSION:= 1.0.0
AML_LIBGDC_SITE:=$(TOPDIR)/../hardware/aml-4.9/amlogic/libgdc
AML_LIBGDC_SITE_METHOD:=local
AML_LIBGDC_DEPENDENCIES = aml_libion
AML_LIBGDC_INSTALL_STAGING:=YES

define AML_LIBGDC_BUILD_CMDS
    $(MAKE) CC=$(TARGET_CC) -C $(@D)
endef

define AML_LIBGDC_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0644 $(@D)/libgdc.so $(TARGET_DIR)/usr/lib/
    $(INSTALL) -D -m 0755 $(@D)/gdc_test  $(TARGET_DIR)/usr/bin/
endef

define AML_LIBGDC_INSTALL_STAGING_CMDS
    $(INSTALL) -D -m 0644 $(@D)/libgdc.so \
        $(STAGING_DIR)/usr/lib/libgdc.so
    $(INSTALL) -m 0644 $(@D)/include/gdc/gdc_api.h \
        $(STAGING_DIR)/usr/include/
endef

define AML_LIBGDC_INSTALL_CLEAN_CMDS
    $(MAKE) CC=$(TARGET_CC) -C $(@D) clean
endef

$(eval $(generic-package))
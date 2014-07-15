################################################################################
#
# mali_utility
#
################################################################################

MALI_EXAMPLES_VERSION = 2.0.0.9444
MALI_EXAMPLES_SITE = $(TOPDIR)/package/mali_examples/src
MALI_EXAMPLES_SITE_METHOD = local
MALI_EXAMPLES_CONF_OPT = -DTARGET=ARM -DCMAKE_INSTALL_PREFIX=/usr/share/arm

define MALI_EXAMPLES_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) DESTDIR=$(TARGET_DIR) install -C $(@D); \
	install -D -m 0755 $(@D)/simple-framework/libsimple-framework2.so $(TARGET_DIR)/usr/lib; \
	install -D -m 0755 $(@D)/simple-framework/libsimple-framework3.so $(TARGET_DIR)/usr/lib
endef

$(eval $(cmake-package))

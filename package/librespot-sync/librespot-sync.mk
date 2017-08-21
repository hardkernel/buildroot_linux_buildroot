######################################################################################
#
#librespot-sync
#
######################################################################################
LIBRESPOT_SYNC_SITE = $(TOPDIR)/package/librespot
LIBRESPOT_SYNC_METHOD = local

ifeq ($(BR2_aarch64),y)
TMP_NAME = librespot-64
else
TMP_NAME = librespot-32
endif

define LIBRESPOT_SYNC_INSTALL_TARGET_CMDS

	${INSTALL} -D -m 0755 $(TOPDIR)/package/librespot-sync/$(TMP_NAME)  $(TARGET_DIR)/usr/bin/librespot
endef

$(eval $(generic-package))

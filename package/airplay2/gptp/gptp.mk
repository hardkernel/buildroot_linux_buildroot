#############################################################
#
# gptp
#
#############################################################

GPTP_VERSION = master
GPTP_SITE_METHOD = git
GPTP_SITE = https://github.com/AVnu/gptp.git

define GPTP_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC=$(TARGET_CC) CXX=$(TARGET_CXX) -C $(@D)/linux/build/
endef

define GPTP_INSTALL_TARGET_CMDS
	$(INSTALL) -m 755 -D $(@D)/linux/build/obj/daemon_cl $(TARGET_DIR)/usr/bin/
	$(INSTALL) -m 755 -D $(GPTP_PKGDIR)/S92gptp $(TARGET_DIR)/etc/init.d/
endef

$(eval $(generic-package))



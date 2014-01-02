################################################################################
#
# cpio to archive target filesystem
#
################################################################################

ifeq ($(BR2_ROOTFS_DEVICE_CREATION_STATIC),y)

define ROOTFS_CPIO_ADD_INIT
        if [ ! -e $(TARGET_DIR)/init ]; then \
                ln -sf sbin/init $(TARGET_DIR)/init; \
        fi
endef

else
# devtmpfs does not get automounted when initramfs is used.
# Add a pre-init script to mount it before running init
ifeq ($(BR2_TARGET_ROOTFS_INITRAMFS), y)
define ROOTFS_CPIO_ADD_INIT
        if [ ! -e $(TARGET_DIR)/init ]; then \
                $(INSTALL) -m 0755 fs/cpio/init $(TARGET_DIR)/init; \
        fi
        if [ ! -e $(TARGET_DIR)/init2 ]; then \
                $(INSTALL) -m 0755 fs/cpio/init2 $(TARGET_DIR)/init2; \
        fi
endef
else
define ROOTFS_CPIO_ADD_INIT
        if [ ! -e $(TARGET_DIR)/init2 ]; then \
                $(INSTALL) -m 0755 fs/cpio/init2 $(TARGET_DIR)/init; \
        fi
endef
endif # BR2_TARGET_ROOTFS_INITRAMFS

PACKAGES_PERMISSIONS_TABLE += /dev/console c 622 0 0 5 1 - - -$(sep)

endif # BR2_ROOTFS_DEVICE_CREATION_STATIC

ROOTFS_CPIO_PRE_GEN_HOOKS += ROOTFS_CPIO_ADD_INIT

ifneq ($(BR2_TARGET_ROOTFS_INITRAMFS_LIST),"")
define ROOTFS_CPIO_CMD
	cd $(TARGET_DIR) && cat $(CONFIG_DIR)/$(BR2_TARGET_ROOTFS_INITRAMFS_LIST) | cpio --quiet -o -H newc > $@
endef
else
define ROOTFS_CPIO_CMD
	cd $(TARGET_DIR) && find . | cpio --quiet -o -H newc > $@
endef
endif # BR2_TARGET_ROOTFS_INITRAMFS_LIST

$(eval $(call ROOTFS_TARGET,cpio))

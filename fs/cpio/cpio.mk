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
define ROOTFS_CPIO_ADD_INIT
        if [ ! -e $(TARGET_DIR)/init ]; then \
                $(INSTALL) -m 0755 fs/cpio/init $(TARGET_DIR)/init; \
        fi
endef

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

ifeq ($(BR2_TARGET_ROOTFS_CPIO_GZIP),y)
	ROOTFS_CPIO = rootfs.cpio.gz
else
	ROOTFS_CPIO = rootfs.cpio
endif

WORD_NUMBER := $(words $(BR2_LINUX_KERNEL_INTREE_DTS_NAME))
ifeq ($(WORD_NUMBER),1)
mkbootimg: $(BINARIES_DIR)/$(LINUX_IMAGE_NAME) $(BINARIES_DIR)/$(ROOTFS_CPIO)
	@$(call MESSAGE,"Generating boot image")
	$(LINUX_DIR)/mkbootimg --kernel $(LINUX_IMAGE_PATH) --ramdisk $(BINARIES_DIR)/$(ROOTFS_CPIO) --second $(BINARIES_DIR)/$(KERNEL_DTBS) --output $(BINARIES_DIR)/boot.img
else
mkbootimg: $(BINARIES_DIR)/$(LINUX_IMAGE_NAME) $(BINARIES_DIR)/$(ROOTFS_CPIO)
	@$(call MESSAGE,"Generating boot image")
	linux/dtbTool -o $(BINARIES_DIR)/dt.img -p $(LINUX_DIR)/scripts/dtc/ $(BINARIES_DIR)/
	$(LINUX_DIR)/mkbootimg --kernel $(LINUX_IMAGE_PATH) --ramdisk  $(BINARIES_DIR)/$(ROOTFS_CPIO) --second $(BINARIES_DIR)/dt.img --output $(BINARIES_DIR)/boot.img
endif

ifeq ($(BR2_LINUX_KERNEL_AMLOGIC_DTD),y)
define ROOTFS_CPIO_POST_TARGETS
	mkbootimg
endef
endif

$(BINARIES_DIR)/rootfs.cpio.uboot: $(BINARIES_DIR)/rootfs.cpio host-uboot-tools
	$(MKIMAGE) -A $(MKIMAGE_ARCH) -T ramdisk \
		-C none -d $<$(ROOTFS_CPIO_COMPRESS_EXT) $@

ifeq ($(BR2_TARGET_ROOTFS_CPIO_UIMAGE),y)
ROOTFS_CPIO_POST_TARGETS += $(BINARIES_DIR)/rootfs.cpio.uboot
endif

$(eval $(call ROOTFS_TARGET,cpio))

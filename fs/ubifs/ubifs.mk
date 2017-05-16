################################################################################
#
# Build the ubifs root filesystem image
#
################################################################################

UBIFS_OPTS := -e $(BR2_TARGET_ROOTFS_UBIFS_LEBSIZE) -c $(BR2_TARGET_ROOTFS_UBIFS_MAXLEBCNT) -m $(BR2_TARGET_ROOTFS_UBIFS_MINIOSIZE)

ifeq ($(BR2_TARGET_ROOTFS_UBIFS_RT_ZLIB),y)
UBIFS_OPTS += -x zlib
endif
ifeq ($(BR2_TARGET_ROOTFS_UBIFS_RT_LZO),y)
UBIFS_OPTS += -x lzo
endif
ifeq ($(BR2_TARGET_ROOTFS_UBIFS_RT_NONE),y)
UBIFS_OPTS += -x none
endif

UBIFS_OPTS += $(call qstrip,$(BR2_TARGET_ROOTFS_UBIFS_OPTS))

ROOTFS_UBIFS_DEPENDENCIES = host-mtd

define ROOTFS_UBIFS_CMD
	$(HOST_DIR)/usr/sbin/mkfs.ubifs -d $(TARGET_DIR) $(UBIFS_OPTS) -o $@
endef

DEVICE_DIR := $(patsubst "%",%,$(BR2_ROOTFS_OVERLAY))
UPGRADE_DIR := $(DEVICE_DIR)../upgrade
ifeq ($(BR2_TARGET_USBTOOL_AMLOGIC),y)
ifeq ($(BR2_TARGET_UBOOT_AMLOGIC_2015),y)
rootfs-usb-image-pack-ubifs:
	cp -rf $(UPGRADE_DIR)/* $(BINARIES_DIR)
	BINARIES_DIR=$(BINARIES_DIR) \
	TOOL_DIR=$(HOST_DIR)/usr/bin \
	$(HOST_DIR)/usr/bin/aml_upgrade_pkg_gen.sh \
	$(BR2_TARGET_UBOOT_PLATFORM) $(BR2_TARGET_UBOOT_ENCRYPTION)
else #BR2_TARGET_UBOOT_AMLOGIC_2015
rootfs-usb-image-pack-ubifs:
	cp -v -f $(UPGRADE_DIR)/platform.conf $(BINARIES_DIR)
	$(HOST_DIR)/usr/bin/aml_image_v2_packer -r $(BINARIES_DIR)/usb_burn_package.conf $(BINARIES_DIR)/ $(BINARIES_DIR)/aml_upgrade_package.img
endif #BR2_TARGET_UBOOT_AMLOGIC_2015
ROOTFS_UBIFS_POST_TARGETS += rootfs-usb-image-pack-ubifs
endif #BR2_TARGET_USBTOOL_AMLOGIC

ifeq ($(BR2_TARGET_UBOOT_AMLOGIC_2015),y)
SD_BOOT = $(BINARIES_DIR)/u-boot.bin.sd.bin
else
SD_BOOT = $(BINARIES_DIR)/u-boot.bin
endif
$(eval $(call ROOTFS_TARGET,ubifs))

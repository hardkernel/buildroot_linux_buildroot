################################################################################
#
# Build the ext2 root filesystem image
#
################################################################################
EXT2_OPTS = -G $(BR2_TARGET_ROOTFS_EXT2_GEN) -R $(BR2_TARGET_ROOTFS_EXT2_REV)

ifneq ($(strip $(BR2_TARGET_ROOTFS_EXT2_BLOCKS)),0)
EXT2_OPTS += -b $(BR2_TARGET_ROOTFS_EXT2_BLOCKS)
endif
EXT2_OPTS += -B $(BR2_TARGET_ROOTFS_EXT2_EXTRA_BLOCKS)

ifneq ($(strip $(BR2_TARGET_ROOTFS_EXT2_INODES)),0)
EXT2_OPTS += -i $(BR2_TARGET_ROOTFS_EXT2_INODES)
endif
EXT2_OPTS += -I $(BR2_TARGET_ROOTFS_EXT2_EXTRA_INODES)

ifneq ($(strip $(BR2_TARGET_ROOTFS_EXT2_RESBLKS)),0)
EXT2_OPTS += -r $(BR2_TARGET_ROOTFS_EXT2_RESBLKS)
endif

# qstrip results in stripping consecutive spaces into a single one. So the
# variable is not qstrip-ed to preserve the integrity of the string value.
EXT2_LABEL := $(subst ",,$(BR2_TARGET_ROOTFS_EXT2_LABEL))
ifneq ($(EXT2_LABEL),)
EXT2_OPTS += -l "$(EXT2_LABEL)"
endif

ROOTFS_EXT2_DEPENDENCIES = host-mke2img host-parted

define ROOTFS_EXT2_CMD
	PATH=$(BR_PATH) mke2img -d $(TARGET_DIR) $(EXT2_OPTS) -o $@
endef

rootfs-ext2-symlink:
	ln -sf rootfs.ext2$(ROOTFS_EXT2_COMPRESS_EXT) $(BINARIES_DIR)/rootfs.ext$(BR2_TARGET_ROOTFS_EXT2_GEN)$(ROOTFS_EXT2_COMPRESS_EXT)

.PHONY: rootfs-ext2-symlink

ifneq ($(BR2_TARGET_ROOTFS_EXT2_GEN),2)
ROOTFS_EXT2_POST_TARGETS += rootfs-ext2-symlink
endif

DEVICE_DIR := $(patsubst "%",%,$(BR2_ROOTFS_OVERLAY))
UPGRADE_DIR := $(patsubst "%",%,$(BR2_ROOTFS_UPGRADE_DIR))
UPGRADE_DIR_OVERLAY := $(patsubst "%",%,$(BR2_ROOTFS_UPGRADE_DIR_OVERLAY))
ifeq ($(BR2_TARGET_USBTOOL_AMLOGIC),y)
ifeq ($(filter y,$(BR2_TARGET_UBOOT_AMLOGIC_2015) $(BR2_TARGET_UBOOT_AMLOGIC_REPO)),y)
ifneq ($(UPGRADE_DIR_OVERLAY),)
rootfs-usb-image-pack:
	cp -rf $(UPGRADE_DIR)/* $(BINARIES_DIR)
	cp -rf $(UPGRADE_DIR_OVERLAY)/* $(BINARIES_DIR)
	BINARIES_DIR=$(BINARIES_DIR) \
	TOOL_DIR=$(HOST_DIR)/usr/bin \
	$(HOST_DIR)/usr/bin/aml_upgrade_pkg_gen.sh \
	$(BR2_TARGET_UBOOT_PLATFORM) $(BR2_TARGET_UBOOT_ENCRYPTION) $(BR2_PACKAGE_SWUPDATE_AB_SUPPORT)
else
rootfs-usb-image-pack:
	cp -rf $(UPGRADE_DIR)/* $(BINARIES_DIR)
	BINARIES_DIR=$(BINARIES_DIR) \
	TOOL_DIR=$(HOST_DIR)/usr/bin \
	$(HOST_DIR)/usr/bin/aml_upgrade_pkg_gen.sh \
	$(BR2_TARGET_UBOOT_PLATFORM) $(BR2_TARGET_UBOOT_ENCRYPTION) $(BR2_PACKAGE_SWUPDATE_AB_SUPPORT)
endif

else #BR2_TARGET_UBOOT_AMLOGIC_2015
rootfs-usb-image-pack:
	cp -rf $(UPGRADE_DIR)/* $(BINARIES_DIR)
	$(HOST_DIR)/usr/bin/aml_image_v2_packer_new -r $(BINARIES_DIR)/aml_upgrade_package.conf $(BINARIES_DIR)/ $(BINARIES_DIR)/aml_upgrade_package.img
endif #BR2_TARGET_UBOOT_AMLOGIC_2015
ROOTFS_EXT2_POST_TARGETS += rootfs-usb-image-pack
endif #BR2_TARGET_USBTOOL_AMLOGIC

RECOVERY_OTA_DIR := $(patsubst "%",%,$(BR2_RECOVERY_OTA_DIR))
ifneq ($(RECOVERY_OTA_DIR),)
rootfs-ota-swu-pack-ext4fs:
	$(INSTALL) -m 0755 $(RECOVERY_OTA_DIR)/../swu/* $(BINARIES_DIR)
	$(INSTALL) -m 0644 $(RECOVERY_OTA_DIR)/sw-description-emmc $(BINARIES_DIR)/sw-description
	$(INSTALL) -m 0644 $(RECOVERY_OTA_DIR)/sw-description-emmc-increment $(BINARIES_DIR)/sw-description-increment
	$(INSTALL) -m 0644 $(RECOVERY_OTA_DIR)/ota-package-filelist-emmc $(BINARIES_DIR)/ota-package-filelist
	$(BINARIES_DIR)/ota_package_create.sh
ROOTFS_EXT2_POST_TARGETS += rootfs-ota-swu-pack-ext4fs
endif

ifeq ($(BR2_TARGET_UBOOT_AMLOGIC_2015),y)
SD_BOOT = $(BINARIES_DIR)/u-boot.bin.sd.bin
else
SD_BOOT = $(BINARIES_DIR)/u-boot.bin
endif
mbr-image:
	@$(call MESSAGE,"Creating mbr image")
	filesz=`stat -c '%s' $(BINARIES_DIR)/rootfs.ext2`; \
	dd if=/dev/zero of=$(BINARIES_DIR)/disk.img bs=512 count=$$(($$filesz/512+2048)); \
	parted $(BINARIES_DIR)/disk.img mklabel msdos; \
	parted $(BINARIES_DIR)/disk.img mkpart primary ext2 2048s 100%; \
	dd if=$(BINARIES_DIR)/rootfs.ext2 of=$(BINARIES_DIR)/disk.img bs=512 count=$$(($$filesz/512)) seek=2048;

odroid-uboot:
	dd if=$(BINARIES_DIR)/bl1.bin.hardkernel of=$(BINARIES_DIR)/disk.img bs=1 count=442 conv=notrunc; \
	dd if=$(BINARIES_DIR)/bl1.bin.hardkernel of=$(BINARIES_DIR)/disk.img bs=512 skip=1 seek=1 conv=notrunc; \
	dd if=$(BINARIES_DIR)/u-boot.bin of=$(BINARIES_DIR)/disk.img bs=512 seek=64 conv=notrunc;

amlogic-uboot:
	dd if=$(SD_BOOT) of=$(BINARIES_DIR)/disk.img bs=1 count=442 conv=notrunc; \
	dd if=$(SD_BOOT) of=$(BINARIES_DIR)/disk.img bs=512 skip=1 seek=1 conv=notrunc;

ifeq ($(BR2_TARGET_MBR_IMAGE),y)
ROOTFS_EXT2_POST_TARGETS += mbr-image
ifeq ($(BR2_TARGET_UBOOT_ODROID),y)
ROOTFS_EXT2_POST_TARGETS += odroid-uboot
else
ROOTFS_EXT2_POST_TARGETS += amlogic-uboot
endif
endif

$(eval $(call ROOTFS_TARGET,ext2))

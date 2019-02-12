#!/bin/sh

BOARD_DIR="$(dirname $0)"
COMMON_DIR=${BOARD_DIR}

GENIMAGE_CFG="${BOARD_DIR}/genimage.cfg"
GENIMAGE_TMP="${BUILD_DIR}/genimage.tmp"

rm -rf "${GENIMAGE_TMP}"

cp ${BOARD_DIR}/boot.cmd ${BINARIES_DIR}/

echo "ODROIDN2-UBOOT-CONFIG" > ${BINARIES_DIR}/boot.ini
cat ${BINARIES_DIR}/boot.cmd >> ${BINARIES_DIR}/boot.ini

kernel_addr_r="0x01080000"

${HOST_DIR}/usr/bin/mkimage -A arm64 -T kernel -O linux -C gzip \
	-a ${kernel_addr_r} -e ${kernel_addr_r} \
	-d ${BINARIES_DIR}/Image.gz \
	${BINARIES_DIR}/uImage

#
# Generating SPI boot image
#
SPIBOOT_IMAGE=${BINARIES_DIR}/spiboot.img

rm -f ${SPIBOOT_IMAGE}
dd if=/dev/zero of=${SPIBOOT_IMAGE} bs=1M count=8 conv=fsync,notrunc
dd if=${BINARIES_DIR}/u-boot.bin of=${SPIBOOT_IMAGE} bs=512 seek=0 conv=fsync,notrunc
dd if=${BINARIES_DIR}/meson64_odroidn2_spibios.dtb of=${SPIBOOT_IMAGE} bs=512 seek=2048 conv=fsync,notrunc
dd if=${BINARIES_DIR}/uImage of=${SPIBOOT_IMAGE} bs=512 seek=2248 conv=fsync,notrunc
dd if=${BINARIES_DIR}/rootfs.cpio.uboot of=${SPIBOOT_IMAGE} bs=512 seek=10038 conv=fsync,notrunc

#
# Dropping MMC boot image
#
genimage --config "${GENIMAGE_CFG}"	\
	--rootpath "${TARGET_DIR}"	\
	--tmppath "${GENIMAGE_TMP}"	\
	--inputpath "${BINARIES_DIR}"	\
	--outputpath "${BINARIES_DIR}"
#
# Generating MMC boot image
#
dd if=${BINARIES_DIR}/u-boot.bin of=${BINARIES_DIR}/sdcard.img bs=512 seek=1 conv=fsync,notrunc

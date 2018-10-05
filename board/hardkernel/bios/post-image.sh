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

${HOST_DIR}/usr/bin/mkimage -A arm64 -T script -O linux -C none \
	-d ${BINARIES_DIR}/boot.cmd ${BINARIES_DIR}/boot.scr

genimage --config "${GENIMAGE_CFG}"	\
	--rootpath "${TARGET_DIR}"	\
	--tmppath "${GENIMAGE_TMP}"	\
	--inputpath "${BINARIES_DIR}"	\
	--outputpath "${BINARIES_DIR}"

#
# Generating MMC boot image
#
dd if=${BINARIES_DIR}/u-boot.bin of=${BINARIES_DIR}/sdcard.img bs=512 seek=1 conv=fsync,notrunc

#
# Generating SPI boot image
#
rm -f ${BINARIES_DIR}/spiboot.img
dd if=/dev/zero of=${BINARIES_DIR}/spiboot.img bs=1M count=8 conv=fsync,notrunc
dd if=${BINARIES_DIR}/u-boot.bin of=${BINARIES_DIR}/spiboot.img bs=512 seek=1 conv=fsync,notrunc
dd if=${BINARIES_DIR}/uImage of=${BINARIES_DIR}/spiboot.img bs=512 seek=1984 conv=fsync,notrunc
dd if=${BINARIES_DIR}/s922d_odroidn2.dtb of=${BINARIES_DIR}/spiboot.img bs=512 seek=9234 conv=fsync,notrunc
dd if=${BINARIES_DIR}/rootfs.cpio.uboot of=${BINARIES_DIR}/spiboot.img bs=512 seek=9343 conv=fsync,notrunc

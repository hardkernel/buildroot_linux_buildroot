#!/bin/sh

BOARD_DIR="$(dirname $0)"
COMMON_DIR=${BOARD_DIR}/../common

GENIMAGE_CFG="${BOARD_DIR}/genimage.cfg"
GENIMAGE_TMP="${BUILD_DIR}/genimage.tmp"

rm -rf "${GENIMAGE_TMP}"

cp ${COMMON_DIR}/logo.bmp.gz ${BINARIES_DIR}/
cp ${BOARD_DIR}/boot.cmd ${BINARIES_DIR}/

echo "ODROIDN2-UBOOT-CONFIG" > ${BINARIES_DIR}/boot.ini
cat ${BINARIES_DIR}/boot.cmd >> ${BINARIES_DIR}/boot.ini

${HOST_DIR}/usr/bin/mkimage -A arm64 -T script -C none \
	-d ${BINARIES_DIR}/boot.cmd ${BINARIES_DIR}/boot.scr

genimage                           \
	--rootpath "${TARGET_DIR}"     \
	--tmppath "${GENIMAGE_TMP}"    \
	--inputpath "${BINARIES_DIR}"  \
	--outputpath "${BINARIES_DIR}" \
	--config "${GENIMAGE_CFG}"

dd if=${BINARIES_DIR}/u-boot.bin of=${BINARIES_DIR}/sdcard.img bs=512 seek=1 conv=fsync,notrunc

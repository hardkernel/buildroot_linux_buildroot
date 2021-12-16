#!/bin/sh

BOARD_DIR="$(dirname $0)"
COMMON_DIR=${BOARD_DIR}/../common
ROOTUUID=$(uuidgen)

OSIMAGE=${BINARIES_DIR}/sdcard.img

. ${COMMON_DIR}/functions

cp ${COMMON_DIR}/logo.bmp.gz ${BINARIES_DIR}/
cp ${BOARD_DIR}/boot.cmd ${BINARIES_DIR}/
cp ${BOARD_DIR}/genimage.cfg ${BINARIES_DIR}/

sed -i "s/@@ROOTUUID@@/${ROOTUUID}/g" \
	${BINARIES_DIR}/boot.cmd \
	${BINARIES_DIR}/genimage.cfg

echo "ODROIDC4-UBOOT-CONFIG" > ${BINARIES_DIR}/boot.ini
cat ${BINARIES_DIR}/boot.cmd >> ${BINARIES_DIR}/boot.ini

${HOST_DIR}/usr/bin/mkimage -A arm64 -T script -C none \
	-d ${BINARIES_DIR}/boot.cmd ${BINARIES_DIR}/boot.scr

yes | tune2fs -U ${ROOTUUID} ${BINARIES_DIR}/rootfs.ext4 || exit 1

GENIMAGE_TMP="${BUILD_DIR}/genimage.tmp" \
	&& rm -rf "${GENIMAGE_TMP}"
genimage                           \
	--rootpath "${TARGET_DIR}"     \
	--tmppath "${GENIMAGE_TMP}"    \
	--inputpath "${BINARIES_DIR}"  \
	--outputpath "${BINARIES_DIR}" \
	--config "${BINARIES_DIR}/genimage.cfg"

reset_uuid 2 ${ROOTUUID} ${BINARIES_DIR}/sdcard.img

dd if=${BINARIES_DIR}/u-boot.bin of=${BINARIES_DIR}/sdcard.img bs=512 seek=1 conv=fsync,notrunc

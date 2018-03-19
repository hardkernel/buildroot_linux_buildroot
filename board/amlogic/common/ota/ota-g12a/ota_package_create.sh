#!/bin/bash
CONTAINER_VER="1.0"
PRODUCT_NAME="aml-software"
cd ${BINARIES_DIR}

platform=$1
if [ ${platform} = "emmc" ];then
	HASH_FILES="u-boot.bin dtb.img boot.img rootfs.ext2.img2simg update.sh"
	FILES="sw-description sw-description.sig u-boot.bin dtb.img boot.img rootfs.ext2.img2simg update.sh"
else
	HASH_FILES="u-boot.bin.usb.bl2 u-boot.bin.usb.tpl dtb.img boot.img rootfs.ubifs update.sh"
	FILES="sw-description sw-description.sig u-boot.bin.usb.bl2 u-boot.bin.usb.tpl dtb.img boot.img rootfs.ubifs update.sh"
fi

for i in $HASH_FILES;do
	value_tmp=$(sha256sum $i);
	value_sha256=${value_tmp:0:64};
	key_value="sha256 = \""$value_sha256"\"";
	sed -i "/filename = \"$i\";/a\\\t\t\t$key_value"  sw-description;
done

openssl dgst -sha256 -sign swupdate-priv.pem sw-description > sw-description.sig
for i in $FILES;do
	echo $i;done | cpio -ov -H crc >  software.swu
echo software.swu  | cpio -ov -H crc >  ${PRODUCT_NAME}_${CONTAINER_VER}.swu
cd -
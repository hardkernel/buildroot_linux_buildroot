#
# U-boot booting script
#

setenv videoargs "logo=osd0,loaded,0x3d800000 vout=1080p60hz,enable hdmimode=1080p60hz osd_reverse=0 video_reverse=0"
setenv bootargs "root=/dev/ram0 init=/int console=tty0 console=ttyS0,115200 no_console_suspend earlyprintk=aml-uart,0xff803000 ramoops.pstore_en1=1 ramoops.record_size=0x8000 ramoops.console_size=0x4000 console=tty0 ${videoargs}"
setenv bootargs "${bootargs} quiet"

setenv fdt_addr_r "0x1000000"
setenv kernel_addr_r "0x1080000"
setenv kernel_addr_load "0x4000000"
setenv ramdisk_addr_r "0x3080000"

setenv fdtfile "meson64_odroidn2.dtb"

load mmc ${devno}:1 ${kernel_addr_load} uImage
load mmc ${devno}:1 ${ramdisk_addr_r} rootfs.cpio.uboot
load mmc ${devno}:1 ${fdt_addr_r} ${fdtfile}

if test -e mmc ${devno}:1 spiboot.img; then fdt addr ${fdt_addr_r}; fdt resize; fdt set /emmc@ffe07000 status "disabled"; fdt set /soc/cbus/spifc@14000 status "okay"; fi

bootm ${kernel_addr_load} ${ramdisk_addr_r} ${fdt_addr_r}

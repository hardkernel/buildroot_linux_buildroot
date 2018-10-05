#
# U-boot booting script
#

setenv bootargs "root=/dev/ram0 init=/int console=tty0 console=ttyS0,115200 no_console_suspend earlyprintk=aml-uart,0xff803000"

setenv fdt_addr_r "0x1000000"
setenv kernel_addr_r "0x1080000"
setenv kernel_addr_load "0x4000000"
setenv ramdisk_addr_r "0x3080000"

setenv fdtfile "s922d_odroidn2.dtb"

load mmc ${devno}:1 ${kernel_addr_load} uImage
&& load mmc ${devno}:1 ${ramdisk_addr_r} rootfs.cpio.uboot
&& load mmc ${devno}:1 ${fdt_addr_r} ${fdtfile}
&& bootm ${kernel_addr_load} ${ramdisk_addr_r} ${fdt_addr_r}

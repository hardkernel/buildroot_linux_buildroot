#
# U-boot booting script
#

setenv bootargs "root=/dev/mmcblk0p2 rootfstype=ext4 rootwait init=/sbin/init console=ttyS0,115200 no_console_suspend earlyprintk=aml-uart,0xff803000 ramoops.pstore_en=1 ramoops.record_size=0x8000 ramoops.console_size=0x4000 vout=${outputmode} hdmimode=${hdmimode} cvbsmode=${cvbsmode} vdaccfg=${vdac_config}"

setenv fdt_addr_r "0x1000000"
setenv kernel_addr_r "0x1080000"
setenv kernel_addr_load "0x3000000"
setenv ramdisk_addr_r "0x3080000"

setenv fdtfile "meson64_odroid${variant}.dtb"

fatload mmc ${devno}:1 ${kernel_addr_load} Image.gz
&& unzip ${kernel_addr_load} ${kernel_addr_r}
&& fatload mmc ${devno}:1 ${fdt_addr_r} ${fdtfile}
&& if test -e mmc ${devno} initrd.gz; then fatload mmc ${devno}:1 ${ramdisk_addr_r} initrd.gz; else setenv ramdisk_addr_r "-"; fi
&& booti ${kernel_addr_r} ${ramdisk_addr_r} ${fdt_addr_r}

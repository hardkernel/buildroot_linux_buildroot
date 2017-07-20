#############################################################
#
# Amlogic UBOOT CodeSourcery Toolchain
#
#############################################################

AML_UBOOT_TOOLCHAIN_GCC_LINARO_ARM_NONE_SOURCE          = gcc-arm-none-eabi-6-2017-q2-update-linux.tar.bz2
AML_UBOOT_TOOLCHAIN_GCC_LINARO_ARM_NONE_SITE            = http://openlinux.amlogic.com:8000/deploy
AML_UBOOT_TOOLCHAIN_GCC_LINARO_ARM_NONE_STAGING = YES

AML_UBOOT_TOOLCHAIN_GCC_LINARO_ARM_NONE_INSTALL_DIR = $(HOST_DIR)/usr/gcc-arm-none-eabi-6-2017-q2-update

define AML_UBOOT_TOOLCHAIN_GCC_LINARO_ARM_NONE_INSTALL_TARGET_CMDS
	rm -rf $(AML_UBOOT_TOOLCHAIN_GCC_LINARO_ARM_NONE_INSTALL_DIR)/*
	mkdir -p $(AML_UBOOT_TOOLCHAIN_GCC_LINARO_ARM_NONE_INSTALL_DIR)
	mv $(@D)/* $(AML_UBOOT_TOOLCHAIN_GCC_LINARO_ARM_NONE_INSTALL_DIR)/
endef

$(eval $(generic-package))

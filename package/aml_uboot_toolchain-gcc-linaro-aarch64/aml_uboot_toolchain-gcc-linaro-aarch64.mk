#############################################################
#
# Amlogic UBOOT GCC Linaro Aarch64 Toolchain
#
#############################################################

AML_UBOOT_TOOLCHAIN_GCC_LINARO_AARCH64_VERSION         = 4.8-2013.11
AML_UBOOT_TOOLCHAIN_GCC_LINARO_AARCH64_SOURCE          = gcc-linaro-aarch64-none-elf-$(AML_UBOOT_TOOLCHAIN_GCC_LINARO_AARCH64_VERSION)_linux.tar
AML_UBOOT_TOOLCHAIN_GCC_LINARO_AARCH64_SITE            = http://openlinux.amlogic.com:8000/deploy
AML_UBOOT_TOOLCHAIN_GCC_LINARO_AARCH64_INSTALL_STAGING = YES

AML_UBOOT_TOOLCHAIN_GCC_LINARO_AARCH64_INSTALL_DIR = $(HOST_DIR)/usr/aarch64-buildroot-none-gnu

define AML_UBOOT_TOOLCHAIN_GCC_LINARO_AARCH64_INSTALL_TARGET_CMDS
	rm -rf $(AML_UBOOT_TOOLCHAIN_GCC_LINARO_AARCH64_INSTALL_DIR)/*
	mkdir -p $(AML_UBOOT_TOOLCHAIN_GCC_LINARO_AARCH64_INSTALL_DIR)
	mv $(@D)/* $(AML_UBOOT_TOOLCHAIN_GCC_LINARO_AARCH64_INSTALL_DIR)/
endef

$(eval $(generic-package))

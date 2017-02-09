#############################################################
#
# Amlogic UBOOT arc Toolchain
#
#############################################################

AML_UBOOT_TOOLCHAIN_ARC_SOURCE          = arc-4.8-amlogic-20130904-r2.tar.gz
AML_UBOOT_TOOLCHAIN_ARC_SITE            = http://openlinux.amlogic.com:8000/deploy
AML_UBOOT_TOOLCHAIN_ARC_INSTALL_STAGING = YES

AML_UBOOT_TOOLCHAIN_ARC_INSTALL_DIR = $(HOST_DIR)/usr/arc-4.8-amlogic-20130904-r2

define AML_UBOOT_TOOLCHAIN_ARC_INSTALL_TARGET_CMDS
	rm -rf $(AML_UBOOT_TOOLCHAIN_ARC_INSTALL_DIR)/*
	mkdir -p $(AML_UBOOT_TOOLCHAIN_ARC_INSTALL_DIR)
	mv $(@D)/* $(AML_UBOOT_TOOLCHAIN_ARC_INSTALL_DIR)/
endef

$(eval $(generic-package))

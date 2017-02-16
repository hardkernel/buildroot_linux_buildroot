#############################################################
#
# Amlogic UBOOT CodeSourcery Toolchain
#
#############################################################

AML_UBOOT_TOOLCHAIN_CODESOURCERY_SOURCE          = CodeSourcery.tar.gz
AML_UBOOT_TOOLCHAIN_CODESOURCERY_SITE            = http://openlinux.amlogic.com:8000/deploy
AML_UBOOT_TOOLCHAIN_CODESOURCERY_INSTALL_STAGING = YES

AML_UBOOT_TOOLCHAIN_CODESOURCERY_INSTALL_DIR = $(HOST_DIR)/usr/codesourcery

define AML_UBOOT_TOOLCHAIN_CODESOURCERY_INSTALL_TARGET_CMDS
	rm -rf $(AML_UBOOT_TOOLCHAIN_CODESOURCERY_INSTALL_DIR)/*
	mkdir -p $(AML_UBOOT_TOOLCHAIN_CODESOURCERY_INSTALL_DIR)
	mv $(@D)/* $(AML_UBOOT_TOOLCHAIN_CODESOURCERY_INSTALL_DIR)/
endef

$(eval $(generic-package))

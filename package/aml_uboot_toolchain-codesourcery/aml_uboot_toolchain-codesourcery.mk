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

define AML_UBOOT_TOOCHAIN_CODESOURCERY_CHOICE_DOWNLOAD_PATH
	bash $(TOPDIR)/build/download.sh $(TOPDIR)/dl/  CodeSourcery.tar.gz
endef

AML_UBOOT_TOOLCHAIN_CODESOURCERY_PRE_DOWNLOAD_HOOKS += AML_UBOOT_TOOCHAIN_CODESOURCERY_CHOICE_DOWNLOAD_PATH

$(eval $(generic-package))

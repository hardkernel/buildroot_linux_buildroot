################################################################################
#
# amlogic nand ftl
#
################################################################################

AML_NAND_VERSION = $(call qstrip,$(BR2_PACKAGE_AML_NAND_VERSION))
AML_NAND_SITE = $(call qstrip,$(BR2_PACKAGE_AML_NAND_GIT_URL))

define AML_NAND_BUILD_CMDS
	@echo ============================================
endef

define AML_NAND_INSTALL_TARGET_CMDS
	@echo **********************************************
endef

$(eval $(generic-package))

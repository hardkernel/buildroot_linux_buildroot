################################################################################
#
# amlogic 8192cu driver
#
################################################################################

RTK8192CU_VERSION = $(call qstrip,$(BR2_PACKAGE_RTK8192CU_GIT_VERSION))
RTK8192CU_SITE = $(call qstrip,$(BR2_PACKAGE_RTK8192CU_GIT_REPO_URL))
define RTK8192CU_BUILD_CMDS
	mkdir -p $(LINUX_DIR)/../hardware/wifi/realtek/drivers;
	ln -sf $(RTK8192CU_DIR) $(LINUX_DIR)/../hardware/wifi/realtek/drivers/8192cu
endef
$(eval $(generic-package))

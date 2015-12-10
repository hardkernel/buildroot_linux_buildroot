################################################################################
#
# amlogic 8192eu driver
#
################################################################################

RTK8192EU_VERSION = $(call qstrip,$(BR2_PACKAGE_RTK8192EU_GIT_VERSION))
RTK8192EU_SITE = $(call qstrip,$(BR2_PACKAGE_RTK8192EU_GIT_REPO_URL))
define RTK8192EU_BUILD_CMDS
	mkdir -p $(LINUX_DIR)/../hardware/wifi/realtek/drivers;
	ln -sf $(RTK8192EU_DIR) $(LINUX_DIR)/../hardware/wifi/realtek/drivers/8192eu
endef
$(eval $(generic-package))

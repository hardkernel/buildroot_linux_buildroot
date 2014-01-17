################################################################################
#
# wifi-fw
#
################################################################################

WIFI_FW_VERSION = $(call qstrip,$(BR2_PACKAGE_WIFI_CUSTOM_GIT_VERSION))
WIFI_FW_SITE = $(call qstrip,$(BR2_PACKAGE_WIFI_FW_CUSTOM_GIT_REPO_URL))
WIFI_MODULE = $(call qstrip,$(BR2_PACKAGE_WIFI_FW_WIFI_MODULE))

ifeq ($(BR2_PACKAGE_WIFI_FW_CUSTOM_GIT_REPO_URL),"jb-mr1-amlogic")

ifeq ($(WIFI_MODULE),"AP6181")
define WIFI_FW_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/etc/wifi/
	$(INSTALL) -D -m 0644 $(@D)/AP6xxx/AP6181/Wi-Fi/*.bin $(TARGET_DIR)/etc/wifi/
	$(INSTALL) -D -m 0644 $(@D)/AP6xxx/AP6181/Wi-Fi/nvram_ap6181.txt $(TARGET_DIR)/etc/wifi/nvram.txt
endef
endif

ifeq ($(WIFI_MODULE),"AP6210")
define WIFI_FW_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/etc/wifi/
	$(INSTALL) -D -m 0644 $(@D)/AP6xxx/AP6210/Wi-Fi/*.bin $(TARGET_DIR)/etc/wifi/
	$(INSTALL) -D -m 0644 $(@D)/AP6xxx/AP6210/Wi-Fi/nvram_ap6210.txt $(TARGET_DIR)/etc/wifi/nvram.txt
	$(INSTALL) -D -m 0644 $(@D)/AP6xxx/AP6210/BT/*.hcd $(TARGET_DIR)/etc/wifi/
endef
endif

ifeq ($(WIFI_MODULE),"AP6476")
define WIFI_FW_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/etc/wifi/
	$(INSTALL) -D -m 0644 $(@D)/AP6xxx/AP6476/Wi-Fi/*.bin $(TARGET_DIR)/etc/wifi/
	$(INSTALL) -D -m 0644 $(@D)/AP6xxx/AP6476/Wi-Fi/nvram_ap6476.txt $(TARGET_DIR)/etc/wifi/nvram.txt
	$(INSTALL) -D -m 0644 $(@D)/AP6xxx/AP6476/GPS/*.hcd $(TARGET_DIR)/etc/wifi/
endef
endif

ifeq ($(WIFI_MODULE),"AP6493")
define WIFI_FW_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/etc/wifi/
	$(INSTALL) -D -m 0644 $(@D)/AP6xxx/AP6493/Wi-Fi/*.bin $(TARGET_DIR)/etc/wifi/
	$(INSTALL) -D -m 0644 $(@D)/AP6xxx/AP6493/Wi-Fi/nvram_ap6493.txt $(TARGET_DIR)/etc/wifi/nvram.txt
	$(INSTALL) -D -m 0644 $(@D)/AP6xxx/AP6493/BT/*.hcd $(TARGET_DIR)/etc/wifi/
endef
endif

ifeq ($(WIFI_MODULE),"AP6330")
define WIFI_FW_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/etc/wifi/
	$(INSTALL) -D -m 0644 $(@D)/AP6xxx/AP6330/Wi-Fi/*.bin $(TARGET_DIR)/etc/wifi/
	$(INSTALL) -D -m 0644 $(@D)/AP6xxx/AP6330/Wi-Fi/nvram_ap6330.txt $(TARGET_DIR)/etc/wifi/nvram.txt
	$(INSTALL) -D -m 0644 $(@D)/AP6xxx/AP6330/BT/*.hcd $(TARGET_DIR)/etc/wifi/
endef
endif

else #BR2_PACKAGE_WIFI_FW_CUSTOM_GIT_REPO_URL

ifeq ($(WIFI_MODULE),"AP6181")
define WIFI_FW_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/etc/wifi/
	$(INSTALL) -D -m 0644 $(@D)/bcm_ampak/config/AP6181/Wi-Fi/*.bin $(TARGET_DIR)/etc/wifi/
	$(INSTALL) -D -m 0644 $(@D)/bcm_ampak/config/AP6181/Wi-Fi/nvram_ap6181.txt $(TARGET_DIR)/etc/wifi/nvram.txt
endef
endif

ifeq ($(WIFI_MODULE),"AP6210")
define WIFI_FW_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/etc/wifi/
	$(INSTALL) -D -m 0644 $(@D)/bcm_ampak/config/AP6210/Wi-Fi/*.bin $(TARGET_DIR)/etc/wifi/
	$(INSTALL) -D -m 0644 $(@D)/bcm_ampak/config/AP6210/Wi-Fi/nvram_ap6210.txt $(TARGET_DIR)/etc/wifi/nvram.txt
	$(INSTALL) -D -m 0644 $(@D)/bcm_ampak/config/AP6210/BT/*.hcd $(TARGET_DIR)/etc/wifi/
endef
endif

ifeq ($(WIFI_MODULE),"AP6476")
define WIFI_FW_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/etc/wifi/
	$(INSTALL) -D -m 0644 $(@D)/bcm_ampak/config/AP6476/Wi-Fi/*.bin $(TARGET_DIR)/etc/wifi/
	$(INSTALL) -D -m 0644 $(@D)/bcm_ampak/config/AP6476/Wi-Fi/nvram_ap6476.txt $(TARGET_DIR)/etc/wifi/nvram.txt
	$(INSTALL) -D -m 0644 $(@D)/bcm_ampak/config/AP6476/GPS/*.hcd $(TARGET_DIR)/etc/wifi/
endef
endif

ifeq ($(WIFI_MODULE),"AP6493")
define WIFI_FW_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/etc/wifi/
	$(INSTALL) -D -m 0644 $(@D)/bcm_ampak/config/AP6493/Wi-Fi/*.bin $(TARGET_DIR)/etc/wifi/
	$(INSTALL) -D -m 0644 $(@D)/bcm_ampak/config/AP6493/Wi-Fi/nvram_ap6493.txt $(TARGET_DIR)/etc/wifi/nvram.txt
endef
endif

ifeq ($(WIFI_MODULE),"AP6330")
define WIFI_FW_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/etc/wifi/
	$(INSTALL) -D -m 0644 $(@D)/bcm_ampak/config/AP6330/Wi-Fi/*.bin $(TARGET_DIR)/etc/wifi/
	$(INSTALL) -D -m 0644 $(@D)/bcm_ampak/config/AP6330/Wi-Fi/nvram_ap6476.txt $(TARGET_DIR)/etc/wifi/nvram.txt
	$(INSTALL) -D -m 0644 $(@D)/bcm_ampak/config/AP6330/BT/*.hcd $(TARGET_DIR)/etc/wifi/
endef
endif

endif #BR2_PACKAGE_WIFI_FW_CUSTOM_GIT_REPO_URL

$(eval $(generic-package))

#############################################################
#
# aml utility
#
#############################################################
AML_UTIL_VERSION = 0.1
AML_UTIL_SITE = $(TOPDIR)/package/aml_util/src
AML_UTIL_SITE_METHOD = local

AML_UTIL_DEPENDENCIES += linux libusb

BCM := FALSE
RTK := FALSE

# AP6242 AP6269 AP62x8 not set
BCM_MODULES := bcm40181 bcm40183 bcm43458 bcm4354 bcm4356 bcm4358 AP6212 \
	AP6234 AP6255 AP62x2 AP6335 AP6441 AP6181 AP6210 AP6330 AP6476 AP6493 AP6398

BCM_WIFI_MODULE = $(call qstrip,$(BR2_PACKAGE_WIFI_FW_WIFI_MODULE))

ifneq ($(filter $(BCM_WIFI_MODULE),$(BCM_MODULES)),)
	BCM := TRUE
endif

RTK_MODULES := 8189es 8189ftv 8192cu 8192du 8192eu 8192es 8723au 8723bu 8723bs 8811au 8812au 8822bu 8188eu

ifneq ($(BR2_PACKAGE_RTK8188EU)$(BR2_PACKAGE_RTK8189ES)$(BR2_PACKAGE_RTK8189FTV)$(BR2_PACKAGE_RTK8192CU)$(BR2_PACKAGE_RTK8192DU)$(BR2_PACKAGE_RTK8192EU)$(BR2_PACKAGE_RTK8192ES)$(BR2_PACKAGE_RTK8723AU)$(BR2_PACKAGE_RTK8723BU)$(BR2_PACKAGE_RTK8723BS)$(BR2_PACKAGE_RTK8822BU)$(BR2_PACKAGE_RTK8811AU)$(BR2_PACKAGE_RTK8812AU),)
	RTK := TRUE
endif

ifeq ($(BCM)$(RTK),TRUETRUE)
	FLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include -lusb-1.0 -DBROADCOM_MODULES_PATH=/lib/modules/$(LINUX_VERSION_PROBED)/kernel/broadcom/wifi -DBROADCOM_CONFIG_PATH=/etc/wifi -DREALTEK_MODULES_PATH=/lib/modules/$(LINUX_VERSION_PROBED)/kernel/realtek/wifi"
else ifeq ($(BCM)$(RTK), TRUEFALSE)
	FLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include -lusb-1.0 -DBROADCOM_MODULES_PATH=/lib/modules/$(LINUX_VERSION_PROBED)/kernel/broadcom/wifi -DBROADCOM_CONFIG_PATH=/etc/wifi"
else ifeq ($(BCM)$(RTK), FALSETRUE)
	FLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include -lusb-1.0 -DREALTEK_MODULES_PATH=/lib/modules/$(LINUX_VERSION_PROBED)/kernel/realtek/wifi"
else
	FLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include -lusb-1.0"
endif

define AML_UTIL_BUILD_CMDS
	$(TARGET_CONFIGURE_OPTS) $(MAKE) CFLAGS=$(FLAGS) -C $(@D) all
endef

define AML_UTIL_INSTALL_TARGET_CMDS
	$(TARGET_CONFIGURE_OPTS) $(MAKE) CC=$(TARGET_CC) -C $(@D) install
endef

$(eval $(generic-package))

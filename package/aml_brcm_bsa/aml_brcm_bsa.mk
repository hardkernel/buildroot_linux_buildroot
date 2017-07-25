##############################################################

AML_BRCM_BSA_VERSION = 0107_00.26.00
AML_BRCM_BSA_SITE = $(TOPDIR)/../vendor/broadcom/brcm-bsa
AML_BRCM_BSA_SITE_METHOD = local

AML_BRCM_BSA_PATH = 3rdparty/embedded/bsa_examples/linux
AML_BRCM_BSA_LIBBSA = libbsa
AML_BRCM_BSA_APP = app_manager app_3d app_ag app_av app_avk app_ble \
	app_ble_ancs app_ble_blp app_ble_cscc app_ble_eddystone app_ble_hrc \
	app_ble_htp app_ble_pm app_ble_rscc app_ble_tvselect app_ble_wifi \
	app_cce app_dg app_fm app_ftc app_fts app_hd app_headless \
	app_hh app_hl app_hs app_nsa app_opc app_ops app_pan \
	app_pbc app_pbs app_sac app_sc app_switch app_tm

ifeq ($(BR2_aarch64),y)
AML_BRCM_BSA_BUILD_TYPE = arm64
else
AML_BRCM_BSA_BUILD_TYPE = arm
endif

AML_BRCM_BSA_DEPENDENCIES = linux

##if qt5 configured, we can use bsa qt app####
ifeq ($(BR2_PACKAGE_QT5),y)
AML_BRCM_BSA_DEPENDENCIES += qt5base
BSA_QT_APP = $(@D)/$(AML_BRCM_BSA_PATH)/qt_app

define AML_BRCM_BSA_CONFIGURE_CMDS
	(cd $(BSA_QT_APP); $(TARGET_MAKE_ENV) $(QT5_QMAKE) PREFIX=/usr BSA_QT_ARCH=$(AML_BRCM_BSA_BUILD_TYPE))
endef

define AML_BRCM_BSA_BUILD_CMDS_QT
	$(TARGET_MAKE_ENV) $(MAKE) -C $(BSA_QT_APP)
endef

define AML_BRCM_BSA_INSTALL_TARGET_CMDS_QT
	$(TARGET_MAKE_ENV) $(MAKE) -C $(BSA_QT_APP) install INSTALL_ROOT=$(TARGET_DIR)
endef

endif

define AML_BRCM_BSA_BUILD_CMDS
	$(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D)/$(AML_BRCM_BSA_PATH)/$(AML_BRCM_BSA_LIBBSA)/build \
		CPU=$(AML_BRCM_BSA_BUILD_TYPE) ARMGCC=$(TARGET_CC)
	for ff in $(AML_BRCM_BSA_APP); do \
		$(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D)/$(AML_BRCM_BSA_PATH)/$$ff/build \
			CPU=$(AML_BRCM_BSA_BUILD_TYPE) ARMGCC=$(TARGET_CC) BSASHAREDLIB=TRUE; \
	done

	$(AML_BRCM_BSA_BUILD_CMDS_QT)
	$(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(LINUX_DIR) M=$(@D)/3rdparty/embedded/brcm/linux/bthid ARCH=$(KERNEL_ARCH) \
		CROSS_COMPILE=$(TARGET_KERNEL_CROSS) modules

endef

define AML_BRCM_BSA_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/server/$(AML_BRCM_BSA_BUILD_TYPE)/bsa_server \
		$(TARGET_DIR)/usr/bin/bsa_server
	$(INSTALL) -D -m 755 $(@D)/$(AML_BRCM_BSA_PATH)/$(AML_BRCM_BSA_LIBBSA)/build/$(AML_BRCM_BSA_BUILD_TYPE)/sharedlib/libbsa.so \
		$(TARGET_DIR)/usr/lib/libbsa.so
	for ff in $(AML_BRCM_BSA_APP); do \
		$(INSTALL) -D -m 755 $(@D)/$(AML_BRCM_BSA_PATH)/$${ff}/build/$(AML_BRCM_BSA_BUILD_TYPE)/$${ff} $(TARGET_DIR)/usr/bin/${ff}; \
	done

	$(AML_BRCM_BSA_INSTALL_TARGET_CMDS_QT)
	$(INSTALL) -D -m 755 $(@D)/3rdparty/embedded/brcm/linux/bthid/bthid.ko $(TARGET_DIR)/usr/lib
	mkdir -p $(TARGET_DIR)/etc/bsa
	mkdir -p $(TARGET_DIR)/etc/bsa/config
	$(INSTALL) -D -m 755 $(@D)/test_files/av/44k8bpsStereo.wav $(TARGET_DIR)/etc/bsa
	$(INSTALL) -D -m 755 $(@D)/test_files/dg/tx_test_file.txt $(TARGET_DIR)/etc/bsa

endef


$(eval $(generic-package))

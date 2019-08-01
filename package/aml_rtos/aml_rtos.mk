BR2_PACKAGE_AML_RTOS_LOCAL_PATH:= $(call qstrip,$(BR2_PACKAGE_AML_RTOS_LOCAL_PATH))
ifneq ($(BR2_PACKAGE_AML_RTOS_LOCAL_PATH),)

AML_RTOS_VERSION = 1.0.0
AML_RTOS_SITE := $(call qstrip,$(BR2_PACKAGE_AML_RTOS_LOCAL_PATH))
AML_RTOS_SITE_METHOD = local
AML_RTOS_SOC_NAME = $(strip $(BR2_PACKAGE_AML_SOC_FAMILY_NAME))
AML_RTOS_PREBUILT = rtos-prebuilt-$(AML_RTOS_SOC_NAME)
AML_RTOS_DEPENDENCIES += aml_dsp_util

define AML_RTOS_BUILD_CMDS
	if [ -n "$(BR2_PACKAGE_AML_RTOS_ARM_BUILD_OPTION)" ]; then \
		pushd $(@D);  \
			set -e; ./scripts/amlogic/mk.sh $(BR2_PACKAGE_AML_RTOS_ARM_BUILD_OPTION); \
			$(INSTALL) -D -m 644 ./out_armv8/rtos-uImage $(BINARIES_DIR)/;\
		popd; \
	fi
	if [ -n "$(BR2_PACKAGE_AML_RTOS_DSPA_BUILD_OPTION)" ]; then \
		pushd $(@D);  \
			set -e; ./scripts/amlogic/mk.sh $(BR2_PACKAGE_AML_RTOS_DSPA_BUILD_OPTION); \
			$(INSTALL) -D -m 644 ./demos/amlogic/xcc/xtensa_hifi4/dspboot.bin $(BINARIES_DIR)/dspbootA.bin;\
		popd; \
	fi
	if [ -n "$(BR2_PACKAGE_AML_RTOS_DSPB_BUILD_OPTION)" ]; then \
		pushd $(@D);  \
			set -e; ./scripts/amlogic/mk.sh $(BR2_PACKAGE_AML_RTOS_DSPB_BUILD_OPTION); \
			$(INSTALL) -D -m 644 ./demos/amlogic/xcc/xtensa_hifi4/dspboot.bin $(BINARIES_DIR)/dspbootB.bin;\
		popd; \
	fi
endef

define AML_RTOS_INSTALL_TARGET_CMDS
	if [ -n "$(BR2_PACKAGE_AML_RTOS_DSPA_INSTALL)" ]; then \
		mkdir -p $(TARGET_DIR)/lib/firmware/; \
		$(INSTALL) -D -m 644 $(BINARIES_DIR)/dspbootA.bin $(TARGET_DIR)/lib/firmware/;\
	fi
	if [ -n "$(BR2_PACKAGE_AML_RTOS_DSPB_INSTALL)" ]; then \
		mkdir -p $(TARGET_DIR)/lib/firmware/; \
		$(INSTALL) -D -m 644 $(BINARIES_DIR)/dspbootB.bin $(TARGET_DIR)/lib/firmware/;\
	fi
	#Package RTOS build result
	pushd $(BINARIES_DIR); \
		mkdir -p $(AML_RTOS_PREBUILT); \
		test -f rtos-uImage && cp -fv rtos-uImage $(AML_RTOS_PREBUILT); \
		test -f dspbootA.bin && cp -fv dspbootA.bin $(AML_RTOS_PREBUILT); \
		test -f dspbootB.bin && cp -fv dspbootB.bin $(AML_RTOS_PREBUILT); \
		tar -zcf $(AML_RTOS_PREBUILT).tgz -C $(AML_RTOS_PREBUILT) ./; \
	popd
endef

endif

BR2_PACKAGE_AML_RTOS_PREBUILT_PATH:= $(call qstrip,$(BR2_PACKAGE_AML_RTOS_PREBUILT_PATH))
ifneq ($(BR2_PACKAGE_AML_RTOS_PREBUILT_PATH),)

AML_RTOS_VERSION = 1.0.0
AML_RTOS_SITE := $(call qstrip,$(BR2_PACKAGE_AML_RTOS_PREBUILT_PATH))
AML_RTOS_SITE_METHOD = local
AML_RTOS_DEPENDENCIES += aml_dsp_util

define AML_RTOS_INSTALL_TARGET_CMDS
	test -f $(@D)/rtos-uImage && $(INSTALL) -D -m 644 $(@D)/rtos-uImage $(BINARIES_DIR)/
	test -f $(@D)/dspbootA.bin && $(INSTALL) -D -m 644 $(@D)/dspbootA.bin $(BINARIES_DIR)/
	test -f $(@D)/dspbootB.bin && $(INSTALL) -D -m 644 $(@D)/dspbootB.bin $(BINARIES_DIR)/
	if [ -n "$(BR2_PACKAGE_AML_RTOS_DSPA_INSTALL)" ]; then \
		mkdir -p $(TARGET_DIR)/lib/firmware/; \
		$(INSTALL) -D -m 644 $(@D)/dspbootA.bin $(TARGET_DIR)/lib/firmware/;\
	fi
	if [ -n "$(BR2_PACKAGE_AML_RTOS_DSPB_INSTALL)" ]; then \
		mkdir -p $(TARGET_DIR)/lib/firmware/; \
		$(INSTALL) -D -m 644 $(@D)/dspbootB.bin $(TARGET_DIR)/lib/firmware/;\
	fi
endef

endif
$(eval $(generic-package))


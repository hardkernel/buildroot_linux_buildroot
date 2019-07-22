BR2_PACKAGE_AML_RTOS_LOCAL_PATH:= $(call qstrip,$(BR2_PACKAGE_AML_RTOS_LOCAL_PATH))
ifneq ($(BR2_PACKAGE_AML_RTOS_LOCAL_PATH),)

AML_RTOS_VERSION = 1.0.0
AML_RTOS_SITE := $(call qstrip,$(BR2_PACKAGE_AML_RTOS_LOCAL_PATH))
AML_RTOS_SITE_METHOD = local


define AML_RTOS_BUILD_CMDS
	if [ -n "$(BR2_PACKAGE_AML_RTOS_ARM_BUILD_OPTION)" ]; then \
		pushd $(@D);  \
			./scripts/amlogic/mk.sh $(BR2_PACKAGE_AML_RTOS_ARM_BUILD_OPTION); \
			$(INSTALL) -D -m 644 ./out_armv8/rtos-uImage $(BINARIES_DIR)/;\
		popd; \
	fi
	if [ -n "$(BR2_PACKAGE_AML_RTOS_DSPA_BUILD_OPTION)" ]; then \
		pushd $(@D);  \
			./scripts/amlogic/mk.sh $(BR2_PACKAGE_AML_RTOS_DSPA_BUILD_OPTION); \
			$(INSTALL) -D -m 644 ./demos/amlogic/xcc/xtensa_hifi4/dspboot.bin $(BINARIES_DIR)/dspbootA.bin;\
		popd; \
	fi
	if [ -n "$(BR2_PACKAGE_AML_RTOS_DSPB_BUILD_OPTION)" ]; then \
		pushd $(@D);  \
			./scripts/amlogic/mk.sh $(BR2_PACKAGE_AML_RTOS_DSPB_BUILD_OPTION); \
			$(INSTALL) -D -m 644 ./demos/amlogic/xcc/xtensa_hifi4/dspboot.bin $(BINARIES_DIR)/dspbootB.bin;\
		popd; \
	fi
endef

define AML_RTOS_INSTALL_TARGET_CMDS
	if [ -n "$(BR2_PACKAGE_AML_RTOS_DSPA_INSTALL)" ]; then \
		mkdir -p $(TARGET_DIR)/usr/dsp/; \
		$(INSTALL) -D -m 644 $(BINARIES_DIR)/dspbootA.bin $(TARGET_DIR)/usr/dsp/;\
	fi
	if [ -n "$(BR2_PACKAGE_AML_RTOS_DSPB_INSTALL)" ]; then \
		mkdir -p $(TARGET_DIR)/usr/dsp/; \
		$(INSTALL) -D -m 644 $(BINARIES_DIR)/dspbootB.bin $(TARGET_DIR)/usr/dsp/;\
	fi
endef

endif

BR2_PACKAGE_AML_RTOS_PREBUILT_PATH:= $(call qstrip,$(BR2_PACKAGE_AML_RTOS_PREBUILT_PATH))
ifneq ($(BR2_PACKAGE_AML_RTOS_PREBUILT_PATH),)

AML_RTOS_VERSION = 1.0.0
AML_RTOS_SITE := $(call qstrip,$(BR2_PACKAGE_AML_RTOS_PREBUILT_PATH))
AML_RTOS_SITE_METHOD = local

define AML_RTOS_INSTALL_TARGET_CMDS
	test -f $(@D)/rtos-uImage && $(INSTALL) -D -m 644 $(@D)/rtos-uImage $(BINARIES_DIR)/
	test -f $(@D)/dspbootA.bin && $(INSTALL) -D -m 644 $(@D)/dspbootA.bin $(BINARIES_DIR)/
	test -f $(@D)/dspbootB.bin && $(INSTALL) -D -m 644 $(@D)/dspbootB.bin $(BINARIES_DIR)/
	if [ -n "$(BR2_PACKAGE_AML_RTOS_DSPA_INSTALL)" ]; then \
		mkdir -p $(TARGET_DIR)/usr/dsp/; \
		$(INSTALL) -D -m 644 $(@D)/dspbootA.bin $(TARGET_DIR)/usr/dsp/;\
	fi
	if [ -n "$(BR2_PACKAGE_AML_RTOS_DSPB_INSTALL)" ]; then \
		mkdir -p $(TARGET_DIR)/usr/dsp/; \
		$(INSTALL) -D -m 644 $(@D)/dspbootB.bin $(TARGET_DIR)/usr/dsp/;\
	fi
endef

endif
$(eval $(generic-package))


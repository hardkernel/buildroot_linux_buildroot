AML_HIFI4_VERSION = projects/amlogic-dev
AML_HIFI4_SITE = ssh://scgit.amlogic.com:29418/rtos/freertos
AML_HIFI4_SITE_METHOD = git

define AML_HIFI4_BUILD_CMDS
	pushd $(@D);  \
	./scripts/amlogic/mk.sh $(BR2_PACKAGE_AML_HIFI4_BUILD_OPTION); \
	popd
endef

define AML_HIFI4_INSTALL_TARGET_CMDS
	pushd $(@D); \
	$(INSTALL) -D -m 644 ./demos/amlogic/xcc/xtensa_hifi4/dspboot.bin $(BINARIES_DIR)/;\
	mkdir -p $(TARGET_DIR)/usr/dsp/;\
	$(INSTALL) -D -m 644 ./demos/amlogic/xcc/xtensa_hifi4/dspboot.bin $(TARGET_DIR)/usr/dsp/;\
	popd
endef

$(eval $(generic-package))


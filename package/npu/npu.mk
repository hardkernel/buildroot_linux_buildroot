################################################################################
#
# amlogic npu driver
#
################################################################################
ifeq ($(BR2_PACKAGE_NPU_LOCAL),y)
BR2_PACKAGE_NPU_LOCAL_PATH=$(TOPDIR)/../hardware/aml-4.9/npu/nanoq
NPU_SITE = $(call qstrip,$(BR2_PACKAGE_NPU_LOCAL_PATH))
NPU_SITE_METHOD = local
ARM_NPU_MODULE_DIR = kernel/amlogic/npu
NPU_KO_INSTALL_DIR=$(TARGET_DIR)/lib/modules/$(LINUX_VERSION_PROBED)/kernel/amlogic/npu
NPU_SO_INSTALL_DIR=$(TARGET_DIR)/lib
NPU_DEPENDENCIES = linux

ARM_NPU_DEP = $(TARGET_DIR)/lib/modules/$(LINUX_VERSION_PROBED)/modules.dep

define copy-arm-npu
        $(foreach m, $(shell find $(strip $(1)) -name "*.ko"),\
                $(shell [ ! -e $(2) ] && mkdir $(2) -p;\
                cp $(m) $(strip $(2))/ -rfa;\
                echo $(4)/$(notdir $(m)): >> $(3)))
endef

define ARM_NPU_DEP_INSTALL_TARGET_CMDS
        $(call copy-arm-npu,$(@D),\
                $(shell echo $(NPU_KO_INSTALL_DIR)),\
                $(shell echo $(ARM_NPU_DEP)),\
                $(ARM_NPU_MODULE_DIR))
endef

ifeq ($(BR2_aarch64), y)
NPU_INSTALL_TARGETS_CMDS = \
	$(INSTALL) -m 0755 $(@D)/build/sdk/drivers/galcore.ko $(NPU_KO_INSTALL_DIR); \
	$(INSTALL) -m 0755 $(@D)/sharelib/lib64/libGAL.so $(NPU_SO_INSTALL_DIR); \
	$(INSTALL) -m 0755 $(@D)/sharelib/lib64/libVSC.so $(NPU_SO_INSTALL_DIR); \
	$(INSTALL) -m 0755 $(@D)/sharelib/lib64/libCLC.so $(NPU_SO_INSTALL_DIR); \
	$(INSTALL) -m 0755 $(@D)/sharelib/lib64/libLLVM_viv.so $(NPU_SO_INSTALL_DIR); \
	$(INSTALL) -m 0755 $(@D)/sharelib/lib64/libOpenCL.so $(NPU_SO_INSTALL_DIR); \
	$(INSTALL) -m 0755 $(@D)/sharelib/lib64/libOpenVX.so $(NPU_SO_INSTALL_DIR); \
	$(INSTALL) -m 0755 $(@D)/sharelib/lib64/libOpenVXU.so $(NPU_SO_INSTALL_DIR); \
	$(INSTALL) -m 0755 $(@D)/sharelib/lib64/libNNVXCBinary_7d.so $(NPU_SO_INSTALL_DIR); \
	$(INSTALL) -m 0755 $(@D)/sharelib/lib64/libNNVXCBinary_88.so $(NPU_SO_INSTALL_DIR); \
	$(INSTALL) -m 0755 $(@D)/sharelib/lib64/libOvx12VXCBinary_7d.so $(NPU_SO_INSTALL_DIR); \
	$(INSTALL) -m 0755 $(@D)/sharelib/lib64/libOvx12VXCBinary_88.so $(NPU_SO_INSTALL_DIR); \
	$(INSTALL) -m 0755 $(@D)/sharelib/lib64/libovxlib.so $(NPU_SO_INSTALL_DIR); \
	$(INSTALL) -m 0755 $(@D)/sharelib/lib64/libVivanteOpenCL.so $(NPU_SO_INSTALL_DIR);
else
NPU_INSTALL_TARGETS_CMDS = \
	$(INSTALL) -m 0755 $(@D)/build/sdk/drivers/galcore.ko $(NPU_KO_INSTALL_DIR); \
	$(INSTALL) -m 0755 $(@D)/sharelib/lib32/libGAL.so $(NPU_SO_INSTALL_DIR); \
	$(INSTALL) -m 0755 $(@D)/sharelib/lib32/libVSC.so $(NPU_SO_INSTALL_DIR); \
	$(INSTALL) -m 0755 $(@D)/sharelib/lib32/libCLC.so $(NPU_SO_INSTALL_DIR); \
	$(INSTALL) -m 0755 $(@D)/sharelib/lib32/libLLVM_viv.so $(NPU_SO_INSTALL_DIR); \
	$(INSTALL) -m 0755 $(@D)/sharelib/lib32/libOpenCL.so $(NPU_SO_INSTALL_DIR); \
	$(INSTALL) -m 0755 $(@D)/sharelib/lib32/libOpenVX.so $(NPU_SO_INSTALL_DIR); \
	$(INSTALL) -m 0755 $(@D)/sharelib/lib32/libOpenVXU.so $(NPU_SO_INSTALL_DIR); \
	$(INSTALL) -m 0755 $(@D)/sharelib/lib32/libNNVXCBinary_7d.so $(NPU_SO_INSTALL_DIR); \
	$(INSTALL) -m 0755 $(@D)/sharelib/lib32/libNNVXCBinary_88.so $(NPU_SO_INSTALL_DIR); \
	$(INSTALL) -m 0755 $(@D)/sharelib/lib32/libOvx12VXCBinary_7d.so $(NPU_SO_INSTALL_DIR); \
	$(INSTALL) -m 0755 $(@D)/sharelib/lib32/libOvx12VXCBinary_88.so $(NPU_SO_INSTALL_DIR); \
	$(INSTALL) -m 0755 $(@D)/sharelib/lib32/libovxlib.so $(NPU_SO_INSTALL_DIR); \
	$(INSTALL) -m 0755 $(@D)/sharelib/lib32/libVivanteOpenCL.so $(NPU_SO_INSTALL_DIR);
endif
path = 	$(@D)
define NPU_BUILD_CMDS
	cd $(@D);./aml_buildroot.sh $(KERNEL_ARCH) $(LINUX_DIR) $(TARGET_KERNEL_CROSS)
endef
define NPU_INSTALL_TARGET_CMDS
	mkdir -p $(NPU_KO_INSTALL_DIR)
	$(NPU_INSTALL_TARGETS_CMDS)
	$(ARM_NPU_DEP_INSTALL_TARGET_CMDS)
endef

endif
$(eval $(generic-package))

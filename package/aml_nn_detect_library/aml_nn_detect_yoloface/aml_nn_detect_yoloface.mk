#
# AML_NN_DETECT_YOLOFACE
#
AML_NN_DETECT_YOLOFACE_VERSION = 1.0
AML_NN_DETECT_YOLOFACE_SITE = $(TOPDIR)/../vendor/amlogic/slt/npu_app/detect_library/model_code/detect_yoloface
AML_NN_DETECT_YOLOFACE_SITE_METHOD = local
AML_NN_DETECT_YOLOFACE_INSTALL_STAGING = YES
AML_NN_DETECT_YOLOFACE_DEPENDENCIES = npu


define AML_NN_DETECT_YOLOFACE_BUILD_CMDS
    cd $(@D);mkdir -p obj;$(MAKE) CC=$(TARGET_CC)
endef


define AML_NN_DETECT_YOLOFACE_INSTALL_TARGET_CMDS
    mkdir -p $(TARGET_DIR)/etc/nn_data
    $(INSTALL) -D -m 0644 $(@D)/libnn_yoloface.so  $(TARGET_DIR)/usr/lib/
    $(INSTALL) -D -m 0644 $(@D)/nn_data/* $(TARGET_DIR)/etc/nn_data/
endef

define AML_NN_DETECT_YOLOFACE_INSTALL_STAGING_CMDS
    $(INSTALL) -D -m 0644 $(@D)/libnn_yoloface.so  $(TARGET_DIR)/usr/lib/
endef

define AML_NN_DETECT_YOLOFACE_INSTALL_CLEAN_CMDS
    $(MAKE) CC=$(TARGET_CC) -C $(@D) clean
endef

$(eval $(generic-package))

#
# AML_NN_DETECT_MTCNN_V1
#
AML_NN_DETECT_MTCNN_V1_VERSION = 1.0
AML_NN_DETECT_MTCNN_V1_SITE = $(TOPDIR)/../vendor/amlogic/slt/npu_app/detect_library/model_code/detect_mtcnn
AML_NN_DETECT_MTCNN_V1_SITE_METHOD = local
AML_NN_DETECT_MTCNN_V1_INSTALL_STAGING = YES


define AML_NN_DETECT_MTCNN_V1_BUILD_CMDS
    cd $(@D);
endef


define AML_NN_DETECT_MTCNN_V1_INSTALL_TARGET_CMDS
    mkdir -p $(TARGET_DIR)/etc/nn_data
    mkdir -p $(TARGET_DIR)/etc/nn_data/config
	mkdir -p $(TARGET_DIR)/etc/nn_data/config/BoxRegress
	mkdir -p $(TARGET_DIR)/etc/nn_data/config/GenerateBoundingBox
	mkdir -p $(TARGET_DIR)/etc/nn_data/config/libNNExt
	mkdir -p $(TARGET_DIR)/etc/nn_data/config/NonMaxSuppression
	mkdir -p $(TARGET_DIR)/etc/nn_data/config/OddEvenSort
	mkdir -p $(TARGET_DIR)/etc/nn_data/config/rgbScale
    $(INSTALL) -D -m 0644 $(@D)/so/*  $(TARGET_DIR)/usr/lib/
    $(INSTALL) -D -m 0644 $(@D)/nn_data/* $(TARGET_DIR)/etc/nn_data/
	$(INSTALL) -D -m 0644 $(@D)/config/BoxRegress/* $(TARGET_DIR)/etc/nn_data/config/BoxRegress/
	$(INSTALL) -D -m 0644 $(@D)/config/GenerateBoundingBox/* $(TARGET_DIR)/etc/nn_data/config/GenerateBoundingBox/
	$(INSTALL) -D -m 0644 $(@D)/config/libNNExt/* $(TARGET_DIR)/etc/nn_data/config/libNNExt/
	$(INSTALL) -D -m 0644 $(@D)/config/NonMaxSuppression/* $(TARGET_DIR)/etc/nn_data/config/NonMaxSuppression/
	$(INSTALL) -D -m 0644 $(@D)/config/OddEvenSort/* $(TARGET_DIR)/etc/nn_data/config/OddEvenSort/
	$(INSTALL) -D -m 0644 $(@D)/config/rgbScale/* $(TARGET_DIR)/etc/nn_data/config/rgbScale/
endef

define AML_NN_DETECT_MTCNN_V1_INSTALL_STAGING_CMDS
    $(INSTALL) -D -m 0644 $(@D)/so/*  $(TARGET_DIR)/usr/lib/
endef

$(eval $(generic-package))

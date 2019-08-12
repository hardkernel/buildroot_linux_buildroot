#
# AML_NN_DETECT
#
AML_NN_DETECT_VERSION = 1.0
AML_NN_DETECT_SITE = $(TOPDIR)/../vendor/amlogic/slt/npu_app/detect_library/source_code
AML_NN_DETECT_SITE_METHOD = local
AML_NN_DETECT_INSTALL_STAGING = YES


ifeq ($(BR2_PACKAGE_AML_NN_DETECT_MTCNN_V1), y)
AML_NN_DETECT_DEPENDENCIES += aml_nn_detect_mtcnn_v1
endif
ifeq ($(BR2_PACKAGE_AML_NN_DETECT_YOLO_V2), y)
AML_NN_DETECT_DEPENDENCIES += aml_nn_detect_yolo_v2
endif
ifeq ($(BR2_PACKAGE_AML_NN_DETECT_YOLO_V3), y)
AML_NN_DETECT_DEPENDENCIES += aml_nn_detect_yolo_v3
endif
ifeq ($(BR2_PACKAGE_AML_NN_DETECT_YOLOFACE), y)
AML_NN_DETECT_DEPENDENCIES += aml_nn_detect_yoloface
endif
ifeq ($(BR2_PACKAGE_AML_NN_FACENET), y)
AML_NN_DETECT_DEPENDENCIES += aml_nn_facenet
endif
ifeq ($(BR2_PACKAGE_AML_NN_INSIGHTFACE), y)
AML_NN_DETECT_DEPENDENCIES += aml_nn_insightface
endif

define AML_NN_DETECT_BUILD_CMDS
    cd $(@D);mkdir -p obj;$(MAKE) CC=$(TARGET_CC)
endef


define AML_NN_DETECT_INSTALL_TARGET_CMDS
    mkdir -p $(TARGET_DIR)/etc/nn_data
    $(INSTALL) -D -m 0644 $(@D)/libnn_detect.so $(TARGET_DIR)/usr/lib/libnn_detect.so
endef

define AML_NN_DETECT_INSTALL_STAGING_CMDS
    $(INSTALL) -D -m 0644 $(@D)/libnn_detect.so $(STAGING_DIR)/usr/lib/libnn_detect.so
    $(INSTALL) -D -m 0644 $(@D)/include/nn_detect.h $(STAGING_DIR)/usr/include/nn_detect.h
    $(INSTALL) -D -m 0644 $(@D)/include/nn_detect_common.h $(STAGING_DIR)/usr/include/nn_detect_common.h
    $(INSTALL) -D -m 0644 $(@D)/include/nn_detect_utils.h $(STAGING_DIR)/usr/include/nn_detect_utils.h
endef

define AML_NN_DETECT_INSTALL_CLEAN_CMDS
    $(MAKE) CC=$(TARGET_CC) -C $(@D) clean
endef

$(eval $(generic-package))



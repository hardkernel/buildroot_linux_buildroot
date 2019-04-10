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

ifeq ($(BR2_aarch64), y)
AML_NN_DETECT_MTCNN_V1_INSTALL_TARGETS_CMDS = \
    $(INSTALL) -D -m 0644 $(@D)/so/lib64/*  $(TARGET_DIR)/usr/lib/
else
AML_NN_DETECT_MTCNN_V1_INSTALL_TARGETS_CMDS = \
    $(INSTALL) -D -m 0644 $(@D)/so/lib32/*  $(TARGET_DIR)/usr/lib/
endif

define AML_NN_DETECT_MTCNN_V1_INSTALL_TARGET_CMDS
    $(AML_NN_DETECT_MTCNN_V1_INSTALL_TARGETS_CMDS)
    mkdir -p $(TARGET_DIR)/etc/nn_data
    mkdir -p $(TARGET_DIR)/etc/nn_data/config
    mkdir -p $(TARGET_DIR)/etc/nn_data/config/revA
    mkdir -p $(TARGET_DIR)/etc/nn_data/config/revB
    $(INSTALL) -D -m 0644 $(@D)/nn_data/* $(TARGET_DIR)/etc/nn_data/
    $(INSTALL) -D -m 0644 $(@D)/config/revA/* $(TARGET_DIR)/etc/nn_data/config/revA/
    $(INSTALL) -D -m 0644 $(@D)/config/revB/* $(TARGET_DIR)/etc/nn_data/config/revB/
endef

define AML_NN_DETECT_MTCNN_V1_INSTALL_STAGING_CMDS
    $(AML_NN_DETECT_MTCNN_V1_INSTALL_TARGETS_CMDS)
endef

$(eval $(generic-package))

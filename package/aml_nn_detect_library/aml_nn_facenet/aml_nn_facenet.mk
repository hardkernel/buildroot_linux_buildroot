#
# AML_NN_FACENET
#
AML_NN_FACENET_VERSION = 1.0
AML_NN_FACENET_SITE = $(TOPDIR)/../vendor/amlogic/slt/npu_app/detect_library/model_code/facenet
AML_NN_FACENET_SITE_METHOD = local
AML_NN_FACENET_INSTALL_STAGING = YES
AML_NN_FACENET_DEPENDENCIES = npu


define AML_NN_FACENET_BUILD_CMDS
    cd $(@D);mkdir -p obj;$(MAKE) CC=$(TARGET_CC)
endef


define AML_NN_FACENET_INSTALL_TARGET_CMDS
    mkdir -p $(TARGET_DIR)/etc/nn_data
    $(INSTALL) -D -m 0644 $(@D)/libnn_facenet.so  $(TARGET_DIR)/usr/lib/
    $(INSTALL) -D -m 0644 $(@D)/nn_data/* $(TARGET_DIR)/etc/nn_data/
endef

define AML_NN_FACENET_INSTALL_STAGING_CMDS
    $(INSTALL) -D -m 0644 $(@D)/libnn_facenet.so  $(TARGET_DIR)/usr/lib/
endef

define AML_NN_FACENET_INSTALL_CLEAN_CMDS
    $(MAKE) CC=$(TARGET_CC) -C $(@D) clean
endef

$(eval $(generic-package))

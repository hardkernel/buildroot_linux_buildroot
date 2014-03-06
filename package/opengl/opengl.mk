#############################################################
#
# opengl
#
#############################################################
OPENGL_VERSION:=0.9.9
OPENGL_DIR=$(BUILD_DIR)/opengl
OPENGL_SOURCE=src
OPENGL_SITE=.

ifeq ($(BR2_PACKAGE_GPU_VERSION),"r3p2")
MALI_LIB_DIR=r3p2-EAC
else ifeq ($(BR2_PACKAGE_GPU_VERSION),"r3p2-01rel3")
ifeq ($(BR2_PACKAGE_OPENGL_MALI_VERSION),"MALI400")
MALI_LIB_DIR=r3p2-01rel3/m400
else ifeq ($(BR2_PACKAGE_OPENGL_MALI_VERSION),"MALI450")
MALI_LIB_DIR=r3p2-01rel3/m450
endif
else ifeq ($(BR2_PACKAGE_GPU_VERSION),"r4p0-01")
MALI_LIB_DIR=r4p0-01
endif

$(OPENGL_DIR)/.unpacked:
	mkdir -p $(OPENGL_DIR)
	cp -arf ./package/opengl/src/* $(OPENGL_DIR)
	cp $(OPENGL_DIR)/lib/$(MALI_LIB_DIR)/libMali.so $(OPENGL_DIR)/lib/libMali.so
	touch $(OPENGL_DIR)/.unpacked

$(OPENGL_DIR)/.installed: $(OPENGL_DIR)/.unpacked
	cp -arf $(OPENGL_DIR)/* $(STAGING_DIR)/usr
	cp -d $(OPENGL_DIR)/lib/*.so* $(TARGET_DIR)/usr/lib
	install -m 755 $(OPENGL_DIR)/lib/$(MALI_LIB_DIR)/libMali.so $(TARGET_DIR)/usr/lib
	touch $(OPENGL_DIR)/.installed

opengl: $(OPENGL_DIR)/.installed

opengl-clean:
	rm -rf $(OPENGL_DIR)

opengl-dirclean:
	rm -rf $(OPENGL_DIR)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(BR2_PACKAGE_OPENGL),y)
TARGETS+=opengl
endif
include package/opengl/*/*.mk

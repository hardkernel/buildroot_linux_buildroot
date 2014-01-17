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
MALI_LIB=libMali-r3p2.so
else ifeq ($(BR2_PACKAGE_GPU_VERSION),"r3p2-01rel3")
MALI_LIB=libMali-r3p2-01rel3.so
endif

$(OPENGL_DIR)/.unpacked:
	mkdir -p $(OPENGL_DIR)
	cp -arf ./package/opengl/src/* $(OPENGL_DIR)
	touch $(OPENGL_DIR)/.unpacked

$(OPENGL_DIR)/.installed: $(OPENGL_DIR)/.unpacked
	cp -arf $(OPENGL_DIR)/* $(STAGING_DIR)/usr
	cp $(OPENGL_DIR)/lib/$(MALI_LIB) $(OPENGL_DIR)/lib/libMali.so
	install -m 755 $(OPENGL_DIR)/lib/*.so $(TARGET_DIR)/usr/lib
	rm  $(TARGET_DIR)/usr/lib/$(MALI_LIB)
	rm  $(OPENGL_DIR)/lib/libMali.so
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

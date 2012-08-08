#############################################################
#
# opengl
#
#############################################################
OPENGL_VERSION:=0.9.9
OPENGL_DIR=$(BUILD_DIR)/opengl
OPENGL_SOURCE=src
OPENGL_SITE=.

$(OPENGL_DIR)/.unpacked:
	mkdir -p $(OPENGL_DIR)
	cp -arf ./package/opengl/src/* $(OPENGL_DIR)
	touch $(OPENGL_DIR)/.unpacked

$(OPENGL_DIR)/.installed: $(OPENGL_DIR)/.unpacked
	cp -arf $(OPENGL_DIR)/* $(STAGING_DIR)/usr
	install -m 755 $(OPENGL_DIR)/lib/*.so $(TARGET_DIR)/usr/lib
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

#############################################################
#
# libplayer
#
#############################################################
LIBPLAYER_VERSION:=0.4.0
LIBPLAYER_DIR=$(BUILD_DIR)/libplayer
LIBPLAYER_SOURCE=src
LIBPLAYER_SITE=.
export LIBPLAYER_BUILD_DIR = $(BUILD_DIR)
export LIBPLAYER_STAGING_DIR = $(STAGING_DIR)
export LIBPLAYER_TARGET_DIR = $(TARGET_DIR)

DEPENDS=zlib alsa-lib

LIBMMS_DEPENDENCIES = libmms 
$(LIBPLAYER_DIR)/.unpacked:
	mkdir -p $(LIBPLAYER_DIR)
	cp -arf ./package/multimedia/libplayer/src/* $(LIBPLAYER_DIR)
	touch $(LIBPLAYER_DIR)/.unpacked

$(LIBPLAYER_DIR)/libplayer: $(LIBPLAYER_DIR)/.unpacked
	$(MAKE) CC=$(TARGET_CC) -C $(LIBPLAYER_DIR) all
	$(MAKE) CC=$(TARGET_CC) -C $(LIBPLAYER_DIR) install

libplayer:$(DEPENDS) $(LIBPLAYER_DIR)/libplayer

libplayer-source: $(DL_DIR)/$(LIBPLAYER_SOURCE)

libplayer-clean:
	-$(MAKE) -C $(LIBPLAYER_DIR) clean

libplayer-dirclean:
	rm -rf $(LIBPLAYER_DIR)

#before_cmd:depends

#depends:
#	@if [   "${DEPENDS}" != "" ]; then \
                cd ${TOPDIR};make ${DEPENDS};    \
        fi
#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(BR2_PACKAGE_LIBPLAYER),y)
TARGETS+=libplayer
endif

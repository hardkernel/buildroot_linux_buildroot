PREFIX=$(LIBPLAYER_STAGING_DIR)/usr
TARGET_DIR=$(LIBPLAYER_TARGET_DIR)/usr
TARGET_FIRWARE_DIR=$(LIBPLAYER_TARGET_DIR)
LIB_DIR=$(PREFIX)/lib/libplayer
BUILD_DIR=$(shell pwd)
INC_DIR=$(LIBPLAYER_STAGING_DIR)/usr/include
CROSS=$(CC:%gcc=%)

kplayer_s=kplayer.c

INSTALL_FLAGS=-m 755

TARGET=kplayer

LDFLAGS= -L$(PREFIX)/lib/libplayer -lavutil -lavformat -lavcodec -lm -lz -lpthread -lamcodec -ldl -lamplayer -lamadec -lasound -lamstreaming -lamavutils -lrt
#CFLAGS+= -I${BUILD_DIR}/../amffmpeg -I${BUILD_DIR}/../amcodec/include -I${BUILD_DIR}/../amadec/include 
#CFLAGS+= -I${BUILD_DIR}/../amplayer/player/include -I${BUILD_DIR}/../amplayer/player
CFLAGS+= -I$(INC_DIR)
CFLAGS+=-O2 -g

.PHONY: all config  before_cmd install clean distclean

all: ${TARGET}

install:all
	install $(INSTALL_FLAGS) $(TARGET) $(TARGET_DIR)/bin
	cp -rf $(LIB_DIR)/libavutil* $(TARGET_DIR)/lib
	cp -rf $(LIB_DIR)/libavformat* $(TARGET_DIR)/lib
	cp -rf $(LIB_DIR)/libavcodec* $(TARGET_DIR)/lib
	cp -rf $(LIB_DIR)/libamcodec* $(TARGET_DIR)/lib
	cp -rf $(LIB_DIR)/libamplayer* $(TARGET_DIR)/lib
	cp -rf $(LIB_DIR)/libamadec* $(TARGET_DIR)/lib
	cp -rf $(LIB_DIR)/libamstreaming* $(TARGET_DIR)/lib
	cp -rf $(LIB_DIR)/libamavutils* $(TARGET_DIR)/lib
	mkdir -p $(TARGET_FIRWARE_DIR)/lib/firmware
	cp -rf ${BUILD_DIR}/../amadec/firmware/*.bin $(TARGET_FIRWARE_DIR)/lib/firmware

distclean:clean
    
clean:
	rm $(TARGET_DIR)/bin/$(TARGET)

${TARGET}:kplayer.o
	$(CC) $(CFLAGS) $(LDFLAGS) $($(@:%=%_s)) -o $@

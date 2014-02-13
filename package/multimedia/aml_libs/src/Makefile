#CC=${HOST_GCC}
ifeq ($(BR2_TARGET_BOARD_PLATFORM),"meson8")
	FIRMDIR := firmware-m8
else 
ifeq ($(BR2_TARGET_BOARD_PLATFORM),"meson6")
	FIRMDIR := firmware-m6
else 
	FIRMDIR := firmware
endif
endif
#export CC BUILD_DIR STAGING_DIR TARGET_DIR
all:
	mkdir -p $(shell pwd)/../../staging/usr/lib/aml_libs
	-make -C amadec all
	-make -C amavutils all
	-make -C amcodec all
	-make -C example all
	echo $(BR2_TARGET_BOARD_PLATFORM)
	echo $(FIRMDIR) 
install:
	-make -C amadec install
	-make -C amavutils install
	-make -C amcodec install
	-make -C example install
	cp  -rf $(AML_LIBS_STAGING_DIR)/usr/lib/aml_libs/*.so $(TARGET_DIR)/usr/lib 
	mkdir -p $(TARGET_DIR)/lib/firmware
	install -D -m 0644 amadec/${FIRMDIR}/*.bin $(TARGET_DIR)/lib/firmware 
	
clean:
	-make -C amadec clean
	-make -C amavutils clean
	-make -C amcodec clean
	-make -C example clean



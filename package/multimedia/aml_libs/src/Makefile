#CC=${HOST_GCC}

#export CC BUILD_DIR STAGING_DIR TARGET_DIR
all:
	mkdir -p $(shell pwd)/../../staging/usr/lib/aml_libs
	-make -C amadec all
	-make -C amavutils all
	-make -C amcodec all
	

    
install:
	-make -C amadec install
	-make -C amavutils install
	-make -C amcodec install
	cp  -rf $(AML_LIBS_STAGING_DIR)/usr/lib/aml_libs/*.so $(TARGET_DIR)/usr/lib 
	mkdir -p $(TARGET_DIR)/lib/firmware
	install -D -m 0644 amadec/firmware/*.bin $(TARGET_DIR)/lib/firmware 
	
clean:
	-make -C amadec clean
	-make -C amavutils clean
	-make -C amcodec clean



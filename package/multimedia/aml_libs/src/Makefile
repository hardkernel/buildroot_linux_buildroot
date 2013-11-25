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
	
clean:
	-make -C amadec clean
	-make -C amavutils clean
	-make -C amcodec clean



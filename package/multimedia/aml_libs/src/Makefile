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

clean:
	-make -C amadec clean
	-make -C amavutils clean
	-make -C amcodec clean



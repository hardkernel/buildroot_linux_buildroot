#CC=${HOST_GCC}

#export CC BUILD_DIR STAGING_DIR TARGET_DIR
all:
	mkdir -p $(shell pwd)/../../staging/usr/lib/libplayer
	-make -C amadec all
	-make -C amadec install
	-make -C amavutils all
	-make -C amavutils install
	-make -C amcodec all
	-make -C amcodec install
	-make -C amffmpeg all
	-make -C amffmpeg install
	-make -C amplayer all
	-make -C amplayer install
	-make -C streamsource all
	-make -C streamsource install
	-make -C examples all
    
install:
	-make -C examples install

clean:
	-make -C amadec clean
	-make -C amavutils clean
	-make -C amcodec clean
	-make -C amffmpeg clean
	-make -C streamsource clean
	-make -C examples clean


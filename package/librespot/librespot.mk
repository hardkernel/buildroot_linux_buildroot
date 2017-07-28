####################################################################################
#
#librespot
#
####################################################################################

LIBRESPOT_VERSION = 0.1
LIBRESPOT_SOURCE = librespot.tar.gz
LIBRESPOT_LICENSE = MIT LICENSE
LIBRESPOT_LICENSE_FILES = LICENSE
LIBRESPOT_STAGING = YES
LIBRESPOT_SITE = http://openlinux.amlogic.com:8000/download/GPL_code_release/ThirdParty

SYSROOT_CONFIG = $(@D)/../../host/usr/bin/gcc-sysroot


define LIBRESPOT_INSTALL_TARGET_CMDS

	curl -OL http://mirrordirector.raspbian.org/raspbian/pool/main/a/alsa-lib/libasound2_1.0.25-4_armhf.deb
	curl -OL http://mirrordirector.raspbian.org/raspbian/pool/main/a/alsa-lib/libasound2-dev_1.0.25-4_armhf.deb
	ar p $(TOPDIR)/libasound2_1.0.25-4_armhf.deb data.tar.gz | tar -xz -C  $(STAGING_DIR)
	ar p $(TOPDIR)/libasound2-dev_1.0.25-4_armhf.deb data.tar.gz | tar -xz -C $(STAGING_DIR)
	rm $(TOPDIR)/libasound2-dev_1.0.25-4_armhf.deb
	rm $(TOPDIR)/libasound2_1.0.25-4_armhf.deb

	echo -e '#!/bin/bash' > $(SYSROOT_CONFIG)
	echo  "arm-linux-gnueabihf-gcc --sysroot $(STAGING_DIR) \"\$$@\"" >> $(SYSROOT_CONFIG)


	chmod +x $(SYSROOT_CONFIG)
	mkdir -p ~/.cargo
	echo -e '[target.armv7-unknown-linux-gnueabihf]\nlinker = "$(SYSROOT_CONFIG)"' > $(@D)/.cargo/config
	cp $(@D)/.cargo ~/ -rf
	$(@D)/.cargo/bin/rustup default nightly
	$(@D)/.cargo/bin/rustup target add armv7-unknown-linux-gnueabihf
	$(@D)/.cargo/bin/cargo build --no-default-features --features "alsa-backend" --target armv7-unknown-linux-gnueabihf --release --manifest-path $(@D)/Cargo.toml


	${INSTALL} -D -m 0755 ${@D}/target/armv7-unknown-linux-gnueabihf/release/librespot  ${TARGET_DIR}/usr/bin/librespot
	rm ~/.cargo -rf
endef

$(eval $(generic-package))

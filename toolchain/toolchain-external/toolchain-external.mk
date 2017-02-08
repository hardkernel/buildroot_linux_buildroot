################################################################################
#
# toolchain-external
#
################################################################################

ifneq ($(BR2_KERNEL_TOOLCHAIN_EXTERNAL_PATH),)
KERNEL_TOOLCHAIN_EXTERNAL_INSTALL_DIR = $(call qstrip,$(BR2_KERNEL_TOOLCHAIN_EXTERNAL_PATH))
endif

TOOLCHAIN_EXTERNAL_ADD_TOOLCHAIN_DEPENDENCY = NO

# musl does not provide an implementation for sys/queue.h or sys/cdefs.h.
# So, add the musl-compat-headers package that will install those files,
# into the staging directory:
#   sys/queue.h:  header from NetBSD
#   sys/cdefs.h:  minimalist header bundled in Buildroot
ifeq ($(BR2_TOOLCHAIN_USES_MUSL),y)
TOOLCHAIN_EXTERNAL_DEPENDENCIES += musl-compat-headers
endif

# The Linaro toolchain expects the libraries in
# {/usr,}/lib/<tuple>, but Buildroot copies them to
# {/usr,}/lib, so we need to create a symbolic link.
define TOOLCHAIN_OLD_EXTERNAL_LINARO_SYMLINK
       ln -snf . $(TARGET_DIR)/lib/$(TOOLCHAIN_EXTERNAL_PREFIX)
       ln -snf . $(TARGET_DIR)/usr/lib/$(TOOLCHAIN_EXTERNAL_PREFIX)
endef

$(eval $(virtual-package))

# Ensure the external-toolchain package has a prefix defined.
# This comes after the virtual-package definition, which checks the provider.
else ifeq ($(BR2_TOOLCHAIN_EXTERNAL_LINARO_ARM_201405),y)
TOOLCHAIN_EXTERNAL_SITE = http://releases.linaro.org/14.05/components/toolchain/binaries
TOOLCHAIN_EXTERNAL_SOURCE = gcc-linaro-arm-linux-gnueabihf-4.9-2014.05_linux.tar.xz
TOOLCHAIN_EXTERNAL_POST_INSTALL_STAGING_HOOKS += TOOLCHAIN_OLD_EXTERNAL_LINARO_SYMLINK
else ifeq ($(BR2_TOOLCHAIN_EXTERNAL_LINARO_ARM201204),y)
TOOLCHAIN_EXTERNAL_SITE = https://launchpad.net/linaro-toolchain-binaries/trunk/2012.04/+download/
TOOLCHAIN_EXTERNAL_SOURCE = gcc-linaro-arm-linux-gnueabi-2012.04-20120426_linux.tar.bz2
ifeq ($(BR2_TOOLCHAIN_EXTERNAL),y)
ifeq ($(call qstrip,$(BR2_TOOLCHAIN_EXTERNAL_PREFIX)),)
else ifeq ($(BR2_TOOLCHAIN_EXTERNAL_LINARO_AARCH64_201409),y)
TOOLCHAIN_EXTERNAL_SITE = http://releases.linaro.org/14.09/components/toolchain/binaries
TOOLCHAIN_EXTERNAL_SOURCE = gcc-linaro-aarch64-linux-gnu-4.9-2014.09_linux.tar.xz
TOOLCHAIN_EXTERNAL_POST_INSTALL_STAGING_HOOKS += TOOLCHAIN_OLD_EXTERNAL_LINARO_SYMLINK
$(error No prefix selected for external toolchain package $(BR2_PACKAGE_PROVIDES_TOOLCHAIN_EXTERNAL). Configuration error)
endif
endif

include toolchain/toolchain-external/*/*.mk

###############################################################
#
# x264
#
###############################################################

X264_VERSION = 20140930-2245-stable
X264_SOURCE = x264-snapshot-$(X264_VERSION).tar.bz2
X264_SITE = ftp://ftp.videolan.org/pub/videolan/x264/snapshots
X264_LICENSE = GPLv2+
X264_DEPENDENCIES = host-pkgconf
X264_LICENSE_FILES = COPYING
X264_INSTALL_STAGING = YES

ifeq ($(BR2_i386)$(BR2_x86_64),y)
X264_DEPENDENCIES += host-yasm
else ifeq ($(BR2_ARM_CPU_ARMV7A),y)
# We need to pass gcc as AS, because the ARM assembly files have to be
# preprocessed
X264_CONF_ENV += AS="$(TARGET_CC)"
else
X264_CONF_OPTS += --disable-asm
endif

ifeq ($(BR2_PREFER_STATIC_LIB),)
X264_CONF_OPTS += --enable-pic --enable-shared
endif

ifeq ($(BR2_PACKAGE_X264_CLI),)
X264_CONF_OPTS += --disable-cli
endif

ifeq ($(BR2_TOOLCHAIN_HAS_THREADS),)
X264_CONF_OPTS += --disable-thread
endif

# the configure script is not generated by autoconf
define X264_CONFIGURE_CMDS
	(cd $(@D); $(TARGET_CONFIGURE_OPTS) $(X264_CONF_ENV) ./configure \
		--prefix=/usr \
		--host="$(GNU_TARGET_NAME)" \
		--cross-prefix="$(TARGET_CROSS)" \
		--disable-ffms \
		--enable-static \
		--disable-opencl \
		$(X264_CONF_OPTS) \
	)
endef

define X264_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define X264_INSTALL_STAGING_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) DESTDIR="$(STAGING_DIR)" -C $(@D) install
endef

define X264_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) DESTDIR="$(TARGET_DIR)" -C $(@D) install
endef

$(eval $(generic-package))

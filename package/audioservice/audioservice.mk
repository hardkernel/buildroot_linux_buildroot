################################################################################
#
# audioservice
#
################################################################################

AUDIOSERVICE_VERSION = 1.0.0
AUDIOSERVICE_SITE = $(TOPDIR)/../vendor/amlogic/audioservice
AUDIOSERVICE_SITE_METHOD = local
#AUDIOSERVICE_LICENSE = GPLv2+, GPLv2 (py-smbus)
#AUDIOSERVICE_LICENSE_FILES = COPYING
AUDIOSERVICE_INSTALL_STAGING = YES
AUDIOSERVICE_AUTORECONF = YES

AUDIOSERVICE_DEPENDENCIES = ffmpeg dbus cjson aml_halaudio

define AUDIOSERVICE_LIB_INSTALL_CMD

endef


AUDIOSERVICe_CONF_OPTS = --prefix=$(TARGET_DIR)/usr

define AUDIOSERVICE_INSTALL_INIT_SYSV
	$(INSTALL) -D -m 755 package/audioservice/S90audioservice \
		$(TARGET_DIR)/etc/init.d/S90audioservice
endef

# Autoreconf requires an m4 directory to exist
define AUDIOSERVICE_PATCH_M4
	mkdir -p $(@D)/m4
endef

AUDIOSERVICE_POST_PATCH_HOOKS += AUDIOSERVICE_PATCH_M4



$(eval $(autotools-package))

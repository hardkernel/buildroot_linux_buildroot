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

AUDIOSERVICE_DEPENDENCIES = dbus cjson aml_halaudio

ifeq ($(BR2_PACKAGE_AUDIOSERVICE_ASR),y)
AUDIOSERVICE_CONF_OPTS += --enable-asr
endif

ifeq ($(BR2_PACKAGE_AUDIOSERVICE_PULSEAUDIO),y)
AUDIOSERVICE_CONF_OPTS += --enable-pulseaudio
AUDIOSERVICE_DEPENDENCIES += pulseaudio
endif

ifeq ($(BR2_PACKAGE_AUDIOSERVICE_FFMPEG),y)
AUDIOSERVICE_CONF_OPTS += --enable-ffmpeg
AUDIOSERVICE_DEPENDENCIES += ffmpeg
endif

ifeq ($(BR2_PACKAGE_AUDIOSERVICE_PYTHON),y)
AUDIOSERVICE_CONF_OPTS += --enable-python
AUDIOSERVICE_DEPENDENCIES += python
endif

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

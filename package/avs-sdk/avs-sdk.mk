################################################################################
#
#avs-sdk
#
################################################################################

AVS_SDK_SITE = $(TOPDIR)/../multimedia/avs
AVS_SDK_SITE_METHOD = local
AVS_SDK_LICENSE = Apache License 2.0
AVS_SDK_LICENSE_FILES = LICENSE
AVS_SDK_DEPENDENCIES = libgmime \
                       libtotem \
                       libcurl \
                       nghttp2 \
                       openssl \
                       sqlite \
                       gstreamer1 \
                       gst1-plugins-base \
                       pkgconf
AVS_SDK_CONF_OPTS = \
                    -DCMAKE_TOOLCHAIN_FILE=$(TOPDIR)/package/avs-sdk/CMakeLists.txt \
                    -DGSTREAMER_MEDIA_PLAYER=ON \
                    -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON \
                    -DTOTEM_PLPARSER=ON

AVS_OUT_DIR = $(@D)/../AVS_output
ifeq ($(BR2_PACKAGE_LIBXML2),y)
AVS_SDK_DEPENDENCIES += libxml2
endif

ifeq ($(BR2_PACKAGE_LIBFFI),y)
AVS_SDK_DEPENDENCIES += libffi
endif

ifeq ($(BR2_PACKAGE_LIBSOUP),y)
AVS_SDK_DEPENDENCIES += libsoup
endif

ifeq ($(BR2_PACKAGE_ZLIB),y)
AVS_SDK_DEPENDENCIES += zlib
endif

ifeq ($(BR2_PACKAGE_UTIL_LINUX),y)
AVS_SDK_DEPENDENCIES += util-linux
endif

ifeq ($(BR2_PACKAGE_LIBFAAD_DEV),y)
AVS_SDK_DEPENDENCIES += libfaad-dev
endif

ifeq ($(BR2_PACKAGE_PORTAUDIO),y)
AVS_SDK_DEPENDENCIES += portaudio
AVS_SDK_CONF_OPTS += -DPORTAUDIO:BOOL=ON
AVS_SDK_CONF_OPTS += -DPORTAUDIO_LIB_PATH=${BUILD_DIR}/portaudio-v190600_20161030/lib/.libs/libportaudio.so
AVS_SDK_CONF_OPTS += -DPORTAUDIO_INCLUDE_DIR=$(BUILD_DIR)/portaudio-v190600_20161030/include
endif

ifeq ($(BR2_PACKAGE_SENSORY),y)
AVS_SDK_DEPENDENCIES += sensory
AVS_SDK_CONF_OPTS += -DSENSORY_KEY_WORD_DETECTOR=ON
endif

ifeq ($(BR2_PACKAGE_KITT_AI),y)
AVS_SDK_DEPENDENCIES += kitt-ai
AVS_SDK_CONF_OPTS += -DKITTAI_KEY_WORD_DETECTOR=ON
endif

ifeq ($(BR2_AVS_DSPC),y)
#AVS_SDK_DEPENDENCIES += dsp
AVS_SDK_CONF_OPTS += -DDSP_KEY_WORD_DETECTOR=ON
AVS_SDK_CONF_OPTS += -DDSP_KEY_WORD_DETECTOR_LIB_PATH=$(BUILD_DIR)/avs-sdk/KWD/DSP/lib/libAWELib.so
AVS_SDK_CONF_OPTS += -DDSP_KEY_WORD_DETECTOR_INCLUDE_DIR=$(BUILD_DIR)/avs-sdk/KWD/DSP/include
endif


define AVS_SDK_CONFIGURE_CMDS
	rm -f $(STAGING_DIR)/usr/lib/libgtest.a
	rm -f $(STAGING_DIR)/usr/lib/libgtest_main.a
	rm -rf $(STAGING_DIR)/usr/include/gtest
	rm -f $(STAGING_DIR)/usr/lib/gmock.a
	rm -f $(STAGING_DIR)/usr/lib/gmock_mian.a
	rm -rf $(STAGING_DIR)/usr/include/gmock
	echo -e 'SET(CMAKE_SYSTEM_NAME Linux)' > $(TOPDIR)/package/avs-sdk/CMakeLists.txt
	echo -e 'SET(CMAKE_C_COMPILER $(TARGET_CC))' >> $(TOPDIR)/package/avs-sdk/CMakeLists.txt
	echo -e 'SET(CMAKE_CXX_COMPILER $(TARGET_CXX))' >> $(TOPDIR)/package/avs-sdk/CMakeLists.txt
	echo -e 'SET(CMAKE_FIND_ROOT_PATH $(HOST_DIR) $(STAGING_DIR))' >> $(TOPDIR)/package/avs-sdk/CMakeLists.txt
	echo -e 'SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)' >> $(TOPDIR)/package/avs-sdk/CMakeLists.txt
	echo -e 'SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)' >> $(TOPDIR)/package/avs-sdk/CMakeLists.txt
endef

define AVS_SDK_BUILD_CMDS
	mkdir -p $(@D)/../AVS_output
	(export LDFLAGS="-Wl,-rpath-link=$(STAGING_DIR)/lib -L$(STAGING_DIR)/lib" && cd $(@D)/../AVS_output && export PKG_CONFIG_PATH=$(STAGING_DIR)/usr/lib/pkgconfig:$$PKG_CONFIG_PATH && cmake $(AVS_SDK_CONF_OPTS) $(@D) && make)

endef
define AVS_SDK_INSTALL_TARGET_CMDS
	cp $(@D)/KWD/DSP/lib/libAWELib.so $(TARGET_DIR)/usr/lib/AWELib.so
	cp $(AVS_OUT_DIR)/Integration/src/libIntegration.so $(TARGET_DIR)/usr/lib
	cp $(AVS_OUT_DIR)/ADSL/src/libADSL.so  $(TARGET_DIR)/usr/lib
	cp $(AVS_OUT_DIR)/MediaPlayer/src/libMediaPlayer.so  $(TARGET_DIR)/usr/lib
	cp $(AVS_OUT_DIR)/PlaylistParser/src/libPlaylistParser.so  $(TARGET_DIR)/usr/lib
	cp $(AVS_OUT_DIR)/ACL/src/libACL.so  $(TARGET_DIR)/usr/lib
	cp $(AVS_OUT_DIR)/AFML/src/libAFML.so   $(TARGET_DIR)/usr/lib
	cp $(AVS_OUT_DIR)/AuthDelegate/src/libAuthDelegate.so  $(TARGET_DIR)/usr/lib
	cp $(AVS_OUT_DIR)/CapabilityAgents/AIP/src/libAIP.so  $(TARGET_DIR)/usr/lib
	cp $(AVS_OUT_DIR)/CapabilityAgents/Alerts/src/libAlerts.so   $(TARGET_DIR)/usr/lib
	cp $(AVS_OUT_DIR)/CapabilityAgents/System/src/libAVSSystem.so  $(TARGET_DIR)/usr/lib
	cp $(AVS_OUT_DIR)/CapabilityAgents/SpeechSynthesizer/src/libSpeechSynthesizer.so  $(TARGET_DIR)/usr/lib
	cp $(AVS_OUT_DIR)/KWD/DSP/src/libDSP.so   $(TARGET_DIR)/usr/lib
	cp $(AVS_OUT_DIR)/KWD/src/libKWD.so  $(TARGET_DIR)/usr/lib
	cp $(AVS_OUT_DIR)/ContextManager/src/libContextManager.so   $(TARGET_DIR)/usr/lib
	cp $(AVS_OUT_DIR)/AVSCommon/libAVSCommon.so  $(TARGET_DIR)/usr/lib
	cp $(AVS_OUT_DIR)/CapabilityAgents/AudioPlayer/src/libAudioPlayer.so   $(TARGET_DIR)/usr/lib
	cp $(AVS_OUT_DIR)/ApplicationUtilities/DefaultClient/src/libDefaultClient.so   $(TARGET_DIR)/usr/lib

	cp $(AVS_OUT_DIR)/Integration/test/AlertsIntegrationTest $(TARGET_DIR)/usr/bin
	cp $(AVS_OUT_DIR)/Integration/test/AlexaCommunicationsLibraryTest  $(TARGET_DIR)/usr/bin
	cp $(AVS_OUT_DIR)/Integration/test/AudioInputProcessorIntegrationTest  $(TARGET_DIR)/usr/bin
	cp $(AVS_OUT_DIR)/Integration/test/AlexaAuthorizationDelegateTest  $(TARGET_DIR)/usr/bin
	cp $(AVS_OUT_DIR)/Integration/test/AlexaDirectiveSequencerLibraryTest  $(TARGET_DIR)/usr/bin
	cp $(AVS_OUT_DIR)/SampleApp/src/SampleApp   $(TARGET_DIR)/usr/bin
	mkdir -p $(TARGET_DIR)/usr/share/avs/
	cp $(@D)/Media/*.raw $(TARGET_DIR)/usr/bin/
	cp $(@D)/Media/*.mp3 $(TARGET_DIR)/usr/share/avs/
	cp $(@D)/Media/*.awb $(TARGET_DIR)/usr/bin/
	cp $(@D)/Media/ca-certificates.crt $(TARGET_DIR)/etc/ssl/certs
endef

define AVS_SDK_CLEAN_CMDS
	rm -rf $(AVS_OUT_DIR)
	rm -rf $(TARGET_DIR)/avs
	rm -rf $(@D)
	rm $(TARGET_DIR)/usr/lib/AWELib.so
	rm $(TARGET_DIR)/usr/lib/libIntegration.so
	rm $(TARGET_DIR)/usr/lib/libADSL.so
	rm $(TARGET_DIR)/usr/lib/libMediaPlayer.so
	rm $(TARGET_DIR)/usr/lib/libPlaylistParser.so
	rm $(TARGET_DIR)/usr/lib/libACL.so
	rm $(TARGET_DIR)/usr/lib/libAFML.so
	rm $(TARGET_DIR)/usr/lib/libAuthDelegate.so
	rm $(TARGET_DIR)/usr/lib/libAIP.so
	rm $(TARGET_DIR)/usr/lib/libAlerts.so
	rm $(TARGET_DIR)/usr/lib/libAVSSystem.so
	rm $(TARGET_DIR)/usr/lib/libSpeechSynthesizer.so
	rm $(TARGET_DIR)/usr/lib/libDSP.so
	rm $(TARGET_DIR)/usr/lib/libKWD.so
	rm $(TARGET_DIR)/usr/lib/libKWD.so
	rm $(TARGET_DIR)/usr/lib/libContextManager.so
	rm $(TARGET_DIR)/usr/lib/libAVSCommon.so
	rm $(TARGET_DIR)/usr/lib/libAudioPlayer.so
	rm $(TARGET_DIR)/usr/lib/libDefaultClient.so
	rm $(TARGET_DIR)/usr/bin/AlertsIntegrationTest
	rm $(TARGET_DIR)/usr/bin/AlexaCommunicationsLibraryTest
	rm $(TARGET_DIR)/usr/bin/AudioInputProcessorIntegrationTest
	rm $(TARGET_DIR)/usr/bin/AlexaAuthorizationDelegateTest
	rm $(TARGET_DIR)/usr/bin/AlexaDirectiveSequencerLibraryTest
	rm $(TARGET_DIR)/usr/bin/SampleApp
	rm $(TARGET_DIR)/usr/bin/
	rm -rf $(TARGET_DIR)/usr/share/avs/
endef

$(eval $(generic-package))

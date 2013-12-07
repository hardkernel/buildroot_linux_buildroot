LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS:= optional

LOCAL_SHARED_LIBRARIES := \
	libutils \
	libcutils \
	libmedia \
	
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \

ifeq ($(BOARD_ALSA_AUDIO),tiny)

LOCAL_SHARED_LIBRARIES += \
	libtinyalsa \

LOCAL_C_INCLUDES += \
	external/tinyalsa/include/tinyalsa \

else

LOCAL_SHARED_LIBRARIES += \
	libasound \

LOCAL_C_INCLUDES += \
	external/alsa-lib/include \
	   
endif
	
	
LOCAL_SRC_FILES:= \
	amAudio.cpp \
	line_in_select_channel.cpp \

LOCAL_MODULE:= AmlogicAudioTest

include $(BUILD_EXECUTABLE)
###########################
#
# SDL2_codec Prebuilt library
#
###########################

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := SDL2_codec
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libSDL2_codec.so

include $(PREBUILT_SHARED_LIBRARY)

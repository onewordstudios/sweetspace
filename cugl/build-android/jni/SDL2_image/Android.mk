###########################
#
# SDL2_image Prebuilt library
#
###########################

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := SDL2_image
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libSDL2_image.so

include $(PREBUILT_SHARED_LIBRARY)

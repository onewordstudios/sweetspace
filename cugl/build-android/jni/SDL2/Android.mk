###########################
#
# SDL2 Prebuilt library
#
###########################

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := SDL2
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libSDL2.so

include $(PREBUILT_SHARED_LIBRARY)
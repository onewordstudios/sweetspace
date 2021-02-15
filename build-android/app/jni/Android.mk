########################
# 
# Linker Makefile
#
########################

LOCAL_SHORT_COMMANDS := true

CACHE_PATH := $(call my-dir)
LOCAL_PATH := $(CACHE_PATH)
PROJ_PATH  := $(CACHE_PATH)
CUGL_PATH  := $(CACHE_PATH)/../../../cugl/build-android
FIREBASE_CPP_SDK_DIR  := $(CACHE_PATH)/../../../firebase

STL:=$(firstword $(subst _, ,$(APP_STL)))
FIREBASE_LIBRARY_PATH := $(FIREBASE_CPP_SDK_DIR)/libs/android/$(TARGET_ARCH_ABI)/$(STL)

include $(CUGL_PATH)/jni/Android.mk
include $(PROJ_PATH)/source/Android.mk

# basic firebase library
include $(CLEAR_VARS)
LOCAL_MODULE:=firebase_app
LOCAL_SRC_FILES:=$(FIREBASE_LIBRARY_PATH)/libfirebase_app.a
LOCAL_EXPORT_C_INCLUDES:=$(FIREBASE_CPP_SDK_DIR)/include
include $(PREBUILT_STATIC_LIBRARY)

# analytics
include $(CLEAR_VARS)
LOCAL_MODULE:=firebase_analytics
LOCAL_SRC_FILES:=$(FIREBASE_LIBRARY_PATH)/libfirebase_analytics.a
LOCAL_EXPORT_C_INCLUDES:=$(FIREBASE_CPP_SDK_DIR)/include
include $(PREBUILT_STATIC_LIBRARY)

# admob
include $(CLEAR_VARS)
LOCAL_MODULE:=firebase_admob
LOCAL_SRC_FILES:=$(FIREBASE_LIBRARY_PATH)/libfirebase_admob.a
LOCAL_EXPORT_C_INCLUDES:=$(FIREBASE_CPP_SDK_DIR)/include
include $(PREBUILT_STATIC_LIBRARY)



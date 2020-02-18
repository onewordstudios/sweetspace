###########################
#
# CUGL static library
#
###########################
LOCAL_PATH := $(call my-dir)
CUGL_PATH  := $(LOCAL_PATH)/../../..
include $(CLEAR_VARS)

LOCAL_MODULE := CUGL

LOCAL_C_INCLUDES := $(CUGL_PATH)/include

# Add your application source files here...
LOCAL_SRC_FILES := \
	$(subst $(LOCAL_PATH)/,, \
	$(wildcard $(CUGL_PATH)/lib/base/*.cpp) \
	$(wildcard $(CUGL_PATH)/lib/base/platform/*.cpp) \
	$(wildcard $(CUGL_PATH)/lib/util/*.cpp) \
	$(wildcard $(CUGL_PATH)/lib/math/*.cpp) \
	$(wildcard $(CUGL_PATH)/lib/math/*.c) \
	$(wildcard $(CUGL_PATH)/lib/math/polygon/*.cpp) \
	$(wildcard $(CUGL_PATH)/lib/math/dsp/*.cpp) \
	$(wildcard $(CUGL_PATH)/lib/input/*.cpp) \
	$(wildcard $(CUGL_PATH)/lib/input/gestures/*.cpp) \
	$(wildcard $(CUGL_PATH)/lib/io/*.cpp) \
	$(wildcard $(CUGL_PATH)/lib/renderer/*.cpp) \
	$(wildcard $(CUGL_PATH)/lib/audio/*.cpp) \
	$(wildcard $(CUGL_PATH)/lib/audio/codecs/*.cpp) \
	$(wildcard $(CUGL_PATH)/lib/audio/graph/*.cpp) \
	$(wildcard $(CUGL_PATH)/lib/assets/*.cpp) \
	$(wildcard $(CUGL_PATH)/lib/2d/*.cpp) \
	$(wildcard $(CUGL_PATH)/lib/2d/actions/*.cpp) \
	$(wildcard $(CUGL_PATH)/lib/2d/layout/*.cpp) \
	$(wildcard $(CUGL_PATH)/lib/2d/physics/*.cpp) \
	$(wildcard $(CUGL_PATH)/lib/ai/behavior/*.cpp) \
	$(wildcard $(CUGL_PATH)/external/cJSON/*.c) \
	$(wildcard $(CUGL_PATH)/external/Box2D/Collision/*.cpp) \
	$(wildcard $(CUGL_PATH)/external/Box2D/Collision/Shapes/*.cpp) \
	$(wildcard $(CUGL_PATH)/external/Box2D/Common/*.cpp) \
	$(wildcard $(CUGL_PATH)/external/Box2D/Dynamics/*.cpp) \
	$(wildcard $(CUGL_PATH)/external/Box2D/Dynamics/Contacts/*.cpp) \
	$(wildcard $(CUGL_PATH)/external/Box2D/Dynamics/Joints/*.cpp) \
	$(wildcard $(CUGL_PATH)/external/Box2D/Rope/*.cpp))

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES

LOCAL_EXPORT_LDLIBS := -Wl,--undefined=Java_org_libsdl_app_SDLActivity_nativeInit -ldl -lGLESv1_CM -lGLESv2 -lGLESv3 -llog -landroid -latomic
include $(BUILD_STATIC_LIBRARY)

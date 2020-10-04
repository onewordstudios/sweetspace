//
//  CUDisplay-SDL.cpp
//  Cornell University Game Library (CUGL)
//
//  The SDL display information tools fail on OS X and iOS.  Therefore, we have
//  factored this information out into platform specific files.  This module
//  is the fallback for devices properly supported by SDL.
//
//  CUGL MIT License:
//      This software is provided 'as-is', without any express or implied
//      warranty.  In no event will the authors be held liable for any damages
//      arising from the use of this software.
//
//      Permission is granted to anyone to use this software for any purpose,
//      including commercial applications, and to alter it and redistribute it
//      freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//
//  Author: Walker White
//  Version: 12/12/18
#include "CUDisplay-impl.h"
#include <cugl/base/CUDisplay.h>
#if defined __ANDROID__
	#include <jni.h>
#endif

/**
 * Returns the full screen resolution for this display in points
 *
 * This method returns the bounds for the current resolution, not the
 * maximum resolution.  You should never change the resolution of a display.
 * Allow the user to have their preferred resolution.  Instead, you should
 * adjust your camera to scale the viewport.
 *
 * The value returned represents points, not pixels.  If you are using a
 * traditional display, these are the same.  However, on Retina displays
 * and other high DPI monitors, these may be different.  Regardless, you
 * should always work with points, not pixels, when computing the screen
 * size.  In particular, this is what you should assign the OpenGL viewport
 * when using fullscreen.
 *
 * @return the full screen resolution for this display in points
 */
cugl::RectCugl cugl::impl::DisplayBounds() {
	cugl::RectCugl result;
	SDL_Rect bounds;
	SDL_GetDisplayBounds(0,&bounds);
	Vec2 scale = DisplayPixelDensity();
	
	result.origin.x = bounds.x/scale.x;
	result.origin.y = bounds.y/scale.x;
	result.size.width  = bounds.w/scale.x;
	result.size.height = bounds.h/scale.x;

	return result;
}

/**
 * Returns the usable full screen resolution for this display in points
 *
 * Usable is a subjective term defined by the operating system.  In
 * general, it means the full screen minus any space used by important
 * user interface elements, like a status bar (iPhone), menu bar (OS X),
 * or task bar (Windows).
 *
 * Because the usable bounds depends on orientation, it is possible to
 * specify the orientation to compute the bounds.  If the orientation
 * is unknown or on face (face-up/face-down), this will use the current
 * orientation of the display (not the device).
 *
 * The value returned represents points, not pixels.  If you are using a
 * traditional display, these are the same.  However, on Retina displays
 * and other high DPI monitors, these may be different.  Regardless, you
 * should always work with points, not pixels, when computing the screen
 * size.
 *
 * @param orientation   The orientation to compute the bounds for
 *
 * @return the usable full screen resolution for this display in points
 */
cugl::RectCugl cugl::impl::DisplayUsableBounds(cugl::Display::Orientation orientation) {
    cugl::RectCugl result;
    SDL_Rect bounds;
    SDL_GetDisplayBounds(0,&bounds);
    Vec2 scale = DisplayPixelDensity();

#if defined __ANDROID__
	bool displayportrait = bounds.w < bounds.h;
	bool deviceportrait  = displayportrait;
    if (orientation != cugl::Display::Orientation::UNKNOWN &&
        orientation != cugl::Display::Orientation::FACE_UP &&
        orientation != cugl::Display::Orientation::FACE_DOWN) {
        deviceportrait = (orientation == cugl::Display::Orientation::PORTRAIT ||
                    	  orientation == cugl::Display::Orientation::UPSIDE_DOWN);
    }
    
    // Usable bounds on Android are given by the viewport
	GLint m_viewport[4];
    glGetIntegerv( GL_VIEWPORT, m_viewport );
    bounds.x = m_viewport[0];
    bounds.y = m_viewport[1];
    bounds.w = m_viewport[2];
    bounds.h = m_viewport[3];
    if (displayportrait != deviceportrait) {
    	int temp = bounds.w;
    	bounds.w = bounds.h;
    	bounds.h = temp;
    }
    
	// Except we override this later
	// TODO: Track state for android
    
#endif	
	result.origin.x = bounds.x/scale.x;
    result.origin.y = bounds.y/scale.x;
    result.size.width  = bounds.w/scale.x;
    result.size.height = bounds.h/scale.x;

    return result;
}

/**
 * Returns the number of pixels for each point.
 *
 * A point is a logical screen pixel.  If you are using a traditional
 * display, points and pixels are the same.  However, on Retina displays
 * and other high dpi monitors, they may be different.  In particular,
 * the number of pixels per point is a scaling factor times the point.
 *
 * You should never need to use these scaling factor for anything, as it
 * is not useful for determining anything other than whether a high DPI
 * display is present. It does not necessarily refer to physical pixel on
 * the screen.
 *
 * @return the number of pixels for each point.
 */
cugl::Vec2 cugl::impl::DisplayPixelDensity() {
#if defined __ANDROID__
	// retrieve the JNI environment.
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass clazz(env->GetObjectClass(activity));
    jmethodID method_id = env->GetStaticMethodID(clazz, "convertDpToPixel", "(F)I");
    int result = env->CallStaticIntMethod(clazz, method_id, 1.0f);

    // Clean up
	env->DeleteLocalRef(activity);
    env->DeleteLocalRef(clazz);
    return Vec2(result,result);
#elif defined __WINDOWS__
    float ddpi;
    cugl::Vec2 result;
    SDL_GetDisplayDPI(0,&ddpi,&result.x,&result.y);
    result /= 72;
    if (result.isZero()) { result.set(1,1); }
    return result;
#else
    float ddpi;
    cugl::Vec2 result;
    SDL_GetDisplayDPI(0,&ddpi,&result.x,&result.y);
    result /= 72;
    if (result.isZero()) { result.set(1,1); }
    return result;
#endif
}

/**
 * Returns the current orientation (display or device).
 *
 * The display orientation is the orientation of the coordinate space.
 * In other words, the origin is at the bottom left of the screen in
 * this orientation.  The device orientation is a mobile device, as
 * held by the user.
 *
 * This may or may not agree with the each other.  In particular, they
 * will not agree if the display orientation is locked (to say portrait or
 * landscape only).
 *
 * @param display   Whether to use the display (as opposed to the device) orientation
 *
 * @return the current orientation (display or device).
 */
cugl::Display::Orientation cugl::impl::DisplayOrientation(bool display) {
#if defined __ANDROID__
	// retrieve the JNI environment.
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass clazz(env->GetObjectClass(activity));
    jmethodID method_id = env->GetStaticMethodID(clazz, display ? "getCurrentOrientation" : "getDeviceOrientation", "()I");
    int current = env->CallStaticIntMethod(clazz, method_id);
    
    // Clean up
	env->DeleteLocalRef(activity);
    env->DeleteLocalRef(clazz);
    
	switch (current) {
		case 0:
			return cugl::Display::Orientation::UNKNOWN;
		case 1:
			return cugl::Display::Orientation::LANDSCAPE;
		case 2:
			return cugl::Display::Orientation::LANDSCAPE_REVERSED;
		case 3:
			return cugl::Display::Orientation::PORTRAIT;
		case 4:
			return cugl::Display::Orientation::UPSIDE_DOWN;
		default:
			return cugl::Display::Orientation::UNKNOWN;
	}
#else
	return cugl::Display::Orientation::FIXED;
#endif
}

/**
 * Returns the default orientation of this device.
 *
 * The default orientation corresponds to the intended orientiation
 * that this mobile device should be held.  For devices with home
 * buttons, this home button is always expected at the bottom. For
 * the vast majority of devices, this means the intended orientation
 * is Portrait.  However, some Samsung tablets have the home button
 * oriented for Landscape.
 *
 * This is important because the accelerometer axis is oriented
 * relative to the default orientation.  So a default landscape device
 * will have a different accelerometer orientation than a portrait
 * device.
 *
 * If this display is not a mobile device, it will always return
 * {@link Display::Orientation::Fixed}.
 *
 * @return the default orientation of this device.
 */

cugl::Display::Orientation cugl::impl::DisplayDefaultOrientation() {
#if defined __ANDROID__
	// retrieve the JNI environment.
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass clazz(env->GetObjectClass(activity));
    jmethodID method_id = env->GetStaticMethodID(clazz, "getDeviceDefaultOrientation", "()I");
    int current = env->CallStaticIntMethod(clazz, method_id);
    
    // Clean up
	env->DeleteLocalRef(activity);
    env->DeleteLocalRef(clazz);
    
	switch (current) {
		case 0:
			return cugl::Display::Orientation::UNKNOWN;
		case 1:
			return cugl::Display::Orientation::LANDSCAPE;
		case 2:
			return cugl::Display::Orientation::LANDSCAPE_REVERSED;
		case 3:
			return cugl::Display::Orientation::PORTRAIT;
		case 4:
			return cugl::Display::Orientation::UPSIDE_DOWN;
		default:
			return cugl::Display::Orientation::UNKNOWN;
	}
#else
	return cugl::Display::Orientation::FIXED;
#endif
}

/**
 * Returns true if this device has a notch.
 *
 * Notched devices are edgeless smartphones or tablets that include at
 * dedicated area in the screen for a camera.  Examples include the
 * iPhone X.
 *
 * If a device is notched you should call {@link getUsableDisplayBounds()}
 * before laying out UI elements.  It is acceptable to animate and draw
 * backgrounds behind the notch, but it is not acceptable to place UI
 * elements outside of these bounds.
 *
 * @return true if this device has a notch.
 */
bool cugl::impl::DisplayNotch() {
#if defined __ANDROID__
	// retrieve the JNI environment.
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass clazz(env->GetObjectClass(activity));
    jmethodID method_id = env->GetStaticMethodID(clazz, "hasNotch", "()Z");
    bool result = env->CallStaticBooleanMethod(clazz, method_id);

    // Clean up
	env->DeleteLocalRef(activity);
    env->DeleteLocalRef(clazz);

    return result;
#else
	return false;
#endif
}

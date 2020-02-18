//
//  CUDisplay-Mac.mm
//  Cornell University Game Library (CUGL)
//
//  The SDL display information tools fail on OS X and iOS.  Therefore, we have
//  factored this information out into platform specific files.  This module
//  is the OS X implementation.
//
//  Note the .mm suffix.  That suffix is necessary in XCode for any module that
//  combines C++ and Objective-C code.
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
#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <AppKit/AppKit.h>

#include "CUDisplay-impl.h"

/**
 * Returns the full screen resolution for this display
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
 */
cugl::Rect cugl::impl::DisplayBounds() {
    cugl::Rect result;
    
    NSRect screenRect;
    NSScreen* screen = [NSScreen mainScreen];
    screenRect = [screen frame];
    result.origin.x = screenRect.origin.x;
    result.origin.y = screenRect.origin.y;
    result.size.width  = screenRect.size.width;
    result.size.height = screenRect.size.height;
    
    return result;
}

/**
 * Returns the usable full screen resolution for this display
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
 * @return the usable full screen resolution for this display
 */
cugl::Rect cugl::impl::DisplayUsableBounds(cugl::Display::Orientation orientation) {
    cugl::Rect result;
    
    NSRect screenRect;
    NSScreen* screen = [NSScreen mainScreen];
    
    // This only matters if full screen
    //screenRect = [screen visibleFrame];
    screenRect = [screen frame];
    result.origin.x = screenRect.origin.x;
    result.origin.y = screenRect.origin.y;
    result.size.width  = screenRect.size.width;
    result.size.height = screenRect.size.height;
    
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
 * the screen.  In this case it refers to the pixel density of the backing
 * framebuffer, which may be different from the physical framebuffer.
 *
 * @return the number of pixels for each point.
 */
cugl::Vec2 cugl::impl::DisplayPixelDensity() {
    CGFloat scale = [[NSScreen mainScreen] backingScaleFactor];
    return Vec2(scale,scale);
}

/**
 * Returns the current orientation (display or device).
 *
 * The display orientation is the orientation of the coordinate space.
 * In other words, the origin is at the bottom left of the screen in
 * this orientation.  The device orientation is the orientation of a
 * mobile device, as held by the user.
 *
 * This may or may not agree with the each other.  In particular, they
 * will not agree if the display orientation is locked (to say portrait or
 * landscape only).
 *
 * If this display is not a mobile device, it will always return
 * {@link Display::Orientation::Fixed}.
 *
 * @param display   Whether to use the display (as opposed to the device) orientation
 *
 * @return the current orientation (display or device).
 */
cugl::Display::Orientation cugl::impl::DisplayOrientation(bool display) {
    return cugl::Display::Orientation::FIXED;
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
    return cugl::Display::Orientation::FIXED;
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
    return false;
}

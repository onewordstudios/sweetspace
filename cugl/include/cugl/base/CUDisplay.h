//
//  CUDisplay.h
//  Cornell University Game Library (CUGL)
//
//  This module is a singleton providing display information about the device.
//  Originally, we had made this part of Application.  However, we discovered
//  that we needed platform specfic code for this, so we factored it out.
//
//  This singleton is also responsible for initializing (and disposing) the
//  OpenGL context.  That is because that context is tightly coupled to the
//  orientation information, which is provided by this class.
//
//  Because this is a singleton, there are no publicly accessible constructors
//  or intializers.  Use the static methods instead.
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
#ifndef __CU_DISPLAY_H__
#define __CU_DISPLAY_H__
#include <cugl/math/CURect.h>

namespace cugl {
    
/**
 * This class is a singleton representing the native display.
 *
 * The static methods of this class {@link start()} and {@link stop()} the
 * SDL video system.  Without it, you cannot draw anything.  This should be
 * the first and last methods called in any application. The {@link Application}
 * class does this for you automatically.
 *
 * The primary purpose of the display object is to initialize (and dispose)
 * the OpenGL context.  Any start-up features for OpenGL should go in this
 * class.  That are set via static attributes (like 
 * {@link Application#setFullscreen(bool)} that are read by the method {@link start()}. 
 * Setting these attributes after the display has started will have no effect 
 * (though this may be a feature for later releases).
 *
 * The singleton display object also has several methods to get the (current)
 * screen resolution and aspect ratio. The most important of these two is the
 * the aspect ratio.  Aspect ratio is one of the most unportable parts of
 * cross-platform development.  Everything else can be scaled to the screen,
 * but the aspect ratio is fixed from the very beginning.
 *
 * The singleton display also has information about the display and device
 * orientation for mobile devices.  In fact, is is possible to assign a
 * listener to the object to respond to changes in device orientation.
 *
 * If the device has multiple displays, this singleton will only refer to the
 * main display.
 */
class Display {
public:
    /**
     * The display aspect ratio.
     *
     * This enum includes support for almost every shipping aspect rations.
     * For information on your device, see
     *
     *      https://www.mydevice.io/#compare-devices
     *
     * With that said, Apple is making this impossible to keep up with, so
     * it is unclear how much longer we will support this enum.
     */
    enum class Aspect : unsigned int {
        /**
         * Aspect ratio of 1:1
         *
         * This is the aspect ratio of many early Blackberry devices.
         */
        SQUARE = 0,
        /**
         * Portrait aspect ratio of 3:4
         *
         * This is the portrait aspect ratio of most Apple iPads except for the
         * iPad Pro.
         */
        PORTRAIT_3_4 = 1,
        /**
         * Portrait aspect ratio of 2:3
         *
         * This is the portrait aspect ratio of older iPhones (before the 4s)
         * and Microsoft Surface 3.
         */
        PORTRAIT_2_3 = 2,
        /**
         * Portrait aspect ratio of 10:16
         *
         * This is the portrait aspect ratio of 8" and 10" Samsung tablets.
         */
        PORTRAIT_10_16 = 3,
        /**
         * Portrait aspect ratio of 3:5
         *
         * This is the portrait aspect ratio of Samsung Galaxy S tablets.
         */
        PORTRAIT_3_5  = 4,
        /**
         * Portrait aspect ratio of 9:16
         *
         * This is the portrait aspect ratio of almost all modern smart phones.
         * This includes newer iPhones and Samsung phones.
         */
        PORTRAIT_9_16 = 5,
        /**
         * Portrait aspect ratio of 9:19.5
         *
         * This is the portrait aspect ratio of iPhone X (including plus) models.
         * However, this is includes the notch, which should be accounted for.
         */
        PORTRAIT_9_19p5 = 6,
        /**
         * Portrait aspect ratio of 600:1024
         *
         * This is an unusual portrait aspect ratio for the Kindle Fire tablet
         * and the Samsung Galaxy 7.
         */
        PORTRAIT_600_1024 = 7,
        /**
         * Portrait aspect ratio of 512:683
         *
         * This is a unique portrait aspect ratio for the first generation
         * (12.9") Apple iPad Pro, carried over to later generations.
         */
        PORTRAIT_IPAD_PRO_LARGE = 8,
        /**
         * Portrait aspect ratio of 417:556
         *
         * This is a unique portrait aspect ratio for the first generation
         * (10.5") Apple iPad Pro, which only lasted one year (sigh).
         */
        PORTRAIT_IPAD_PRO_SMALL_2017 = 9,
        /**
         * Portrait aspect ratio of 417:597
         *
         * This is a unique portrait aspect ratio for the second generation
         * (11") Apple iPad Pro, which has no home button.
         */
        PORTRAIT_IPAD_PRO_SMALL_2018 = 10,
        /**
         * Landscape aspect ratio of 4:3
         *
         * This is the landscape aspect ratio of most Apple iPads except for the
         * iPad Pro.  It is also the standard definition TV aspect ratio.
         */
        LANDSCAPE_4_3 = 11,
        /**
         * Landscape aspect ratio of 3:2
         *
         * This is the landscape aspect ratio of older iPhones (before the 4s)
         * and Microsoft Surface 3.
         */
        LANDSCAPE_3_2 = 12,
        /**
         * Landscape aspect ratio of 16:10
         *
         * This is the landscape aspect ratio of 8" and 10" Samsung tablets. It
         * is also the aspect ratio of Apple desktop and notebook displays.
         */
        LANDSCAPE_16_10 = 13,
        /**
         * Landscape aspect ratio of 5:3
         *
         * This is the landscape aspect ratio of Samsung Galaxy S tablets.
         */
        LANDSCAPE_5_3 = 14,
        /**
         * Landscape aspect ratio of 16:9
         *
         * This is the landscape aspect ratio of almost all modern smart phones.
         * This includes newer iPhones and Samsung phones.  It is also the high
         * definition TV aspect ratio.
         */
        LANDSCAPE_16_9 = 15,
        /**
         * Landscape aspect ratio of 19.5:9
         *
         * This is the landscape aspect ratio of iPhone X (including plus) models.
         * However, this is includes the notch, which should be accounted for.
         */
        LANDSCAPE_19p5_19 = 16,
        /**
         * Landscape aspect ratio of 1024:600
         *
         * This is an unusual landscape aspect ratio for the Kindle Fire tablet
         * and the Samsung Galaxy 7.
         */
        LANDSCAPE_1024_600 = 17,
        /**
         * Landscape aspect ratio of 683:512
         *
         * This is a unique landscape aspect ratio for the first generation
         * (12.9") Apple iPad Pro, carried over to later generations.
         */
        LANDSCAPE_IPAD_PRO_LARGE = 18,
        /**
         * Landscape aspect ratio of 556:417
         *
         * This is a unique landscape aspect ratio for the first generation
         * (10.5") Apple iPad Pro, which only lasted one year (sigh).
         */
        LANDSCAPE_IPAD_PRO_SMALL_2017 = 19,
        /**
         * Landscape aspect ratio of 597:417
         *
         * This is a unique landscape aspect ratio for the second generation
         * (11") Apple iPad Pro, which has no home button.
         */
        LANDSCAPE_IPAD_PRO_SMALL_2018 = 20,
        /**
         * The landscape aspect ratio is unknown
         *
         * This is an error value for type safety.  It evaluates to an aspect
         * ration of 0.
         */
        UNKNOWN = 21
    };
    
    /**
     * The possible device/display orientations.
     *
     * We use the same orientations for device and display even though these
     * may not always agree (such as when the user has locked the display)
     */
    enum class Orientation : unsigned int {
        /**
         * The orientation that of a fixed display.
         *
         * This is the orientation for desktops and laptops.  This orientation
         * will never change.
         */
        FIXED = 0,
        /**
         * Landscape orientation with the right side up.
         *
         * One notched devices, this will put the notch to the left.  On devices
         * with a home button, the button will be to the right.
         */
        LANDSCAPE = 1,
        /**
         * Standard portrait orientation
         *
         * One notched devices, this will put the notch to the top.  On devices
         * with a home button, the button will be to the bottom.
         */
        PORTRAIT  = 2,
        /**
         * Landscape orientation with the left side up.
         *
         * One notched devices, this will put the notch to the right.  On devices
         * with a home button, the button will be to the left.
         */
        LANDSCAPE_REVERSED = 3,
        /**
         * Reversed portrait orientation
         *
         * One notched devices, this will put the notch to the bottom.  On devices
         * with a home button, the button will be to the top.
         *
         * Many devices (e.g. iPhones) do not allow this mode as it interferes
         * with the camera and incoming calls.
         */
        UPSIDE_DOWN = 4,
        /**
         * The device is face up.
         *
         * This is a device-only orientation, not a display orientation. Some
         * devices will not report this orientation.
         */
        FACE_UP = 5,
        /**
         * The device is face down.
         *
         * This is a device-only orientation, not a display orientation. Some
         * devices will not report this orientation.
         */
        FACE_DOWN = 6,
        /**
         * The orientation is unknown.
         *
         * This is rarely ever reported, and may mean an issue with the
         * accelerometer in the case of a mobile device.
         */
        UNKNOWN = 7,
    };
    
    /**
     * @typedef Listener
     *
     * This type represents a listener for an orientation change.
     *
     * In CUGL, listeners are implemented as a set of callback functions, not as
     * objects. For simplicity, Displays only have a single listener that handles
     * both display and device changes (see {@link getDisplayOrientation()} and
     * {@link getDeviceOrientation()}.) If you wish for more than one listener,
     * then your listener should handle its own dispatch.
     *
     * Since the device orientation will always change when the display
     * orientation does, this callback can easily safely handle both. The
     * boolean parameter in the callback indicates whether or not a display
     * orientation change has happened as well.
     *
     * Unlike other events, this callback will be invoked at the end of an
     * animation frame, after the screen has been drawn.  So it will be
     * processed before any input events waiting for the next frame.
     *
     * The function type is equivalent to
     *
     *      std::function<void(Orientation previous, Orientation current, bool display)>
     *
     * @param previous  The previous device orientation (before the change)
     * @param current   The current device orientation (after the change)
     * @param display   Whether the display orientation has changed as well
     */
    typedef std::function<void(Orientation previous, Orientation current, bool display)> Listener;
    
    // Flags for the device initialization
    /** Whether this display should use the fullscreen */
    static Uint32 INIT_FULLSCREEN;
    /** Whether this display should support a High DPI screen */
    static Uint32 INIT_HIGH_DPI;
    /** Whether this display should be multisampled */
    static Uint32 INIT_MULTISAMPLED;
    /** Whether this display should be centered (on windowed screens) */
    static Uint32 INIT_CENTERED;
    
#pragma mark Values
protected:
    /** The display singleton */
    static Display* _thedisplay;

    /** The title (Window name) of the display */
    std::string _title;
    
    /** The SDL window, which provides the OpenGL drawing context */
    SDL_Window*   _window;
    /** The associated OpenGL drawing context */
    SDL_GLContext _glContext;
    
    /** The aspect ratio (coded as the enum) */
    Aspect _aspect;
    
    /** The full screen resolution of this device */
    RectCugl _bounds;
    /** The full screen resolution minus menu bars and other features */
    RectCugl _usable;
    /** The pixel density of the device */
    Vec2 _scale;
    
    /** Whether this device has a notch in it */
    bool _notched;
    
    /** A listener for the orientation */
    Listener _orientationListener;
    /** The value of the initial orientation */
    Orientation _initialOrientation;
    /** The value of the display orientation */
    Orientation _displayOrientation;
    /** The value of the device orientation */
    Orientation _deviceOrientation;
    /** The value of the default orientation */
    Orientation _defaultOrientation;

#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new, unitialized Display.
     *
     * All of the values are set to 0 or UNKNOWN, depending on their type. You
     * must initialize the Display to access its values.
     *
     * WARNING: This class is a singleton.  You should never access this
     * constructor directly.  Use the {@link start()} method instead.
     */
    Display();
    
    /**
     * Initializes the display with the current screen information.
     *
     * This method creates a display with the given title and bounds. As part
     * of this initialization, it will create the OpenGL context, using
     * the flags provided.  The bounds are ignored if the display is fullscreen.
     * In that case, it will use the bounds of the display.
     *
     * This method gathers the native resolution bounds, pixel density, and
     * orientation  using platform-specific tools.
     *
     * WARNING: This class is a singleton.  You should never access this
     * initializer directly.  Use the {@link start()} method instead.
     *
     * @param title     The window/display title
     * @param bounds    The window/display bounds
     * @param flags     The initialization flags
     *
     * @return true if initialization was successful.
     */
    bool init(std::string title, RectCugl bounds, Uint32 flags);
    
    /**
     * Uninitializes this object, releasing all resources.
     *
     * This method quits the SDL video system and disposes the OpenGL context,
     * effectively exitting and shutting down the entire program.
     *
     * WARNING: This class is a singleton.  You should never access this
     * method directly.  Use the {@link stop()} method instead.
     */
    void dispose();
    
    /**
     * Deletes this object, releasing all resources.
     *
     * This method quits the SDL video system and disposes the OpenGL context,
     * effectively exitting and shutting down the entire program.
     *
     * WARNING: This class is a singleton.  You should never access this
     * destructor directly.  Use the {@link stop()} method instead.
     */
    ~Display() { dispose(); }
    
#pragma mark -
#pragma mark Static Accessors
public:
    /**
     * Starts up the SDL display and video system.
     *
     * This static method needs to be the first line of any application, though
     * it is handled automatically in the {@link Application} class.
     *
     * This method creates the display with the given title and bounds. As part
     * of this initialization, it will create the OpenGL context, using
     * the flags provided.  The bounds are ignored if the display is fullscreen.
     * In that case, it will use the bounds of the display.
     *
     * Once this method is called, the {@link get()} method will no longer
     * return a null value.
     *
     * @param title     The window/display title
     * @param bounds    The window/display bounds
     * @param flags     The initialization flags
     *
     * @return true if the display was successfully initialized
     */
    static bool start(std::string title, RectCugl bounds, Uint32 flags);

    /**
     * Shuts down the SDL display and video system.
     *
     * This static method needs to be the last line of any application, though
     * it is handled automatically in the {@link Application} class. It will
     * dipose of the display and the OpenGL context.
     *
     * Once this method is called, the {@link get()} method will return nullptr.
     * More importantly, no SDL function calls will work anymore.
     */
    static void stop();
    
    /**
     * Returns the singleton instance for the display
     *
     * You must call this static method first to get information about your
     * specific display.  This method will return nullptr until {@link start()}
     * is called first.
     *
     * @return the singleton instance for the display
     */
    static Display* get() { return _thedisplay; }
    
#pragma mark -
#pragma mark Window Management
    /**
     * Returns the title of this display
     *
     * On a desktop, this title will be displayed at the top of the window.
     *
     * @return the title of this display
     */
    std::string getTitle() const { return _title; }
    
    /**
     * Sets the title of this display
     *
     * On a desktop, the title will be displayed at the top of the window.
     *
     * @param title  The title of this display
     */
    void setTitle(const std::string& title) {
        setTitle(title.c_str());
    }
    
    /**
     * Sets the title of this display
     *
     * On a desktop, the title will be displayed at the top of the window.
     *
     * @param title  The title of this display
     */
    void setTitle(const char* title);
    
    /**
     * Shows the window for this display (assuming it was hidden).
     *
     * This method does nothing if the window was not hidden.
     */
    void show();

    /**
     * Hides the window for this display (assuming it was visible).
     *
     * This method does nothing if the window was not visible.
     */
    void hide();

#pragma mark -
#pragma mark Attributes
    /**
     * Returns the full screen resolution for this display in points.
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
     * @return the full screen resolution for this display in points.
     */
    RectCugl getBounds() const { return _bounds;   }
    
    /**
     * Returns the full screen resolution for this display in pixels.
     *
     * This method returns the bounds for the current resolution, not the
     * maximum resolution.  You should never change the resolution of a display.
     * Allow the user to have their preferred resolution.  Instead, you should
     * adjust your camera to scale the viewport.
     *
     * The value returned represents the value pixels, not points.  This is to
     * help align the results with input devices on Retina displays
     * and other high DPI monitors.
     *
     * @return the full screen resolution for this display in pixels.
     */
    RectCugl getPixelBounds() const { return RectCugl(_bounds.origin*_scale,_bounds.size*_scale);   }
    
    /**
     * Returns the usable full screen resolution for this display in points.
     *
     * Usable is a subjective term defined by the operating system.  In
     * general, it means the full screen minus any space used by important
     * user interface elements, like a status bar (iPhone), menu bar (OS X),
     * or task bar (Windows), or a notch (iPhone X).  In the case of the
     * latter, you can specify whether you want to use the display orientation
     * or the device orientation.
     *
     * This method computes the bounds for the current resolution, not the
     * maximum resolution.  You should never change the resolution of a display.
     * Allow the user to have their preferred resolution.  Instead, you should
     * adjust your camera to scale the viewport.
     *
     * The value returned represents points, not pixels.  If you are using a
     * traditional display, these are the same.  However, on Retina displays
     * and other high DPI monitors, these may be different.  Regardless, you
     * should always work with points, not pixels, when computing the screen
     * size.
     *
     * @param display   Whether to use the display (as opposed to the device) orientation
     *
     * @return the usable full screen resolution for this display in points.
     */
    RectCugl getUsableBounds(bool display=true);
    
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
     * the screen.  In some cases (OS X Retina displays), it refers to the
     * pixel density of the backing framebuffer, which may be different from
     * the physical framebuffer.
     *
     * @return the number of pixels for each point.
     */
    Vec2 getPixelDensity() const { return _scale;    }
    
    /**
     * Returns the aspect of this monitor.
     *
     * The aspect is returned as an enum, not a ratio.  Round off error might
     * cause devices with very similar aspect ratios to have slightly different
     * ratio values.  Therefore, the enum is a way of normalizing device aspects.
     *
     * If you would like to know the actual ratio, use the method
     * {@link getAspectRatio()} instead.  In addition, there are methods for
     * computing width from height and vice versa.
     *
     * Device aspects are relatively standardized.  For information on your
     * device, see
     *
     *      http://mydevice.io/devices/
     *
     * @return the aspect of this monitor.
     */
    Aspect getAspect() const { return _aspect;   }
    
    /**
     * Returns true if this device has a landscape orientation
     *
     * @return true if this device has a landscape orientation
     */
    bool isLandscape() const {
        return (int)_aspect >= (int)Aspect::LANDSCAPE_4_3;
    }
    
    /**
     * Returns true if this device has a portrait orientation
     *
     * @return true if this device has a portrait orientation
     */
    bool isPortrait() const {
        return ((int)_aspect < (int)Aspect::LANDSCAPE_4_3 &&
                _aspect != Aspect::SQUARE);
    }
    
    /**
     * Returns true if this device has a notch.
     *
     * Notched devices are edgeless smartphones or tablets that include at
     * dedicated area in the screen for a camera.  Examples include the
     * iPhone X.
     *
     * If a device is notched you should call {@link getUsableBounds()}
     * before laying out UI elements.  It is acceptable to animate and draw
     * backgrounds behind the notch, but it is not acceptable to place UI
     * elements outside of these bounds.
     *
     * @return true if this device has a notch.
     */
    bool hasNotch() const {
        return _notched;
    }

#pragma mark -
#pragma mark Orientation
    /**
     * Returns the initial display orientation.
     *
     * This value is the display orientation at startup. The display orientation
     * is the orientation of the coordinate space for drawing on a mobile
     * device. In other words, the origin is at the bottom left of the screen
     * in this device orientation.
     *
     * This display orientation may or may not agree with the device
     * orientation.  In particular, it will not agree if the display orientation is
     * locked (to say portrait or landscape only).  However, this is the
     * orientation that is important for drawing to the screen.  To get the
     * device orientation, call {@link getDeviceOrientation()}.
     *
     * If this display is not a mobile device,  this method will always
     * return {@link Orientation::FIXED}.
     *
     * @return the initial display orientation.
     */
    Orientation getInitialOrientation() const { return _initialOrientation; }
    
    /**
     * Returns the current display orientation.
     *
     * This value is the current display orientation. The display orientation
     * is the orientation of the coordinate space for drawing on a mobile
     * device. In other words, the origin is at the bottom left of the screen
     * in this device orientation.
     *
     * This display orientation may or may not agree with the device
     * orientation.  In particular, it will not agree if the display orientation is
     * locked (to say portrait or landscape only).  However, this is the
     * orientation that is important for drawing to the screen.  To get the
     * device orientation, call {@link getDeviceOrientation()}.
     *
     * If this display is not a mobile device,  this method will always
     * return {@link Orientation::FIXED}.
     *
     * @return the current display orientation.
     */
    Orientation getDisplayOrientation() const { return _displayOrientation; }
    
    /**
     * Returns the current device orientation.
     *
     * The device orientation is the orientation of a mobile device, as held
     * by the user.  It may or may not agree with the display orientation,
     * which is how the screen is drawn.  For example, if the screen is
     * locked to landscape orientation, it is still possible for the device
     * to have portrait orientation when it is held on its side.
     *
     * This method is useful when you want to switch game modes for a
     * different orientation (e.g. Powerpuff Girls Flipped Out).
     *
     * If this display is not a mobile device,  this method will always
     * return {@link Orientation::FIXED}.
     *
     * @return the current device orientation.
     */
    Orientation getDeviceOrientation() const  { return _deviceOrientation; }
    
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
     * {@link Orientation::FIXED}.
     *
     * @return the default orientation of this device.
     */
    Orientation getDefaultOrientation() const  { return _defaultOrientation; }
    
    /**
     * Returns true if this display has an orientation listener
     *
     * This listener handles changes in either the device orientation (see
     * {@link getDeviceOrientation()} or the display orientation (see
     * {@link getDeviceOrientation()}. Since the device orientation will always
     * change when the display orientation does, this callback can easily safely
     * handle both. The boolean parameter in the callback indicates whether or
     * not a display orientation change has happened as well.
     *
     * Unlike other events, this listener will be invoked at the end of an
     * animation frame, after the screen has been drawn.  So it will be
     * processed before any input events waiting for the next frame.
     *
     * The display may only have one orientation listener at a time.
     *
     * @return true if this display has an orientation listener
     */
    bool hasOrientationListener() const { return _orientationListener != nullptr; }
    
    /**
     * Returns the listener for the display orientation.
     *
     * This listener handles changes in either the device orientation (see
     * {@link getDeviceOrientation()} or the display orientation (see
     * {@link getDeviceOrientation()}. Since the device orientation will always
     * change when the display orientation does, this callback can easily safely
     * handle both. The boolean parameter in the callback indicates whether or
     * not a display orientation change has happened as well.
     *
     * Unlike other events, this listener will be invoked at the end of an
     * animation frame, after the screen has been drawn.  So it will be
     * processed before any input events waiting for the next frame.
     *
     * The display may only have one orientation listener at a time. If there is
     * no listener, this method returns nullptr.
     *
     * @return the orientation listener (if any) for this display
     */
    const Listener getOrientationListener() const { return _orientationListener; }
    
    /**
     * Sets the orientation listener for this display.
     *
     * This listener handles changes in either the device orientation (see
     * {@link getDeviceOrientation()} or the display orientation (see
     * {@link getDeviceOrientation()}. Since the device orientation will always
     * change when the display orientation does, this callback can easily safely
     * handle both. The boolean parameter in the callback indicates whether or
     * not a display orientation change has happened as well.
     *
     * A display may only have one orientation listener at a time.  If this
     * display already has an orientation listener, this method will replace it
     * for the once specified.
     *
     * Unlike other events, this listener will be invoked at the end of an
     * animation frame, after the screen has been drawn.  So it will be
     * processed before any input events waiting for the next frame.
     *
     * @param listener  The listener to use
     */
    void setOrientationListener(Listener listener) { _orientationListener = listener; }
    
    /**
     * Removes the display orientation listener for this display.
     *
     * This listener handles changes in either the device orientation (see
     * {@link getDeviceOrientation()} or the display orientation (see
     * {@link getDeviceOrientation()}. Since the device orientation will always
     * change when the display orientation does, this callback can easily safely
     * handle both. The boolean parameter in the callback indicates whether or
     * not a display orientation change has happened as well.
     *
     * Unlike other events, this listener will be invoked at the end of an
     * animation frame, after the screen has been drawn.  So it will be
     * processed before any input events waiting for the next frame.
     *
     * A display may only have one orientation listener at a time.  If this
     * display does not have an orientation listener, this method will fail.
     *
     * @return true if the listener was succesfully removed
     */
    bool removeOrientationListener();
    
#pragma mark -
#pragma mark Aspect Utilities
    /**
     * Returns the device aspect ratio
     *
     * The value is computed width/height.
     *
     * @return the device aspect ratio
     */
    float getAspectRatio() const { return getAspectRatio(_aspect); }
    
    /**
     * Returns a string representation of the device aspect ratio
     *
     * This value is useful for debugging.  The first part of the string,
     * before the space, is guaranteed to be in the format x:y
     *
     * @return a string representation of the aspect ratio
     */
    const std::string getAspectName() const { return getAspectName(_aspect); }
    
    /**
     * Returns the closest width value for the device aspect ratio.
     *
     * This value is used when you want to scale a viewpoint to match the
     * display.  The value returned is rounded up to the nearest int, assuming
     * that you want the viewport in points.
     *
     * @param height    The height in points
     *
     * @return the closest width value for the device aspect ratio.
     */
    int widthForHeight(int height) const {
        return (int)(ceilf(getAspectRatio(_aspect)/height));
    }
    
    /**
     * Returns the closest height value for the device aspect ratio.
     *
     * This value is used when you want to scale a viewpoint to match the
     * display.  The value returned is rounded up to the nearest int, assuming
     * that you want the viewport in points.
     *
     * @param width     The width in points
     *
     * @return the closest height value for the device aspect ratio.
     */
    int heightForWidth(int width) const {
        return (int)(ceilf(width/getAspectRatio(_aspect)));
    }
    
    /**
     * Returns the aspect for the given aspect ratio.
     *
     * It is safest to represent aspects as an enum, not a ratio.  Round off
     * error might cause devices with very similar aspect ratios to have
     * slightly different ratio values.  Therefore, the enum is a way of
     * normalizing device aspects.
     *
     * Device aspects are relatively standardized.  For information on your
     * device, see
     *
     *      http://mydevice.io/devices/
     *
     * This method is guaranteed to match every aspect ratio on that page.
     * If the aspect ratio is not on that page, it will return UNKNOWN.
     *
     * @param ratio The aspect ratio in the form width/height
     *
     * @return the aspect for the given aspect ratio.
     */
    static Aspect getAspect(float ratio);
    
    /**
     * Returns the aspect ratio for the given aspect.
     *
     * The value is computed width/height. If the aspect is UNKNOWN, it will
     * return 0.
     *
     * @param aspect    The device aspect value
     *
     * @return the aspect ratio for the given aspect.
     */
    static float getAspectRatio(Aspect aspect);
    
    /**
     * Returns a string representation of the given aspect
     *
     * This value is useful for debugging.  The first part of the string,
     * before the space, is guaranteed to be in the format x:y
     *
     * @param aspect    The device aspect value
     *
     * @return a string representation of the given aspect
     */
    static const std::string getAspectName(Aspect aspect);
    
    /**
     * Returns the closest width value for the given aspect
     *
     * This value is used when you want to scale a viewpoint to match the
     * display.  The value returned is rounded up to the nearest int, assuming
     * that you want the viewport in points.
     *
     * @param height    The height in points
     * @param aspect    The device aspect value
     *
     * @return the closest width value for the given aspect
     */
    static int widthForHeight(int height, Aspect aspect) {
        return (int)(ceilf(getAspectRatio(aspect)/height));
    }
    
    /**
     * Returns the closest height value for the given aspect
     *
     * This value is used when you want to scale a viewpoint to match the
     * display.  The value returned is rounded up to the nearest int, assuming
     * that you want the viewport in points.
     *
     * @param width     The width in points
     * @param aspect    The device aspect value
     *
     * @return the closest height value for the given aspect
     */
    static int heightForWidth(int width, Aspect aspect) {
        return (int)(ceilf(width/getAspectRatio(aspect)));
    }
    
#pragma mark -
#pragma mark OpenGL Management
//private:
    /**
     * Refreshes the display.
     *
     * This method will swap the OpenGL framebuffers, drawing the screen.
     *
     * It will also reassess the orientation state and call the listener as
     * necessary
     */
    void refresh();

    /**
     * Assign the default settings for OpenGL
     *
     * This has to be done before the Window is created.
     *
     * @param multisample   Whether to support multisampling.
     *
     * @return true if preparation was successful
     */
    bool prepareOpenGL(bool multisample);
    
    /**
     * Initializes the OpenGL context
     *
     * This has to be done after the Window is created.
     *
     * @param multisample   Whether to support multisampling.
     *
     * @return true if initialization was successful
     */
    bool initOpenGL(bool multisample);
    
    /** This is called by the application loop */
    friend class Application;
};
    
}

#endif /* __CU_DISPLAY_H__ */

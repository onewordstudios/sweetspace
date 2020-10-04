//
//  CUDisplay.cpp
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
#include <cugl/base/CUBase.h>
#include <cugl/base/CUDisplay.h>
#include <cugl/util/CUDebug.h>
#include "platform/CUDisplay-impl.h"
#include <SDL/SDL_ttf.h>

using namespace cugl;
using namespace cugl::impl;

/** The display singleton */
Display* Display::_thedisplay = nullptr;

// Flags for the device initialization
/** Whether this display should use the fullscreen */
Uint32 Display::INIT_FULLSCREEN   = 1;
/** Whether this display should support a High DPI screen */
Uint32 Display::INIT_HIGH_DPI     = 2;
/** Whether this display should be multisampled */
Uint32 Display::INIT_MULTISAMPLED = 4;
/** Whether this display should be centered (on windowed screens) */
Uint32 Display::INIT_CENTERED     = 8;

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
Display::Display() :
_window(nullptr),
_glContext(NULL),
_initialOrientation(Orientation::UNKNOWN),
_displayOrientation(Orientation::UNKNOWN),
_deviceOrientation(Orientation::UNKNOWN),
_aspect(Aspect::UNKNOWN) {}

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
bool Display::init(std::string title, RectCugl bounds, Uint32 flags) {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        CULogError("Could not initialize display: %s",SDL_GetError());
        return false;
    }
    
    // Initialize the TTF library
    if ( TTF_Init() < 0 ) {
        CULogError("Could not initialize TTF: %s",SDL_GetError());
        return false;
    }
    
    // We have to set the OpenGL prefs BEFORE creating window
    if (!prepareOpenGL(flags & INIT_MULTISAMPLED)) {
        return false;
    }

    Uint32 sdlflags = SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL;
    if (flags & INIT_HIGH_DPI) {
        sdlflags |= SDL_WINDOW_ALLOW_HIGHDPI;
#if CU_PLATFORM == CU_PLATFORM_WINDOWS
		typedef enum PROCESS_DPI_AWARENESS {
			PROCESS_DPI_UNAWARE = 0,
			PROCESS_SYSTEM_DPI_AWARE = 1,
			PROCESS_PER_MONITOR_DPI_AWARE = 2
		} PROCESS_DPI_AWARENESS;

		void* userDLL;
		BOOL(WINAPI * SetProcessDPIAware)(void); // Vista and later
		void* shcoreDLL;
		HRESULT(WINAPI * SetProcessDpiAwareness)(PROCESS_DPI_AWARENESS dpiAwareness); // Windows 8.1 and later

		userDLL = SDL_LoadObject("USER32.DLL");
		if (userDLL) {
			SetProcessDPIAware = (BOOL(WINAPI*)(void)) SDL_LoadFunction(userDLL, "SetProcessDPIAware");
		}

		shcoreDLL = SDL_LoadObject("SHCORE.DLL");
		if (shcoreDLL) {
			SetProcessDpiAwareness = (HRESULT(WINAPI*)(PROCESS_DPI_AWARENESS)) SDL_LoadFunction(shcoreDLL, "SetProcessDpiAwareness");
		}

		if (SetProcessDpiAwareness) {
			/* Try Windows 8.1+ version */
			HRESULT result = SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
			SDL_Log("called SetProcessDpiAwareness: %d", (result == S_OK) ? 1 : 0);
		}
		else if (SetProcessDPIAware) {
			/* Try Vista - Windows 8 version.
			This has a constant scale factor for all monitors.
			*/
			BOOL success = SetProcessDPIAware();
			SDL_Log("called SetProcessDPIAware: %d", (int)success);
		}
#endif
    }

    _bounds  = DisplayBounds();
    _scale   = DisplayPixelDensity();
    if (flags & INIT_FULLSCREEN) {
        SDL_ShowCursor(0);
        sdlflags |= SDL_WINDOW_FULLSCREEN;
        bounds.origin = _bounds.origin*_scale;
        bounds.size   = _bounds.size*_scale;
    } else if (flags & INIT_CENTERED) {
        Size size = Display::get()->getBounds().size;
        bounds.origin.x = (size.width  - bounds.size.width)/2.0f;
        bounds.origin.y = (size.height - bounds.size.height)/2.0f;
    }
    
    // Make the window
    _title  = title;
    _window = SDL_CreateWindow(title.c_str(),
                               (int)bounds.origin.x,   (int)bounds.origin.y,
                               (int)bounds.size.width, (int)bounds.size.height,
                               sdlflags);
    
    if (!_window) {
        CULogError("Could not create window: %s", SDL_GetError());
        return false;
    }

    // Now we can create the OpenGL context
    if (!initOpenGL(flags & INIT_MULTISAMPLED)) {
        SDL_DestroyWindow(_window);
        _window = nullptr;
        return false;
    }
    
    // On Android, we have to call this first.
	_usable  = DisplayUsableBounds();
    _aspect  = Display::getAspect(_bounds.size.width/_bounds.size.height);
     _notched = DisplayNotch();

#if CU_PLATFORM == CU_PLATFORM_ANDROID
	// Status bar hack for now
	const int STATUS_HEIGHT = 40;
	if (_usable.size.width <= 0 || _usable.size.height <= 0) {
		_usable = _bounds;
	} else {
		if (_bounds.size.width-_usable.size.width >= STATUS_HEIGHT) {
			_usable.origin.x = (_bounds.size.width-_usable.size.width)/2.0f;
		} else if (_bounds.size.height-_usable.size.height > STATUS_HEIGHT) {
			_usable.origin.y = (_bounds.size.height-_usable.size.height)/2.0f;
		}
	}

	// Lately, I get status bar problems if I do not do this
    glViewport(0, 0, (int)bounds.size.width, (int)bounds.size.height);
#elif CU_PLATFORM == CU_PLATFORM_IPHONE
    // Apparently the iOS viewport does not get set correctly
    glViewport(0, 0, (int)bounds.size.width, (int)bounds.size.height);
#endif

    _initialOrientation = DisplayOrientation(true);
    _displayOrientation = _initialOrientation;
    _deviceOrientation  = DisplayOrientation(false);
    _defaultOrientation = DisplayDefaultOrientation();
    return true;
}

/**
 * Uninitializes this object, releasing all resources.
 *
 * This method quits the SDL video system and disposes the OpenGL context,
 * effectively exitting and shutting down the entire program.
 *
 * WARNING: This class is a singleton.  You should never access this
 * method directly.  Use the {@link stop()} method instead.
 */
void Display::dispose() {
    if (_window != nullptr) {
        SDL_GL_DeleteContext(_glContext);
        SDL_DestroyWindow(_window);
        _window = nullptr;
        _glContext = NULL;
    }
    _bounds.size.set(0,0);
    _usable.size.set(0,0);
    _scale.setZero();
    _aspect = Aspect::LANDSCAPE_16_9;
    _initialOrientation = Orientation::UNKNOWN;
    _displayOrientation = Orientation::UNKNOWN;
    _deviceOrientation = Orientation::UNKNOWN;
    SDL_Quit();
}

#pragma mark -
#pragma mark Static Accessors
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
bool Display::start(std::string name, RectCugl bounds, Uint32 flags) {
    if (_thedisplay != nullptr) {
        CUAssertLog(false, "The display is already initialized");
        return false;
    }
    _thedisplay = new Display();
    return _thedisplay->init(name,bounds,flags);
}

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
void Display::stop() {
    if (_thedisplay == nullptr) {
        CUAssertLog(false, "The display is not initialized");
    }
    delete _thedisplay;
    _thedisplay = nullptr;
}

#pragma mark -
#pragma mark Window Management
/**
 * Sets the title of this display
 *
 * On a desktop, the title will be displayed at the top of the window.
 *
 * @param title  The title of this display
 */
void Display::setTitle(const char* title) {
    _title = title;
    if (_window != nullptr) {
        SDL_SetWindowTitle(_window, title);
    }
}

/**
 * Shows the window for this display (assuming it was hidden).
 *
 * This method does nothing if the window was not hidden.
 */
void Display::show() {
    SDL_ShowWindow(_window);
}

/**
 * Hides the window for this display (assuming it was visible).
 *
 * This method does nothing if the window was not visible.
 */
void Display::hide() {
    SDL_HideWindow(_window);
}

#pragma mark -
#pragma mark Orientation
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
RectCugl Display::getUsableBounds(bool display) {
    if (display) {
		return _usable;
    } else {
#if CU_PLATFORM == CU_PLATFORM_ANDROID
		// This is a hack for now until I get something better
		// Otherwise, we have to make the Display stuff stateful
		RectCugl result;
		switch (_deviceOrientation) {
			case Orientation::LANDSCAPE:
			case Orientation::LANDSCAPE_REVERSED:
				if (_usable.origin.x > 0 || _usable.size.width < _bounds.size.width) {
					result = _usable;
				} else {
					result.origin.x = _usable.origin.y;
					result.origin.y = _usable.origin.x;
					result.size.width  = _bounds.size.width  - (_bounds.size.height-_usable.size.height);
					result.size.height = _bounds.size.height - (_bounds.size.width -_usable.size.width);
				}
				break;
			case Orientation::PORTRAIT:
			case Orientation::UPSIDE_DOWN:
				if (_usable.origin.x > 0 || _usable.size.width < _bounds.size.width) {
					result.origin.x = _usable.origin.y;
					result.origin.y = _usable.origin.x;
					result.size.width  = _bounds.size.width  - (_bounds.size.height-_usable.size.height);
					result.size.height = _bounds.size.height - (_bounds.size.width -_usable.size.width);
				} else {
					result = _usable;
				}
				break;
			default:
				result = _usable;
		}
		return result;
#else
        return DisplayUsableBounds(_deviceOrientation);
#endif
    }
}

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
bool Display::removeOrientationListener() {
    bool result = _orientationListener != nullptr;
    _orientationListener = nullptr;
    return result;
}

#pragma mark -
#pragma mark Aspect Utilities
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
Display::Aspect Display::getAspect(float ratio) {
    const float tolerance = 0.003f;
    if (CU_MATH_APPROX(ratio, 1.0f, tolerance)) {
        return Aspect::SQUARE;
    } else if (ratio < 1) {
        if (CU_MATH_APPROX(ratio, 9.0f/16.0f, tolerance)) {
            return Aspect::PORTRAIT_9_16;
        } else if (CU_MATH_APPROX(ratio, 3.0f/4.0f, tolerance)) {
            return Aspect::PORTRAIT_3_4;
        } else if (CU_MATH_APPROX(ratio, 2.0f/3.0f, tolerance)) {
            return Aspect::PORTRAIT_2_3;
        } else if (CU_MATH_APPROX(ratio, 10.0f/16.0f, tolerance)) {
            return Aspect::PORTRAIT_10_16;
        } else if (CU_MATH_APPROX(ratio, 375.0f/812.0f, tolerance)) {
            return Aspect::PORTRAIT_9_19p5;
        } else if (CU_MATH_APPROX(ratio, 3.0f/5.0f, tolerance)) {
            return Aspect::PORTRAIT_3_5;
        } else if (CU_MATH_APPROX(ratio, 600.0f/1024.0f, tolerance)) {
            return Aspect::PORTRAIT_600_1024;
        } else if (CU_MATH_APPROX(ratio, 512.0f/683.0f, tolerance)) {
            return Aspect::LANDSCAPE_IPAD_PRO_LARGE;
        } else if (CU_MATH_APPROX(ratio, 417.0f/556.0f, tolerance)) {
            return Aspect::LANDSCAPE_IPAD_PRO_SMALL_2017;
        } else if (CU_MATH_APPROX(ratio, 417.0f/597.0f, tolerance)) {
            return Aspect::LANDSCAPE_IPAD_PRO_SMALL_2018;
        }
    } else {
        if (CU_MATH_APPROX(ratio, 16.0f/9.0f, tolerance)) {
            return Aspect::LANDSCAPE_16_9;
        } else if (CU_MATH_APPROX(ratio, 4.0f/2.0f, tolerance)) {
            return Aspect::LANDSCAPE_4_3;
        } else if (CU_MATH_APPROX(ratio, 3.0f/2.0f, tolerance)) {
            return Aspect::LANDSCAPE_3_2;
        } else if (CU_MATH_APPROX(ratio, 16.0f/10.0f, tolerance)) {
            return Aspect::LANDSCAPE_16_10;
        } else if (CU_MATH_APPROX(ratio, 812.0f/375.0f, tolerance)) {
            return Aspect::LANDSCAPE_19p5_19;
        } else if (CU_MATH_APPROX(ratio, 5.0f/2.0f, tolerance)) {
            return Aspect::LANDSCAPE_5_3;
        } else if (CU_MATH_APPROX(ratio, 1024.0f/600.0f, tolerance)) {
            return Aspect::LANDSCAPE_1024_600;
        } else if (CU_MATH_APPROX(ratio, 683.0f/512.0f, tolerance)) {
            return Aspect::LANDSCAPE_IPAD_PRO_LARGE;
        } else if (CU_MATH_APPROX(ratio, 556.0f/417.0f, tolerance)) {
            return Aspect::LANDSCAPE_IPAD_PRO_SMALL_2017;
        } else if (CU_MATH_APPROX(ratio, 597.0f/417.0f, tolerance)) {
            return Aspect::LANDSCAPE_IPAD_PRO_SMALL_2018;
        }
    }
    
    return Aspect::UNKNOWN;
}

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
float Display::getAspectRatio(Aspect aspect) {
    switch (aspect) {
        case Aspect::SQUARE:
            return 1.0f;
        case Aspect::PORTRAIT_3_4:
            return 3.0f/4.0f;
        case Aspect::PORTRAIT_2_3:
            return 2.0f/3.0f;
        case Aspect::PORTRAIT_10_16:
            return 10.0f/16.0f;
        case Aspect::PORTRAIT_3_5:
            return 3.0f/5.0f;
        case Aspect::PORTRAIT_9_16:
            return 9.0f/16.0f;
        case Aspect::PORTRAIT_9_19p5:
            return 375.0f/812.0f;
        case Aspect::PORTRAIT_600_1024:
            return 600.0f/1024.0f;
        case Aspect::PORTRAIT_IPAD_PRO_LARGE:
            return 512.0f/683.0f;
        case Aspect::PORTRAIT_IPAD_PRO_SMALL_2017:
            return 417.0f/597.0f;
        case Aspect::PORTRAIT_IPAD_PRO_SMALL_2018:
            return 417.0f/683.0f;
        case Aspect::LANDSCAPE_4_3:
            return 4.0f/3.0f;
        case Aspect::LANDSCAPE_3_2:
            return 3.0f/2.0f;
        case Aspect::LANDSCAPE_16_10:
            return 16.0f/10.0f;
        case Aspect::LANDSCAPE_5_3:
            return 5.0f/3.0f;
        case Aspect::LANDSCAPE_16_9:
            return 16.0f/9.0f;
        case Aspect::LANDSCAPE_19p5_19:
            return 812.0f/375.0f;
        case Aspect::LANDSCAPE_1024_600:
            return 1024.0f/600.0f;
        case Aspect::LANDSCAPE_IPAD_PRO_LARGE:
            return 683.0f/512.0f;
        case Aspect::LANDSCAPE_IPAD_PRO_SMALL_2017:
            return 556.0f/417.0f;
        case Aspect::LANDSCAPE_IPAD_PRO_SMALL_2018:
            return 597.0f/512.0f;
        default:
            break;
    }
    return 0;
}

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
const std::string Display::getAspectName(Aspect aspect) {
    switch (aspect) {
        case Aspect::SQUARE:
            return "1:1 Square";
        case Aspect::PORTRAIT_3_4:
            return "3:4 Portrait";
        case Aspect::PORTRAIT_2_3:
            return "2:3 Portrait";
        case Aspect::PORTRAIT_10_16:
            return "10:16 Portrait";
        case Aspect::PORTRAIT_3_5:
            return "3:5 Portrait";
        case Aspect::PORTRAIT_9_16:
            return "9:16 Portrait";
        case Aspect::PORTRAIT_9_19p5:
            return "9:19.5 Portrait (iPhone X)";
        case Aspect::PORTRAIT_600_1024:
            return "600:1024 Portrait";
        case Aspect::PORTRAIT_IPAD_PRO_LARGE:
            return "iPad Pro (12.9, 9.7) Portrait";
        case Aspect::PORTRAIT_IPAD_PRO_SMALL_2017:
            return "iPad Pro (10.5, 2017) Portrait";
        case Aspect::PORTRAIT_IPAD_PRO_SMALL_2018:
            return "iPad Pro (11, 2018) Portrait";
        case Aspect::LANDSCAPE_4_3:
            return "4:3 Landscape";
        case Aspect::LANDSCAPE_3_2:
            return "3:2 Landscape";
        case Aspect::LANDSCAPE_16_10:
            return "16:10 Landscape";
        case Aspect::LANDSCAPE_5_3:
            return "5:3 Landscape";
        case Aspect::LANDSCAPE_16_9:
            return "16:9 Landscape";
        case Aspect::LANDSCAPE_19p5_19:
            return "19.5:9 Portrait (iPhone X)";
        case Aspect::LANDSCAPE_1024_600:
            return "1024:600 Landscape";
        case Aspect::LANDSCAPE_IPAD_PRO_LARGE:
            return "iPad Pro (12.9, 9.7) LANDSCAPE";
        case Aspect::LANDSCAPE_IPAD_PRO_SMALL_2017:
            return "iPad Pro (10.5, 2017) LANDSCAPE";
        case Aspect::LANDSCAPE_IPAD_PRO_SMALL_2018:
            return "iPad Pro (11, 2018) LANDSCAPE";
        default:
            break;
    }
    return "Unknown";
}

#pragma mark -
#pragma mark OpenGL Support
/**
 * Assign the default settings for OpenGL
 *
 * This has to be done before the Window is created.
 *
 * @param mutlisample   Whether to support multisampling.
 *
 * @return true if preparation was successful
 */
bool Display::prepareOpenGL(bool multisample) {
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    
#if CU_GL_PLATFORM == CU_GL_OPENGLES
    int profile = SDL_GL_CONTEXT_PROFILE_ES;
    int version = 3; // Force 3 on mobile
#else
    int profile = SDL_GL_CONTEXT_PROFILE_CORE;
    int version = 4; // Force 4 on desktop
    if (multisample) {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    }
#endif
    
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, profile) != 0) {
        CULogError("OpenGL is not supported on this platform: %s", SDL_GetError());
        return false;
    }
    
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, version) != 0) {
        CULogError("OpenGL %d is not supported on this platform: %s", version, SDL_GetError());
        return false;
    }
    
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    
    return true;
}

/**
 * Initializes the OpenGL context
 *
 * This has to be done after the Window is created.
 *
 * @param mutlisample   Whether to support multisampling.
 *
 * @return true if initialization was successful
 */
bool Display::initOpenGL(bool multisample) {
#if CU_GL_PLATFORM != CU_GL_OPENGLES
    if (multisample) {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    }
#endif
    
    // Create the OpenGL context
    _glContext = SDL_GL_CreateContext( _window );
    if( _glContext == NULL )  {
        CULogError("Could not create OpenGL context: %s", SDL_GetError() );
        return false;
    }
    
    // Multisampling support
#if CU_GL_PLATFORM != CU_GL_OPENGLES
    glEnable(GL_LINE_SMOOTH);
    if (multisample) {
        glEnable(GL_MULTISAMPLE);
    }
#endif
    
#if CU_PLATFORM == CU_PLATFORM_WINDOWS
    //Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        SDL_Log("Error initializing GLEW: %s", glewGetErrorString(glewError));
    }
#endif
    
    return true;
}

/**
 * Refreshes the display.
 *
 * This method will swap the OpenGL framebuffers, drawing the screen.
 *
 * It will also reassess the orientation state and call the listener as
 * necessary
 */
void Display::refresh() {
    SDL_GL_SwapWindow(_window);
    Orientation oldDisplay = _displayOrientation;
    Orientation oldDevice  = _deviceOrientation;
    _displayOrientation = DisplayOrientation(true);
    _deviceOrientation  = DisplayOrientation(false);
    if (oldDisplay != _displayOrientation) {
        _usable = DisplayUsableBounds();
    }
    if (_orientationListener &&
        (oldDevice != _deviceOrientation || oldDisplay != _displayOrientation)) {
        _orientationListener(oldDevice,_deviceOrientation,oldDisplay != _displayOrientation);
    }
}



//
//  CUBase.h
//  Cornell University Game Library (CUGL)
//
//  This header includes the bear minimum defines that any CUGL class needs.
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
//  Version: 5/30/16

#ifndef __CU_BASE_H__
#define __CU_BASE_H__

#include <memory>
#include <string>
#include <SDL/SDL.h>

// The platforms
/** An unknown platform not supported by CUGL */
#define CU_PLATFORM_UNKNOWN 0
/** The Macintosh platform */
#define CU_PLATFORM_MACOS   1
/** The iOS platform (including iPads) */
#define CU_PLATFORM_IPHONE  2
/** The Android platform */
#define CU_PLATFORM_ANDROID 3
/** The traditional (not mobile) Windows 10 platform */
#define CU_PLATFORM_WINDOWS 4
// Windows RT is discontinued, so we will not support it

// Determine the correct platform
#if defined (__MACOSX__)
    /** The current platform being compiled */
    #define CU_PLATFORM 1
#elif defined (__IPHONEOS__)
/** The current platform being compiled */
    #define CU_TOUCH_SCREEN 1
    #define CU_PLATFORM     2
#elif defined (__ANDROID__)
/** The current platform being compiled */
    #define CU_TOUCH_SCREEN 1
    #define CU_PLATFORM     3
#elif defined (__WINDOWS__)
/** The current platform being compiled */
    #define CU_PLATFORM 4
#else
/** The current platform being compiled */
    #define CU_PLATFORM 0
#endif


// Memory representation
/** Big Endian (or network order) */
#define CU_ORDER_STANDARD 0
/** Little Endian (standard for Intel) */
#define CU_ORDER_REVERSED 1

// I have absolutely no idea what is going on with Windows
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    #define CU_MEMORY_ORDER   CU_ORDER_REVERSED
#else
    #define CU_MEMORY_ORDER   CU_ORDER_STANDARD
#endif


// OpenGL support
/** Support for standard OpenGL   */
#define CU_GL_OPENGL   0
/** Support for standard OpenGLES */
#define CU_GL_OPENGLES 1

// Load the libraries and define the platform
#if defined (__IPHONEOS__)
    #include <OpenGLES/ES3/gl.h>
    #include <OpenGLES/ES3/glext.h>
    /** The current OpenGL platform */
    #define CU_GL_PLATFORM   CU_GL_OPENGLES
#elif defined (__ANDROID__)
    #include <GLES3/gl3platform.h>
    #include <GLES3/gl3.h>
    #include <GLES3/gl3ext.h>
    /** The current OpenGL platform */
    #define CU_GL_PLATFORM   CU_GL_OPENGLES
#elif defined (__MACOSX__)
    #include <OpenGL/OpenGL.h>
    #include <OpenGL/gl3.h>
    #include <OpenGL/gl3ext.h>
    /** The current OpenGL platform */
    #define CU_GL_PLATFORM   CU_GL_OPENGL
#elif defined (__WINDOWS__)
	#define NOMINMAX
	#include <windows.h>
	#include <GL/glew.h>
	#include <SDL/SDL_opengl.h>
	#include <GL/gl.h>	
	#include <GL/glu.h>	
	/** The current OpenGL platform */
	#define CU_GL_PLATFORM   CU_GL_OPENGL
#endif

#ifdef _MSC_VER 
	//not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

// Macros to disable copying for select classes.
#if defined(__GNUC__) && ((__GNUC__ >= 5) || ((__GNUG__ == 4) && (__GNUC_MINOR__ >= 4))) \
|| (defined(__clang__) && (__clang_major__ >= 3)) || (_MSC_VER >= 1800)
#define CU_DISALLOW_COPY_AND_ASSIGN(TypeName) \
TypeName(const TypeName &) = delete; \
TypeName &operator =(const TypeName &) = delete;
#else
#define CU_DISALLOW_COPY_AND_ASSIGN(TypeName) \
TypeName(const TypeName &); \
TypeName &operator =(const TypeName &);
#endif

#endif /* __CU_BASE_H__ */

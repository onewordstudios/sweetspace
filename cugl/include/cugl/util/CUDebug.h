//
//  CUDebug.h
//  Cornell University Game Library (CUGL)
//
//  The SDL_assert functionality is VERY heavy weight.  In addition, it beach
//  balls on OS X.  This module provides debugging functionality that is much
//  more lightweight.  However, you can still access the SDL functionality
//  by setting the alter level to paranoid (assert level 3).
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
//  Version: 5/31/16

#ifndef __CU_DEBUG_H__
#define __CU_DEBUG_H__

#include <SDL/SDL.h>
#include <cugl/math/CUMathBase.h>
#include <cassert>

/**
* @def CULog(msg,args...)
*
* Writes an info message to the application log.
*
* This is the default logging function and should be used for any form
* of logging that is not properly an error.  The log message takes printf
* style formatting arguments.
*
* @param msg       The message to display
* @param args...   Formatting arguments for printf
*/
#if defined(__WINDOWS__)
#define CULog(msg,...)			SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION,SDL_LOG_PRIORITY_INFO,msg, ##__VA_ARGS__)
#else
#define CULog(msg,args...)		SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION,SDL_LOG_PRIORITY_INFO,msg, ##args)
#endif

/**
* @def CULogError(msg,args...)
*
* Writes an error message to the application log.
*
* This is the logging function for identifying errors.  It is called by
* CUAssertLog.  In addition, you can use it to mark any non-halting errors
* as well.  The error message takes printf style formatting arguments.
*
* @param msg       The message to display
* @param args...   Formatting arguments for printf
*/
#if defined(__WINDOWS__)
#define CULogError(msg,...)		SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION,SDL_LOG_PRIORITY_ERROR,msg, ##__VA_ARGS__)
#else
#define CULogError(msg,args...)		SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION,SDL_LOG_PRIORITY_ERROR,msg, ##args)
#endif

/**
* @def CULogCritical(msg,args...)
*
* Writes an critical message to the application log.
*
* This is a special logging function for identifying critical (stand-out) places
* in your code.  It is useful for log parsing when you have a very verbose log
* and you need a tag to easy searching. The log message takes printf style
* formatting arguments.
*
* @param msg       The message to display
* @param args...   Formatting arguments for printf
*/
#if defined(__WINDOWS__)
#define CULogCritical(msg,...)		SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION,SDL_LOG_PRIORITY_CRITICAL,msg, ##__VA_ARGS__)
#else
#define CULogCritical(msg,args...)	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION,SDL_LOG_PRIORITY_CRITICAL,msg, ##args)
#endif

/**
* @def CUWarn(msg,args...)
*
* Writes a warning message to the application log.
*
* A warning is an error message that is not serious enough to stop the
* application. In general, you should use this instead of CULogError when an
* assert is not warranted. The warning message takes printf style formatting
* arguments.
*
* @param msg       The message to display
* @param args...   Formatting arguments for printf
*/
#if defined(__WINDOWS__)
#define CUWarn(msg,...)		SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION,SDL_LOG_PRIORITY_WARN,msg,##__VA_ARGS__)
#else
#define CUWarn(msg,args...)		SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION,SDL_LOG_PRIORITY_WARN,msg, ##args)
#endif

/* Internal assert code to combine logging with the assert */
#define __cu_assert__(condition,msg,...)  do {  \
        if (!(condition)) {                     \
            CULogError(msg, ##__VA_ARGS__);     \
            assert(condition);                  \
        }                                       \
    } while (0)


// Let's Doxgen the functions before the variable block.

/**
* @def CUAssert(condition)
*
* Asserts a condition, potentially halting on false.
*
* The style of the assert, and whether or not it actually pauses the applcation
* depends on the assert level of the build.  On disabled and release setting,
* assertions are disabled.  On normal settings, it will use a standard C++
* assert, stopping the application.  On paranoid settings, it will use the
* heavier-weight SDL_assert, which creates a dialog pop-up.  The latter is
* ideal when running on mobile devices not attached to a debugger.
*
* @param condition The assert condition
*/

/**
* @def CUAssertLog(condition,msg,args...)
*
* Asserts a condition, potentially logging and halting on false.
*
* The style of the assert, and whether or not it actually pauses the applcation
* depends on the assert level of the build.  On disabled and release setting,
* assertions are disabled.  On normal settings, it will use a standard C++
* assert, stopping the application.  On paranoid settings, it will use the
* heavier-weight SDL_assert, which creates a dialog pop-up.  The latter is
* ideal when running on mobile devices not attached to a debugger.
*
* If the assert does halt, it will write the given message to the error log.
* The assert takes printf style formatting argument.
*
* @param condition The assert condition
* @param msg       The message to display
* @param args...   Formatting arguments for printf
*/

/**
* @def CUAssertAlways(condition)
*
* Always asserts a condition, halting on false.
*
* This form of assert will always be active, in both release and debug mode,
* unless asserts are completely disabled (SDL_ASSERT_LEVEL 0).  However, the
* style of the assert depends on the assert level of the build.  On normal
* setting, it will use a standard C++ assert, stopping the application.  On
* release and paranoid settings, it will use the heavier-weight SDL_assert,
* which creates a dialog pop-up.  The latter is ideal when running on mobile
* devices not attached to a debugger.
*
* @param condition The assert condition
*/

/**
* @def CUAssertAlwaysLog(condition,msg,args...)
*
* Always asserts a condition, halting on false.
*
* This form of assert will always be active, in both release and debug mode,
* unless asserts are completely disabled (SDL_ASSERT_LEVEL 0).  However, the
* style of the assert depends on the assert level of the build.  On normal
* setting, it will use a standard C++ assert, stopping the application.  On
* release and paranoid settings, it will use the heavier-weight SDL_assert,
* which creates a dialog pop-up.  The latter is ideal when running on mobile
* devices not attached to a debugger.
*
* If the assert does halt, it will write the given message to the error log.
* The assert takes printf style formatting argument.
*
* @param condition The assert condition
* @param msg       The message to display
* @param args...   Formatting arguments for printf
*/

#if SDL_ASSERT_LEVEL == 0   /* assertions disabled */
#   define CUAssert(condition)                      SDL_disabled_assert(condition)
#   define CUAssertLog(condition,msg,args...)       SDL_disabled_assert(condition)
#   define CUAssertAlways(condition)                SDL_disabled_assert(condition)
#   define CUAssertAlwaysLog(condition,msg,args...) SDL_disabled_assert(condition)
#elif SDL_ASSERT_LEVEL == 1  /* release settings. */
#   define CUAssert(condition)                      SDL_disabled_assert(condition)
#   define CUAssertLog(condition,msg,...)			SDL_disabled_assert(condition)
#   define CUAssertAlways(condition)                SDL_enabled_assert(condition)
#   define CUAssertAlwaysLog(condition,msg,...)		__cu_assert__(condition,msg, ##__VA_ARGS__)
#elif SDL_ASSERT_LEVEL == 2  /* normal settings. */
#   define CUAssert(condition)                      assert(condition)
#   define CUAssertLog(condition,msg,...)           __cu_assert__(condition,msg, ##__VA_ARGS__)
#   define CUAssertAlways(condition)                assert(condition)
#   define CUAssertAlwaysLog(condition,msg,...)     __cu_assert__(condition,msg, ##__VA_ARGS__)
#elif SDL_ASSERT_LEVEL == 3  /* paranoid settings. */
#   define CUAssert(condition)                      SDL_enabled_assert(condition)
#   define CUAssertLog(condition,msg,...)			__cu_assert__(condition,msg, ##__VA_ARGS__)
#   define CUAssertAlways(condition)                SDL_enabled_assert(condition)
#   define CUAssertAlwaysLog(condition,msg,...)		__cu_assert__(condition,msg, ##__VA_ARGS__)
#else
#   error Unknown assertion level.
#endif

/**
* @def CULogGLError()
*
* This function checks if there is an OpenGL error, and if so, logs the
* offending line to the error log.  The design of this macros is inspired by
*
* https://blog.nobel-joergensen.com/2013/01/29/debugging-opengl-using-glgeterror/
*/
#define CULogGLError() _check_gl_error(__FILE__,__LINE__)

void _check_gl_error(const char *file, int line);

#endif /* __CU_DEBUG_H__ */

//
//  CUDebug.cpp
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
#include <cugl/util/CUDebug.h>

/**
 * @def CULogGLError()
 *
 * This function checks if there is an OpenGL error, and if so, logs the
 * offending line to the error log.  The design of this macros is inspired by
 *
 * https://blog.nobel-joergensen.com/2013/01/29/debugging-opengl-using-glgeterror/
 */
void _check_gl_error(const char *file, int line) {
    GLenum err (glGetError());
    
    while(err!=GL_NO_ERROR) {
        std::string error;
        
        switch(err) {
            case GL_INVALID_OPERATION:
                error="INVALID_OPERATION";      break;
            case GL_INVALID_ENUM:
                error="INVALID_ENUM";           break;
            case GL_INVALID_VALUE:
                error="INVALID_VALUE";          break;
            case GL_OUT_OF_MEMORY:
                error="OUT_OF_MEMORY";          break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error="INVALID_FRAMEBUFFER_OPERATION";  break;
        }
        
        CULogError("GL_%s at %s:%d", error.c_str(), file, line);
        err = glGetError();
    }
}

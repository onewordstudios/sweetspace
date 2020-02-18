//
//  CUShader.h
//  Cornell University Game Library (CUGL)
//
//  This module provides the abstract base class for our shader classes.
//  It supports compilations and has diagnostic tools for errors.  It is not
//  useful for rendering, because there is no way to get the data there.
//
//  Because it is abstract, it has only a basic constructor.  It has no
//  initializers or allocator.
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
//  Version: 6/23/16

#ifndef __CU_SHADER_H__
#define __CU_SHADER_H__

#include <cugl/math/CUMathBase.h>
#include <string>

// A stringification trick to define shaders easily
// Note the version lockdown for non-OpenGLES systems
#if CU_GL_PLATFORM == CU_GL_OPENGLES
    #define SHADER(A) "#version 300 es\n" #A
#else
    #define SHADER(A) "#version 140\n" #A
#endif

namespace cugl {

/**
 * This class is the base abstract class for any GLSL shader.
 *
 * Specific shaders have attached attributes and uniforms.  Since it is 
 * difficult to write a class that takes all of the possibilities into 
 * consideration (and is type-safe), we use subclasses to implement specific
 * shaders.  This class just provides support for compilation and binding.
 *
 * This class is written to be agnostic about whether we are using OpenGL
 * or OpenGLES.  Because these shader languages are slightly different, 
 * subclasses should take this difference into account.
 */
class Shader {
#pragma mark Values
protected:
    /** The OpenGL program for this shader */
    GLuint _program;
    /** The OpenGL vertex shader for this shader */
    GLuint _vertShader;
    /** The OpenGL fragment shader for this shader */
    GLuint _fragShader;
    /** The source string for the vertex shader */
    const char* _vertSource;
    /** The source string for the fragment shader */
    const char* _fragSource;
    /** Whether or not this shader is currently active */
    bool _active;
    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates an uninitialized shader with no source.
     *
     * You must initialize the shader to add a source and compiled it.
     */
    Shader() :_program(0), _vertShader(0), _fragShader(0),
            _vertSource(nullptr), _fragSource(nullptr),
            _active(false) {};

    /**
     * Deletes this shader, disposing all resources.
     */
    ~Shader() { dispose(); }

    /**
     * Deletes the OpenGL shader and resets all attributes.
     * 
     * You must reinitialize the shader to use it.
     */
    virtual void dispose();

#pragma mark -
#pragma mark Attributes
    /**
     * Returns the source string for the vertex shader.
     *
     * The string is empty if not defined.
     *
     * @return the source string for the vertex shader.
     */
    std::string getVertSource() const {
        return (_vertSource ? std::string(_vertSource) : "");
    }
    
    /**
     * Returns the source string for the fragment shader.
     *
     * The string is empty if not defined.
     *
     * @return the source string for the fragment shader.
     */
    std::string getFragSource() const {
        return (_fragSource ? std::string(_fragSource) : "");
    }

    /**
     * Returns true if this shader has been compiled and is ready for use.
     *
     * @return true if this shader has been compiled and is ready for use.
     */
    bool isReady() const { return _program != 0; }

    /**
     * Returns true if this shader is currently active.
     * 
     * The shader is active if its program is currently bound.  Any OpenGL
     * calls will then be sent to this shader.
     *
     * @return true if this shader is currently active.
     */
    bool isActive() const { return _active; }
    
    /**
     * Returns the OpenGL program associated with this shader.
     *
     * This method will return 0 if the program is not initialized.
     *
     * @return the OpenGL program associated with this shader.
     */
    GLuint getProgram() const { return _program; }

#pragma mark -
#pragma mark Rendering
    /**
     * Binds this shader, making it active.
     *
     * Once bound, any OpenGL calls will then be sent to this shader.
     */
    virtual void bind();
    
    /**
     * Unbinds this shader, making it no longer active.
     *
     * Once unbound, OpenGL calls will no longer be sent to this shader.
     */
    virtual void unbind();

#pragma mark -
#pragma mark Compilation
protected:
    /**
     * Compiles this shader from the given vertex and fragment shader sources.
     *
     * When compilation is complete, the shader will not be bound.  However,
     * any shader that was actively bound during compilation also be unbound 
     * as well.
     *
     * If compilation fails, it will display error messages on the log.
     *
     * @return true if compilation was successful.
     */
    virtual bool compile();
    
    /**
     * Returns true if the shader was compiled properly.
     *
     * If compilation fails, it will display error messages on the log.
     *
     * @param shader    The shader to test
     * @param type      The shader type (vertex or fragment)
     *
     * @return true if the shader was compiled properly.
     */
    static bool validateShader(GLuint shader, const char* type);

    /**
     * Displays the shader compilation errors to the log.
     *
     * If there were no errors, this method will do nothing.
     *
     * @param shader    The shader to test
     */
    static void logShaderError(GLuint shader);

    /**
     * Displays the program linker errors to the log.
     *
     * If there were no errors, this method will do nothing.
     *
     * @param shader    The program to test
     */
    static void logProgramError(GLuint shader);
};
    
}

#endif /* __CU_SHADER_H__ */

//
//  CUShader.cpp
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

#include <cugl/renderer/CUShader.h>
#include <cugl/util/CUDebug.h>

using namespace cugl;

#pragma mark Rendering
/**
 * Binds this shader, making it active.
 *
 * Once bound, any OpenGL calls will then be sent to this shader.
 */
void Shader::bind() {
    CUAssertLog(_program, "Shader is not ready for use");
    glUseProgram( _program );
    _active = true;
}

/**
 * Unbinds this shader, making it no longer active.
 *
 * Once unbound, OpenGL calls will no longer be sent to this shader.
 */
void Shader::unbind() {
    CUAssertLog(_program, "Shader is not ready for use");
    glUseProgram( NULL );
    _active = false;
}

#pragma mark -
#pragma mark Compilation
/**
 * Compiles this shader from the given vertex and fragment shader sources.
 *
 * If compilation fails, it will display error messages on the log.
 *
 * @return true if compilation was successful.
 */
bool Shader::compile() {
    CUAssertLog(_vertSource,    "Vertex shader source is not defined");
    CUAssertLog(_fragSource,    "Fragment shader source is not defined");
    CUAssertLog(!_program,      "This shader is already compiled");
    
    _program = glCreateProgram();
    if (!_program) {
        CULogError("Unable to allocate shader program");
        return false;
    }
    
    //Create vertex shader and compile it
    _vertShader = glCreateShader( GL_VERTEX_SHADER );
    glShaderSource( _vertShader, 1, &_vertSource, nullptr );
    glCompileShader( _vertShader );
    
    // Validate and quit if failed
    if (!validateShader(_vertShader, "vertex")) {
        dispose();
        return false;
    }
    
    //Create fragment shader and compile it
    _fragShader = glCreateShader( GL_FRAGMENT_SHADER );
    glShaderSource( _fragShader, 1, &_fragSource, nullptr );
    glCompileShader( _fragShader );
    
    // Validate and quit if failed
    if (!validateShader(_fragShader, "fragment")) {
        dispose();
        return false;
    }
    
    // Now kiss
    glAttachShader( _program, _vertShader );
    glAttachShader( _program, _fragShader );
    glLinkProgram( _program );
    
    //Check for errors
    GLint programSuccess = GL_TRUE;
    glGetProgramiv( _program, GL_LINK_STATUS, &programSuccess );
    if( programSuccess != GL_TRUE ) {
        CULogError( "Unable to link program %d.\n", _program );
        logProgramError(_program);
        dispose();
        return false;
    }
    
    return true;
}

/**
 * Deletes the OpenGL shader and resets all attributes.
 *
 * You must reinitialize the shader to use it.
 */
void Shader::dispose() {
    if (_active) { unbind(); }
    if (_fragShader) { glDeleteShader(_fragShader); _fragShader = 0;}
    if (_vertShader) { glDeleteShader(_vertShader); _vertShader = 0;}
    if (_program) { glDeleteShader(_program); _program = 0;}
    _vertSource = nullptr;
    _fragSource = nullptr;
}

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
bool Shader::validateShader(GLuint shader, const char* type) {
    CUAssertLog( glIsShader( shader ), "Shader %d is not a valid shader", shader);
    GLint shaderCompiled = GL_FALSE;
    glGetShaderiv( shader, GL_COMPILE_STATUS, &shaderCompiled );
    if( shaderCompiled != GL_TRUE ) {
        CULogError( "Unable to compile %s shader %d.\n", type, shader );
        logShaderError(shader);
        return false;
    }
    return true;
}

/**
 * Displays the shader compilation errors to the log.
 *
 * If there were no errors, this method will do nothing.
 *
 * @param shader    The shader to test
 */
void Shader::logShaderError(GLuint shader) {
    CUAssertLog( glIsShader( shader ), "Shader %d is not a valid shader", shader);
    //Make sure name is shader

    //Shader log length
    int infoLogLength = 0;
    int maxLength = infoLogLength;
        
    //Get info string length
    glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &maxLength );
        
    //Allocate string
    char* infoLog = new char[ maxLength ];
        
    //Get info log
    glGetShaderInfoLog( shader, maxLength, &infoLogLength, infoLog );
    if( infoLogLength > 0 ) {
        // Print Log
        CULogError( "%s\n", infoLog );
    }
        
    //Deallocate string
    delete[] infoLog;
}

/**
 * Displays the program linker errors to the log.
 *
 * If there were no errors, this method will do nothing.
 *
 * @param shader    The program to test
 */
void Shader::logProgramError( GLuint program ) {
    CUAssertLog( glIsProgram( program ), "Program %d is not a valid shader", program);

    //Program log length
    int infoLogLength = 0;
    int maxLength = infoLogLength;
        
    //Get info string length
    glGetProgramiv( program, GL_INFO_LOG_LENGTH, &maxLength );
        
    //Allocate string
    char* infoLog = new char[ maxLength ];
        
    //Get info log
    glGetProgramInfoLog( program, maxLength, &infoLogLength, infoLog );
    if( infoLogLength > 0 ) {
        // Print Log
        CULogError( "%s\n", infoLog );
    }
    
    //Deallocate string
    delete[] infoLog;
}

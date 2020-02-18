//
//  CUSpriteBatch.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides one-stop shopping for basic 2d graphics.  Despite the
//  name, it is also capable of drawing solid shapes, as well as wireframes.
//  The only time you might want a separate class is when you need another
//  shader.  And even then, you can swap out the shader for this class.
//
//  This class uses our standard shared-pointer architecture.
//
//  1. The constructor does not perform any initialization; it just sets all
//     attributes to their defaults.
//
//  2. All initialization takes place via init methods, which can fail if an
//     object is initialized more than once.
//
//  3. All allocation takes place via static constructors which return a shared
//     pointer.
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

#include <cugl/renderer/CUSpriteShader.h>
#include <cugl/renderer/CUVertex.h>
#include <cugl/util/CUDebug.h>

// The shaders
#include "ColorTextureOpenGL.vert"
#include "ColorTextureOpenGL.frag"

// The names of the shader attributes and uniforms
#define POSITION_ATTRIBUTE  "aPosition"
#define COLOR_ATTRIBUTE     "aColor"
#define TEXCOORD_ATTRIBUTE  "aTexCoord"
#define PERSPECTIVE_UNIFORM "uPerspective"
#define TEXTURE_UNIFORM     "uTexture"
#define TEXTURE_POSITION    0

using namespace cugl;


#pragma mark -
#pragma mark Initialization
/**
 * Initializes this shader with the default vertex and fragment source.
 *
 * The shader will compile the vertex and fragment sources and link
 * them together. When compilation is complete, the shader will not be
 * bound.  However, any shader that was actively bound during compilation
 * also be unbound as well.
 *
 * @return true if initialization was successful.
 */
bool SpriteShader::init() {
    _vertSource = oglColorTextureVert;
    _fragSource = oglColorTextureFrag;
    return compile();
}
/**
 * Initializes this shader with the given vertex and fragment source.
 *
 * The shader will compile the vertex and fragment sources and link
 * them together. When compilation is complete, the shader will not be
 * bound.  However, any shader that was actively bound during compilation
 * also be unbound as well.
 *
 * @param vsource   The source string for the vertex shader.
 * @param fsource   The source string for the fragment shader.
 *
 * @return true if initialization was successful.
 */
bool SpriteShader::init(const char* vsource, const char* fsource) {
    _vertSource = vsource;
    _fragSource = fsource;
    return compile();
}

#pragma mark -
#pragma mark Attributes
/**
 * Sets the perspective matrix to use in the shader.
 *
 * @param matrix    The perspective matrix
 */
void SpriteShader::setPerspective(const Mat4& matrix) {
    _mPerspective = matrix;
    if (_active) {
        glUniformMatrix4fv(_uPerspective,1,false,_mPerspective.m);
    }
}

/**
 * Sets the texture in use in the shader
 *
 * @param testure   The shader texture
 */
void SpriteShader::setTexture(const std::shared_ptr<Texture>& texture) {
    _mTexture = texture;
    if (_active) {
        glActiveTexture(GL_TEXTURE0 + TEXTURE_POSITION);
        glBindTexture(GL_TEXTURE_2D, _mTexture->getBuffer());
    }
}

#pragma mark -
#pragma mark Rendering

/**
 * Attaches the given memory buffer to this shader.
 *
 * Because of limitations in OpenGL ES, we cannot draw anything without
 * both a vertex buffer object and an vertex array object.
 *
 * @param vArray    The vertex array object
 * @param vBuffer   The vertex buffer object
 */
void SpriteShader::attach(GLuint vArray, GLuint vBuffer) {
    CUAssertLog(_active, "This shader is not currently active");

    glBindVertexArray(vArray);
    glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
    
    glVertexAttribPointer( _aPosition, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2),
                          Vertex2::positionOffset());
    glVertexAttribPointer( _aColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex2),
                          Vertex2::colorOffset());
    glVertexAttribPointer( _aTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2),
                          Vertex2::texcoordOffset());
}

/**
 * Binds this shader, making it active.
 *
 * Once bound, any OpenGL calls will then be sent to this shader.
 */
void SpriteShader::bind() {
    Shader::bind();
    glEnableVertexAttribArray(_aPosition);
    glEnableVertexAttribArray(_aColor);
    glEnableVertexAttribArray(_aTexCoord);
    if (_mTexture != nullptr) {
        glActiveTexture(GL_TEXTURE0 + TEXTURE_POSITION);
        glBindTexture(GL_TEXTURE_2D, _mTexture->getBuffer());
    }
}

/**
 * Unbinds this shader, making it no longer active.
 *
 * Once unbound, OpenGL calls will no longer be sent to this shader.
 */
void SpriteShader::unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisableVertexAttribArray(_aPosition);
    glDisableVertexAttribArray(_aColor);
    glDisableVertexAttribArray(_aTexCoord);
    Shader::unbind();
}



#pragma mark -
#pragma mark Compilation
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
bool SpriteShader::compile() {
    if (!Shader::compile()) return false;
    
    // Find each of the attributes
    _aPosition = glGetAttribLocation( _program, POSITION_ATTRIBUTE );
    if( !validateVariable(_aPosition, POSITION_ATTRIBUTE)) {
        dispose();
        return false;
    }
    
    _aColor = glGetAttribLocation( _program, COLOR_ATTRIBUTE );
    if( !validateVariable(_aColor, COLOR_ATTRIBUTE)) {
        dispose();
        return false;
    }
    
    _aTexCoord = glGetAttribLocation( _program, TEXCOORD_ATTRIBUTE );
    if( !validateVariable(_aTexCoord, TEXCOORD_ATTRIBUTE)) {
        dispose();
        return false;
    }
    
    _uPerspective = glGetUniformLocation( _program, PERSPECTIVE_UNIFORM );
    if( !validateVariable(_uPerspective, PERSPECTIVE_UNIFORM)) {
        dispose();
        return false;
    }
    
    _uTexture = glGetUniformLocation( _program, TEXTURE_UNIFORM );
    if( !validateVariable(_uTexture, TEXTURE_UNIFORM)) {
        dispose();
        return false;
    }
    
    // Set the texture location and matrix
    bind();
    glUniformMatrix4fv(_uPerspective,1,false,_mPerspective.m);
    glUniform1i(_uTexture, TEXTURE_POSITION);
    unbind();
    
    return true;
}

/**
 * Returns true if the GLSL variable was found in this shader.
 *
 * If variable is not found, it will display error messages on the log.
 *
 * @param variable  The variable (reference) to test
 * @param name      The variable (name) to test
 *
 * @return true if the GLSL variable was found in this shader.
 */
bool SpriteShader::validateVariable(GLint variable, const char* name) {
    if( variable == -1 ) {
        CULogError( "%s is not a valid GLSL program variable.\n", name );
        Shader::logProgramError(_program);
        return false;
    }
    return true;
}

/**
 * Deletes the OpenGL shader and resets all attributes.
 *
 * You must reinitialize the shader to use it.
 */
void SpriteShader::dispose() {
    if (_mTexture != nullptr) { _mTexture.reset(); }
    Shader::dispose();
}


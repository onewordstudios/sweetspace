//
//  CUSpriteShader.h
//  Cornell University Game Library (CUGL)
//
//  This module provides the default shader for a sprite batch.  You may replace
//  the shader code, but all shaders must support the attributes and uniforms
//  listed below.
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

#ifndef __CU_SPRITE_SHADER_H__
#define __CU_SPRITE_SHADER_H__

#include <cugl/renderer/CUShader.h>
#include <cugl/renderer/CUTexture.h>
#include <cugl/math/CUMat4.h>

namespace cugl {

/**
 * This class is a GLSL shader to use with a sprite batch.
 *
 * This class provides you the option to use your own shader sources.  However,
 * any shader used with this class must have the following properties.
 *
 * First, it must provide three attributes corresponding to the Vertex2 class.
 * These attributes must be names as follows.
 *
 *      aPosition:      The position attribute
 *
 *      aColor:         The color attribute
 *
 *      aTexCoord:      The texture coordinate attribute
 *
 * In addition, it must include the following two uniforms.
 *
 *      uPerspective:   The perspective matrix (combined modelview projection)
 *
 *      uTexture:       The shading texture
 * 
 * Any other attributes or uniforms will be ignored.
 */
class SpriteShader : public Shader {
#pragma mark Values
private:
    /** The shader location for the position attribute */
    GLint _aPosition;
    /** The shader location for the color attribute */
    GLint _aColor;
    /** The shader location for the texure coordinate attribute */
    GLint _aTexCoord;
    /** The shader location for the perspective uniform */
    GLint _uPerspective;
    /** The shader location for the texture uniform */
    GLint _uTexture;

    /** The current perspective matrix */
    Mat4  _mPerspective;
    
    /** The current shader texture */
    std::shared_ptr<Texture> _mTexture;
    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates an uninitialized shader with no source.
     *
     * You must initialize the shader to add a source and compiled it.
     */
    SpriteShader() : Shader(), _aPosition(-1), _aColor(-1), _aTexCoord(-1),
                               _uPerspective(-1), _uTexture(-1) { }

    /**
     * Deletes this shader, disposing all resources.
     */
    ~SpriteShader() { dispose(); }

    /**
     * Deletes the OpenGL shader and resets all attributes.
     *
     * You must reinitialize the shader to use it.
     */
    void dispose() override;

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
    bool init();
    
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
    bool init(std::string vsource, std::string fsource) {
        return init(vsource.c_str(),fsource.c_str());
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
    bool init(const char* vsource, const char* fsource);

#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a new shader with the default vertex and fragment source.
     *
     * The shader will compile the vertex and fragment sources and link
     * them together. When compilation is complete, the shader will not be
     * bound.  However, any shader that was actively bound during compilation
     * also be unbound as well.
     *
     * @return a new shader with the default vertex and fragment source.
     */
    static std::shared_ptr<SpriteShader> alloc() {
        std::shared_ptr<SpriteShader> result = std::make_shared<SpriteShader>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a new shader with the given vertex and fragment source.
     *
     * The shader will compile the vertex and fragment sources and link
     * them together. When compilation is complete, the shader will not be
     * bound.  However, any shader that was actively bound during compilation
     * also be unbound as well.
     *
     * @param vsource   The source string for the vertex shader.
     * @param fsource   The source string for the fragment shader.
     *
     * @return a new shader with the given vertex and fragment source.
     */
    static std::shared_ptr<SpriteShader> alloc(std::string vsource, std::string fsource) {
        std::shared_ptr<SpriteShader> result = std::make_shared<SpriteShader>();
        return (result->init(vsource, fsource) ? result : nullptr);
    }
    
    /**
     * Returns a new shader with the given vertex and fragment source.
     *
     * The shader will compile the vertex and fragment sources and link
     * them together. When compilation is complete, the shader will not be
     * bound.  However, any shader that was actively bound during compilation
     * also be unbound as well.
     *
     * @param vsource   The source string for the vertex shader.
     * @param fsource   The source string for the fragment shader.
     *
     * @return a new shader with the given vertex and fragment source.
     */
    static std::shared_ptr<SpriteShader> alloc(const char* vsource, const char* fsource) {
        std::shared_ptr<SpriteShader> result = std::make_shared<SpriteShader>();
        return (result->init(vsource, fsource) ? result : nullptr);
    }
    
#pragma mark -
#pragma mark Attributes
    /**
     * Returns the GLSL location for the position attribute
     *
     * This method will return -1 if the program is not initialized.
     *
     * @return the GLSL location for the position attribute
     */
    GLint getPositionAttr() const { return _aPosition; }

    /**
     * Returns the GLSL location for the color attribute
     *
     * This method will return -1 if the program is not initialized.
     *
     * @return the GLSL location for the color attribute
     */
    GLint getColorAttr() const { return _aColor; }

    /**
     * Returns the GLSL location for the texture coordinate attribute
     *
     * This method will return -1 if the program is not initialized.
     *
     * @return the GLSL location for the texture coordinate attribute
     */
    GLint getTexCoodAttr() const { return _aTexCoord; }
    
    /**
     * Returns the GLSL location for the perspective matrix uniform
     *
     * This method will return -1 if the program is not initialized.
     *
     * @return the GLSL location for the perspective matrix uniform
     */
    GLint getPerspectiveUni() const { return _uPerspective; }

    /**
     * Returns the GLSL location for the texture uniform
     *
     * This method will return -1 if the program is not initialized.
     *
     * @return the GLSL location for the testure uniform
     */
    GLint getTextureUni() const { return _uTexture; }
    
    /**
     * Sets the perspective matrix to use in the shader.
     *
     * @param matrix    The perspective matrix
     */
    void setPerspective(const Mat4&  matrix);

    /**
     * Returns the current perspective matrix in use.
     *
     * @return the current perspective matrix in use.
     */
    const Mat4& getPerspective() { return _mPerspective; }

    /**
     * Sets the texture in use in the shader
     *
     * @param texture   The shader texture
     */
    void setTexture(const std::shared_ptr<Texture>& texture);
    
    /**
     * Returns the current texture in use.
     *
     * @return the current texture in use.
     */
    std::shared_ptr<Texture> getTexture() { return _mTexture; }

    /**
     * Returns the current texture in use.
     *
     * @return the current texture in use.
     */
    const std::shared_ptr<Texture>& getTexture() const { return _mTexture; }

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
    void attach(GLuint vArray, GLuint vBuffer);

    /**
     * Binds this shader, making it active.
     *
     * Once bound, any OpenGL calls will then be sent to this shader.
     */
    void bind() override;
    
    /**
     * Unbinds this shader, making it no longer active.
     *
     * Once unbound, OpenGL calls will no longer be sent to this shader.
     */
    void unbind() override;

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
    bool compile() override;

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
    bool validateVariable(GLint variable, const char* name);

};

}

#endif /* __CU_SPRITE_SHADER_H__ */

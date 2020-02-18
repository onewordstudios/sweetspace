//
//  CUTexture.h
//  Cornell University Game Library (CUGL)
//
//  This module provides the class for representing a 2D OpenGL texture. This
//  class also provides support for texture atlases.  Any non-repeating texture
//  can produce a subtexture.  A subtexture wraps the same texture data (and so
//  does not require a context switch in the rendering pipeline), but has
//  different start and end boundaries, as defined by minS, maxS, minT and maxT
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

#ifndef _CU_TEXTURE_H__
#define _CU_TEXTURE_H__

#include <cugl/math/CUMathBase.h>
#include <cugl/math/CUSize.h>

namespace cugl {

/**
 * This is a class representing an OpenGL texture.
 *
 * We enforce that all textures must be a power-of-two along each dimension
 * (though they need not be square).  This is still required by some mobile
 * devices and so it is easiest to require it across the board.
 *
 * This class also provides support for texture atlases.  Any non-repeating
 * texture can produce a subtexture.  A subtexture wraps the same texture data
 * (and so does not require a context switch in the rendering pipeline), but
 * has different start and end boundaries, as defined by minS, maxS, minT and
 * maxT. See getSubtexture() for more information.
 */
class Texture : public std::enable_shared_from_this<Texture> {
#pragma mark Values
public:
    /**
     * This enum lists the possible texture pixel formats.
     *
     * Because of cross-platform issues (we must support both OpenGL and 
     * OpenGLES), our textures only support a small subset of formats.
     * No other formats are supported.
     */
    enum class PixelFormat : GLenum {
        /** The default type; RGB with alpha transparency */
        RGBA = GL_RGBA,
        /** RGB with no alpha (all blending assumes alpha is 1.0) */
        RGB  = GL_RGBA,
        /** A single color channel of red (all blending assumes alpha is 1.0) */
        RED  = GL_RED,
        /** An alpha-only channel */
        ALPHA = GL_ALPHA,
        /** The image is not yet defined */
        UNDEFINED = GL_RG
    };
    
private:
    /** A reference to the allocated texture in OpenGL; 0 is not allocated. */
    GLuint _buffer;

    /** The width in pixels */
    unsigned int _width;
    
    /** The height in pixels */
    unsigned int _height;

    /** The pixel format of the texture */
    PixelFormat _pixelFormat;

    /** The decriptive texture name */
    std::string _name;
    
    /** The minimization algorithm */
    GLuint _minFilter;

    /** The maximization algorithm */
    GLuint _magFilter;
    
    /** The wrap-style for the horizontal texture coordinate */
    GLuint _wrapS;

    /** The wrap-style for the vertical texture coordinate */
    GLuint _wrapT;

    /** Whether or not the texture has mip maps */
    bool _hasMipmaps;

    /// Texture atlas support
    /** Our parent, who owns the OpenGL texture (or nullptr if we own it) */
    std::shared_ptr<Texture> _parent;
    
    /** The texture min S (used for texture atlases) */
    GLfloat _minS;

    /** The texture max S (used for texture atlases) */
    GLfloat _maxS;
    
    /** The texture min T (used for texture atlases) */
    GLfloat _minT;

    /** The texture max T (used for texture atlases) */
    GLfloat _maxT;

    /** Whether or not this texture is currently active */
    bool _active;
    
#pragma mark -
#pragma mark Constructors
public:
    /** 
     * Creates a new empty texture with no size. 
     *
     * This method performs no allocations.  You must call init to generate
     * a proper OpenGL texture.
     */
    Texture();
    
    /**
     * Deletes this texture, disposing all resources
     */
    ~Texture() { dispose(); }
    
    /**
     * Deletes the OpenGL texture and resets all attributes.
     *
     * You must reinitialize the texture to use it.
     */
    void dispose();

    /**
     * Initializes an empty texture with the given dimensions.
     *
     * When initialization is done, the texture is no longer bound.  However,
     * any other texture that was bound during initialization is also no longer
     * bound.
     *
     * You must use the set() method to load data into the texture.
     *
     * @param width     The texture width in pixels
     * @param height    The texture height in pixels
     * @param format    The texture data format
     *
     * @return true if initialization was successful.
     */
    bool init(int width, int height, PixelFormat format = PixelFormat::RGBA);

    /**
     * Initializes an texture with the given data.
     *
     * When initialization is done, the texture is no longer bound.  However, 
     * any other texture that was bound during initialization is also no longer 
     * bound.
     *
     * The data format must match the one given.
     *
     * @param data      The texture data (size width*height*format)
     * @param width     The texture width in pixels
     * @param height    The texture height in pixels
     * @param format    The texture data format
     *
     * @return true if initialization was successful.
     */
    bool initWithData(const void *data, int width, int height,
                      PixelFormat format = PixelFormat::RGBA);
    
    /**
     * Initializes an texture with the data from the given file.
     *
     * When initialization is done, the texture is no longer bound.  However,
     * any other texture that was bound during initialization is also no longer
     * bound.
     *
     * This method can load any file format supported by SDL_Image.  This
     * includes (but is not limited to) PNG, JPEG, GIF, TIFF, BMP and PCX.
     *
     * The texture will be stored in RGBA format, even if it is a file format
     * that does not support transparency (e.g. JPEG).
     *
     * @param filename  The file supporting the texture file.
     *
     * @return true if initialization was successful.
     */
    bool initWithFile(const std::string& filename);

#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a new empty texture with the given dimensions.
     *
     * When initialization is done, the texture is no longer bound.  However,
     * any other texture that was bound during initialization is also no longer
     * bound.
     *
     * You must use the set() method to load data into the texture.
     *
     * @param width     The texture width in pixels
     * @param height    The texture height in pixels
     * @param format    The texture data format
     *
     * @return a new empty texture with the given dimensions.
     */
    static std::shared_ptr<Texture> alloc(int width, int height,
                                          PixelFormat format = PixelFormat::RGBA) {
        std::shared_ptr<Texture> result = std::make_shared<Texture>();
        return (result->init(width, height, (cugl::Texture::PixelFormat)format) ? result : nullptr);
    }

    /**
     * Returns a new texture with the given data.
     *
     * When initialization is done, the texture is no longer bound.  However,
     * any other texture that was bound during initialization is also no longer
     * bound.
     *
     * The data format must match the one given.
     *
     * @param data      The texture data (size width*height*format)
     * @param width     The texture width in pixels
     * @param height    The texture height in pixels
     * @param format    The texture data format
     *
     * @return a new texture with the given data.
     */
    static std::shared_ptr<Texture> allocWithData(const void *data, int width, int height,
                                                  PixelFormat format = PixelFormat::RGBA) {
        std::shared_ptr<Texture> result = std::make_shared<Texture>();
        return (result->initWithData(data, width, height, (cugl::Texture::PixelFormat)format) ? result : nullptr);

    }
   
    
    /**
     * Returns a new texture with the data from the given file.
     *
     * When initialization is done, the texture is no longer bound.  However,
     * any other texture that was bound during initialization is also no longer
     * bound.
     *
     * This method can load any file format supported by SDL_Image.  This
     * includes (but is not limited to) PNG, JPEG, GIF, TIFF, BMP and PCX.
     *
     * The texture will be stored in RGBA format, even if it is a file format
     * that does not support transparency (e.g. JPEG).
     *
     * @param filename  The file supporting the texture file.
     *
     * @return a new texture with the given data
     */
    static std::shared_ptr<Texture> allocWithFile(const std::string& filename) {
        std::shared_ptr<Texture> result = std::make_shared<Texture>();
        return (result->initWithFile(filename) ? result : nullptr);
    }

#pragma mark -
#pragma mark Setters
    /**
     * Sets this texture to have the contents of the given buffer.
     *
     * The buffer must have the correct data format.  In addition, the buffer
     * must be size width*height*format.
     *
     * This method binds the texture if it is not currently active.
     *
     * @param data  The buffer to read into the texture
     *
     * @return a reference to this (modified) texture for chaining.
     */
    const Texture& operator=(const void *data) {
        return set(data);
    }

    /**
     * Sets this texture to have the contents of the given buffer.
     *
     * The buffer must have the correct data format.  In addition, the buffer
     * must be size width*height*format.
     *
     * This method binds the texture if it is not currently active.
     *
     * @param data  The buffer to read into the texture
     *
     * @return a reference to this (modified) texture for chaining.
     */
    const Texture& set(const void *data);

#pragma mark -
#pragma mark Attributes
    /**
     * Returns true if this shader has been compiled and is ready for use.
     *
     * @return true if this shader has been compiled and is ready for use.
     */
    bool isReady() const { return _buffer != 0; }

    /**
     * Returns whether this texture is actively in use.
     *
     * If this texture is a subtexture of texture in use, this method will also
     * return true (and vice versa)
     *
     * If this texture is the parent of a subtexture in use, this method
     * will return true.
     *
     * @return whether this texture is actively in use.
     */
    bool isActive() const   {
        return (_parent != nullptr ? _parent->isActive() : _active);
    }
    
    /**
     * Returns whether this texture has generated mipmaps.
     *
     * If this texture is a subtexture of texture with mipmaps, this method will
     * also return true (and vice versa)
     *
     * @return whether this texture has generated mipmaps.
     */
    bool hasMipMaps() const {
        return (_parent != nullptr ? _parent->hasMipMaps() : _hasMipmaps);
    }

    /**
     * Builds mipmaps for the current texture.
     *
     * This method will fail if this texture is a subtexture.  Only the parent
     * texture can have mipmaps.  In addition, mipmaps can only be built if the
     * texture size is a power of two.
     */
    void buildMipMaps();
    
    /**
     * Returns the OpenGL buffer for this texture.
     *
     * This method will return 0 if the texture is not initialized.
     *
     * @return the OpenGL buffer for this texture.
     */
    GLuint getBuffer() { return _buffer; }
    
    /**
     * Sets the name of this texture.
     *
     * A name is a user-defined way of identifying a texture.  Subtextures are
     * permitted to have different names than their parents.
     *
     * @param name  The name of this texture.
     */
    void setName(std::string name) { _name = name; }
    
    /**
     * Returns the name of this texture.
     *
     * A name is a user-defined way of identifying a texture.  Subtextures are
     * permitted to have different names than their parents.
     *
     * @return the name of this texture.
     */
    const std::string& getName() const { return _name; }
    
    /**
     * Returns the width of this texture in pixels.
     *
     * @return the width of this texture in pixels.
     */
    unsigned int getWidth()  const { return _width;  }
    
    /**
     * Returns the height of this texture in pixels.
     *
     * @return the height of this texture in pixels.
     */
    unsigned int getHeight() const { return _height; }
    
    /**
     * Returns the size of this texture in pixels.
     *
     * @return the size of this texture in pixels.
     */
    Size getSize() { return Size((float)_width,(float)_height); }
    
    /** 
     * Returns the data format of this texture.
     *
     * The data format determines what type of data can be assigned
     * to this texture.
     *
     * @return the data format of this texture.
     */
    PixelFormat getFormat() const { return _pixelFormat; }
    
    /**
     * Returns the min filter of this texture.
     *
     * The min filter is the algorithm hint that OpenGL uses to make an
     * image smaller.  The default is GL_NEAREST.
     *
     * @return the min filter of this texture.
     */
    GLuint getMinFilter() const {
        return (_parent != nullptr ? _parent->getMinFilter() : _minFilter);
    }

    /**
     * Returns the mag filter of this texture.
     *
     * The mag filter is the algorithm hint that OpenGL uses to make an
     * image larger.  The default is GL_LINEAR.
     *
     * @return the mag filter of this texture.
     */
    GLuint getMagFilter() const {
        return (_parent != nullptr ? _parent->getMagFilter() : _magFilter);
    }

    /**
     * Sets the min filter of this texture.
     *
     * The min filter is the algorithm hint that OpenGL uses to make an
     * image smaller.  The default is GL_NEAREST.
     *
     * @param minFilter The min filter of this texture.
     */
    void setMinFilter(GLuint minFilter);
    
    /**
     * Sets the mag filter of this texture.
     *
     * The mag filter is the algorithm hint that OpenGL uses to make an
     * image larger.  The default is GL_LINEAR.
     *
     * @param magFilter The mag filter of this texture.
     */
    void setMagFilter(GLuint magFilter);

    /**
     * Returns the horizontal wrap of this texture.
     * 
     * The default is GL_CLAMP_TO_EDGE.
     *
     * @return the horizontal wrap of this texture.
     */
    GLuint getWrapS() const {
        return (_parent != nullptr ? _parent->getWrapS() : _wrapS);
    }

    /**
     * Returns the vertical wrap of this texture.
     *
     * The default is GL_CLAMP_TO_EDGE.
     *
     * @return the vertical wrap of this texture.
     */
    GLuint getWrapT() const {
        return (_parent != nullptr ? _parent->getWrapT() : _wrapT);
    }

    /**
     * Sets the horizontal wrap of this texture.
     *
     * The default is GL_CLAMP_TO_EDGE.
     */
    void setWrapS(GLuint wrap);

    /**
     * Sets the vertical wrap of this texture.
     *
     * The default is GL_CLAMP_TO_EDGE.
     */
    void setWrapT(GLuint wrap);

    
#pragma mark -
#pragma mark Atlas Support
    /** 
     * Returns the parent texture of this subtexture.
     *
     * This method will return nullptr is this is not a subtexture.
     *
     * Returns the parent texture of this subtexture.
     */
    const std::shared_ptr<Texture>& getParent() const { return _parent; }

    /**
     * Returns the parent texture of this subtexture.
     *
     * This method will return nullptr is this is not a subtexture.
     *
     * Returns the parent texture of this subtexture.
     */
    std::shared_ptr<Texture> getParent() { return _parent; }
    
    /**
     * Returns a subtexture with the given dimensions.
     *
     * The values must be 0 <= minS <= maxS <= 1 and 0 <= minT <= maxT <= 1.
     * They specify the region of the texture to extract the subtexture.
     *
     * It is the responsibility of the user to rescale the texture coordinates
     * when using subtexture.  Otherwise, the OpenGL pipeline will just use 
     * the original texture instead.  See the method internal method prepare of
     * {@link SpriteBatch} for an example of how to scale texture coordinates.
     *
     * It is possible to make a subtexture of a subtexture.  However, in that
     * case, the minS, maxS, minT and maxT values are all with respect to the
     * original root texture.  Furthermore, the parent of the new subtexture 
     * will be the original root texture.  So no tree of subtextures is more
     * than one level deep.
     *
     * Returns  a subtexture with the given dimensions.
     */
    std::shared_ptr<Texture> getSubTexture(GLfloat minS, GLfloat maxS, GLfloat minT, GLfloat maxT);
    
    /**
     * Returns true if this texture is a subtexture.
     *
     * This is the same as checking if the parent is not nullptr.
     *
     * @return true if this texture is a subtexture.
     */
    bool isSubTexture() const { return _parent != nullptr; }

    /**
     * Returns the minimum S texture coordinate for this texture.
     *
     * When rescaling texture coordinates for a subtexture, this value is 
     * used in place of 0.
     *
     * @return the minimum S texture coordinate for this texture.
     */
    GLfloat getMinS() const { return _minS; }

    /**
     * Returns the minimum T texture coordinate for this texture.
     *
     * When rescaling texture coordinates for a subtexture, this value is
     * used in place of 0.
     *
     * @return the minimum T texture coordinate for this texture.
     */
    GLfloat getMinT() const { return _minT; }

    /**
     * Returns the maximum S texture coordinate for this texture.
     *
     * When rescaling texture coordinates for a subtexture, this value is
     * used in place of 0.
     *
     * @return the maximum S texture coordinate for this texture.
     */
    GLfloat getMaxS() const { return _maxS; }

    /**
     * Returns the maximum T texture coordinate for this texture.
     *
     * When rescaling texture coordinates for a subtexture, this value is
     * used in place of 0.
     *
     * @return the maximum T texture coordinate for this texture.
     */
    GLfloat getMaxT() const { return _maxT; }

// TODO:: Ninepatch support
    
#pragma mark -
#pragma mark Rendering
    /**
     * Binds this texture, making it active.
     *
     * Once bound, any OpenGL calls will then use this texture.
     */
    void bind();
    
    /**
     * Uninds this texture, making it no longer active.
     *
     * Once unbound, OpenGL calls will no longer use this texture.
     */
    void unbind();
    
#pragma mark -
#pragma mark Conversions
    /**
     * Returns a string representation of this texture for debugging purposes.
     *
     * If verbose is true, the string will include class information.  This
     * allows us to unambiguously identify the class.
     *
     * @param verbose Whether to include class information
     *
     * @return a string representation of this texture for debugging purposes.
     */
    std::string toString(bool verbose = false) const;
    
    /** Cast from Texture to a string. */
    operator std::string() const { return toString(); }

};

}

#endif /* _CU_TEXTURE_H__ */

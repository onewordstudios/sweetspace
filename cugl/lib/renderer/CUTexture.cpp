//
//  CUTexture.cpp
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
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <cugl/renderer/CUTexture.h>
#include <cugl/util/CUDebug.h>
#include <sstream>

using namespace cugl;


#pragma mark Constructors
/**
 * Creates a new empty texture with no size.
 *
 * This method performs no allocations.  You must call init to generate
 * a proper OpenGL texture.
 */
Texture::Texture() :
_buffer(0),
_width(0),
_height(0),
_pixelFormat(PixelFormat::UNDEFINED),
_name(""),
_minFilter(GL_NEAREST),
_magFilter(GL_LINEAR),
_wrapS(GL_CLAMP_TO_EDGE),
_wrapT(GL_CLAMP_TO_EDGE),
_hasMipmaps(false),
_parent(nullptr),
_minS(0),
_maxS(1),
_minT(0),
_maxT(1),
_active(false) {}

/**
 * Deletes the OpenGL texture and resets all attributes.
 *
 * You must reinitialize the texture to use it.
 */
void Texture::dispose() {
    if (_buffer != 0) {
        // Do we own the texture?
        if (_parent == nullptr) {
            glDeleteTextures(1, &_buffer);
        }
        _buffer = 0;
        _width = 0; _height = 0;
        _pixelFormat = PixelFormat::UNDEFINED;
        _name = "";
        _minFilter = GL_NEAREST; _magFilter = GL_LINEAR;
        _wrapS = GL_CLAMP_TO_EDGE; _wrapT = GL_CLAMP_TO_EDGE;
        _parent = nullptr;
        _minS = _minT = 0;
        _maxS = _maxT = 1;
        _hasMipmaps = false;
        _active = false;
    }
}

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
bool Texture::init(int width, int height, Texture::PixelFormat format) {
    CUAssertLog(nextPOT(width)  == width, "Width  %d is not a power of two", width);
    CUAssertLog(nextPOT(height) == width, "Height %d is not a power of two", height);
    if (_buffer) {
        CUAssertLog(false, "Texture is already initialized");
        return false; // In case asserts are off.
    }
    
    glGenTextures(1, &_buffer);
    if (_buffer == 0) {
        return false;
    }
    
    _width  = width;
    _height = height;
    _pixelFormat = format;
    glBindTexture(GL_TEXTURE_2D, _buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _magFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _wrapT);
    glTexImage2D(GL_TEXTURE_2D, 0, (GLenum)format, width, height, 0, (GLenum)format, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
    setName("<empty>");
    return true;
}

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
bool Texture::initWithData(const void *data, int width, int height, Texture::PixelFormat format) {
    if (_buffer) {
        CUAssertLog(false, "Texture is already initialized");
        return false; // In case asserts are off.
    }
    
    glGenTextures(1, &_buffer);
    if (_buffer == 0) {
        return false;
    }
    
    _width  = width;
    _height = height;
    _pixelFormat = format;
    glBindTexture(GL_TEXTURE_2D, _buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _magFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _wrapT);
    glTexImage2D(GL_TEXTURE_2D, 0, (GLenum)format, width, height, 0, (GLenum)format, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    std::stringstream ss;
    ss << "@" << data;
    setName(ss.str());
    return true;
}

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
 * @param height    The texture height in pixels
 * @param format    The texture data format
 *
 * @return true if initialization was successful.
 */
bool Texture::initWithFile(const std::string& filename) {
    SDL_Surface* surface = IMG_Load(filename.c_str());
    if (surface == nullptr) {
        return false;
    }
    
    SDL_Surface* normal;
#if CU_MEMORY_ORDER == CU_ORDER_REVERSED
    normal = SDL_ConvertSurfaceFormat(surface,SDL_PIXELFORMAT_ABGR8888,0);
#else
    normal = SDL_ConvertSurfaceFormat(surface,SDL_PIXELFORMAT_RGBA8888,0);
#endif
    SDL_FreeSurface(surface);
    if (normal == nullptr) {
        return false;
    }

    bool result = initWithData(normal->pixels, normal->w, normal->h);
    SDL_FreeSurface(normal);
    if (result) setName(filename);
    return result;
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
const Texture& Texture::set(const void *data) {
    if (!_active) { bind(); }
    glTexImage2D(GL_TEXTURE_2D, 0, (GLenum)_pixelFormat, _width, _height, 0,
                 (GLenum)_pixelFormat, GL_UNSIGNED_BYTE, data);
    return *this;
}


#pragma mark -
#pragma mark Attributes

/**
 * Builds mipmaps for the current texture.
 *
 * This method will fail if this texture is a subtexture.  Only the parent
 * texture can have mipmaps.  In addition, mipmaps can only be built if the
 * texture size is a power of two.
 */
void Texture::buildMipMaps() {
    CUAssertLog(nextPOT(_width)  == _width,  "Width  %d is not a power of two", _width);
    CUAssertLog(nextPOT(_height) == _height, "Height %d is not a power of two", _height);
    CUAssertLog(_parent == nullptr, "Cannot build mipmaps for a subtexture");
    CUAssertLog(_active, "Texture is not active");
    glGenerateMipmap(GL_TEXTURE_2D);
    _hasMipmaps = true;
}

/**
 * Sets the min filter of this texture.
 *
 * The min filter is the algorithm hint that OpenGL uses to make an
 * image smaller.  The default is GL_NEAREST.
 *
 * @param minFilter The min filter of this texture.
 */
void Texture::setMinFilter(GLuint minFilter) {
    CUAssertLog(_parent == nullptr, "Cannot set filters for a subtexture");
    CUAssertLog(!_buffer || _active, "Texture is not active");
    _minFilter = minFilter;
    if (_buffer) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _minFilter);
    }
}

/**
 * Sets the mag filter of this texture.
 *
 * The mag filter is the algorithm hint that OpenGL uses to make an
 * image larger.  The default is GL_LINEAR.
 *
 * @param miagFilter The mag filter of this texture.
 */
void Texture::setMagFilter(GLuint magFilter) {
    CUAssertLog(_parent == nullptr, "Cannot set filters for a subtexture");
    CUAssertLog(!_buffer || _active, "Texture is not active");
    _magFilter = magFilter;
    if (_buffer) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _magFilter);
    }
}

/**
 * Sets the horizontal wrap of this texture.
 *
 * The default is GL_CLAMP_TO_EDGE.
 */
void Texture::setWrapS(GLuint wrap) {
    CUAssertLog(_parent == nullptr, "Cannot set wrap S for a subtexture");
    CUAssertLog(!_buffer || _active, "Texture is not active");
    _wrapS = wrap;
    if (_buffer) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _wrapS);
    }
}

/**
 * Sets the vertical wrap of this texture.
 *
 * The default is GL_CLAMP_TO_EDGE.
 */
void Texture::setWrapT(GLuint wrap) {
    CUAssertLog(_parent == nullptr, "Cannot set wrap T for a subtexture");
    CUAssertLog(!_buffer || _active, "Texture is not active");
    _wrapT = wrap;
    if (_buffer) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _wrapT);
    }
}

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
std::string Texture::toString(bool verbose) const {
    std::stringstream ss;
    ss << (verbose ? "cugl::Texture[" : "[");
    ss << "data:" << getName() << ",";
    ss << "w:" << getWidth() << ",";
    ss << "h:" << getWidth();
    if (_parent != nullptr) {
        ss << ",";
        ss << " (" << _minS << "," << _maxS << ")";
        ss << "x(" << _minT << "," << _maxT << ")";
    }
    ss << "]";
    return ss.str();
}


#pragma mark -
#pragma mark Atlas Support

/**
 * Returns a subtexture with the given dimensions.
 *
 * The values must be 0 <= minS <= maxS <= 1 and 0 <= minT <= maxT <= 1.
 * They specify the region of the texture to extract the subtexture.
 *
 * It is the responsibility of the user to rescale the texture coordinates
 * when using subtexture.  Otherwise, the OpenGL pipeline will just use
 * the original texture instead.  See the method {@link SpriteBatch#prepare()}
 * for an example of how to scale texture coordinates.
 *
 * It is possible to make a subtexture of a subtexture.  However, in that
 * case, the minS, maxS, minT and maxT values are all with respect to the
 * original root texture.  Furthermore, the parent of the new subtexture
 * will be the original root texture.  So no tree of subtextures is more
 * than one level deep.
 *
 * Returns  a subtexture with the given dimensions.
 */
std::shared_ptr<Texture> Texture::getSubTexture(GLfloat minS, GLfloat maxS,
                                                GLfloat minT, GLfloat maxT) {
    CUAssertLog(_buffer, "Texture is not initialized");
    CUAssertLog(minS >= _minS && minS <= maxS, "Value minS is out of range");
    CUAssertLog(maxS <= _maxS, "Value maxS is out of range");
    CUAssertLog(minT >= _minT && minT <= maxT, "Value minT is out of range");
    CUAssertLog(maxT <= _maxT, "Value maxT is out of range");
    
    std::shared_ptr<Texture> result = std::make_shared<Texture>();

    // Make sure the tree is not deep
    std::shared_ptr<Texture> source = (_parent == nullptr ? shared_from_this() : _parent);
    
    // Shared values
    result->_buffer = source->_buffer;
    result->_parent = source;
    result->_pixelFormat = source->_pixelFormat;
    result->_name = source->_name;
    
    // Filters, wrap, and binding defer to parent.
    // These values can be left alone.
    
    // Set the size information
    result->_width  = (unsigned int)((maxS-minS)*source->_width);
    result->_height = (unsigned int)((maxT-minT)*source->_height);
    result->_minS = minS;
    result->_maxS = maxS;
    result->_minT = minT;
    result->_maxT = maxT;

    return result;
}

#pragma mark -
#pragma mark Rendering
/**
 * Binds this texture, making it active.
 *
 * Once bound, any OpenGL calls will then use this texture.
 */
void Texture::bind() {
    if (_parent != nullptr) {
        _parent->bind();
        return;
    }
    
    CUAssertLog(_buffer, "Texture is not defined");
    CUAssertLog(!_active, "Texture is already active");
    glBindTexture(GL_TEXTURE_2D, _buffer);
    _active = true;
}

/**
 * Uninds this texture, making it no longer active.
 *
 * Once unbound, OpenGL calls will no longer use this texture.
 */
void Texture::unbind() {
    if (_parent != nullptr) {
        _parent->unbind();
        return;
    }
    
    CUAssertLog(_active, "Texture is not active");
    glBindTexture(GL_TEXTURE_2D, 0);
    _active = false;
}




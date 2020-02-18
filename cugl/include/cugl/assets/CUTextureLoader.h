//
//  CUTextureLoader.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a specific implementation of the Loader class to load
//  textures. A texture asset is identified by both its source file and its
//  texture parameters.  Hence you may wish to load a texture asset multiple
//  times, though this is potentially wasteful regarding memory.
//
//  As with all of our loaders, this loader is designed to be attached to an
//  asset manager.  In addition, this class uses our standard shared-pointer
//  architecture.
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
//  Version: 1/7/16
//
#ifndef __CU_TEXTURE_LOADER_H__
#define __CU_TEXTURE_LOADER_H__
#include <cugl/assets/CULoader.h>
#include <cugl/renderer/CUTexture.h>

namespace cugl {

/**
 * This class is a specific implementation of Loader<Texture>
 *
 * This asset loader allows us to allocate texture objects from the associated
 * image files. A texture asset is identified by both its source file and its
 * texture parameters.  Hence you may wish to load a texture asset multiple
 * times, though this is potentially wasteful regarding memory. However, 
 * changing the parameters for a texture asset will change the asset parameters
 * in this loader as well.
 *
 * Note that this implementation uses a two phase loading system.  First, it
 * loads as much of the asset as possible without using OpenGL.  This allows 
 * us to load the texture in a separate thread.  It then finishes off the 
 * remainder of asset loading using {@link Application#schedule}.  This is a
 * good template for asset loaders in general.
 *
 * As with all of our loaders, this loader is designed to be attached to an
 * asset manager. Use the method {@link getHook()} to get the appropriate
 * pointer for attaching the loader.
 */
class TextureLoader : public Loader<Texture> {
private:
    /** This macro disables the copy constructor (not allowed on assets) */
    CU_DISALLOW_COPY_AND_ASSIGN(TextureLoader);
    
protected:
    /** The default min filter */
    GLuint _minfilter;
    /** The default mag filter */
    GLuint _magfilter;
    /** The default s-coordinate wrap */
    GLuint _wraps;
    /** The default t-coordinate wrap */
    GLuint _wrapt;
    /** The default support for mipmaps */
    bool _mipmaps;
    
#pragma mark Asset Loading
    /**
     * Extracts any subtextures specified in an atlas
     *
     * An atlas is specified as a list of named, four-element integer arrays.
     * Each integer array specifies the left, top, right, and bottom pixels of
     * the subtexture, respectively.  Each subtexture will have the key of the
     * main texture as the prefix (together with an underscore _) of its key.
     *
     * @param json      The asset directory entry
     * @param texture   The texture loaded for this asset
     */
    void parseAtlas(const std::shared_ptr<JsonValue>& json, const std::shared_ptr<Texture>& texture);
    
    /**
     * Loads the portion of this asset that is safe to load outside the main thread.
     *
     * It is not safe to create an OpenGL texture in a separate thread.  However,
     * it is safe to create an SDL_surface, which contains all of the data that
     * we need to create an OpenGL texture.  Hence this method does the maximum
     * amount of work that can be done in asynchronous texture loading.
     *
     * @param source    The pathname to the asset
     *
     * @return the SDL_Surface with the texture information
     */
    SDL_Surface* preload(const std::string& source);
    
    /**
     * Creates an OpenGL texture from the SDL_Surface, and assigns it the given key.
     *
     * This method finishes the asset loading started in {@link preload}.  This
     * step is not safe to be done in a separate thread.  Instead, it takes
     * place in the main CUGL thread via {@link Application#schedule}.
     *
     * The loaded texture will have default parameters for scaling and wrap.
     * It will only have a mipmap if that is the default.
     *
     * This method supports an optional callback function which reports whether
     * the asset was successfully materialized.
     *
     * @param key       The key to access the asset after loading
     * @param surface   The SDL_Surface to convert
     * @param callback  An optional callback for asynchronous loading
     */
    void materialize(const std::string& key, SDL_Surface* surface, LoaderCallback callback);
    
    /**
     * Creates an OpenGL texture from the SDL_Surface accoring to the directory entry.
     *
     * This method finishes the asset loading started in {@link preload}.  This
     * step is not safe to be done in a separate thread.  Instead, it takes
     * place in the main CUGL thread via {@link Application#schedule}.
     *
     * This version of read provides support for JSON directories. A texture
     * directory entry has the following values
     *
     *      "file":         The path to the asset
     *      "mipmaps":      Whether to generate mipmaps (bool)
     *      "minfilter":    The name of the min filter ("nearest", "linear";
     *                      with mipmaps, "nearest-nearest", "linear-nearest",
     *                      "nearest-linear", or "linear-linear")
     *      "magfilter":    The name of the min filter ("nearest" or "linear")
     *      "wrapS":        The s-coord wrap rule ("clamp", "repeat", or "mirrored")
     *      "wrapT":        The t-coord wrap rule ("clamp", "repeat", or "mirrored")
     *
     * The asset key is the key for the JSON directory entry
     *
     * This method supports an optional callback function which reports whether
     * the asset was successfully materialized.
     *
     * @param json      The asset directory entry
     * @param surface   The SDL_Surface to convert
     * @param callback  An optional callback for asynchronous loading
     */
    void materialize(const std::shared_ptr<JsonValue>& json, SDL_Surface* surface, LoaderCallback callback);
    

    /**
     * Internal method to support asset loading.
     *
     * This method supports either synchronous or asynchronous loading, as
     * specified by the given parameter.  If the loading is asynchronous,
     * the user may specify an optional callback function.
     *
     * This method will split the loading across the {@link preload} and 
     * {@link materialize} methods.  This ensures that asynchronous loading
     * is safe.
     *
     * @param key       The key to access the asset after loading
     * @param source    The pathname to the asset
     * @param callback  An optional callback for asynchronous loading
     * @param async     Whether the asset was loaded asynchronously
     *
     * @return true if the asset was successfully loaded
     */
    virtual bool read(const std::string& key, const std::string& source,
                      LoaderCallback callback, bool async) override;
    
    /**
     * Internal method to support asset loading.
     *
     * This method supports either synchronous or asynchronous loading, as
     * specified by the given parameter.  If the loading is asynchronous,
     * the user may specify an optional callback function.
     *
     * This method will split the loading across the {@link preload} and
     * {@link materialize} methods.  This ensures that asynchronous loading
     * is safe.
     *
     * This version of read provides support for JSON directories. A texture
     * directory entry has the following values
     *
     *      "file":         The path to the asset
     *      "mipmaps":      Whether to generate mipmaps (bool)
     *      "minfilter":    The name of the min filter ("nearest", "linear";
     *                      with mipmaps, "nearest-nearest", "linear-nearest",
     *                      "nearest-linear", or "linear-linear")
     *      "magfilter":    The name of the min filter ("nearest" or "linear")
     *      "wrapS":        The s-coord wrap rule ("clamp", "repeat", or "mirrored")
     *      "wrapT":        The t-coord wrap rule ("clamp", "repeat", or "mirrored")
     *
     * @param json      The directory entry for the asset
     * @param callback  An optional callback for asynchronous loading
     * @param async     Whether the asset was loaded asynchronously
     *
     * @return true if the asset was successfully loaded
     */
    virtual bool read(const std::shared_ptr<JsonValue>& json,
                      LoaderCallback callback, bool async) override;

    /**
     * Unloads the asset for the given directory entry
     *
     * An asset may still be available if it is referenced by a smart pointer.
     * See the description of the specific implementation for how assets
     * are released.
     *
     * This method is abstract and should be overridden in child classes.  You
     * will notice that this method is essentially identical to unload.  We
     * separated the methods because overloading and virtual methods do not
     * place nice.
     *
     * @param json      The directory entry for the asset
     *
     * @return true if the asset was successfully unloaded
     */
    virtual bool purge(const std::shared_ptr<JsonValue>& json) override;
    
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new, uninitialized texture loader
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a loader on
     * the heap, use one of the static constructors instead.
     */
    TextureLoader();
    
    /**
     * Disposes all resources and assets of this loader
     *
     * Any assets loaded by this object will be immediately released by the
     * loader.  However, a texture may still be available if it is referenced
     * by another smart pointer.  OpenGL will only release a texture asset
     * once all smart pointer attached to the asset are null.
     *
     * Once the loader is disposed, any attempts to load a new asset will
     * fail.  You must reinitialize the loader to begin loading assets again.
     */
    void dispose() override {
        _assets.clear();
        _loader = nullptr;
    }
    
    /**
     * Returns a newly allocated texture loader.
     *
     * This method bootstraps the loader with any initial resources that it
     * needs to load assets. In particular, the OpenGL context must be active.
     * Attempts to load an asset before this method is called will fail.
     *
     * This loader will have no associated threads. That means any asynchronous
     * loading will fail until a thread is provided via {@link setThreadPool}.
     *
     * @return a newly allocated texture loader.
     */
    static std::shared_ptr<TextureLoader> alloc() {
        std::shared_ptr<TextureLoader> result = std::make_shared<TextureLoader>();
        return (result->init() ? result : nullptr);
    }

    /**
     * Returns a newly allocated texture loader.
     *
     * This method bootstraps the loader with any initial resources that it
     * needs to load assets. In particular, the OpenGL context must be active.
     * Attempts to load an asset before this method is called will fail.
     *
     * @param threads   The thread pool for asynchronous loading
     *
     * @return a newly allocated texture loader.
     */
    static std::shared_ptr<TextureLoader> alloc(const std::shared_ptr<ThreadPool>& threads) {
        std::shared_ptr<TextureLoader> result = std::make_shared<TextureLoader>();
        return (result->init(threads) ? result : nullptr);
    }

#pragma mark -
#pragma mark Properties
    
    /**
     * Returns the default min filter for this loader.
     *
     * The min filter is the algorithm hint that OpenGL uses to make an
     * image smaller.  The default is GL_NEAREST.  Once this value is set,
     * all future textures processed by this loader will have this min filter.
     *
     * @return the default min filter for this loader.
     */
    GLuint getMinFilter() const { return _minfilter; }
    
    /**
     * Sets the default min filter for this loader.
     *
     * The min filter is the algorithm hint that OpenGL uses to make an
     * image smaller.  The default is GL_NEAREST.  Once this value is set,
     * all future textures processed by this loader will have this min filter.
     *
     * @param minFilter The default min filter for this loader.
     */
    void setMinFilter(GLuint minFilter) { _minfilter = minFilter; }
    
    /**
     * Returns the default mage filter for this loader.
     *
     * The mag filter is the algorithm hint that OpenGL uses to make an
     * image larger.  The default is GL_LINEAR. Once this value is set,
     * all future textures processed by this loader will have this mag filter.
     *
     * @return the default mage filter for this loader
     */
    GLuint getMagFilter() const { return _magfilter; }

    /**
     * Sets the default mage filter for this loader.
     *
     * The mag filter is the algorithm hint that OpenGL uses to make an
     * image larger.  The default is GL_LINEAR. Once this value is set,
     * all future textures processed by this loader will have this mag filter.
     *
     * @param magFilter The default mage filter for this loader.
     */
    void setMagFilter(GLuint magFilter) { _magfilter = magFilter; }
    
    /**
     * Returns the default horizontal wrap for this loader.
     *
     * The default is GL_CLAMP_TO_EDGE.  Once this value is set, all future 
     * textures processed by this loader will have this horizontal wrap.
     *
     * @return the default horizontal wrap for this loader.
     */
    GLuint getWrapS() const { return _wraps; }
    
    /**
     * Sets the default horizontal wrap for this loader.
     *
     * The default is GL_CLAMP_TO_EDGE.  Once this value is set, all future
     * textures processed by this loader will have this horizontal wrap.
     *
     * @param wrap  The default horizontal wrap for this loader.
     */
    void setWrapS(GLuint wrap) { _wraps = wrap; }
    
    /**
     * Returns the default vertical wrap for this loader.
     *
     * The default is GL_CLAMP_TO_EDGE.  Once this value is set, all future
     * textures processed by this loader will have this vertical wrap.
     *
     * @return the default vertical wrap for this loader.
     */
    GLuint getWrapT() const { return _wrapt; }

    /**
     * Sets the default vertical wrap for this loader.
     *
     * The default is GL_CLAMP_TO_EDGE.  Once this value is set, all future
     * textures processed by this loader will have this vertical wrap.
     *
     * @param wrap  The default vertical wrap for this loader.
     */
    void setWrapT(GLuint wrap) { _wrapt = wrap; }

    /**
     * Returns true if this loader generates mipmaps by default.
     *
     * The default is false.  If this value is set to true, all future textures
     * processed by this loader will build mipmaps by default.  Similarly,
     * setting it to false suppresses mipmaps in all future textures.
     *
     * @return true if this loader generates mipmaps by default.
     */
    bool hasMipMaps() const { return _mipmaps; }
    
    /**
     * Sets whether this loader generates mipmaps by default.
     *
     * The default is false.  If this value is set to true, all future textures
     * processed by this loader will build mipmaps by default.  Similarly,
     * setting it to false suppresses mipmaps in all future textures.
     *
     * @param flag  Whether this loader generates mipmaps by default.
     */
    void setMipMaps(bool flag) { _mipmaps = flag; }

};

}

#endif /* __CU_TEXTURE_LOADER_H__ */

//
//  CUFontLoader.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a specific implementation of the Loader class to load
//  fonts. A font asset is identified by both its source file and its size.
//  The size is required as load time.  Hence you may wish to load the same
//  font file several times.
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
#ifndef __CU_FONT_LOADER_H__
#define __CU_FONT_LOADER_H__
#include <cugl/assets/CULoader.h>
#include <cugl/2d/CUFont.h>

namespace cugl {
    
/**
 * This class is a implementation of Loader<Font>
 *
 * This asset loader allows us to allocate fonts from the associated TrueType
 * files.  At load time you must specify both the TrueType file and the font
 * size.  Hence you may wish to load a font asset multiple times. If you do
 * this, you may wish to adjust the character set.  The size of the font atlas
 * texture is determined by both the font size and the character set.
 *
 * Note that this implementation uses a two phase loading system.  First, it
 * loads as much of the asset as possible without using OpenGL.  This allows
 * us to load the texture in a separate thread.  It then finishes off the
 * remainder of asset loading (particularly the OpenGL atlas generation) using
 * {@link Application#schedule}.  This is a good template for asset loaders in
 * general.
 *
 * As with all of our loaders, this loader is designed to be attached to an
 * asset manager. Use the method {@link getHook()} to get the appropriate
 * pointer for attaching the loader.
 */
class FontLoader : public Loader<Font> {
private:
    /** This macro disables the copy constructor (not allowed on assets) */
    CU_DISALLOW_COPY_AND_ASSIGN(FontLoader);

protected:
    /** The default font size */
    int _fontsize;
    /** The default atlas character set ("" for ASCII) */
    std::string _charset;
    
#pragma mark Asset Loading
    /**
     * Loads the portion of this asset that is safe to load outside the main thread.
     *
     * It is not safe to create an font atlas (which requires OpenGL) in a 
     * separate thread.  However, it is safe to load the TTF data from the file.
     * Hence this method does the maximum amount of work that can be done in 
     * asynchronous font loading.
     *
     * @param source    The pathname to the asset
     * @param charset   The atlas character set
     * @param size      The font size
     *
     * @return the font asset with no generated atlas
     */
    std::shared_ptr<Font> preload(const std::string& source, const std::string& charset, int size);
    
    /**
     * Creates an atlas for the font asset, and assigns it the given key.
     *
     * This method finishes the asset loading started in {@link preload}.  As
     * atlas generation requires OpenGL, this step is not safe to be done in a 
     * separate thread.  Instead, it takes place in the main CUGL thread via 
     ( {@link Application#schedule}.
     *
     * The font atlas will use the character set specified in the asset.
     *
     * This method supports an optional callback function which reports whether
     * the asset was successfully materialized.
     *
     * @param key       The key to access the asset after loading
     * @param font      The font for atlas generation
     * @param callback  An optional callback for asynchronous loading
     */
    void materialize(const std::string& key, const std::shared_ptr<Font>& font, LoaderCallback callback);
    
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
    bool read(const std::string& key, const std::string& source,
              LoaderCallback callback, bool async) override {
        return read(key,source,_fontsize,callback,async);
    }

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
     * @param size      The font size (overriding the default)
     * @param callback  An optional callback for asynchronous loading
     * @param async     Whether the asset was loaded asynchronously
     *
     * @return true if the asset was successfully loaded
     */
    bool read(const std::string& key, const std::string& source, int size,
              LoaderCallback callback, bool async);

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
     * This version of read provides support for JSON directories. A font
     * directory entry has the following values
     *
     *      "file":         The path to the asset
     *      "size":         This font size (int)
     *      "charset":      The set of characters for the font atlas (string)
     *
     * @param json      The directory entry for the asset
     * @param callback  An optional callback for asynchronous loading
     * @param async     Whether the asset was loaded asynchronously
     *
     * @return true if the asset was successfully loaded
     */
    bool read(const std::shared_ptr<JsonValue>& json, LoaderCallback callback, bool async) override;
    
    
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new, uninitialized font loader
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a loader on
     * the heap, use one of the static constructors instead.
     */
    FontLoader();
    
    /**
     * Disposes all resources and assets of this loader
     *
     * Any assets loaded by this object will be immediately released by the
     * loader.  However, a font may still be available if it is referenced
     * by another smart pointer.  OpenGL will only release a font atlas
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
     * Returns a newly allocated font loader.
     *
     * This method bootstraps the loader with any initial resources that it
     * needs to load assets. In particular, the OpenGL context must be active.
     * Attempts to load an asset before this method is called will fail.
     *
     * This loader will have no associated threads. That means any asynchronous
     * loading will fail until a thread is provided via {@link setThreadPool}.
     *
     * @return a newly allocated font loader.
     */
    static std::shared_ptr<FontLoader> alloc() {
        std::shared_ptr<FontLoader> result = std::make_shared<FontLoader>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated font loader.
     *
     * This method bootstraps the loader with any initial resources that it
     * needs to load assets. In particular, the OpenGL context must be active.
     * Attempts to load an asset before this method is called will fail.
     *
     * @param threads   The thread pool for asynchronous loading
     *
     * @return a newly allocated font loader.
     */
    static std::shared_ptr<FontLoader> alloc(const std::shared_ptr<ThreadPool>& threads) {
        std::shared_ptr<FontLoader> result = std::make_shared<FontLoader>();
        return (result->init(threads) ? result : nullptr);
    }
    
#pragma mark -
#pragma mark Loading Interface
    /**
     * Synchronously loads the given asset with the specified key.
     *
     * The asset will be loaded synchronously, which means the main CUGL thread
     * will block until loading is complete. When it is finished loading, the
     * asset will be added to the contents loader, and accessible under the
     * given key.
     *
     * This version of load allows you to specify the font size, overriding 
     * the default value.
     *
     * @param key       The key to access the font after loading
     * @param source    The pathname to the font
     * @param size      The font size
     *
     * @return true if the asset was successfully loaded
     */
    bool load(const std::string& key, const std::string& source, int size) {
        return read(key, source, size, nullptr, false);
    }

    /**
     * Synchronously loads the given asset with the specified key.
     *
     * The asset will be loaded synchronously, which means the main CUGL thread
     * will block until loading is complete. When it is finished loading, the
     * asset will be added to the contents loader, and accessible under the
     * given key.
     *
     * This version of load allows you to specify the font size, overriding
     * the default value.
     *
     * @param key       The key to access the font after loading
     * @param source    The pathname to the font
     * @param size      The font size
     *
     * @return true if the asset was successfully loaded
     */
    bool load(const char* key, const std::string& source, int size) {
        return read(std::string(key), source, size, nullptr, false);
    }

    /**
     * Synchronously loads the given asset with the specified key.
     *
     * The asset will be loaded synchronously, which means the main CUGL thread
     * will block until loading is complete. When it is finished loading, the
     * asset will be added to the contents loader, and accessible under the
     * given key.
     *
     * This version of load allows you to specify the font size, overriding
     * the default value.
     *
     * @param key       The key to access the font after loading
     * @param source    The pathname to the font
     * @param size      The font size
     *
     * @return true if the asset was successfully loaded
     */
    bool load(const std::string& key, const char* source, int size) {
        return read(key, std::string(source), size, nullptr, false);
    }

    /**
     * Synchronously loads the given asset with the specified key.
     *
     * The asset will be loaded synchronously, which means the main CUGL thread
     * will block until loading is complete. When it is finished loading, the
     * asset will be added to the contents loader, and accessible under the
     * given key.
     *
     * This version of load allows you to specify the font size, overriding
     * the default value.
     *
     * @param key       The key to access the font after loading
     * @param source    The pathname to the font
     * @param size      The font size
     *
     * @return true if the asset was successfully loaded
     */
    bool load(const char* key, const char* source, int size) {
        return read(std::string(key), std::string(source), size, nullptr, false);
    }

    /**
     * Asynchronously loads the given asset with the specified key.
     *
     * The asset will be loaded asynchronously.  When it is finished loading,
     * the asset will be added to this loader, and accessible under the given
     * key. This method will mark the loading process as not complete, even if
     * it was completed previously.  It is not safe to access the loaded asset
     * until it is complete again.
     *
     * The optional callback function will be called with the asset status when
     * it either finishes loading or fails to load.
     *
     * This version of loadAsync allows you to specify the font size, overriding
     * the default value.
     *
     * @param key       The key to access the font after loading
     * @param source    The pathname to the font
     * @param size      The font size
     * @param callback  An optional callback for asynchronous loading
     */
    void loadAsync(const std::string& key, const std::string& source, int size,
                   LoaderCallback callback) {
        read(key, source, size, callback, true);
    }

    /**
     * Asynchronously loads the given asset with the specified key.
     *
     * The asset will be loaded asynchronously.  When it is finished loading,
     * the asset will be added to this loader, and accessible under the given
     * key. This method will mark the loading process as not complete, even if
     * it was completed previously.  It is not safe to access the loaded asset
     * until it is complete again.
     *
     * The optional callback function will be called with the asset status when
     * it either finishes loading or fails to load.
     *
     * This version of loadAsync allows you to specify the font size, overriding
     * the default value.
     *
     * @param key       The key to access the font after loading
     * @param source    The pathname to the font
     * @param size      The font size
     * @param callback  An optional callback for asynchronous loading
     */
    void loadAsync(const char* key, const std::string& source, int size,
                   LoaderCallback callback) {
        read(std::string(key), source, size, callback, true);
    }

    /**
     * Asynchronously loads the given asset with the specified key.
     *
     * The asset will be loaded asynchronously.  When it is finished loading,
     * the asset will be added to this loader, and accessible under the given
     * key. This method will mark the loading process as not complete, even if
     * it was completed previously.  It is not safe to access the loaded asset
     * until it is complete again.
     *
     * The optional callback function will be called with the asset status when
     * it either finishes loading or fails to load.
     *
     * This version of loadAsync allows you to specify the font size, overriding
     * the default value.
     *
     * @param key       The key to access the font after loading
     * @param source    The pathname to the font
     * @param size      The font size
     * @param callback  An optional callback for asynchronous loading
     */
    void loadAsync(const std::string& key, const char* source, int size,
                   LoaderCallback callback) {
        read(key, std::string(source), size, callback, true);
    }

    /**
     * Asynchronously loads the given asset with the specified key.
     *
     * The asset will be loaded asynchronously.  When it is finished loading,
     * the asset will be added to this loader, and accessible under the given
     * key. This method will mark the loading process as not complete, even if
     * it was completed previously.  It is not safe to access the loaded asset
     * until it is complete again.
     *
     * The optional callback function will be called with the asset status when
     * it either finishes loading or fails to load.
     *
     * This version of loadAsync allows you to specify the font size, overriding
     * the default value.
     *
     * @param key       The key to access the font after loading
     * @param source    The pathname to the font
     * @param size      The font size
     * @param callback  An optional callback for asynchronous loading
     */
    void loadAsync(const char* key, const char* source, int size,
                   LoaderCallback callback) {
        read(std::string(key), std::string(source), size, callback, true);
    }
    
#pragma mark -
#pragma mark Properties

    /**
     * Returns the default font size
     *
     * Once set, any future font processed by this loader will have this size
     * unless otherwise specified.  The default is 12 point.
     *
     * @return the default font size
     */
    int getFontSize() const    { return _fontsize; }
    
    /**
     * Sets the default font size
     *
     * Once set, any future font processed by this loader will have this size
     * unless otherwise specified.  The default is 12 point.
     *
     * @param size  The default font size
     */
    void setDefaultSize(int size)	{ _fontsize = size; }

    /**
     * Returns the default atlas character set
     *
     * The character set determines exactly those characters in the font atlas.
     * You should keep this value at a minimum, as it reduces the size of the
     * atlas texture.  Once set, any font processed by this loader will use 
     * this character set for its atlas.
     *
     * If character set is the empty string, the atlas will contain all of the
     * ASCII characters.  This is the default value.
     *
     * @return the default atlas character set
     */
    const std::string& getCharacterSet() const { return _charset; }
    
    /**
     * Sets the default atlas character set
     *
     * The character set determines exactly those characters in the font atlas.
     * You should keep this value at a minimum, as it reduces the size of the
     * atlas texture.  Once set, any font processed by this loader will use
     * this character set for its atlas.
     *
     * If character set is the empty string, the atlas will contain all of the
     * ASCII characters.  This is the default value.
     *
     * @param charset   The default atlas character set
     */
    void setCharacterSet(const std::string& charset) { _charset = charset; }
};

}

#endif /* __CU_FONT_LOADER_H__ */

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
#include <cugl/assets/CUFontLoader.h>
#include <cugl/base/CUApplication.h>
#include <cugl/io/CUPathname.h>
#include <SDL/SDL_ttf.h>

using namespace cugl;

/** What the source name is if we do not know it */
#define UNKNOWN_SOURCE  "<unknown>"
/** The default character set (ASCII) */
#define UNKNOWN_CHARS   ""
/** The default character set (ASCII) */
#define UNKNOWN_SIZE    12

#pragma mark -
#pragma mark Constructor

/**
 * Creates a new, uninitialized font loader
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a loader on
 * the heap, use one of the static constructors instead.
 */
FontLoader::FontLoader() : Loader<Font>(),
_fontsize(UNKNOWN_SIZE),
_charset(UNKNOWN_CHARS) {
}


#pragma mark -
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
 * @param charset   The font size
 *
 * @return the font asset with no generated atlas
 */
std::shared_ptr<Font> FontLoader::preload(const std::string& source, const std::string& charset, int size) {
    // Make sure we reference the asset directory
#if defined (__WINDOWS__)
    bool absolute = (bool)strstr(source.c_str(),":") || source[0] == '\\';
#else
    bool absolute = source[0] == '/';
#endif
    CUAssertLog(!absolute, "This loader does not accept absolute paths for assets");
    
    std::string path = Application::get()->getAssetDirectory();
    path.append(source);
    std::shared_ptr<Font> result = Font::alloc(path.c_str(),size);
    if (result == nullptr) {
        return result;
    }
    
    if (charset.empty()) {
        result->buildAtlasAsync();
    } else {
        result->buildAtlasAsync(charset);
    }
   
    return result;
}

/**
 * Creates an an atlas for the font asset, and assigns it the given key.
 *
 * This method finishes the asset loading started in {@link preload}.  As
 * atlas generation requires OpenGL, this step is not safe to be done in a
 * separate thread.  Instead, it takes place in the main CUGL thread via
 ( {@link Application#schedule}.
 *
 * The font atlas will use the default character set.
 *
 * This method supports an optional callback function which reports whether
 * the asset was successfully materialized.
 *
 * @param key       The key to access the asset after loading
 * @param font      The font for atlas generation
 * @param callback  An optional callback for asynchronous loading
 */
void FontLoader::materialize(const std::string& key, const std::shared_ptr<Font>& font, LoaderCallback callback) {
    std::shared_ptr<Texture> texture = font->getAtlas();
    
    bool success = false;
    if (font != nullptr) {
        _assets[key] = font;
        success = true;
    }
    
    if (callback != nullptr) {
        callback(key,success);
    }
    _queue.erase(key);
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
bool FontLoader::read(const std::string& key, const std::string& source, int size,
                      LoaderCallback callback, bool async) {
    if (_assets.find(key) != _assets.end() || _queue.find(key) != _queue.end()) {
        return false;
    }
    _queue.emplace(key);
    
    bool success = false;
    if (_loader == nullptr || !async) {
        std::shared_ptr<Font> font = preload(source,_charset,size);
        if (font != nullptr) {
            success = true;
            materialize(key,font,callback);
        } else {
            _queue.erase(key);
        }
    } else {
        _loader->addTask([=](void) {
            std::shared_ptr<Font> font = this->preload(source,_charset,size);
            Application::get()->schedule([=](void){
                this->materialize(key,font,callback);
                return false;
            });
        });
    }
    
    return success;
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
 * This version of read provides support for JSON directories. A texture
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
bool FontLoader::read(const std::shared_ptr<JsonValue>& json, LoaderCallback callback, bool async) {
    std::string key = json->key();
    if (_assets.find(key) != _assets.end() || _queue.find(key) != _queue.end()) {
        return false;
    }
    _queue.emplace(key);
    
    std::string source  = json->getString("file",UNKNOWN_SOURCE);
    std::string charset = json->getString("charset",UNKNOWN_CHARS);
    int size = json->getInt("size",UNKNOWN_SIZE);
    
    bool success = false;
    if (_loader == nullptr || !async) {
        std::shared_ptr<Font> font = preload(source,charset,size);
        if (font != nullptr) {
            success = true;
            materialize(key,font,callback);
        } else {
            _queue.erase(key);
        }
    } else {
        _loader->addTask([=](void) {
            std::shared_ptr<Font> font = this->preload(source,charset,size);
            Application::get()->schedule([=](void){
                this->materialize(key,font,callback);
                return false;
            });
        });
    }
    
    return success;
}

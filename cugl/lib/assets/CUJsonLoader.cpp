//
//  CUJsonLoader.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides a specific implementation of the Loader class to load
//  (non-directory) json assets.  It is essentially a wrapper around JsonReader
//  that allows it to be used with AssetManager.
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
#include <cugl/assets/CUJsonLoader.h>
#include <cugl/io/CUJsonReader.h>
#include <cugl/base/CUApplication.h>

using namespace cugl;

/** What the source name is if we do not know it */
#define UNKNOWN_SOURCE  "<unknown>"

/**
 * Finishes loading the Json file, cleaning up the wait queues.
 *
 * Allocating a Json asset can be done safely in a separate thread.
 * Hence this method is really just an internal helper for convenience.
 *
 * This method supports an optional callback function which reports whether
 * the asset was successfully materialized.
 *
 * @param key       The key to access the asset after loading
 * @param json      The json asset fully loaded
 * @param callback  An optional callback for asynchronous loading
 */
void JsonLoader::materialize(const std::string& key, const std::shared_ptr<JsonValue>& json,
                              LoaderCallback callback) {
    bool success = false;
    if (json != nullptr) {
        _assets[key] = json;
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
 * This method will split the loading across the {@link Music#alloc} and
 * the internal {@link materialize} method.  This ensures that asynchronous
 * loading is safe.
 *
 * @param key       The key to access the asset after loading
 * @param source    The pathname to the asset
 * @param callback  An optional callback for asynchronous loading
 * @param async     Whether the asset was loaded asynchronously
 *
 * @return true if the asset was successfully loaded
 */
bool JsonLoader::read(const std::string& key, const std::string& source, LoaderCallback callback, bool async) {
    if (_assets.find(key) != _assets.end() || _queue.find(key) != _queue.end()) {
        return false;
    }
    _queue.emplace(key);
    
    bool success = false;
    if (_loader == nullptr || !async) {
        std::shared_ptr<JsonReader> reader = JsonReader::allocWithAsset(source);
        std::shared_ptr<JsonValue> json = (reader == nullptr ? nullptr : reader->readJson());
        success = (json != nullptr);
        materialize(key,json,callback);
    } else {
        _loader->addTask([=](void) {
            std::shared_ptr<JsonReader> reader = JsonReader::allocWithAsset(source);
            std::shared_ptr<JsonValue> json = (reader == nullptr ? nullptr : reader->readJson());
            Application::get()->schedule([=](void) {
                this->materialize(key,json,callback);
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
 * This method will split the loading across the {@link Music#alloc} and
 * the internal {@link materialize} method.  This ensures that asynchronous
 * loading is safe.
 *
 * This version of read provides support for JSON directories. A music
 * directory entry has the following values
 *
 *      "file":         The path to the asset
 *      "volume":       This default sound volume (float)
 *
 * @param json      The directory entry for the asset
 * @param callback  An optional callback for asynchronous loading
 * @param async     Whether the asset was loaded asynchronously
 *
 * @return true if the asset was successfully loaded
 */
bool JsonLoader::read(const std::shared_ptr<JsonValue>& json, LoaderCallback callback, bool async) {
    std::string key = json->key();
    if (_assets.find(key) != _assets.end() || _queue.find(key) != _queue.end()) {
        return false;
    }
    _queue.emplace(key);
    std::string source = json->asString(UNKNOWN_SOURCE);
    
    bool success = false;
    if (_loader == nullptr || !async) {
        std::shared_ptr<JsonReader> reader = JsonReader::allocWithAsset(source);
        std::shared_ptr<JsonValue> json = (reader == nullptr ? nullptr : reader->readJson());
        success = (json != nullptr);
        materialize(key,json,callback);
    } else {
        _loader->addTask([=](void) {
            std::shared_ptr<JsonReader> reader = JsonReader::allocWithAsset(source);
            std::shared_ptr<JsonValue> json = (reader == nullptr ? nullptr : reader->readJson());
            Application::get()->schedule([=](void) {
                this->materialize(key,json,callback);
                return false;
            });
        });
    }
    
    return success;
}


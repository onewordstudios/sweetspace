//
//  CUGenericLoader.h
//  Cornell University Game Library (CUGL)
//
//  This module provides an abstract class for loading generic assets (such
//  as a model file or level layout) not explicitly included in the existing
//  asset classes. It is fairly experimental, so use at your own risk. If there
//  are certain assets that we overlooked that are the same across all projects,
//  we will consider adding them to the engine at a later date.
//
//  This module is meant to be used in conjunction with the Asset class which
//  provides support for loading the asset.  As such, this class really just
//  functions as an asset manager.
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
#ifndef __CU_GENERIC_LOADER_H__
#define __CU_GENERIC_LOADER_H__
#include <cugl/assets/CULoader.h>
#include <cugl/assets/CUAsset.h>

namespace cugl {
    
/**
 * This class is a specialized extension of Loader<T> for type {@link Asset}.
 *
 * This asset loader allows us to allocate generic assets that are subclasses
 * of {@link Asset}.  The rules for loading the asset are defined in the Asset
 * class.  This loader simply converts this interface into the standard one
 * so that it can be used by the {@link AssetManager}.
 *
 * As with all of our loaders, this loader is designed to be attached to an
 * asset manager. Use the method {@link getHook()} to get the appropriate
 * pointer for attaching the loader.
 */
template <class T>
class GenericLoader : public Loader<T> {
private:
    /** This macro disables the copy constructor (not allowed on assets) */
    CU_DISALLOW_COPY_AND_ASSIGN(GenericLoader);

protected:
    /** Access the asset set in the super class */
    using Loader<T>::_assets;
    /** Access the waiting queue in the super class */
    using Loader<T>::_queue;
    /** Access the thread pool in the super class */
    using BaseLoader::_loader;
    
    /**
     * Finishes loading the generic asset, finalizing any features in the main thread.
     *
     * This step effectively calls {@link Asset#materialize()}, and passes the
     * result to the optional callback function.
     *
     * @param key       The key to access the asset after loading
     * @param asset     The generic asset partially loaded
     * @param callback  An optional callback for asynchronous loading
     */
    bool materialize(const std::string& key, const std::shared_ptr<T>& asset, LoaderCallback callback) {
        bool success = false;
        if (asset != nullptr) {
            success = asset->materialize();
            if (success) {
                _assets[key] = asset;
            }
        }
        
        if (callback != nullptr) {
            callback(key,success);
        }
        _queue.erase(key);
        return success;
    }
    
    /**
     * Internal method to support asset loading.
     *
     * This method supports either synchronous or asynchronous loading, as
     * specified by the given parameter.  If the loading is asynchronous,
     * the user may specify an optional callback function.
     *
     * This method will split the loading across the {@link Asset#preload} and
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
    virtual bool read(const std::string& key, const std::string& source,
                      LoaderCallback callback, bool async) override {
        if (_assets.find(key) != _assets.end() || _queue.find(key) != _queue.end()) {
            return false;
        }
        _queue.emplace(key);
        
        bool success = false;
        if (_loader == nullptr || !async) {
            std::shared_ptr<T> asset = std::make_shared<T>();
            if (asset->preload(source)) {
                success = materialize(key,asset,callback);
            }
        } else {
            _loader->addTask([=](void) {
                std::shared_ptr<T> asset = std::make_shared<T>();
                if (!asset->preload(source)) {
                    asset = nullptr;
                }
                Application::get()->schedule([=](void){
                    this->materialize(key,asset,callback);
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
     * This method will split the loading across the {@link Asset#preload} and
     * the internal {@link materialize} method.  This ensures that asynchronous
     * loading is safe.
     *
     * This version of read provides support for JSON directories. The exact
     * format of the directory entry is up to you. However, the directory
     * entry must be loaded manually, as {@link AssetManager} does not yet 
     * support generic JSON directory entries.
     *
     * @param json      The directory entry for the asset
     * @param callback  An optional callback for asynchronous loading
     * @param async     Whether the asset was loaded asynchronously
     *
     * @return true if the asset was successfully loaded
     */
    virtual bool read(const std::shared_ptr<JsonValue>& json,
                      LoaderCallback callback, bool async) override {
        std::string key = json->key();
        if (_assets.find(key) != _assets.end() || _queue.find(key) != _queue.end()) {
            return false;
        }
        _queue.emplace(key);
        
        bool success = false;
        if (_loader == nullptr || !async) {
            std::shared_ptr<T> asset = std::make_shared<T>();
            if (asset->preload(json)) {
                success = materialize(key,asset,callback);
            }
        } else {
            _loader->addTask([=](void) {
                std::shared_ptr<T> asset = std::make_shared<T>();
                if (!asset->preload(json)) {
                    asset = nullptr;
                }
                Application::get()->schedule([=](void){
                    this->materialize(key,asset,callback);
                    return false;
                });
            });
        }
        
        return success;
    }
    
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new, uninitialized asset loader
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a loader on
     * the heap, use one of the static constructors instead.
     */
    GenericLoader() : Loader<T>() {}
    
    /**
     * Disposes all resources and assets of this loader
     *
     * Any assets loaded by this object will be immediately released by the
     * loader.  However, an asset may still be available if it is referenced
     * by another smart pointer.  
     *
     * Once the loader is disposed, any attempts to load a new asset will
     * fail.  You must reinitialize the loader to begin loading assets again.
     */
    void dispose() override {
        _assets.clear();
        _loader = nullptr;
    }
    
    /**
     * Returns a newly allocated asset loader.
     *
     * This method bootstraps the loader with any initial resources that it
     * needs to load assets.
     *
     * This loader will have no associated threads. That means any asynchronous
     * loading will fail until a thread is provided via {@link setThreadPool}.
     *
     * @return a newly allocated asset loader.
     */
    static std::shared_ptr<GenericLoader<T>> alloc() {
        std::shared_ptr<GenericLoader<T>> result = std::make_shared<GenericLoader<T>>();
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
    static std::shared_ptr<GenericLoader<T>> alloc(const std::shared_ptr<ThreadPool>& threads) {
        std::shared_ptr<GenericLoader> result = std::make_shared<GenericLoader>();
        return (result->init(threads) ? result : nullptr);
    }

};
    
}
#endif /* __CU_GENERIC_LOADER_H__ */

//
//  CUJsonLoader.h
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
//  Author: Graham Rutledge
//  Version: 5/20/19
//
#ifndef __CU_WIDGET_LOADER_H__
#define __CU_WIDGET_LOADER_H__
#include <cugl/assets/CULoader.h>
#include <cugl/assets/CUJsonLoader.h>
#include <cugl/assets/CUJsonValue.h>
#include <cugl/assets/CUWidgetValue.h>

namespace cugl {
    
/**
 * This class is a implementation of Loader<JsonValue>
 *
 * This asset loader allows us to allocate json assets.  It is essentially a
 * wrapper around {@link JsonReader} that allows it to be used with an
 * instance of {@link AssetManager}.
 *
 * As with all of our loaders, this loader is designed to be attached to an
 * asset manager. Use the method {@link getHook()} to get the appropriate
 * pointer for attaching the loader.
 */
class WidgetLoader : public Loader<WidgetValue> {
private:
    /** This macro disables the copy constructor (not allowed on assets) */
    CU_DISALLOW_COPY_AND_ASSIGN(WidgetLoader);
    
protected:
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
     * @param widget	The json asset fully loaded
     * @param callback  An optional callback for asynchronous loading
     */
    void materialize(const std::string& key, const std::shared_ptr<WidgetValue>& widget,
                     LoaderCallback callback);
    
    /**
     * Internal method to support asset loading.
     *
     * This method supports either synchronous or asynchronous loading, as
     * specified by the given parameter.  If the loading is asynchronous,
     * the user may specify an optional callback function.
     *
     * Json loading is always asynchronously safe, so this method does not
     * split the loading process.
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
     * Json loading is always asynchronously safe, so this method does not
     * split the loading process.
     *
     * This version of read provides support for JSON directories. A json
     * directory entry is just a key with a string value for the
     * path to the asset.
     *
     * @param json      The directory entry for the asset
     * @param callback  An optional callback for asynchronous loading
     * @param async     Whether the asset was loaded asynchronously
     *
     * @return true if the asset was successfully loaded
     */
    virtual bool read(const std::shared_ptr<JsonValue>& json,
                      LoaderCallback callback, bool async) override;
    
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new, uninitialized Json loader
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a loader on
     * the heap, use one of the static constructors instead.
     */
    WidgetLoader() {}
    
    /**
     * Disposes all resources and assets of this loader
     *
     * Any assets loaded by this object will be immediately released by the
     * loader.  However, a Json asset may still be available if referenced
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
     * Returns a newly allocated music loader.
     *
     * This method bootstraps the loader with any initial resources that it
     * needs to load assets.
     *
     * This loader will have no associated threads. That means any asynchronous
     * loading will fail until a thread is provided via {@link setThreadPool}.
     *
     * @return a newly allocated Json loader.
     */
    static std::shared_ptr<WidgetLoader> alloc() {
        std::shared_ptr<WidgetLoader> result = std::make_shared<WidgetLoader>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated music loader.
     *
     * This method bootstraps the loader with any initial resources that it
     * needs to load assets.
     *
     * This loader will have no associated threads. That means any asynchronous
     * loading will fail until a thread is provided via {@link setThreadPool}.
     *
     * @param threads   The thread pool for asynchronous loading
     *
     * @return a newly allocated Json loader.
     */
    static std::shared_ptr<WidgetLoader> alloc(const std::shared_ptr<ThreadPool>& threads) {
        std::shared_ptr<WidgetLoader> result = std::make_shared<WidgetLoader>();
        return (result->init(threads) ? result : nullptr);
    }
};

}

#endif /* __CU_WIDGET_LOADER_H__ */

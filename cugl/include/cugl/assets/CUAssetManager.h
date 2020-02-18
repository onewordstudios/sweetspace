//
//  CUAssetManager.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a class to support asset management. Assets should
//  always be managed by a central loader.  The loader ensures that the assets
//  are in memory at all times (even when not in use) and that there is a simple
//  way to refer to them using user-defined keys.
//
//  While most game engines implement asset managers as singletons, we have
//  elected not to do that.  This way you can use a different managers for
//  different player modes.
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
//  Version: 1/7/18
//
#ifndef __CU_ASSET_MANAGER_H__
#define __CU_ASSET_MANAGER_H__
#include <cugl/util/CUThreadPool.h>
#include <cugl/util/CUDebug.h>
#include <cugl/assets/CULoader.h>
#include <typeinfo>
#include <atomic>


namespace cugl {
    
/**
 * This class is loader/manager for handling a wide variety of assets.
 *
 * This asset manager is uses to manage a collection of loaders. Loaders must be
 * "attached" to the asset manager. The asset manager does not come with a 
 * collection of loaders pre-installed.  You will need to do this yourself in 
 * the start-up code for each scene. Once a loader is attached to this asset 
 * manager, the manager obtains ownership of the loader.  It will be responsible 
 * for garbage collection when it is done.
 *
 * Like loaders, an asset manager both loads an asset and allows it to be
 * referenced at any time via a key.  This allows us to easily decouple asset
 * loading from the rest of the application. To access an asset, you just
 * need a (weak or strong) reference to the asset loader.  However, we do not 
 * make asset managers a singleton, because different player modes may want 
 * different asset managers.
 *
 * Disposing an asset manager unloads all of the assets.  However, assets may
 * still be used after an asset manager is destroyed, provided that they still
 * have a smart pointer referencing them.
 *
 * IMPORTANT: This class is not even remotely thread-safe.  Do not call any of
 * these methods outside of the main CUGL thread.
 */
class AssetManager {
private:
    /** This macro disables the copy constructor (not allowed on assets) */
    CU_DISALLOW_COPY_AND_ASSIGN(AssetManager);
    
#pragma mark Internal Helpers
protected:
    /** The individual loaders for each type */
    std::unordered_map<size_t,std::shared_ptr<BaseLoader>> _handlers;
    /** The central thread for managing all of the loaders */
    std::shared_ptr<ThreadPool> _workers;

    /** State variable to manage reading JSON directories */
    bool _preload;
    
    /** Wait variable to create a load barrier for directories. */
    std::atomic<bool> _wait;

    /**
     * Synchronously reads an asset category from a JSON file
     *
     * JSON directories provide a robust way for us to load a collection of 
     * assets. Instead of having to define parameters like asset key, font 
     * size, or texture wrap in the code, we can specify them in a JSON file. 
     * This JSON file (called the asset directory) is read by the asset manager, 
     * and directs the various loaders to load in assets.
     *
     * Currently JSON loading supports five types of assets, with the following
     * names: "textures", "fonts", "music", "soundfx", and "jsons".  See the
     * method {@link BaseLoader#read} in each of the individual loaders for a
     * description of the suported JSON format. A loader must still be attached 
     * for the asset manager to read that type of asset.  If the asset directory 
     * contains an asset for which there is no attached asset manager, those 
     * specific assets will not be loaded.
     *
     * @param hash  The hash of the asset type
     * @param json  The child of asset directory with these assets
     *
     * @return true if all assets of this type were successfully loaded.
     */
    bool readCategory(size_t hash, const std::shared_ptr<JsonValue>& json);
    
    /**
     * Asynchronously reads an asset category from a JSON file
     *
     * JSON directories provide a robust way for us to load a collection of
     * assets. Instead of having to define parameters like asset key, font
     * size, or texture wrap in the code, we can specify them in a JSON file.
     * This JSON file (called the asset directory) is read by the asset manager,
     * and directs the various loaders to load in assets.
     *
     * Currently JSON loading supports five types of assets, with the following
     * names: "textures", "fonts", "music", "soundfx", and "jsons".  See the
     * method {@link BaseLoader#read} in each of the individual loaders for a
     * description of the suported JSON format. A loader must still be attached
     * for the asset manager to read that type of asset.  If the asset directory
     * contains an asset for which there is no attached asset manager, those
     * specific assets will not be loaded.
     *
     * As an asynchronous read, all asset loading will take place outside of
     * the main thread.  However, assets such as fonts and textures will need
     * the OpenGL context to complete, so part of their asset loading may take
     * place in the main thread via the {@link Application#schedule} interface.
     * You may either poll this interface to determine when the assets are 
     * loaded or use optional callbacks.
     *
     * The optional callback function will be called each time an individual
     * asset loads or fails to load.  However, if the entire category fails
     * to load, the callback function will be given the asset category name 
     * (e.g. "soundfx") as the asset key.
     *
     * @param hash      The hash of the asset type
     * @param json      The child of asset directory with these assets
     * @param callback  An optional callback after each asset is loaded
     */
    void readCategory(size_t hash, const std::shared_ptr<JsonValue>& json,
                      LoaderCallback callback);
    
    /**
     * Immediately removes an asset category previously loaded from the JSON file
     *
     * This method is used by the {@link unloadDirectory} method to remove
     * assets a category at a time.  Unloading is instantaneous and occurs
     * in the main thread.
     *
     * @param hash      The hash of the asset type
     * @param json      The child of asset directory with these assets
     *
     * @return true if all assets of this type were successfully loaded.
     */
    bool purgeCategory(size_t hash, const std::shared_ptr<JsonValue>& json);

    /**
     * Synchronizes the asset manager to wait until all assets have finished.
     *
     * This method is necessary for assets whose construction depends on
     * previously loaded assets (e.g. scene graphs).  In the current architecture,
     * this method is only correct if the asset manager loads assets in a
     * single thread.
     */
    void sync();
    
    /**
     * Blocks the asset manager until the next animation frame.
     *
     * Any assets queued after a block will not be added to thread pool
     * until at least one animation frame has passed.  This method is used
     * to implement the {@link sync()} method.
     */
    void block();

    /**
     * Resumes a previously blocked the asset manager.
     *
     * Any assets queued after a block will not be added to thread pool
     * until at least one animation frame has passed.  This method is used
     * to implement the {@link sync()} method.
     */
    void resume();
    
    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates a degenerate asset manager with no attached threads.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an asset 
     * manager on the heap, use one of the static constructors instead.
     */
    AssetManager() : _preload(false), _wait(false) {}
    
    /**
     * Deletes this asset manager, disposing of all resources.
     */
    ~AssetManager() { dispose(); }
    
    /**
     * Detaches all the attached loaders and deletes all auxiliary threads.
     *
     * Unlike the destructor, this does not destroy the asset manager.  However,
     * you will need to reinitialize the manager (to restart the auxiliary 
     * threads) and reattach all loaders to use the asset manager again.
     */
    void dispose();

    /**
     * Initializes a new asset manager with two auxiliary threads.
     *
     * The asset manager will have a thread pool of size 2, giving it two 
     * threads to load assets asynchronously.  These threads have no effect on 
     * synchronous loading and will sleep when no assets are being loaded.
     *
     * This initializer does not attach any loaders.  It simply creates an 
     * object that is ready to accept loader objects.
     *
     * @return true if the asset manager was initialized successfully
     */
    bool init();

    //
    // Initializes a new asset manager with the given number of auxiliary threads.
    //
    // The asset manager will have a thread pool of the given size, allowing it
    // load assets asynchronously.  These threads have no effect on synchronous
    // loading and will sleep when no assets are being loaded.  If threads is
    // 0, all assets must be loaded synchronously.
    //
    // This initializer does not attach any loaders.  It simply creates an
    // object that is ready to accept loader objects.
    //
    // @param threads   The number of threads for asynchronous loading
    //
    // @return true if the asset manager was initialized successfully
    //
    // TODO: This class must be refactored for this to be safe to use.
    // bool init(unsigned int threads);
    
#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a newly allocated asset manager with two auxiliary threads.
     *
     * The asset manager will have a thread pool of size 2, giving it two
     * threads to load assets asynchronously.  These threads have no effect on
     * synchronous loading and will sleep when no assets are being loaded.
     *
     * This constructor does not attach any loaders.  It simply creates an
     * object that is ready to accept loader objects.
     *
     * @return a newly allocated asset manager with two auxiliary threads.
     */
    static std::shared_ptr<AssetManager> alloc() {
        std::shared_ptr<AssetManager> result = std::make_shared<AssetManager>();
        return (result->init() ? result : nullptr);
    }
    
    //
    // Returns a newly allocated asset manager with the given number of auxiliary threads.
    //
    // The asset manager will have a thread pool of the given size, allowing it
    // load assets asynchronously.  These threads have no effect on synchronous
    // loading and will sleep when no assets are being loaded.  If threads is
    // 0, all assets must be loaded synchronously.
    //
    // This constructor does not attach any loaders.  It simply creates an
    // object that is ready to accept loader objects.
    //
    // @param threads   The number of threads for asynchronous loading
    //
    // @return a newly allocated asset manager with the given number of auxiliary threads.
    //
    // TODO: This class must be refactored for this to be safe to use.
    // static std::shared_ptr<AssetManager> alloc(unsigned int threads) {
    //     std::shared_ptr<AssetManager> result = std::make_shared<AssetManager>();
    //     return (result->init(threads) ? result : nullptr);
    // }

#pragma mark -
#pragma mark Loader Management
    /**
     * Attaches the given loader to the asset manager
     *
     * The type of the asset is specified by the template parameter T.  Once 
     * attached, all assets of type T will use this loader for this scene.  In 
     * addition, this asset manager will obtain ownership of the loader and be 
     * responsible for its garbage collection.
     *
     * @param  loader   The loader for asset T
     *
     * @return false if there is already a loader for this asset
     */
    template<typename T>
    bool attach(const std::shared_ptr<BaseLoader>& loader) {
        size_t hash = typeid(T).hash_code();
        auto it = _handlers.find(hash);
        if (it != _handlers.end()) {
            return false;
        }
        
        loader->setThreadPool(_workers);
        _handlers[hash] = loader;
        loader->setManager(this);
        return true;
    }
    
    /**
     * Returns true if there is a loader for the given asset Type
     *
     * The type of the asset is specified by the template parameter T.
     *
     * @return true if there is a loader for the given asset Type
     */
    template<typename T>
    bool isAttached() {
        size_t hash = typeid(T).hash_code();
        return _handlers.find(hash) != _handlers.end();
    }
    
    /**
     * Detaches a loader for an asset type
     *
     * The type of the asset is specified by the template parameter T.  The 
     * loader will be released and deleted if there are no further (smart 
     * pointer) references to it.
     *
     * @return true if there was a loader of that Type.
     */
    template<typename T>
    bool detach() {
        size_t hash = typeid(T).hash_code();
        auto it = _handlers.find(hash);
        if (it == _handlers.end()) {
            return false;
        }
        it->second->setThreadPool(nullptr);
        it->second = nullptr;
        _handlers.erase(hash);
        return true;
    }
    
    /**
     * Detaches all loaders from this asset manager
     *
     * The loaders will be released and deleted if there are no further
     * (smart pointer) references to them.
     */
    void detachAll() {
        for(auto it = _handlers.begin(); it != _handlers.end(); ++it) {
            it->second = nullptr;
        }
        _handlers.clear();
    }
    
    /**
     * Returns the loader for the given asset Type
     *
     * The type of the asset is specified by the template parameter T.
     *
     * @return the loader for the given asset Type
     */
    template<typename T>
    std::shared_ptr<Loader<T>> access() const {
        size_t hash = typeid(T).hash_code();
        auto it = _handlers.find(hash);
        if (it == _handlers.end()) {
            CUAssertLog(false, "No loader assigned for given type");
            return nullptr;
        }
        
        return std::dynamic_pointer_cast<Loader<T>>(it->second);
    }
    
#pragma mark -
#pragma mark Progress Monitoring
    /**
     * Returns the number of assets currently loaded.
     *
     * This method is a rough way to determine how many assets have been loaded
     * so far. This method counts each asset equally regardless of the memory
     * requirements of each asset.
     *
     * The value returned is the sum of the loadCount for all attached loaders.
     *
     * @return the number of assets currently loaded.
     */
    size_t loadCount() const;
    
    /**
     * Returns the number of assets waiting to load.
     *
     * This is a rough way to determine how many assets are still pending.
     * An asset is pending if it has been loaded asychronously, and the
     * loading process has not yet finished. This method counts each asset
     * equally regardless of the memory requirements of each asset.
     *
     * The value returned is the sum of the waitCount for all attached loaders.
     *
     * @return the number of assets waiting to load.
     */
    size_t waitCount() const;
    
    /**
     * Returns true if the loader has finished loading all assets.
     *
     * It is not safe to use asynchronously loaded assets until all loading is
     * complete.  This method allows us to determine when asset loading is 
     * complete via polling.
     *
     * @return true if the loader has finished loading
     */
    bool complete() const { return waitCount() == 0; }
    
    /**
     * Returns the loader progress as a percentage.
     *
     * This method returns a value between 0 and 1.  A value of 0 means no
     * assets have been loaded.  A value of 1 means that all assets have been 
     * loaded.
     *
     * Anything in-between indicates that there are assets which have been 
     * loaded asynchronously and have not completed loading. It is not safe to 
     * use asynchronously loaded assets until all loading is complete.
     *
     * @return the loader progress as a percentage.
     */
    float progress() const  {
        size_t size = loadCount()+waitCount();
        return (size == 0 ? 0.0f : ((float)loadCount())/size);
    }

    
#pragma mark -
#pragma mark Loading/Unloading
    /**
     * Returns the asset for the given key.
     *
     * The type of the asset is specified by the template parameter T. Because
     * the method is parameterized by the type, it is safe to reuse keys for
     * different types.  However, this is not recommended.
     *
     * @param  key  The key to identify the given asset
     *
     * @return the asset for the given key.
     */
    template<typename T>
    std::shared_ptr<T> get(const std::string& key) const {
        size_t hash = typeid(T).hash_code();
        auto it = _handlers.find(hash);
        if (it == _handlers.end()) {
            CUAssertLog(false, "No loader assigned for given type");
            return nullptr;
        }
        
        std::shared_ptr<Loader<T>> loader = std::dynamic_pointer_cast<Loader<T>>(it->second);
        return loader->get(key);
    }
    
    /**
     * Returns the asset for the given key.
     *
     * The type of the asset is specified by the template parameter T. Because
     * the method is parameterized by the type, it is safe to reuse keys for
     * different types.  However, this is not recommended.
     *
     * @param  key  The key to identify the given asset
     *
     * @return the asset for the given key.
     */
    template<typename T>
    std::shared_ptr<T> get(const char* key) const {
        return get<T>(std::string(key));
    }
    
    /**
     * Loads an asset and assigns it to the given key.
     *
     * The type of the asset is specified by the template parameter T. Because
     * the method is parameterized by the type, it is safe to reuse keys for
     * different types.  However, this is not recommended.
     *
     * This method essentially calls {@link BaseLoader#load} in the appropriate
     * loader. If there is no loader for the given type, the method will fail.
     *
     * The asset will be loaded synchronously. This means it will be available 
     * immediately. This method should be limited to those times in which an
     * asset is really necessary immediately, such as for a loading screen.
     *
     * @param  key      The key to access the asset after loading
     * @param  source   The pathname to the asset source
     *
     * @return true if the asset was successfully loaded
     */
    template<typename T>
    bool load(const std::string& key, const std::string& source) {
        size_t hash = typeid(T).hash_code();
        auto it = _handlers.find(hash);
        if (it != _handlers.end()) {
            std::shared_ptr<Loader<T>> loader = std::dynamic_pointer_cast<Loader<T>>(it->second);
            return loader->load(key,source);
        }
        
        CUAssertLog(false, "No loader assigned for given type");
        return nullptr;
    }

    /**
     * Loads an asset and assigns it to the given key.
     *
     * The type of the asset is specified by the template parameter T. Because
     * the method is parameterized by the type, it is safe to reuse keys for
     * different types.  However, this is not recommended.
     *
     * This method essentially calls {@link BaseLoader#load} in the appropriate
     * loader. If there is no loader for the given type, the method will fail.
     *
     * The asset will be loaded synchronously. This means it will be available
     * immediately. This method should be limited to those times in which an
     * asset is really necessary immediately, such as for a loading screen.
     *
     * @param  key      The key to access the asset after loading
     * @param  source   The pathname to the asset source
     *
     * @return true if the asset was successfully loaded
     */
    template<typename T>
    bool load(const char* key, const std::string& source) {
        return load<T>(std::string(key),source);
    }

    /**
     * Loads an asset and assigns it to the given key.
     *
     * The type of the asset is specified by the template parameter T. Because
     * the method is parameterized by the type, it is safe to reuse keys for
     * different types.  However, this is not recommended.
     *
     * This method essentially calls {@link BaseLoader#load} in the appropriate
     * loader. If there is no loader for the given type, the method will fail.
     *
     * The asset will be loaded synchronously. This means it will be available
     * immediately. This method should be limited to those times in which an
     * asset is really necessary immediately, such as for a loading screen.
     *
     * @param  key      The key to access the asset after loading
     * @param  source   The pathname to the asset source
     *
     * @return true if the asset was successfully loaded
     */
    template<typename T>
    bool load(const std::string& key, const char* source) {
        return load<T>(key,std::string(source));
    }

    /**
     * Loads an asset and assigns it to the given key.
     *
     * The type of the asset is specified by the template parameter T. Because
     * the method is parameterized by the type, it is safe to reuse keys for
     * different types.  However, this is not recommended.
     *
     * This method essentially calls {@link BaseLoader#load} in the appropriate
     * loader. If there is no loader for the given type, the method will fail.
     *
     * The asset will be loaded synchronously. This means it will be available
     * immediately. This method should be limited to those times in which an
     * asset is really necessary immediately, such as for a loading screen.
     *
     * @param  key      The key to access the asset after loading
     * @param  source   The pathname to the asset source
     *
     * @return true if the asset was successfully loaded
     */
    template<typename T>
    bool load(const char* key, const char* source) {
        return load<T>(std::string(key),std::string(source));
    }
    
    /**
     * Adds a new asset to the loading queue.
     *
     * The type of the asset is specified by the template parameter T. Because
     * the method is parameterized by the type, it is safe to reuse keys for
     * different types.  However, this is not recommended.
     *
     * This method essentially calls {@link BaseLoader#loadAsync} in the 
     * appropriate loader. If there is no loader for the given type, the
     * method will fail.
     *
     * The asset will be loaded asynchronously.  When it is finished loading,
     * it will be added to this loader, and accessible under the given key. This
     * method will mark the loading process as not complete, even if it was
     * completed previously.  It is not safe to access the loaded asset until
     * it is complete again.
     *
     * The optional callback function will be called with the asset status when
     * the loading either completes or fails.
     *
     * @param key       The key to access the asset after loading
     * @param source    The pathname to the asset source
     * @param callback  An optional callback for when the asset is loaded.
     */
    template<typename T>
    void loadAsync(const std::string& key, const std::string& source, LoaderCallback callback) {
        size_t hash = typeid(T).hash_code();
        auto it = _handlers.find(hash);
        if (it != _handlers.end()) {
            it->second->loadAsync(key,source,callback);
            return;
        }
        
        CUAssertLog(false, "No loader assigned for given type");
    }

    /**
     * Adds a new asset to the loading queue.
     *
     * The type of the asset is specified by the template parameter T. Because
     * the method is parameterized by the type, it is safe to reuse keys for
     * different types.  However, this is not recommended.
     *
     * This method essentially calls {@link BaseLoader#loadAsync} in the
     * appropriate loader. If there is no loader for the given type, the
     * method will fail.
     *
     * The asset will be loaded asynchronously.  When it is finished loading,
     * it will be added to this loader, and accessible under the given key. This
     * method will mark the loading process as not complete, even if it was
     * completed previously.  It is not safe to access the loaded asset until
     * it is complete again.
     *
     * The optional callback function will be called with the asset status when
     * the loading either completes or fails.
     *
     * @param key       The key to access the asset after loading
     * @param source    The pathname to the asset source
     * @param callback  An optional callback for when the asset is loaded.
     */
    template<typename T>
    void loadAsync(const char* key, const std::string& source, LoaderCallback callback) {
        loadAsync<T>(std::string(key),source,callback);
    }

    /**
     * Adds a new asset to the loading queue.
     *
     * The type of the asset is specified by the template parameter T. Because
     * the method is parameterized by the type, it is safe to reuse keys for
     * different types.  However, this is not recommended.
     *
     * This method essentially calls {@link BaseLoader#loadAsync} in the
     * appropriate loader. If there is no loader for the given type, the
     * method will fail.
     *
     * The asset will be loaded asynchronously.  When it is finished loading,
     * it will be added to this loader, and accessible under the given key. This
     * method will mark the loading process as not complete, even if it was
     * completed previously.  It is not safe to access the loaded asset until
     * it is complete again.
     *
     * The optional callback function will be called with the asset status when
     * the loading either completes or fails.
     *
     * @param key       The key to access the asset after loading
     * @param source    The pathname to the asset source
     * @param callback  An optional callback for when the asset is loaded.
     */
    template<typename T>
    void loadAsync(const std::string& key, const char* source, LoaderCallback callback) {
        loadAsync<T>(key,std::string(source),callback);
    }

    /**
     * Adds a new asset to the loading queue.
     *
     * The type of the asset is specified by the template parameter T. Because
     * the method is parameterized by the type, it is safe to reuse keys for
     * different types.  However, this is not recommended.
     *
     * This method essentially calls {@link BaseLoader#loadAsync} in the
     * appropriate loader. If there is no loader for the given type, the
     * method will fail.
     *
     * The asset will be loaded asynchronously.  When it is finished loading,
     * it will be added to this loader, and accessible under the given key. This
     * method will mark the loading process as not complete, even if it was
     * completed previously.  It is not safe to access the loaded asset until
     * it is complete again.
     *
     * The optional callback function will be called with the asset status when
     * the loading either completes or fails.
     *
     * @param key       The key to access the asset after loading
     * @param source    The pathname to the asset source
     * @param callback  An optional callback for when the asset is loaded.
     */
    template<typename T>
    void loadAsync(const char* key, const char* source, LoaderCallback callback) {
        loadAsync<T>(std::string(key),std::string(source),callback);
    }
    
    /**
     * Unloads the asset for the given key.
     *
     * The type of the asset is specified by the template parameter T. Because
     * the method is parameterized by the type, it is safe to reuse keys for
     * different types.  However, this is not recommended.
     *
     * This method simply unloads the asset from this asset manager.  If there
     * are active smart pointers still referencing the asset, it still may
     * remain in memory. However, the rest of the program can no longer access
     * the asset by key.
     *
     * @param  key  the key referencing the asset
     */
    template<typename T>
    void unload(const std::string& key) {
        size_t hash = typeid(T).hash_code();
        auto it = _handlers.find(hash);
        if (it != _handlers.end()) {
            it->second->unload(key);
            return;
        }
        
        CUAssertLog(false, "No loader assigned for given type");
    }
    
    /**
     * Unloads the asset for the given key.
     *
     * The type of the asset is specified by the template parameter T. Because
     * the method is parameterized by the type, it is safe to reuse keys for
     * different types.  However, this is not recommended.
     *
     * This method simply unloads the asset from this asset manager.  If there
     * are active smart pointers still referencing the asset, it still may
     * remain in memory. However, the rest of the program can no longer access
     * the asset by key.
     *
     * @param  key  the key referencing the asset
     */
    template<typename T>
    void unload(const char* key) {
        unload<T>(std::string(key));
    }
    
    /**
     * Unloads all assets present in this loader.
     *
     * This method unloads all assets associated with this loader. If there
     * are active smart pointers still referencing the assets, they still may
     * remain in memory. However, the rest of the program can no longer access
     * these assets.
     */
    void unloadAll() {
        for(auto it = _handlers.begin(); it != _handlers.end(); ++it) {
            it->second->unloadAll();
        }
    }
    
#pragma mark -
#pragma mark Directory Support
    /**
     * Synchronously loads all assets in the given directory.
     *
     * JSON directories provide a robust way for us to load a collection of
     * assets. Instead of having to define parameters like asset key, font
     * size, or texture wrap in the code, we can specify them in a JSON file.
     * This JSON file (called the asset directory) is read by the asset manager,
     * and directs the various loaders to load in assets.
     *
     * Currently JSON loading supports five types of assets, with the following
     * names: "textures", "fonts", "music", "soundfx", and "jsons".  See the
     * method {@link BaseLoader#read} in each of the individual loaders for a
     * description of the suported JSON format. A loader must still be attached
     * for the asset manager to read that type of asset.  If the asset directory
     * contains an asset for which there is no attached asset manager, those
     * specific assets will not be loaded.
     *
     * This method will try to load as many assets from the directory as it 
     * can.  If any asset fails to load, it will return false.  However, some
     * assets may still be loaded and safe to access.
     *
     * @param json  The JSON asset directory
     *
     * @return true if all assets specified in the directory were successfully loaded.
     */
    bool loadDirectory(const std::shared_ptr<JsonValue>& json);

    /**
     * Synchronously loads all assets in the given directory.
     *
     * JSON directories provide a robust way for us to load a collection of
     * assets. Instead of having to define parameters like asset key, font
     * size, or texture wrap in the code, we can specify them in a JSON file.
     * This JSON file (called the asset directory) is read by the asset manager,
     * and directs the various loaders to load in assets.
     *
     * Currently JSON loading supports five types of assets, with the following
     * names: "textures", "fonts", "music", "soundfx", and "jsons".  See the
     * method {@link BaseLoader#read} in each of the individual loaders for a
     * description of the suported JSON format. A loader must still be attached
     * for the asset manager to read that type of asset.  If the asset directory
     * contains an asset for which there is no attached asset manager, those
     * specific assets will not be loaded.
     *
     * This method will try to load as many assets from the directory as it
     * can.  If any asset fails to load, it will return false.  However, some
     * assets may still be loaded and safe to access.
     *
     * @param directory The path to the JSON asset directory
     *
     * @return true if all assets specified in the directory were successfully loaded.
     */
    bool loadDirectory(const std::string& directory);

    /**
     * Synchronously loads all assets in the given directory.
     *
     * JSON directories provide a robust way for us to load a collection of
     * assets. Instead of having to define parameters like asset key, font
     * size, or texture wrap in the code, we can specify them in a JSON file.
     * This JSON file (called the asset directory) is read by the asset manager,
     * and directs the various loaders to load in assets.
     *
     * Currently JSON loading supports five types of assets, with the following
     * names: "textures", "fonts", "music", "soundfx", and "jsons".  See the
     * method {@link BaseLoader#read} in each of the individual loaders for a
     * description of the suported JSON format. A loader must still be attached
     * for the asset manager to read that type of asset.  If the asset directory
     * contains an asset for which there is no attached asset manager, those
     * specific assets will not be loaded.
     *
     * This method will try to load as many assets from the directory as it
     * can.  If any asset fails to load, it will return false.  However, some
     * assets may still be loaded and safe to access.
     *
     * @param directory The path to the JSON asset directory
     *
     * @return true if all assets specified in the directory were successfully loaded.
     */
    bool loadDirectory(const char* directory) {
        return loadDirectory(std::string(directory));
    }
   
    /**
     * Asynchronously loads all assets in the given directory.
     *
     * JSON directories provide a robust way for us to load a collection of
     * assets. Instead of having to define parameters like asset key, font
     * size, or texture wrap in the code, we can specify them in a JSON file.
     * This JSON file (called the asset directory) is read by the asset manager,
     * and directs the various loaders to load in assets.
     *
     * Currently JSON loading supports five types of assets, with the following
     * names: "textures", "fonts", "music", "soundfx", and "jsons".  See the
     * method {@link BaseLoader#read} in each of the individual loaders for a
     * description of the suported JSON format. A loader must still be attached
     * for the asset manager to read that type of asset.  If the asset directory
     * contains an asset for which there is no attached asset manager, those
     * specific assets will not be loaded.
     *
     * As an asynchronous load, all asset loading will take place outside of
     * the main thread.  However, assets such as fonts and textures will need
     * the OpenGL context to complete, so part of their asset loading may take
     * place in the main thread via the {@link Application#schedule} interface.
     * You may either poll this interface to determine when the assets are
     * loaded or use optional callbacks.
     *
     * The optional callback function will be called each time an individual
     * asset loads or fails to load.  However, if the entire category fails
     * to load, the callback function will be given the asset category name
     * (e.g. "soundfx") as the asset key.
     *
     * @param json      The JSON asset directory
     * @param callback  An optional callback after each asset is loaded
     */
    void loadDirectoryAsync(const std::shared_ptr<JsonValue>& json, LoaderCallback callback);

    /**
     * Asynchronously loads all assets in the given directory.
     *
     * JSON directories provide a robust way for us to load a collection of
     * assets. Instead of having to define parameters like asset key, font
     * size, or texture wrap in the code, we can specify them in a JSON file.
     * This JSON file (called the asset directory) is read by the asset manager,
     * and directs the various loaders to load in assets.
     *
     * Currently JSON loading supports five types of assets, with the following
     * names: "textures", "fonts", "music", "soundfx", and "jsons".  See the
     * method {@link BaseLoader#read} in each of the individual loaders for a
     * description of the suported JSON format. A loader must still be attached
     * for the asset manager to read that type of asset.  If the asset directory
     * contains an asset for which there is no attached asset manager, those
     * specific assets will not be loaded.
     *
     * As an asynchronous load, all asset loading will take place outside of
     * the main thread.  However, assets such as fonts and textures will need
     * the OpenGL context to complete, so part of their asset loading may take
     * place in the main thread via the {@link Application#schedule} interface.
     * You may either poll this interface to determine when the assets are
     * loaded or use optional callbacks.
     *
     * The optional callback function will be called each time an individual
     * asset loads or fails to load.  However, if the entire category fails
     * to load, the callback function will be given the asset category name
     * (e.g. "soundfx") as the asset key.
     *
     * @param directory The path to the JSON asset directory
     * @param callback  An optional callback after each asset is loaded
     */
    void loadDirectoryAsync(const std::string& directory, LoaderCallback callback);
    
    /**
     * Asynchronously loads all assets in the given directory.
     *
     * JSON directories provide a robust way for us to load a collection of
     * assets. Instead of having to define parameters like asset key, font
     * size, or texture wrap in the code, we can specify them in a JSON file.
     * This JSON file (called the asset directory) is read by the asset manager,
     * and directs the various loaders to load in assets.
     *
     * Currently JSON loading supports five types of assets, with the following
     * names: "textures", "fonts", "music", "soundfx", and "jsons".  See the
     * method {@link BaseLoader#read} in each of the individual loaders for a
     * description of the suported JSON format. A loader must still be attached
     * for the asset manager to read that type of asset.  If the asset directory
     * contains an asset for which there is no attached asset manager, those
     * specific assets will not be loaded.
     *
     * As an asynchronous load, all asset loading will take place outside of
     * the main thread.  However, assets such as fonts and textures will need
     * the OpenGL context to complete, so part of their asset loading may take
     * place in the main thread via the {@link Application#schedule} interface.
     * You may either poll this interface to determine when the assets are
     * loaded or use optional callbacks.
     *
     * The optional callback function will be called each time an individual
     * asset loads or fails to load.  However, if the entire category fails
     * to load, the callback function will be given the asset category name
     * (e.g. "soundfx") as the asset key.
     *
     * @param directory The path to the JSON asset directory
     * @param callback  An optional callback after each asset is loaded
     */
    void loadDirectoryAsync(const char* directory, LoaderCallback callback) {
        loadDirectoryAsync(std::string(directory),callback);
    }
    
    /**
     * Unloads all assets for the given directory.
     *
     * This method unloads only those assets associated with the given directory.
     * If there are active smart pointers still referencing the assets, they
     * still may remain in memory. However, the rest of the program can no
     * longer access these assets.
     *
     * @param json      The JSON asset directory
     */
    bool unloadDirectory(const std::shared_ptr<JsonValue>& json);

    /**
     * Unloads all assets for the given directory.
     *
     * This method unloads only those assets associated with the given directory.
     * If there are active smart pointers still referencing the assets, they
     * still may remain in memory. However, the rest of the program can no
     * longer access these assets.
     *
     * @param directory The path to the JSON asset directory
     */
    bool unloadDirectory(const std::string& directory);

    /**
     * Unloads all assets for the given directory.
     *
     * This method unloads only those assets associated with the given directory.
     * If there are active smart pointers still referencing the assets, they
     * still may remain in memory. However, the rest of the program can no
     * longer access these assets.
     *
     * @param directory The path to the JSON asset directory
     */
    bool unloadDirectory(const char* directory) {
        return unloadDirectory(std::string(directory));
    }

};

}

#endif /* __CU_ASSET_MANAGER_H__ */

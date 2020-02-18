//
//  CULoader.h
//  Cornell University Game Library (CUGL)
//
//  This module provides the base templates for loader classes.  Our goal with
//  this module is to create a modular loader system that is as close as possible
//  to Nathan Sweet's flexible and modular AssetManager for libGDX.  To do this
//  in C++ requires three layers:
//
//  The first layer is a polymorphic base that is used by the AssetManager for
//  adding and removing loaders. It is the C++ equivalent of a Java interface.
//
//  The second layer is a template that provides type correctness when accessing
//  components in the AssetManager.  It has generic functionality that is
//  necessary for all loaders, but is missing the information needed to load
//  the specific asset.
//
//  The top layer is a specific class for each asset that implements the specific
//  loading functionality.
//
//  This module implements the first two layers.  As they are both a template
//  and a pure polymorphic class, only a header file is necessary.  There is no
//  associated CPP file with this header.
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
#ifndef __CU_LOADER_H__
#define __CU_LOADER_H__
#include <string>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <cugl/assets/CUJsonValue.h>
#include <cugl/util/CUThreadPool.h>

namespace cugl {

/** Forward reference to the asset manager */
class AssetManager;

/**
 * @typedef LoaderCallback
 *
 * This type represents a callback for asynchronous asset loading
 *
 * This callback is associated with an asset at the time of the asychronous
 * load request.  When the asset either loads, or fails to load, this callback
 * is called with the asset status.
 *
 * In the case of JSON asset directories, a single callback may be associated
 * with several assets.  In that case, the callback funtion is called each
 * time that an asset loads or fails to load.  If an entire node in the asset
 * directory fails to load, the callback function will be notified using the
 * key for that JSON node.
 *
 * The function type is equivalent to
 *
 *      std::function<void(const std::string& key, bool success)>
 *
 * @param key       The key to associate with the asset (or asset category)
 * @param success   Whether the asset was successfully loaded
 */
typedef std::function<void(const std::string& key, bool success)> LoaderCallback;

#pragma mark -
#pragma mark Polymorphic Base
/**
 * This class provides a polymorphic base to the loader system.
 *
 * This is effectively a Java-style interface.  It identifies the methods that
 * all loaders must have, and provides a type for the {@link AssetManager} to 
 * use in its underlying storage container.
 *
 * IMPORTANT: This class is not even remotely thread-safe.  Do not call any of
 * these methods outside of the main CUGL thread.
 */
class BaseLoader : public std::enable_shared_from_this<BaseLoader> {
protected:
    /** 
     * The associated thread for asynchronous loading
     *
     * If this value is nullptr, only synchronous loading is supported
     */
    std::shared_ptr<ThreadPool> _loader;
    
    /**
     * The parent asset manager for this loader (may be null)
     *
     * This is a weak reference to avoid cycles.
     */
    AssetManager* _manager;
    
    /**
     * Internal method to support asset loading.
     *
     * This method supports either synchronous or asynchronous loading, as 
     * specified by the given parameter.  If the loading is asynchronous,
     * the user may specify an optional callback function.
     *
     * This method is abstract and should be overridden in child classes to
     * support the appropriate asset type.
     *
     * @param key       The key to access the asset after loading
     * @param source    The pathname to the asset
     * @param callback  An optional callback for asynchronous loading
     * @param async     Whether the asset was loaded asynchronously
     *
     * @return true if the asset was successfully loaded
     */
    virtual bool read(const std::string& key, const std::string& source,
                      LoaderCallback callback, bool async) {
        return false;
    }

    /**
     * Internal method to support asset loading.
     *
     * This method supports either synchronous or asynchronous loading, as
     * specified by the given parameter.  If the loading is asynchronous,
     * the user may specify an optional callback function.
     *
     * This method is abstract and should be overridden in child classes to
     * support the appropriate asset type.
     *
     * This version of read provides support for JSON directories. The exact
     * exact format of the directory entry is up to you. However, unless the
     * asset is one of the four basic types, it will not be supported by
     * {@link AssetManager}.  You will need to load the directory manually
     * in that case.
     *
     * @param json      The directory entry for the asset
     * @param callback  An optional callback for asynchronous loading
     * @param async     Whether the asset was loaded asynchronously
     *
     * @return true if the asset was successfully loaded
     */
    virtual bool read(const std::shared_ptr<JsonValue>& json,
                      LoaderCallback callback, bool async) {
        return false;
    }
    
    /**
     * Unloads the asset for the given key
     *
     * An asset may still be available if it is referenced by a smart pointer.
     * See the description of the specific implementation for how assets
     * are released.
     *
     * This method is abstract and should be overridden in child classes.  You
     * will notice that this method is essentially identical to unload.  We
     * separated the methods because overloading and virtual methods do not
     * place nice.
     * @return true if the asset was successfully unloaded
     */
    virtual bool purge(const std::string& key) {
        return false;
    }
    
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
    virtual bool purge(const std::shared_ptr<JsonValue>& json) {
        return purge(json->key());
    }
    
    /**
     * Returns true if the key maps to a loaded asset.
     *
     * This method is useful in case the asset fails to load. You will notice
     * that this method is essentially identical to contains.  We separated the 
     * methods because overloading and virtual methods do not place nice.
     *
     * @param key   The key associated with the asset
     *
     * @return true if the key maps to a loaded asset.
     */
    virtual bool verify(const std::string& key) const { return false; }
   
    
public:
#pragma mark Constructors
    /**
     * Creates a degenerate asset loader with no resources
     *
     * NEVER CALL THIS CONSTRUCTOR. As this is an abstract class, you should 
     * call one of the static constructors of the appropriate child class.
     */
    BaseLoader()    {}
    
    /**
     * Deletes this asset loader, disposing of all resources.
     */
    ~BaseLoader()   { dispose(); }
    
    /**
     * Disposes all resources and assets of this loader
     *
     * Any assets loaded by this object will be immediately released by the
     * loader.  However, an asset may still be available if it is referenced
     * by another smart pointer.  See the description of the specific 
     * implementation for how assets are released.
     *
     * Once the loader is disposed, any attempts to load a new asset will
     * fail.  You must reinitialize the loader to begin loading assets again.
     *
     * This method is abstract and should be overridden in the specific
     * implementation for each asset.
     */
    virtual void dispose() { _manager = nullptr; }
    
    /**
     * Initializes a new asset loader.
     *
     * This method bootstraps the loader with any initial resources that it
     * needs to load assets. Attempts to load an asset before this method is
     * called will fail. 
     *
     * This loader will have no associated threads. That means any asynchronous
     * loading will fail until a thread is provided via {@link setThreadPool}.
     *
     * This method is abstract and should be overridden in the specific
     * implementation for each asset.
     *
     * @return true if the asset loader was initialized successfully
     */
    virtual bool init() {
        return init(nullptr);
    }
    
    /**
     * Initializes a new asset loader.
     *
     * This method bootstraps the loader with any initial resources that it
     * needs to load assets. Attempts to load an asset before this method is
     * called will fail.
     *
     * This method is abstract and should be overridden in the specific
     * implementation for each asset.
     *
     * @param threads   The thread pool for asynchronous loading support
     *
     * @return true if the asset loader was initialized successfully
     */
    virtual bool init(const std::shared_ptr<ThreadPool>& threads) {
        _loader=threads;
        return true;
    }
    
    
#pragma mark AssetManager Support
    /**
     * Returns a pointer for attaching this loader to an {@link AssetManager}.
     *
     * Smart pointers are great, and all asset loaders should be referenced
     * by one.  However, polymorphism and smart pointers really do not mix
     * and type casting can be quite tricky.  This method provides a simple
     * interface for handling this type case.
     *
     * @return a pointer for attaching this loader to an {@link AssetManager}.
     */
    std::shared_ptr<BaseLoader> getHook() {
        return shared_from_this();
    }
    
    /**
     * Returns the thread pool attached to this loader
     *
     * The thread pool is used for asynchronous loading. Multiple asset loaders
     * can share the same thread pool.  This keeps the system from being 
     * overloaded by a large number of threads.
     *
     * @return the thread pool attached to this loader
     */
    std::shared_ptr<ThreadPool> getThreadPool() const { return _loader; }
    
    /**
     * Sets the thread pool attached to this loader
     *
     * If there was a previously attached thread pool, it will be deleted and
     * the threads will be shutdown.  Assets not yet loaded by that thread
     * pool will fail to load.  Hence it is unsafe to call this method if the
     * loader is actively loading assets.
     *
     * The thread pool is used for asynchronous loading. Multiple asset loaders
     * can share the same thread pool.  This keeps the system from being
     * overloaded by a large number of threads.
     *
     * @param threads   The thread pool attached to this loader
     */
    void setThreadPool(const std::shared_ptr<ThreadPool>& threads) {
        _loader = threads;
    }
    
    /**
     * Sets the asset manager for this loader.
     *
     * The asset manager allows this loader to access previously loaded
     * assets.  This allows materialization of complex, dependent assets.
     *
     * @param manager   The asset manager
     */
    void setManager(AssetManager* manager) {
        _manager = manager;
    }
    
    /**
     * Returns the asset manager for this loader.
     *
     * The asset manager allows this loader to access previously loaded
     * assets.  This allows materialization of complex, dependent assets.
     *
     * @return the asset manager for this loader
     */
    const AssetManager* getManager() const {
        return _manager;
    }
    

#pragma mark Loading/Unloading
    /**
     * Synchronously loads the given asset with the specified key.
     *
     * The asset will be loaded synchronously, which means the main CUGL thread
     * will block until loading is complete. When it is finished loading, the
     * asset will be added to the contents loader, and accessible under the
     * given key.
     *
     * This method is abstract and should be overridden in child classes to
     * support the appropriate asset type.
     *
     * @param key       The key to access the asset after loading
     * @param source    The pathname to the asset
     *
     * @return true if the asset was successfully loaded
     */
    bool load(const std::string& key, const std::string& source) {
        return read(key,source,nullptr,false);
    }

    /**
     * Synchronously loads the given asset with the specified key.
     *
     * The asset will be loaded synchronously, which means the main CUGL thread
     * will block until loading is complete. When it is finished loading, the
     * asset will be added to the contents loader, and accessible under the
     * given key.
     *
     * This method is abstract and should be overridden in child classes to
     * support the appropriate asset type.
     *
     * @param key       The key to access the asset after loading
     * @param source    The pathname to the asset
     *
     * @return true if the asset was successfully loaded
     */
    bool load(const char* key, const std::string& source) {
        return read(std::string(key),source,nullptr,false);
    }

    /**
     * Synchronously loads the given asset with the specified key.
     *
     * The asset will be loaded synchronously, which means the main CUGL thread
     * will block until loading is complete. When it is finished loading, the
     * asset will be added to the contents loader, and accessible under the
     * given key.
     *
     * This method is abstract and should be overridden in child classes to
     * support the appropriate asset type.
     *
     * @param key       The key to access the asset after loading
     * @param source    The pathname to the asset
     *
     * @return true if the asset was successfully loaded
     */
    bool load(const std::string& key, const char* source) {
        return read(key,std::string(source),nullptr,false);
    }
    
    /**
     * Synchronously loads the given asset with the specified key.
     *
     * The asset will be loaded synchronously, which means the main CUGL thread
     * will block until loading is complete. When it is finished loading, the
     * asset will be added to the contents loader, and accessible under the
     * given key.
     *
     * This method is abstract and should be overridden in child classes to
     * support the appropriate asset type.
     *
     * @param key       The key to access the asset after loading
     * @param source    The pathname to the asset
     *
     * @return true if the asset was successfully loaded
     */
    bool load(const char* key, const char* source) {
        return read(std::string(key),std::string(source),nullptr,false);
    }

    /**
     * Synchronously loads the given asset with the specified key.
     *
     * The asset will be loaded synchronously, which means the main CUGL thread
     * will block until loading is complete. When it is finished loading, the
     * asset will be added to the contents loader, and accessible under the
     * given key.
     *
     * This version of load provides support for JSON directories. The exact
     * format of the directory entry is up to you. However, unless the
     * asset is one of the five basic types, it will not be supported by
     * {@link AssetManager}.  You will need to load the directory manually
     * in that case.
     *
     * This method is abstract and should be overridden in child classes to
     * support the appropriate asset type.
     *
     * @param json      The directory entry for the asset
     *
     * @return true if the asset was successfully loaded
     */
    bool load(const std::shared_ptr<JsonValue>& json) {
        return read(json,nullptr,false);
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
     * This method is abstract and should be overridden in child classes to
     * support the appropriate asset type.
     *
     * @param key       The key to access the asset after loading
     * @param source    The pathname to the asset
     * @param callback  An optional callback for asynchronous loading
     */
    void loadAsync(const std::string& key, const std::string& source, LoaderCallback callback) {
        read(key, source, callback,true);
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
     * This method is abstract and should be overridden in child classes to
     * support the appropriate asset type.
     *
     * @param key       The key to access the asset after loading
     * @param source    The pathname to the asset
     * @param callback  An optional callback for asynchronous loading
     */
    void loadAsync(const char* key, const std::string& source, LoaderCallback callback) {
        read(std::string(key), source, callback,true);
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
     * This method is abstract and should be overridden in child classes to
     * support the appropriate asset type.
     *
     * @param key       The key to access the asset after loading
     * @param source    The pathname to the asset
     * @param callback  An optional callback for asynchronous loading
     */
    void loadAsync(const std::string& key, const char* source, LoaderCallback callback) {
        read(key, std::string(source), callback,true);
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
     * This method is abstract and should be overridden in child classes to
     * support the appropriate asset type.
     *
     * @param key       The key to access the asset after loading
     * @param source    The pathname to the asset
     * @param callback  An optional callback for asynchronous loading
     */
    void loadAsync(const char* key, const char* source, LoaderCallback callback) {
        read(std::string(key), std::string(source), callback,true);
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
     * This version of loadAsync provides support for JSON directories. The
     * exact format of the directory entry is up to you. However, unless the
     * asset is one of the five basic types, it will not be supported by
     * {@link AssetManager}.  You will need to load the directory manually
     * in that case.
     *
     * The optional callback function will be called with the asset status when
     * it either finishes loading or fails to load.
     *
     * This method is abstract and should be overridden in child classes to
     * support the appropriate asset type.
     *
     * @param json  	The JSON entry (and key) associated with the asset
     * @param callback  An optional callback for asynchronous loading
     */
    void loadAsync(const std::shared_ptr<JsonValue>& json, LoaderCallback callback) {
        read(json, callback,true);
    }

    /**
     * Unloads the asset for the given key
     *
     * An asset may still be available if it is referenced by a smart pointer.
     * See the description of the specific implementation for how assets
     * are released.
     *
     * This method is abstract and should be overridden in child classes. 
     *
     * @param  key  the key associated with the asset
     *
     * @return true if the asset was successfully unloaded
     */
    bool unload(const std::string& key) {
        return purge(key);
    }

    /**
     * Unloads the asset for the given key
     *
     * An asset may still be available if it is referenced by a smart pointer.
     * See the description of the specific implementation for how assets
     * are released.
     *
     * This method is abstract and should be overridden in the child classes.
     *
     * @param  key  the key associated with the asset
     *
     * @return true if the asset was successfully unloaded
     */
    bool unload(const char* key) {
        return purge(key);
    }
    
	/**
     * Unloads the asset for the given JSON entry
     *
     * An asset may still be available if it is referenced by a smart pointer.
     * See the description of the specific implementation for how assets
     * are released.
     *
     * This method is abstract and should be overridden in the child classes.
     *
     * @param  json  the JSON entry (and key) associated with the asset
     *
     * @return true if the asset was successfully unloaded
     */
    bool unload(const std::shared_ptr<JsonValue>& json) {
        return purge(json);
    }
    
    /**
     * Unloads all assets present in this loader.
     *
     * An asset may still be available if it is referenced by a smart pointer.
     * See the description of the specific implementation for how assets
     * are released.
     *
     * This method is abstract and should be overridden in the child classes.
     */
    virtual void unloadAll() {}
    

#pragma mark Progress Monitoring
    /**
     * Returns true if the key maps to a loaded asset.
     *
     * This method is useful in case the asset fails to load.
     *
     * @param  key  the key associated with the asset
     *
     * @return True if the key maps to a loaded asset.
     */
    bool contains(const std::string& key) const {
        return verify(key);
    }

    /**
     * Returns true if the key maps to a loaded asset.
     *
     * This method is useful in case the asset fails to load.
     *
     * @param  key  the key associated with the asset
     *
     * @return True if the key maps to a loaded asset.
     */
    bool contains(const char* key) const {
        return verify(std::string(key));

    }

    /**
     * Returns the number of assets currently loaded.
     *
     * This is a rough way to determine how many assets have been loaded
     * so far. This method counts each asset equally regardless of the memory
     * requirements of each asset.
     *
     * This method is abstract and should be overridden in child classes to
     * support the appropriate asset type.
     *
     * @return the number of assets currently loaded.
     */
    virtual size_t loadCount() const { return 0; }
    
    /**
     * Returns the number of assets waiting to load.
     *
     * This is a rough way to determine how many assets are still pending.
     * An asset is pending if it has been loaded asychronously, and the
     * loading process has not yet finished. This method counts each asset
     * equally regardless of the memory requirements of each asset.
     *
     * This method is abstract and should be overridden in child classes to
     * support the appropriate asset type.
     *
     * @return the number of assets waiting to load.
     */
    virtual size_t waitCount() const { return 0; }
    
    /**
     * Returns true if the loader has finished loading all assets.
     *
     * It is not safe to use asynchronously loaded assets until all loading is
     * complete.  This method allows us to determine when asset loading is
     * complete via polling.
     *
     * @return true if the loader has finished loading all assets.
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
    
};


#pragma mark -
#pragma mark Templated Middle Layer

/**
 * This class is a specific template for each loader.
 *
 * This template works like a generic abstract class in Java.  It provides some
 * type correctness.  It also provides some base functionality that is common
 * for all loaders.  Methods marked as abstract must be overriden in specific
 * loader implementations.
 *
 * All assets are assigned a key and retrieved via that key.  This provides a 
 * quick way to reference assets.
 *
 * IMPORTANT: This class is not even remotely thread-safe.  Do not call any of
 * these methods outside of the main CUGL thread.
 */
template <class T>
class Loader : public BaseLoader {
protected:
    /** Hash map storing the loaded assets */
    std::unordered_map<std::string, std::shared_ptr<T>> _assets;
    
    /** The assets we are expecting that are not yet loaded */
    std::unordered_set<std::string> _queue;

    /**
     * Unloads the asset for the given key
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
     * @return true if the asset was successfully unloaded
     */
    bool purge(const std::string& key) override {
        auto it = _assets.find(key);
        if (it != _assets.end()) {
            _assets.erase(it);
            return true;
        }
        return false;
    }
    
    /**
     * Returns true if the key maps to a loaded asset.
     *
     * This method is useful in case the asset fails to load. You will notice
     * that this method is essentially identical to contains.  We separated the
     * methods because overloading and virtual methods do not place nice.
     *
     * @param key   The key associated with the asset
     *
     * @return true if the key maps to a loaded asset.
     */
    bool verify(const std::string& key) const override {
        return _assets.find(key) != _assets.end();
    }
    
public:
#pragma mark Constructors
    /**
     * Creates a degenerate asset loader with no resources
     *
     * NEVER CALL THIS CONSTRUCTOR. As this is an abstract class, you should
     * call one of the static constructors of the appropriate child class.
     */
    Loader(): BaseLoader() {}

    
#pragma mark Asset Access
    /**
     * Returns the asset for the given key.
     *
     * If the key is valid, the asset is guaranteed not to be null.  Otherwise,
     * this method returns nullptr
     *
     * @param key   The key associated with the asset
     *
     * @return the asset pointer for the given key
     */
    std::shared_ptr<T> get(const std::string& key) const {
        auto it = _assets.find(key);
        return (it == _assets.end() ? nullptr : it->second);
    }

    /**
     * Returns the asset for the given key.
     *
     * If the key is valid, the asset is guaranteed not to be null.  Otherwise,
     * this method returns nullptr
     *
     * @param key   The key associated with the asset
     *
     * @return the asset pointer for the given key
     */
    std::shared_ptr<T> get(const char* key) const {
        auto it = _assets.find(std::string(key));
        return (it == _assets.end() ? nullptr : it->second);
    }
    
    /**
     * Returns the asset for the given key.
     *
     * If the key is valid, the asset is guaranteed not to be null.  Otherwise,
     * this method returns nullptr
     *
     * @param key   The key associated with the asset
     *
     * @return the asset pointer for the given key
     */
    std::shared_ptr<T> operator[](const std::string& key) const { return get(key); }

    /**
     * Returns the asset for the given key.
     *
     * If the key is valid, the asset is guaranteed not to be null.  Otherwise,
     * this method returns nullptr
     *
     * @param key   The key associated with the asset
     *
     * @return the asset pointer for the given key
     */
    std::shared_ptr<T> operator[](const char* key) const { return get(key); }
    

#pragma mark Asset Loading
    /**
     * Returns the number of assets currently loaded.
     *
     * This is a rough way to determine how many assets have been loaded
     * so far. This method counts each asset equally regardless of the memory
     * requirements of each asset.
     *
     * @return the number of assets currently loaded.
     */
    size_t loadCount() const override { return _assets.size(); }
    
    /**
     * Returns the number of textures waiting to load.
     *
     * This is a rough way to determine how many texture are still pending.
     * A texture is pending if it has been loaded asychronously, and the
     * loading process has not yet finished. This method counts each texture
     * equally regardless of the memory requirements of the format.
     *
     * @return the number of textures waiting to load.
     */
    size_t waitCount() const override { return _queue.size(); }

    /**
     * Unloads all assets present in this loader.
     *
     * An asset may still be available if it is referenced by a smart pointer.
     * See the description of the specific implementation for how assets
     * are released.
     */
    void unloadAll() override {
        _assets.clear();
    }
};

}


#endif /* __CU_LOADER_H__ */

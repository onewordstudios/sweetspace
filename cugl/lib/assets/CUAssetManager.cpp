//
//  CUAssetManager.cpp
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
//  Version: 5/20/19
//
#include <cugl/cugl.h>

using namespace cugl;

#pragma mark -
#pragma mark Constructors
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
bool AssetManager::init() {
    _workers = ThreadPool::alloc(1);
    return true;
}

/**
 * Detaches all the attached loaders and deletes all auxiliary threads.
 *
 * Unlike the destructor, this does not destroy the asset manager.  However,
 * you will need to reinitialize the manager (to restart the auxiliary
 * threads) and reattach all loaders to use the asset manager again.
 */
void AssetManager::dispose() {
    detachAll();
    _workers = nullptr;
}

#pragma mark -
#pragma mark Internal Asset Loading
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
bool AssetManager::readCategory(size_t hash, const std::shared_ptr<JsonValue>& json) {
    auto it = _handlers.find(hash);
    if (it == _handlers.end()) {
        return false;
    }
    
    std::shared_ptr<BaseLoader> loader = it->second;    
    if (loader == nullptr) {
        CULogError("No loader for hash %zu",hash);
        return false;
    }
    
    bool success = true;
    for(int ii = 0; ii < json->size(); ii++) {
        std::shared_ptr<JsonValue> child = json->get(ii);
        success = loader->load(child) && success;
    }
    
    return success;
}

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
void AssetManager::readCategory(size_t hash, const std::shared_ptr<JsonValue>& json,
                                LoaderCallback callback) {
    auto it = _handlers.find(hash);
    std::shared_ptr<BaseLoader> loader = it->second;
    if (loader == nullptr) {
        if (callback) {
            Application::get()->schedule([=] {
                callback(json->key(),false);
                return false;
            });
        }
        return;
    }
    
    for(int ii = 0; ii < json->size(); ii++) {
        std::shared_ptr<JsonValue> child = json->get(ii);
        loader->loadAsync(child, callback);
    }
}

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
bool AssetManager::purgeCategory(size_t hash, const std::shared_ptr<JsonValue>& json) {
    auto it = _handlers.find(hash);
    if (it == _handlers.end()) {
        return false;
    }
    
    std::shared_ptr<BaseLoader> loader = it->second;
    if (loader == nullptr) {
        CULogError("No loader for hash %zu",hash);
        return false;
    }
    
    bool success = true;
    for(int ii = 0; ii < json->size(); ii++) {
        std::shared_ptr<JsonValue> child = json->get(ii);
        success = loader->unload(child) && success;
    }
    
    return success;
}

/**
 * Synchronizes the asset manager to wait until all assets have finished.
 *
 * This method is necessary for assets whose construction depends on
 * previously loaded assets (e.g. scene graphs).  In the current architecture,
 * this method is only correct if the asset manager loads assets in a
 * single thread.
 */
void AssetManager::sync() {
    _workers->addTask([=](void) {
        this->block();
        this->block(); // Two blocks force one complete cycle
    });
}

/**
 * Blocks the asset manager until the next animation frame.
 *
 * Any assets queued after a block will not be added to thread pool
 * until at least one animation frame has passed.  This method is used
 * to implement the {@link sync()} method.
 */
void AssetManager::block() {
    Application::get()->schedule([=](void){
        this->resume();
        return false;
    });
    _wait = true;
    while (_wait) {
        int delay = (int)(500/Application::get()->getFPS());
        SDL_Delay(delay);
    }
    return;
}

/**
 * Resumes a previously blocked the asset manager.
 *
 * Any assets queued after a block will not be added to thread pool
 * until at least one animation frame has passed.  This method is used
 * to implement the {@link sync()} method.
 */
void AssetManager::resume() {
    _wait = false;
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
bool AssetManager::loadDirectory(const std::shared_ptr<JsonValue>& json) {
    bool success = true;
    for(int ii = 0; ii < json->size(); ii++) {
        std::shared_ptr<JsonValue> child = json->get(ii);
        if (child->key() == "textures") {
            success = readCategory(typeid(Texture).hash_code(),child) && success;
        } else if (child->key() == "sounds") {
            success = readCategory(typeid(Sound).hash_code(),child) && success;
        } else if (child->key() == "fonts") {
            success = readCategory(typeid(Font).hash_code(),child) && success;
		} else if (child->key() == "jsons") {
			success = readCategory(typeid(JsonValue).hash_code(), child) && success;
		} else if (child->key() == "widgets") {
			success = readCategory(typeid(WidgetValue).hash_code(), child) && success;
        } else if (child->key() == "scenes") {
            success = readCategory(typeid(Node).hash_code(),child) && success;
        } else {
            CULogError("Unknown asset category '%s'",child->key().c_str());
            success = false;
        }
    }
    return success;
}

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
bool AssetManager::loadDirectory(const std::string& directory) {
    std::shared_ptr<JsonReader> reader = JsonReader::allocWithAsset(directory);
    if (reader == nullptr) {
        CULogError("No asset directory located at '%s'",directory.c_str());
        return false;
    }
    
    std::shared_ptr<JsonValue> json = reader->readJson();
    return loadDirectory(json);
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
void AssetManager::loadDirectoryAsync(const std::shared_ptr<JsonValue>& json, LoaderCallback callback) {
    for(int ii = 0; ii < json->size(); ii++) {
        std::shared_ptr<JsonValue> child = json->get(ii);
        if (child->key() == "textures") {
            readCategory(typeid(Texture).hash_code(),child,callback);
        } else if (child->key() == "sounds") {
            readCategory(typeid(Sound).hash_code(),child,callback);
        } else if (child->key() == "fonts") {
            readCategory(typeid(Font).hash_code(),child,callback);
        } else if (child->key() == "jsons") {
            readCategory(typeid(JsonValue).hash_code(),child,callback);
        } else if (child->key() == "widgets") {
            readCategory(typeid(WidgetValue).hash_code(),child,callback);
        } else if (child->key() != "scenes") {
            CULogError("Unknown asset category '%s'",child->key().c_str());
        }
    }
    
    // Scenes are read after everything else.
    std::shared_ptr<JsonValue> child = json->get("scenes");
    sync();
    if (child) {
        readCategory(typeid(Node).hash_code(),child,callback);
    }
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
 * @param directory The path to the JSON asset directory
 * @param callback  An optional callback after each asset is loaded
 */
void AssetManager::loadDirectoryAsync(const std::string& directory, LoaderCallback callback) {
    _preload = true;
    
    std::shared_ptr<JsonReader> reader = JsonReader::allocWithAsset(directory);
    if (reader == nullptr && callback != nullptr) {
        callback("",false);
        return;
    }
    
    _workers->addTask([=](void) {
        std::shared_ptr<JsonValue> json = reader->readJson();
        loadDirectoryAsync(json,callback);
        _preload = false;
    });
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
bool AssetManager::unloadDirectory(const std::shared_ptr<JsonValue>& json) {
    bool success = true;
    for(int ii = 0; ii < json->size(); ii++) {
        std::shared_ptr<JsonValue> child = json->get(ii);
        if (child->key() == "textures") {
            success = purgeCategory(typeid(Texture).hash_code(),child) && success;
        } else if (child->key() == "sounds") {
            success = purgeCategory(typeid(Sound).hash_code(),child) && success;
        } else if (child->key() == "fonts") {
            success = purgeCategory(typeid(Font).hash_code(),child) && success;
        } else if (child->key() == "jsons") {
            success = purgeCategory(typeid(JsonValue).hash_code(),child) && success;
        } else if (child->key() == "widgets") {
            success = purgeCategory(typeid(WidgetValue).hash_code(),child) && success;
        } else if (child->key() == "scenes") {
            success = purgeCategory(typeid(Node).hash_code(),child) && success;
        } else {
            CULogError("Unknown asset category '%s'",child->key().c_str());
            success = false;
        }
    }
    return success;

}

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
bool AssetManager::unloadDirectory(const std::string& directory) {
    std::shared_ptr<JsonReader> reader = JsonReader::allocWithAsset(directory);
    if (reader == nullptr) {
        CULogError("No asset directory located at '%s'",directory.c_str());
        return false;
    }
    
    std::shared_ptr<JsonValue> json = reader->readJson();
    return unloadDirectory(json);
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

size_t AssetManager::loadCount() const {
    size_t result = 0;
    for(auto it = _handlers.begin(); it != _handlers.end(); ++it) {
        result += it->second->loadCount();
    }
    return result;

}

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
size_t AssetManager::waitCount() const {
    size_t result = 0;
    for(auto it = _handlers.begin(); it != _handlers.end(); ++it) {
        result += it->second->waitCount();
    }
    return _preload ? result+1 : result;
}

//
//  CUAssetManager.h
//  Cornell University Game Library (CUGL)
//
//  This module provides an abstract class for generic assets (such as a model
//  file or level layout) not explicitly included in the existing asset classes.
//  It is fairly experimental, so use at your own risk.  If there are certain
//  assets that we overlooked that are the same across all projects, we will
//  consider adding them to the engine at a later date.
//
//  As this is an abstract class, it has no static constructors.  However, we
//  still separate initialization from the constructor as with all classes in
//  this engine.
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
#ifndef __CU_ASSET_H__
#define __CU_ASSET_H__
#include <cugl/assets/CULoader.h>

namespace cugl {

/**
 * An abstract class for a generic asset
 *
 * This class is the base class for any generic asset (such as a model file or 
 * level layout) not explicitly included in the existing asset classes. It has 
 * abstract methods for loading and unloading from a file, which any subclass 
 * should implement.
 *
 * To support both synchronous and asynchronous loading, the asset splits loading
 * into two phases: preloading and materialization.  Preloading is the part of
 * asset loading that can safely take place outside of the main CUGL thread.
 * The second phase, materialization, will take place in the main CUGL thread.
 * Any operations that require access to an OpenGL or audio context should take
 * place in that phase.
 *
 * This class SHOULD NOT make any references to {@link AssetManager} in the 
 * load/unload methods. Assets should be treated as if they load in parallel, 
 * not in sequence. Therefore, it is unsafe to assume that one asset loads 
 * before another.  If this asset needs to connect to other assets (sound, 
 * images, etc.) this should take place after asset loading, such as during 
 * scene graph initialization or the like.
 */
class Asset {
public:
#pragma mark -
#pragma mark Initializers
    /**
     * Initializes this asset from the given file.
     *
     * The initializer will load the asset synchronously, first calling 
     * {@link preload} and then calling {@link materialize}.  
     *
     * @param file  The path to the asset to load.
     *
     * @return true if the asset was loaded successfully
     */
    virtual bool init(const std::string& file) {
        bool success = preload(file);
        return success && materialize();
    }

    /**
     * Initializes this asset defined by the given directory entry.
     *
     * This version of initialization provides support for JSON directories.
     * The exact format of the directory entry is up to you. However, the
     * directory entry must be loaded manually via {@link GenericLoader}, as 
     * {@link AssetManager} does not yet support generic JSON directory entries.
     *
     * The initializer will load the asset synchronously, first calling
     * {@link preload} and then calling {@link materialize}.
     *
     * @param json  The directory entry for this asset
     *
     * @return true if the asset was loaded successfully
     */
    virtual bool init(const std::shared_ptr<JsonValue>& json) {
        bool success = preload(json);
        return success && materialize();
    }
    
#pragma mark -
#pragma mark Loading Interface
    /**
     * Loads the portion of this asset that is safe to load outside the main thread.
     *
     * This method is abstract. All subclasses should provide an implementation 
     * of this method.  This method is necessary for an asset to be used with an 
     * instance of {@link GenericLoader}.
     *
     * This method is similar to {@link TextureLoader#preload}.  It safely loads
     * the portion of the asset that can be loaded outside of the main CUGL
     * thread.  That means, if the asset needs access to OpenGL or the audio
     * engine as part of its loading, that should not take place in this method.
     *
     * This class SHOULD NOT make any references to {@link AssetManager} in the
     * load/unload methods. Assets should be treated as if they load in parallel,
     * not in sequence. Therefore, it is unsafe to assume that one asset loads
     * before another.  If this asset needs to connect to other assets (sound,
     * images, etc.) this should take place after asset loading, such as during
     * scene graph initialization or the like.
     *
     * @return true if the asset was preloaded successfully
     */
    virtual bool preload(const std::string& file)                { return false; }
    
    /**
     * Loads the portion of this asset that is safe to load outside the main thread.
     *
     * This method is abstract. All subclasses should provide an implementation
     * of this method.  This method is necessary for an asset to be used with an
     * instance of {@link GenericLoader}.
     *
     * This version of preloading provides support for JSON directories. The
     * exact format of the directory entry is up to you. However, the directory
     * entry must be loaded manually via {@link GenericLoader}, as
     * {@link AssetManager} does not yet support generic JSON directory entries.
     *
     * This method is similar to {@link TextureLoader#preload}.  It safely loads
     * the portion of the asset that can be loaded outside of the main CUGL
     * thread.  That means, if the asset needs access to OpenGL or the audio
     * engine as part of its loading, that should not take place in this method.
     *
     * This class SHOULD NOT make any references to {@link AssetManager} in the
     * load/unload methods. Assets should be treated as if they load in parallel,
     * not in sequence. Therefore, it is unsafe to assume that one asset loads
     * before another.  If this asset needs to connect to other assets (sound,
     * images, etc.) this should take place after asset loading, such as during
     * scene graph initialization or the like.
     *
     * @return true if the asset was preloaded successfully
     */
    virtual bool preload(const std::shared_ptr<JsonValue>& json) { return false; }
    
    /** 
     * Finishes loading within the main CUGL thread.
     *
     * This method is the final step to asset loading. It is the part of asset
     * loading that is not safe to be done asynchronously. If the asset needs
     * access to OpenGL or the audio engine to complete loading, that should
     * be done here.
     *
     * @return true if the asset was loaded successfully
     */     
    virtual bool materialize() { return true; };
};

}

#endif /* __CU_ASSET_H__ */

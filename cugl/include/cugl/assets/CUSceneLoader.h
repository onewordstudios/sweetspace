//
//  CUSceneLoader.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a specific implementation of the Loader class to load
//  a scene graph. Scene graphs are always specified as a JSON tree.  This
//  loader is very experiment, as the language  is still evolving, particularly
//  with regards to layout managers.
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
//  WARNING: This loader is highly experimental.  It has only minimal error
//  checking.  It is provided as-is for the UX designers to mock-up simple
//  scenes.
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
//  Version: 1/8/18
//
#ifndef __CU_SCENE_LOADER_H__
#define __CU_SCENE_LOADER_H__
#include <cugl/assets/CULoader.h>
#include <cugl/2d/CUNode.h>
#include <unordered_map>

namespace cugl {
    
class AssetManager;

/**
 * This class is a specific implementation of Loader<Node>
 *
 * This asset loader allows us to specify a scene graph subtree, which can
 * be attached to a new or existing scene graph.  This is particularly useful
 * for desiging UI elements with a JSON directory structure rather than having
 * to create them programmatically.  While not exactly visual wireframing,
 * this makes it easier for UX designers to layout user-interface elements,
 * HUDS and the like.
 *
 * As UI widgets typically require fonts and images to be loaded already,
 * these should always be the last elements loaded in a loading phase.
 *
 * As with all of our loaders, this loader is designed to be attached to an
 * asset manager. Use the method {@link getHook()} to get the appropriate
 * pointer for attaching the loader.
 */
class SceneLoader : public Loader<Node> {
private:
    /** This macro disables the copy constructor (not allowed on assets) */
    CU_DISALLOW_COPY_AND_ASSIGN(SceneLoader);
    
protected:
    /**
     * This is an enumeration for identifying scene node types.
     *
     * Each time you add a new UI widget, it should be added to this list.
     */
    enum class Widget {
        /** The base Node type */
        NODE,
        /** An image (PolygoNode) type */
        IMAGE,
        /** A (complex) PolygonNode type */
        POLY,
        /** A PathNode type */
        PATH,
        /** A WireNode type */
        WIRE,
        /** An animation node type */
        ANIMATE,
        /** A nine-patch type */
        NINE,
        /** A text label (uneditable) type */
        LABEL,
        /** A button type */
        BUTTON,
        /** A progress bar type */
        PROGRESS,
        /** A slider type */
        SLIDER,
        /** A single-line text field type */
        TEXTFIELD,
		/** A Node implied by an imported file */
		EXTERNAL_IMPORT,
        /** An unsupported type */
        UNKNOWN
    };

    /**
     * This is an enumeration for identifying layout managers.
     *
     * Each time you add a new layout, it should be added to this list.
     */
    enum class Form {
        /** The default layout manager, using absolute position */
        NONE,
        /** A layout manager using anchor points */
        ANCHORED,
        /** A float layout manager */
        FLOAT,
        /** A grid layout manager */
        GRID,
        /** An unsupported form */
        UNKNOWN
    };

    /** The type map for managing constructors */
    std::unordered_map<std::string,Widget> _types;
    
    /** The type map for managing layout */
    std::unordered_map<std::string,Form> _forms;
    
    /**
     * Records the given Node with this loader, so that it may be unloaded later.
     *
     * This method finishes the asset loading started in {@link load}. This
     * step is not safe to be done in a separate thread, as it accesses the
     * main asset table.  Therefore, it takes place in the main CUGL thread
     * via {@link Application#schedule}.  The scene is stored using the name
     * of the root Node as a key.
     *
     * This method supports an optional callback function which reports whether
     * the asset was successfully materialized.
     *
     * @param node      The scene asset
     * @param callback  An optional callback for asynchronous loading
     */
    void materialize(const std::shared_ptr<Node>& node, LoaderCallback callback);
    
    /**
     * Internal method to support asset loading.
     *
     * This method supports either synchronous or asynchronous loading, as
     * specified by the given parameter.  If the loading is asynchronous,
     * the user may specify an optional callback function.
     *
     * The sources must be a JSON file.  It will parse the JSON tree of this
     * file, assigning the given key to the root node.  The JSON tree should
     * be a tree of widget objects, where each widget object has the following
     * attribute values:
     *
     *      "type":     The node type (a Node or any subclass)
     *      "data":     Data (images, labels) that define the widget.  This
     *                  JSON object has a node-specific format.
     *      "format":   The layout manager to use for this Node. This layout
     *                  manager will apply to all the children (see below).
     *                  This JSON object has a layout-specific format.
     *      "layout":   Node placement using a the layout manager of the parent.
     *                  This is applied after parsing "data' and will override
     *                  any settings there. This JSON object has a
     *                  layout-specific format.
     *      "children": Any child Nodes of this one. This JSON object has a
     *                  named attribute for each child.
     *
     * With the exception of "type", all of these attributes are JSON objects.
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
     * This method is like the traditional read method except that it assumes
     * the JSON data has already been parsed.
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
    
    /**
     * Attaches all generate nodes to the asset dictionary.
     *
     * As the asset dictionary must be updated in the main thread, we do
     * not update it until the entire node tree has been materialized. This
     * method assumes that each Node is named with its asset look-up key.
     *
     * @param key       The key to access the asset after loading
     * @param node      The scene asset
     *
     * @return true if the node was successfully attached
     */
    bool attach(const std::string& key, const std::shared_ptr<Node>& node);

	/**
	 * Translates the JSON of a widget to the JSON of the node that it encodes.
	 *
	 * If this scene is built before the JSON of any used widgets have been loaded, this will fail.
	 *
	 * @param json      The JSON object specifying the widget's file and the values for its exposed variables
	 *
	 * @return the JSON loaded from the widget file with all variables set based on the values presented in json.
	 */
	std::shared_ptr<JsonValue> getWidgetJson(const std::shared_ptr<JsonValue>& json) const;
    
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new, uninitialized scene loader
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a loader on
     * the heap, use one of the static constructors instead.
     */
    SceneLoader() {}
    
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
    virtual bool init() override {
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
    virtual bool init(const std::shared_ptr<ThreadPool>& threads) override;
    
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
        _manager = nullptr;
        _assets.clear();
        _loader = nullptr;
        _types.clear();
        _forms.clear();
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
    static std::shared_ptr<SceneLoader> alloc() {
        std::shared_ptr<SceneLoader> result = std::make_shared<SceneLoader>();
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
    static std::shared_ptr<SceneLoader> alloc(const std::shared_ptr<ThreadPool>& threads) {
        std::shared_ptr<SceneLoader> result = std::make_shared<SceneLoader>();
        return (result->init(threads) ? result : nullptr);
    }
    
    /**
     * Recursively builds the scene from the given JSON tree.
     *
     * This method allows us to maximize the asynchronous creation of scenes.
     * The key is assigned as the name of the root Node of the scene.
     *
     * The JSON tree should be a tree of widget objects, where each widget
     * object has the following attribute values:
     *
     *      "type":     The node type (a Node or any subclass)
     *      "data":     Data (images, labels) that define the widget.  This
     *                  JSON object has a node-specific format.
     *      "format":   The layout manager to use for this Node. This layout
     *                  manager will apply to all the children (see below).
     *                  This JSON object has a layout-specific format.
     *      "layout":   Node placement using a the layout manager of the parent.
     *                  This is applied after parsing "data' and will override
     *                  any settings there. This JSON object has a
     *                  layout-specific format.
     *      "children": Any child Nodes of this one. This JSON object has a
     *                  named attribute for each child.
     *
     * With the exception of "type", all of these attributes are JSON objects.
     *
     * @param key       The key to access the scene after loading
     * @param json      The JSON object defining the scene
     *
     * @return the SDL_Surface with the texture information
     */
    std::shared_ptr<Node> build(const std::string& key, const std::shared_ptr<JsonValue>& json) const;
    
};
    
}

#endif /* __CU_SCENE_LOADER_H__ */

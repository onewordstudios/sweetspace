//
//  CUSceneLoader.cpp
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
//  Version: 5/20/19
//

#include <cugl/assets/CUAssetManager.h>
#include <cugl/assets/CUSceneLoader.h>
#include <cugl/assets/CUWidgetValue.h>
#include <cugl/base/CUApplication.h>
#include <cugl/io/CUJsonReader.h>
#include <cugl/util/CUStrings.h>
#include <cugl/2d/cu_2d.h>
#include <cugl/2d/layout/cu_layout.h>
#include <locale>
#include <algorithm>

using namespace cugl;

/** If the type is unknown */
#define UNKNOWN_STR  "<unknown>"

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
bool SceneLoader::init(const std::shared_ptr<ThreadPool>& threads) {
    _loader=threads;
    
    // Define the supported types
    _types["node"] = Widget::NODE;
    _types["image"] = Widget::IMAGE;
    _types["polygon"] = Widget::POLY;
    _types["path"] = Widget::PATH;
    _types["wireframe"] = Widget::WIRE;
    _types["animation"] = Widget::ANIMATE;
    _types["ninepatch"] = Widget::NINE;
    _types["label"] = Widget::LABEL;
    _types["button"] = Widget::BUTTON;
    _types["progress"] = Widget::PROGRESS;
    _types["slider"] = Widget::SLIDER;
    _types["textfield"] = Widget::TEXTFIELD;
    _types["text field"] = Widget::TEXTFIELD;
	_types["widget"] = Widget::EXTERNAL_IMPORT;

    // Define the supported layouts
    _forms["none"] = Form::NONE;
    _forms["absolute"] = Form::NONE;
    _forms["anchored"] = Form::ANCHORED;
    _forms["float"]  = Form::FLOAT;
    _forms["grid"] = Form::GRID;

    return true;
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
std::shared_ptr<Node> SceneLoader::build(const std::string& key, const std::shared_ptr<JsonValue>& json) const {
    std::string type = json->getString("type",UNKNOWN_STR);
    auto it = _types.find(cugl::to_lower(type));
    if (it == _types.end()) {
        return nullptr;
    }
    
    std::shared_ptr<JsonValue> data = json->get("data");
    std::shared_ptr<Node> node = nullptr;
    switch (it->second) {
    case Widget::NODE:
        node = Node::allocWithData(this,data);
        break;
    case Widget::IMAGE:
        node = PolygonNode::allocWithData(this,data);
        break;
    case Widget::POLY:
        node = PolygonNode::allocWithData(this,data);
        break;
    case Widget::PATH:
        node = PathNode::allocWithData(this,data);
        break;
    case Widget::WIRE:
        node = WireNode::allocWithData(this,data);
        break;
    case Widget::ANIMATE:
        node = AnimationNode::allocWithData(this,data);
        break;
    case Widget::NINE:
        node = NinePatch::allocWithData(this,data);
        break;
    case Widget::LABEL:
        node = Label::allocWithData(this,data);
        break;
    case Widget::BUTTON:
        node = Button::allocWithData(this,data);
        break;
    case Widget::PROGRESS:
        node = ProgressBar::allocWithData(this,data);
        break;
    case Widget::SLIDER:
        node = Slider::allocWithData(this,data);
        break;
    case Widget::TEXTFIELD:
        node = TextField::allocWithData(this,data);
        break;
	case Widget::EXTERNAL_IMPORT: 
	{
		const std::shared_ptr<JsonValue> widgetJson = getWidgetJson(json);
		return build(key, widgetJson);
	}
    case Widget::UNKNOWN:
        break;
    }
    
    if (node == nullptr) {
        return nullptr;
    }
    
    if (node->getContentSize() == Size::ZERO) {
        node->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
        node->setContentSize(Application::get()->getDisplaySize());
    }
    
    
    std::shared_ptr<JsonValue> form = json->get("format");
    std::string ftype =  (form == nullptr ? UNKNOWN_STR : form->getString("type",UNKNOWN_STR));
    auto jt = _forms.find(cugl::to_lower(ftype));
    
    std::shared_ptr<Layout> layout = nullptr;
    if (jt != _forms.end()) {
        switch (jt->second) {
        case Form::ANCHORED:
            layout = AnchoredLayout::allocWithData(form);
            break;
        case Form::FLOAT:
            layout = FloatLayout::allocWithData(form);
            break;
        case Form::GRID:
            layout = GridLayout::allocWithData(form);
            break;
        case Form::NONE:
        case Form::UNKNOWN:
            break;
        }
    }
    node->setLayout(layout);
    
    std::shared_ptr<JsonValue> children = json->get("children");
	if (children != nullptr) {
		for (int ii = 0; ii < children->size(); ii++) {
			std::shared_ptr<JsonValue> item = children->get(ii);
			std::string key = item->key();
			if (key != "comment") {
				// If this is a widget, use the loaded widget json instead
				if (item->has("type") && item->getString("type") == "Widget") {
					item = getWidgetJson(item);
				}

				std::shared_ptr<Node> kid = build(key, item);
				node->addChild(kid);

				if (layout != nullptr && item->has("layout")) {
					std::shared_ptr<JsonValue> posit = item->get("layout");
					layout->add(key, posit);
				}
			}
		}
	}
    
    // Do not perform layout yet.
    node->setName(key);
    return node;
}


/**
 * Translates the JSON of a widget to the JSON of the node that it encodes.
 *
 * If this scene is built before the JSON of any used widgets have been loaded, this will fail.
 *
 * @param json      The JSON object specifying the widget's file and the values for its exposed variables
 *
 * @return the JSON loaded from the widget file with all variables set based on the values presented in json.
 */
std::shared_ptr<JsonValue> SceneLoader::getWidgetJson(const std::shared_ptr<JsonValue>& json) const {
	std::shared_ptr<JsonValue> data = json->get("data");
	std::string widgetSource = data->getString("key");
	std::shared_ptr<JsonValue> widgetVars = data->get("variables");
	std::shared_ptr<JsonValue> layout = json->get("layout");

	const std::shared_ptr<WidgetValue> widget = _manager->get<WidgetValue>(widgetSource);

	CUAssertLog(widget != nullptr, "No widget found with name %s", widgetSource.c_str());

	const std::shared_ptr<JsonValue> widgetJson = widget->getJson();

	std::shared_ptr<JsonValue> variables = widgetJson->get("variables");
	std::shared_ptr<JsonValue> contents = widgetJson->get("contents");
	std::string contentString = contents->toString();
	std::shared_ptr<JsonValue> contentCopy = JsonValue::allocWithJson(contentString);
    if (widgetVars) {
        for (int ii = 0; ii < widgetVars->size(); ii++) {
            auto child = widgetVars->get(ii);
            if (variables->has(child->key())) {
                bool found = true;
                std::shared_ptr<JsonValue> address = variables->get(child->key());
                std::shared_ptr<JsonValue> spotToChange = contentCopy;
                std::vector<std::string> sAry = address->asStringArray();
                for (std::string s : sAry) {
                    if (spotToChange->has(s)) {
                        spotToChange = spotToChange->get(s);
                    }
                    else {
                        found = false;
                    }
                }
                if (found) {
                    *spotToChange = *child;
                } else {
                    std::string err = "No variable found within widget " + widgetSource + " matching name " + child->key();
                    CULogError("%s",err.c_str());
                }
            }
        }
	}

	// reassign the layout if it exists
	if (layout != nullptr) {
		std::shared_ptr<JsonValue> contentsLayout = contentCopy->get("layout");
		if (contentsLayout == nullptr) {
			contentCopy->appendChild("layout", std::make_shared<JsonValue>());
			contentsLayout = contentCopy->get("layout");
		}
		*contentsLayout = *layout;
	}

	// now recursively check to see if this was a widget
	if (contentCopy->has("type") && contentCopy->getString("type") == "Widget") {
		return getWidgetJson(contentCopy);
	}
	return contentCopy;
}


/**
 * Records the given Node with this loader, so that it may be unloaded later.
 *
 * This method finishes the asset loading started in {@link preload}. This
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
void SceneLoader::materialize(const std::shared_ptr<Node>& node, LoaderCallback callback) {
    bool success = false;
    
    std::string key = "";
    if (node != nullptr) {
        key = node->getName();
        success = attach(key, node);
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
bool SceneLoader::read(const std::string& key, const std::string& source,
                      LoaderCallback callback, bool async) {
    if (_assets.find(key) != _assets.end() || _queue.find(key) != _queue.end()) {
        return false;
    }
    _queue.emplace(key);

    bool success = false;
    if (_loader == nullptr || !async) {
        std::shared_ptr<JsonReader> reader = JsonReader::allocWithAsset(source);
        std::shared_ptr<JsonValue> json = (reader == nullptr ? nullptr : reader->readJson());
        std::shared_ptr<Node> node = build(key,json);
        node->doLayout();
        if (node != nullptr) {
            success = true;
            materialize(node,callback);
        } else {
            _queue.erase(key);
        }
    } else {
        _loader->addTask([=](void) {
            std::shared_ptr<JsonReader> reader = JsonReader::allocWithAsset(source);
            std::shared_ptr<JsonValue> json = (reader == nullptr ? nullptr : reader->readJson());
            std::shared_ptr<Node> node = build(key,json);
            node->doLayout();
            Application::get()->schedule([=](void) {
                this->materialize(node,callback);
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
 * This method is like the traditional read method except that it assumes
 * the JSON data has already been parsed.  The JSON tree should
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
 * @param json      The directory entry for the asset
 * @param callback  An optional callback for asynchronous loading
 * @param async     Whether the asset was loaded asynchronously
 *
 * @return true if the asset was successfully loaded
 */
bool SceneLoader::read(const std::shared_ptr<JsonValue>& json, LoaderCallback callback, bool async) {
    std::string key = json->key();
    if (_assets.find(key) != _assets.end() || _queue.find(key) != _queue.end()) {
        return false;
    }
    _queue.emplace(key);
    
    bool success = false;
    if (_loader == nullptr || !async) {
        std::shared_ptr<Node> node = build(key,json);
        node->doLayout();
        if (node != nullptr) {
            success = true;
            materialize(node,callback);
        } else {
            _queue.erase(key);
        }
    } else {
        _loader->addTask([=](void) {
            std::shared_ptr<Node> node = build(key,json);
            node->doLayout();
            Application::get()->schedule([=](void) {
                this->materialize(node,callback);
                return false;
            });
        });
    }
    
    return success;
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
bool SceneLoader::purge(const std::shared_ptr<JsonValue>& json) {
    return false;
}

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
bool SceneLoader::attach(const std::string& key, const std::shared_ptr<Node>& node) {
    _assets[key] = node;
    bool success = true;
    for(int ii = 0; ii < node->getChildren().size(); ii++) {
        std::shared_ptr<Node> item = node->getChild(ii);
        std::string local = key+"_"+item->getName();
        success = attach(local, item) && success;
    }
    return success;
}




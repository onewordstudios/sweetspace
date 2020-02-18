//
//  CUJsonValue.h
//  Cornell University Game Library (CUGL)
//
//  This module a modern C++ alternative to the cJSON interface for reading
//  JSON files.  In particular, this gives us better type-checking and memory
//  management.  With that said, it still uses cJSON as the underlying parsing
//  engine.
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
//  Author: Graham Rutledge
//  Version: 5/20/19
//
#ifndef __CU_WIDGET_VALUE_H__
#define __CU_WIDGET_VALUE_H__
#include <cugl/base/CUBase.h>
#include <cugl/assets/CUJsonValue.h>
#include <memory>

namespace cugl {

/**
 * This class represents an externally defined widget that may be used within
 * the scene graph JSON.
 *
 * This class wraps a JsonValue for the purposes of safe dependency loading.
 */
class WidgetValue {
protected:
	/** The JSON entry representing this widget */
	std::shared_ptr<JsonValue> json;

public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a null WidgetValue.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
	WidgetValue() {};
    
    /**
     * Deletes this WidgetValue and all of its resources.
     *
     * If no other references own the descendants of this node, they will all
     * be recursively deleted as well.
     */
	~WidgetValue() {};
    
    /**
     * Initializes a new WidgetValue to wrap the give JsonValue.
     *
     * This initializer simply wraps the provided JSON
     *
     * @param json  A shared pointer to JsonValue that defines this widget.
     *
     * @return  true if the JsonValue is not a nullptr, false otherwise.
     */
    bool init(const std::shared_ptr<JsonValue> json) {
		if (json == nullptr) {
			return false;
		}

		this->json = json;
		return true;
    }
    
#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a newly allocated WidgetValue to wrap the given JsonValue.
     *
     * @param json  The JSON definition of this widget.
     *
     * @return a newly allocated WidgetValue wrapping the provided JsonValue.
     */
    static std::shared_ptr<WidgetValue> alloc(std::shared_ptr<JsonValue> json) {
        std::shared_ptr<WidgetValue> result = std::make_shared<WidgetValue>();
        return (result->init(json) ? result : nullptr);
    }

#pragma mark -
#pragma mark Value Access

	/**
	* Returns the JsonValue representation of this widget.
	*
	* @return a shared pointer to the JsonValue representation of this widget.
	*/
	const std::shared_ptr<JsonValue> getJson() const {
		return json;
	}
};

}
#endif /* __CU_JSON_VALUE_H__ */

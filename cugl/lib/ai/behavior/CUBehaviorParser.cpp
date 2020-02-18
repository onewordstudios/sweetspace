//
//  CUBehaviorParser.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a behavior parser in order to create a
//  BodyDefNode from a JSON.  It is hear because we have not yet folded
//  an official AI asset loader into this release.
//
//  EXPERIMENTAL: This module is experimental.  The API may change significantly
//  in future CUGL releases.
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
//  Author: Apurv Sethi and Andrew Matsumoto (with Walker White)
//  Version: 5/22/2018
//
#include <cugl/ai/behavior/CUBehaviorParser.h>
#include <cugl/util/CUDebug.h>
#include <unordered_map>

using namespace cugl::ai;
using namespace cugl;

/** A mapping of the string values to the behavior node types. */
std::unordered_map<std::string, BehaviorNodeDef::Type> typeMap = {
	{"priority", BehaviorNodeDef::Type::PRIORITY_NODE},
	{"selector", BehaviorNodeDef::Type::SELECTOR_NODE},
	{"random",   BehaviorNodeDef::Type::RANDOM_NODE},
	{"inverter", BehaviorNodeDef::Type::INVERTER_NODE},
	{"timer",    BehaviorNodeDef::Type::TIMER_NODE},
	{"leaf",     BehaviorNodeDef::Type::LEAF_NODE}
};

/**
 * Disposes all of the resources used by this parser.
 *
 * A disposed action can be safely reinitialized.
 *
 * It is unsafe to call this on an action that is still currently inside
 * of a running behavior tree.
 */
void BehaviorParser::dispose() {
    _prioritizers.clear();
    _actions.clear();
}

#pragma mark Parser State
/**
 * Adds a prioritizer for the given name.
 *
 * This function should return a value between 0 and 1 representing the
 * priority of a node or action.  This method will fail if the name is
 * already in use.
 *
 * @param name          The name to associate with the priority function
 * @param prioritizer   The priority function
 */
void BehaviorParser::addPrioritizer(const std::string& name, std::function<float()> prioritizer) {
    CUAssertLog(_prioritizers.find(name) == _prioritizers.end(),
                "Name '%s' is already in use",name.c_str());
    _prioritizers[name] = prioritizer;
}

/**
 * Returns the prioritizer for the given name.
 *
 * This function returns nullptr if there is no prioritizer for that name.
 *
 * @param name          The name associated with the priority function
 *
 * @return the prioritizer for the given name.
 */
std::function<float()> BehaviorParser::getPrioritizer(const std::string& name) const {
    if ( _prioritizers.find(name) != _prioritizers.end()) {
        return _prioritizers.at(name);
    }
    return nullptr;
}

/**
 * Removes the prioritizer for the given name.
 *
 * This function returns the prioritizer removed.  It returns nullptr if
 * there is no prioritizer for that name.
 *
 * @param name          The name associated with the priority function
 *
 * @return the prioritizer removed.
 */
std::function<float()> BehaviorParser::removePrioritizer(const std::string& name) {
    auto it = _prioritizers.find(name);
     if ( it != _prioritizers.end()) {
         std::function<float()> result = it->second;
        _prioritizers.erase(it);
         return result;
    }
    return nullptr;
}

/**
 * Adds an action definition for the given name.
 *
 * This method will fail if the name is already in use.
 *
 * @param name          The name to associate with the action definition
 * @param actiondef     The action definition
 */
void BehaviorParser::addAction(const std::string& name, std::shared_ptr<BehaviorActionDef> actiondef) {
    CUAssertLog(_actions.find(name) == _actions.end(),
                "Name '%s' is already in use",name.c_str());
    _actions[name] = actiondef;
}


/**
 * Returns the action definition for the given name.
 *
 * This function returns nullptr if there is no definition for that name.
 *
 * @param name          The name associated with the action definition
 *
 * @return the action definition for the given name.
 */
std::shared_ptr<BehaviorActionDef> BehaviorParser::getAction(const std::string& name) const {
    if ( _actions.find(name) != _actions.end()) {
        return _actions.at(name);
    }
    return nullptr;
}

/**
 * Removes the action definition for the given name.
 *
 * This function returns the definition removed.  It returns nullptr if
 * there is no definition for that name.
 *
 * @param name          The name associated with the action definition
 *
 * @return the definition removed.
 */
std::shared_ptr<BehaviorActionDef> BehaviorParser::removeAction(const std::string& name) {
    auto it = _actions.find(name);
    if ( it != _actions.end()) {
        std::shared_ptr<BehaviorActionDef> result = it->second;
        _actions.erase(it);
        return result;
    }
    return nullptr;
}


#pragma mark Parsing Functions
/**
 * Parses the JSON file provided to create behavior node definitions.
 *
 * This method adds the {@link BehaviorNodeDef} objects to a map, which
 * the user can extract using the name of the root as the key.  Hence
 * all root nodes should have unique names.
 *
 * This function assumes that the file name is a relative path. It will
 * search the application asset directory
 * {@see Application#getAssetDirectory()} for the file and return false if
 * it cannot find it there.
 *
 * @param file  The relative path to the file.
 */
std::unordered_map<std::string, std::shared_ptr<BehaviorNodeDef>> BehaviorParser::parseFile(const char* file) {
    std::unordered_map<std::string, std::shared_ptr<BehaviorNodeDef>> defs;
	std::shared_ptr<JsonReader> reader = JsonReader::allocWithAsset(file);
	std::shared_ptr<JsonValue> json = reader->readJson();
	for (int ii = 0; ii < json->size(); ii++) {
		std::shared_ptr<BehaviorNodeDef> def = parseJson(json->get(ii));
		defs[def->name] = def;
	}
	return defs;
}

/**
 * Returns a {@link BehaviorNodeDef} constructed from the given {@link JsonValue}.
 *
 * This function reads a JSON to produce a {@link BehaviorNodeDef}. The
 * JSON must satisfy the format outlined in the clas description.
 *
 * @param json    The JsonValue representing a set of behavior node defintions.
 *
 * @return a BehaviorNodeDef constructed from the given JsonValue.
 */
std::shared_ptr<BehaviorNodeDef> BehaviorParser::parseJson(const std::shared_ptr<JsonValue>& json) {
    std::shared_ptr<BehaviorNodeDef> node = BehaviorNodeDef::alloc();
	node->name = json->key();
	std::string type = json->getString("type");
	CUAssertLog(!type.empty(), "The type of a BehaviorNodeDef must be defined");
	node->type = typeMap.at(type);

    node->prioritizer = getPrioritizer(json->getString("prioritizer", ""));
	node->preemptive = json->getBool("preemptive", false);
    node->background = json->getBool("background", false);
	node->uniform = json->getBool("uniform", true);
	node->delay = json->getFloat("delay", 1.0f);
    node->action = getAction(json->getString("action", ""));
	std::shared_ptr<JsonValue> children = json->get("children");
	if (children != nullptr) {
		for (int ii = 0; ii < children->size(); ii++) {
			node->children.push_back(parseJson(children->get(ii)));
		}
	}
	return node;
}

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
#ifndef __CU_BEHAVIOR_PARSER_H__
#define __CU_BEHAVIOR_PARSER_H__

#include <cugl/ai/behavior/CUBehaviorNode.h>
#include <cugl/ai/behavior/CUBehaviorAction.h>
#include <cugl/io/CUJsonReader.h>
#include <unordered_map>
#include <string>
#include <vector>

namespace cugl {
    namespace ai {
/**
 * A class designed to parse a JSON file describing a behavior tree.
 *
 * An instance of this class is used to parse a JSON file into a collection
 * of behavior trees.  When finished, this class produces a mapping from the
 * names of each behavior tree to the {@link BehaviorNodeDef} of that root of
 * that tree.
 *
 * The JSON file cannot contain the definitions of actions or priority functions,
 * as these are function definitions.  However, they can contain the names of
 * these functions.  When this parser is created, these functions can be added
 * manually to the parser (before parsing) to support this feature.
 *
 * When parsing a JSON file, each named, top-level object will be considered a
 * BehaviorNodeDef.  The node name is the name of the object, and is not an
 * attribute of the JSON object.  The supported attributes are:
 *
 *    type:         A string representing the type of the node definition
 *    prioritizer:  A string naming a priority function
 *    background:   A boolean indicating whether this node processed in the background
 *    preemptive:   A boolean indicating whether this node can be prempted
 *    uniform:      A boolean indicating whether any random selection is uniform
 *    delay:        A number, giving the time delay in seconds
 *    children:     A list of named objects defining behavior nodes
 *    action:       A string naming a possible action
 *
 * With the exception of type, all attributes are optional and have default
 * values.
 */
class BehaviorParser {
private:
    /** A collection of predefined priority functions */
    std::unordered_map<std::string,std::function<float()>> _prioritizers;
    /** A collection of predefined behavior actions */
    std::unordered_map<std::string,std::shared_ptr<BehaviorActionDef>> _actions;
    
#pragma mark Constructors
public:
    /**
     * Creates an uninitialized parser.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    BehaviorParser() {}
    
    /**
     * Deletes this parser, disposing all resources.
     */
    ~BehaviorParser() { dispose(); }
    
    /**
     * Disposes all of the resources used by this parser.
     *
     * A disposed action can be safely reinitialized.
     *
     * It is unsafe to call this on an action that is still currently inside
     * of a running behavior tree.
     */
    void dispose();
    
    /**
     * Initializes a behavior parser.
     *
     * This parser starts with no predefined prioritizers or actions. They
     * should be added (if needed) before parsing.
     *
     * @return true if initialization was successful.
     */
    bool init() { return true; }
    
    /**
     * Returns a newly allocated behavior parser.
     *
     * This parser starts with no predefined prioritizers or actions. They
     * should be added (if needed) before parsing.
     *
     * @return a newly allocated behavior parser.
     */
    static std::shared_ptr<BehaviorParser> alloc() {
        std::shared_ptr<BehaviorParser> result = std::make_shared<BehaviorParser>();
        return (result->init() ? result : nullptr);
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
    void addPrioritizer(const std::string& name, std::function<float()> prioritizer);

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
    void addPrioritizer(const char* name, std::function<float()> prioritizer) {
        addPrioritizer(std::string(name),prioritizer);
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
    std::function<float()> getPrioritizer(const std::string& name) const;

    /**
     * Returns the prioritizer for the given name.
     *
     * This function returns nullptr if there is no prioritizer for that name.
     *
     * @param name          The name associated with the priority function
     *
     * @return the prioritizer for the given name.
     */
    std::function<float()> getPrioritizer(const char* name) const {
        return getPrioritizer(std::string(name));
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
    std::function<float()> removePrioritizer(const std::string& name);
    
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
    std::function<float()> removePrioritizer(const char* name) {
        return removePrioritizer(std::string(name));
    }

    /**
     * Adds an action definition to the parser.
     *
     * This method assigned the action name to the one in the definition.
     * This method will fail if the name is already in use.
     *
     * @param actiondef     The action definition
     */
    void addAction(std::shared_ptr<BehaviorActionDef> actiondef) {
        addAction(actiondef->name,actiondef);
    }
    
    /**
     * Adds an action definition for the given name.
     *
     * This method will fail if the name is already in use.
     *
     * @param name          The name to associate with the action definition
     * @param actiondef     The action definition
     */
    void addAction(const std::string& name, std::shared_ptr<BehaviorActionDef> actiondef);

    /**
     * Adds an action definition for the given name.
     *
     * This method will fail if the name is already in use.
     *
     * @param name          The name to associate with the action definition
     * @param actiondef     The action definition
     */
    void addAction(const char* name, std::shared_ptr<BehaviorActionDef> actiondef) {
        addAction(std::string(name),actiondef);
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
    std::shared_ptr<BehaviorActionDef> getAction(const std::string& name) const;
    
    /**
     * Returns the action definition for the given name.
     *
     * This function returns nullptr if there is no definition for that name.
     *
     * @param name          The name associated with the action definition
     *
     * @return the action definition for the given name.
     */
    std::shared_ptr<BehaviorActionDef> getAction(const char* name) const {
        return getAction(std::string(name));
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
    std::shared_ptr<BehaviorActionDef> removeAction(const std::string& name);
    
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
    std::shared_ptr<BehaviorActionDef> removeAction(const char* name) {
        return removeAction(std::string(name));
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
	std::unordered_map<std::string, std::shared_ptr<BehaviorNodeDef>>
        parseFile(const std::string& file) {
		return parseFile(file.c_str());
	}

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
	std::unordered_map<std::string, std::shared_ptr<BehaviorNodeDef>>
        parseFile(const char* file);

private:
	/**
     * Returns a {@link BehaviorNodeDef} constructed from the given {@link JsonValue}.
	 *
     * This function reads a JSON to produce a {@link BehaviorNodeDef}. The
     * JSON must satisfy the format outlined in the clas description.
	 *
	 * @param json	The JsonValue representing a set of behavior node defintions.
	 *
	 * @return a BehaviorNodeDef constructed from the given JsonValue.
	 */
	std::shared_ptr<BehaviorNodeDef> parseJson(const std::shared_ptr<JsonValue>& json);
};

    }
}
#endif /* __CU_BEHAVIOR_PARSER_H__ */

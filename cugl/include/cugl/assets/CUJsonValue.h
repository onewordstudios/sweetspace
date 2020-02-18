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
//  Author: Walker White
//  Version: 11/28/16
//
#ifndef __CU_JSON_VALUE_H__
#define __CU_JSON_VALUE_H__
#include <cugl/base/CUBase.h>
#include <cJSON/cJSON.h>
#include <vector>
#include <string>

namespace cugl {

/**
 * This class represents a node in a JSON DOM tree.
 *
 * While each instance is a single node, there are many methods for traversing
 * the node as a tree.  A node only has a child if it is an array or an object
 * type.  All other types are leaves in the tree.
 *
 * Children may be accessed by index or key regardless of whether or not the
 * node is an array or object.  However, keys are only guaranteed to be unique
 * if the node is an object type.  Hence the main usage of this feature is to
 * "cast" object nodes to arrays.
 *
 * This class uses cJSON as the underlying parsing engine.  However, it manages
 * memory automatically so that the user does not need to worry about deleting
 * or allocating memory beyond the initial node itself.
 */
class JsonValue {
public:
    /**
     * This enum represents the possible data types in a JsonValue
     *
     * We do not distinguish numeric types (int, float, long, double) since
     * the JSON specification does not distinguish between them.  Instead, 
     * we freely convert between these values on access.
     */
    enum class Type : int {
        /** The JsonValue contains no value at all */
        NullType   = 0,
        /** The JsonValue contains a boolean value */
        BoolType   = 1,
        /** The JsonValue contains a numeric (int, float, etc.) value */
        NumberType = 2,
        /** The JsonValue contains a string value */
        StringType = 3,
        /** The JsonValue is an array whose contents are children of the node */
        ArrayType  = 4,
        /** The JsonValue is an object whose contents are children of the node */
        ObjectType = 5
    };

private:
    /** The type (see above) of this node */
    Type _type;
    
    /** A weak reference to the parent of this node (nullptr if root). */
    JsonValue* _parent;
    /** The key indexing this node with respect to its parent (maybe "") */
    std::string _key;
    
    /** The string data stored in this node (only defined if StringType) */
    std::string _stringValue;
    /** The number/boolean data stored in this node (only defined if BoolType/NumberType) */
    long   _longValue;
    /** The number data stored in this node (only defined if NumberType) */
    double _doubleValue;
    
    /** The children of this node (only non-empty if array or object) */
    std::vector<std::shared_ptr<JsonValue>> _children;

#pragma mark -
#pragma mark cJSON Conversions
    /**
     * Returns a newly allocated JsonValue equivalent to the cJSON node
     *
     * This allocator recursively allocates child nodes as necessary. These
     * nodes will be owned by the parent node and deleted when it is deleted
     * (provided there are no other references).
     *
     * This method does not delete the cJSON node when done.
     *
     * @param node  The cJSON node to convert
     *
     * @return a newly allocated JsonValue equivalent to the cJSON node
     */
    static std::shared_ptr<JsonValue> toJsonValue(const cJSON* node);

    /**
     * Modifies value so that it is equivalent to the cJSON node
     *
     * This allocator recursively allocates child nodes as necessary. These
     * nodes will be owned by the parent node value and deleted when it is
     * deleted (provided there are no other references).
     *
     * This method does not delete the cJSON node when done.
     *
     * @param node  The cJSON node to convert
     */
    static void toJsonValue(JsonValue* value, const cJSON* node);
    
    /**
     * Returns a newly allocated cJSON node equivalent to value
     *
     * This method recursively allocates child nodes as necessary. These
     * nodes will be owned by the parent node and deleted when it is deleted.
     * However, the returned cJSON node is not stored in a smart pointer, so
     * it must be manually deleted (with {@link cJSON_Delete}) when it is no
     * longer necessary.
     *
     * @param value The JsonValue to convert
     */
    static cJSON* toCJSON(const JsonValue* value);
    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates a null JsonValue.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    JsonValue();
    
    /**
     * Deletes this JsonValue and all of its resources.
     *
     * If no other references own the descendants of this node, they will all
     * be recursively deleted as well.
     */
    ~JsonValue();
    
    /**
     * Initializes a new JsonValue of the given type.
     *
     * The value of this node will be the default value of the type.
     *
     * @param type  The type of the JSON node.
     *
     * @return  true if the JSON node is initialized properly, false otherwise.
     */
    bool init(Type type);

    /**
     * Initializes a new JsonValue with the given string.
     *
     * The node will have type StringType.
     *
     * @param value The (string) value of this JSON node.
     *
     * @return  true if the JSON node is initialized properly, false otherwise.
     */
    bool init(const std::string& value);

    /**
     * Initializes a new JsonValue with the given string.
     *
     * The node will have type StringType.
     *
     * @param value The (string) value of this JSON node.
     *
     * @return  true if the JSON node is initialized properly, false otherwise.
     */
    bool init(const char* value) { return init(std::string(value)); }

    /**
     * Initializes a new JsonValue with the given boolean.
     *
     * The node will have type BoolType.
     *
     * @param value The (boolean) value of this JSON node.
     *
     * @return  true if the JSON node is initialized properly, false otherwise.
     */
    bool init(bool value);

    /**
     * Initializes a new JsonValue with the given number.
     *
     * The node will have type NumberType.
     *
     * @param value The (numeric) value of this JSON node.
     *
     * @return  true if the JSON node is initialized properly, false otherwise.
     */
    bool init(long value);

    /**
     * Initializes a new JsonValue with the given number.
     *
     * The node will have type NumberType.
     *
     * @param value The (numeric) value of this JSON node.
     *
     * @return  true if the JSON node is initialized properly, false otherwise.
     */
    bool init(double value);

    /**
     * Initializes a new null JsonValue (e.g. it has no value).
     *
     * The node will have type NullType.
     *
     * @return  true if the JSON node is initialized properly, false otherwise.
     */
    bool initNull() { return init(Type::NullType); }

    /**
     * Initializes a new JsonValue array.
     *
     * The node will have type ArrayType.  The node itself will have no value.
     * The contents of the array are the children of this node.
     *
     * @return  true if the JSON node is initialized properly, false otherwise.
     */
    bool initArray() { return init(Type::ArrayType); }

    /**
     * Initializes a new JsonValue object.
     *
     * The node will have type ObjectType.  The node itself will have no value.
     * The contents of the object are the children of this node.
     *
     * @return  true if the JSON node is initialized properly, false otherwise.
     */
    bool initObject() { return init(Type::ObjectType); }
    
    /**
     * Initializes a new JsonValue from the given JSON string.
     *
     * This initializer will parse the JSON string and construct a full JSON
     * tree for the string, if possible. The children are all owned by this 
     * node will be deleted when this node is deleted (provided there are
     * no other references).
     *
     * If there is a parsing error, this  method will return false.  Detailed 
     * information about the parsing error will be passed to an assert.  Hence
     * error messages are suppressed if asserts are turned off.
     *
     * @param json  The JSON string to parse.
     *
     * @return  true if the JSON node is initialized properly, false otherwise.
     */
    bool initWithJson(const std::string& json) {
        return initWithJson(json.c_str());
    }
    
    /**
     * Initializes a new JsonValue from the given JSON string.
     *
     * This initializer will parse the JSON string and construct a full JSON
     * tree for the string, if possible. The children are all owned by this
     * node will be deleted when this node is deleted (provided there are
     * no other references).
     *
     * If there is a parsing error, this method will return false.  Detailed
     * information about the parsing error will be passed to an assert.  Hence
     * error messages are suppressed if asserts are turned off.
     *
     * @param json  The JSON string to parse.
     *
     * @return  true if the JSON node is initialized properly, false otherwise.
     */
    bool initWithJson(const char* json);

    
#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a newly allocated JsonValue of the given type.
     *
     * The value of this node will be the default value of the type.
     *
     * @param type  The type of the JSON node.
     *
     * @return a newly allocated JsonValue of the given type.
     */
    static std::shared_ptr<JsonValue> alloc(Type type) {
        std::shared_ptr<JsonValue> result = std::make_shared<JsonValue>();
        return (result->init(type) ? result : nullptr);
    }

    /**
     * Returns a newly allocated JsonValue with the given string.
     *
     * The node will have type StringType.
     *
     * @param value The (string) value of this JSON node.
     *
     * @return a newly allocated JsonValue with the given string.
     */
    static std::shared_ptr<JsonValue> alloc(const std::string& value) {
        std::shared_ptr<JsonValue> result = std::make_shared<JsonValue>();
        return (result->init(value) ? result : nullptr);
    }

    /**
     * Returns a newly allocated JsonValue with the given string.
     *
     * The node will have type StringType.
     *
     * @param value The (string) value of this JSON node.
     *
     * @return a newly allocated JsonValue with the given string.
     */
    static std::shared_ptr<JsonValue> alloc(const char* value) {
        std::shared_ptr<JsonValue> result = std::make_shared<JsonValue>();
        return (result->init(value) ? result : nullptr);
    }

    /**
     * Returns a newly allocated JsonValue with the given boolean.
     *
     * The node will have type BoolType.
     *
     * @param value The (boolean) value of this JSON node.
     *
     * @return a newly allocated JsonValue with the given boolean.
     */
    static std::shared_ptr<JsonValue> alloc(bool value) {
        std::shared_ptr<JsonValue> result = std::make_shared<JsonValue>();
        return (result->init(value) ? result : nullptr);
    }

    /**
     * Returns a newly allocated JsonValue with the given number.
     *
     * The node will have type NumberType.
     *
     * @param value The (numeric) value of this JSON node.
     *
     * @return a newly allocated JsonValue with the given number.
     */
    static std::shared_ptr<JsonValue> alloc(long value) {
        std::shared_ptr<JsonValue> result = std::make_shared<JsonValue>();
        return (result->init(value) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated JsonValue with the given number.
     *
     * The node will have type NumberType.
     *
     * @param value The (numeric) value of this JSON node.
     *
     * @return a newly allocated JsonValue with the given number.
     */
    static std::shared_ptr<JsonValue> alloc(double value) {
        std::shared_ptr<JsonValue> result = std::make_shared<JsonValue>();
        return (result->init(value) ? result : nullptr);
    }

    /**
     * Returns a newly allocated null JsonValue (e.g. it has no value).
     *
     * The node will have type NullType.
     *
     * @return a newly allocated null JsonValue (e.g. it has no value).
     */
    static std::shared_ptr<JsonValue> allocNull() {
        std::shared_ptr<JsonValue> result = std::make_shared<JsonValue>();
        return (result->initNull() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated JsonValue array.
     *
     * The node will have type ArrayType.  The node itself will have no value.
     * The contents of the array are the children of this node.
     *
     * @return a newly allocated JsonValue array.
     */
    static std::shared_ptr<JsonValue> allocArray() {
        std::shared_ptr<JsonValue> result = std::make_shared<JsonValue>();
        return (result->initArray() ? result : nullptr);
    }

    /**
     * Returns a newly allocated JsonValue object.
     *
     * The node will have type ObjectType.  The node itself will have no value.
     * The contents of the object are the children of this node.
     *
     * @return a newly allocated JsonValue object.
     */
    static std::shared_ptr<JsonValue> allocObject() {
        std::shared_ptr<JsonValue> result = std::make_shared<JsonValue>();
        return (result->initObject() ? result : nullptr);
    }

    /**
     * Returns a newly allocated JsonValue from the given JSON string.
     *
     * This initializer will parse the JSON string and construct a full JSON
     * tree for the string, if possible. The children are all owned by this
     * node will be deleted when this node is deleted (provided there are
     * no other references).
     *
     * If there is a parsing error, this  method will return nullptr.  Detailed
     * information about the parsing error will be passed to an assert.  Hence
     * error messages are suppressed if asserts are turned off.
     *
     * @param json  The JSON string to parse.
     *
     * @return a newly allocated JsonValue from the given JSON string.
     */
    static std::shared_ptr<JsonValue> allocWithJson(const std::string& json) {
        std::shared_ptr<JsonValue> result = std::make_shared<JsonValue>();
        return (result->initWithJson(json) ? result : nullptr);
    }

    /**
     * Returns a newly allocated JsonValue from the given JSON string.
     *
     * This initializer will parse the JSON string and construct a full JSON
     * tree for the string, if possible. The children are all owned by this
     * node will be deleted when this node is deleted (provided there are
     * no other references).
     *
     * If there is a parsing error, this  method will return nullptr.  Detailed
     * information about the parsing error will be passed to an assert.  Hence
     * error messages are suppressed if asserts are turned off.
     *
     * @param json  The JSON string to parse.
     *
     * @return a newly allocated JsonValue from the given JSON string.
     */
    static std::shared_ptr<JsonValue> allocWithJson(const char* json) {
        std::shared_ptr<JsonValue> result = std::make_shared<JsonValue>();
        return (result->initWithJson(json) ? result : nullptr);
    }

    
#pragma mark -
#pragma mark Type
    /**
     * Returns the type of this node
     *
     * @return the type of this node
     */
    Type type() const      { return _type; }

    /**
     * Returns true if this node has NULL type (e.g. it has no value)
     *
     * @return true if this node has NULL type (e.g. it has no value)
     */
    bool isNull() const     { return _type == Type::NullType; }
    
    /** 
     * Returns true if this node is a double or long value. 
     *
     * @return true if this node is a double or long value.
     */
    bool isNumber() const   { return _type == Type::NumberType; }
    
    /**
     * Returns true if this node is a boolean value.
     *
     * @return true if this node is a boolean value.
     */
    bool isBool() const     { return _type == Type::BoolType;   }
    
    /**
     * Returns true if this node is a string value.
     *
     * @return true if this node is a string value.
     */
    bool isString() const   { return _type == Type::StringType; }

    /** 
     * Returns true if this node is not NULL nor an array or object.
     *
     * @return true if this node is not NULL nor an array or object.
     */
    bool isValue() const;

    /**
     * Returns true if this node is an array.
     *
     * If this method returns true, it is not safe to access the children by
     * keys, as the keys may not be unique.
     *
     * @return true if this node is an array.
     */
    bool isArray() const    {  return _type == Type::ArrayType; }
    
    /**
     * Returns true if this node is an object.
     *
     * If this method returns true, it is safe to access the children by either
     * index or key.
     *
     * @return true if this node is an array.
     */
    bool isObject() const   { return _type == Type::ObjectType; }


#pragma mark -
#pragma mark Value Access
    /** 
     * Returns this node as a string.
     *
     * This method will fail if the node is not a value type.  Otherwise, if
     * the node is not a StringType, it will return the default value instead.
     *
     * @param defaultValue  The value to return if the node is not a string
     *
     * @return this node as a string.
     */
    const std::string asString(const std::string& defaultValue) const {
        return asString(defaultValue.c_str());
    }
    
    /**
     * Returns this node as a string.
     *
     * This method will fail if the node is not a value type.  Otherwise, if
     * the node is not a StringType, it will return the default value instead.
     *
     * @param defaultValue  The value to return if the node is not a string
     *
     * @return this node as a string.
     */
    const std::string asString(const char* defaultValue="") const;
    
    /**
     * Returns this node as a float.
     *
     * This method will fail if the node is not a value type.  Otherwise, if
     * the node is not a NumberType, it will return the default value instead.
     *
     * @param defaultValue  The value to return if the node is not a number
     *
     * @return this node as a float.
     */
    float asFloat(float defaultValue=0.0f) const;
    
    /**
     * Returns this node as a double.
     *
     * This method will fail if the node is not a value type.  Otherwise, if
     * the node is not a NumberType, it will return the default value instead.
     *
     * @param defaultValue  The value to return if the node is not a number
     *
     * @return this node as a double.
     */
    double asDouble(double defaultValue=0.0) const;
    
    /**
     * Returns this node as a long.
     *
     * This method will fail if the node is not a value type.  Otherwise, if
     * the node is not a NumberType, it will return the default value instead.
     *
     * @param defaultValue  The value to return if the node is not a number
     *
     * @return this node as a long.
     */
    long asLong(long defaultValue=0L) const;
    
    /**
     * Returns this node as a int.
     *
     * This method will fail if the node is not a value type.  Otherwise, if
     * the node is not a NumberType, it will return the default value instead.
     *
     * @param defaultValue  The value to return if the node is not a number
     *
     * @return this node as a int.
     */
    int asInt(int defaultValue=0) const;
    
    /**
     * Returns this node as a bool.
     *
     * This method will fail if the node is not a value type.  Otherwise, if
     * the node is not a BoolType or NumberType, it will return the default 
     * value instead.
     *
     * @param defaultValue  The value to return if the node is not a number
     *
     * @return this node as a bool.
     */
    bool asBool(bool defaultValue=false) const;
    
    /**
     * Returns the children of this value as a vector of strings
     *
     * This method will fail if the node is not an array or object.  For each
     * child, it will attempt to convert it to a string.  If it cannot, it will
     * use the default value instead.
     *
     * @param defaultValue  The value to use if a child is not a string
     *
     * @return the children of this value as a vector of strings
     */
    std::vector<std::string> asStringArray(const std::string& defaultValue) const;
    
    /**
     * Returns the children of this value as a vector of strings
     *
     * This method will fail if the node is not an array or object.  For each
     * child, it will attempt to convert it to a string.  If it cannot, it will
     * use the default value instead.
     *
     * @param defaultValue  The value to use if a child is not a string
     *
     * @return the children of this value as a vector of strings
     */
    std::vector<std::string> asStringArray(const char* defaultValue="") const {
        return asStringArray(std::string(defaultValue));
    }
    
    /**
     * Returns the children of this value as a vector of floats
     *
     * This method will fail if the node is not an array or object.  For each
     * child, it will attempt to convert it to a float.  If it cannot, it will
     * use the default value instead.
     *
     * @param defaultValue  The value to use if a child is not a float
     *
     * @return the children of this value as a vector of floats
     */
    std::vector<float> asFloatArray(float defaultValue=0.0f) const;
    
    /**
     * Returns the children of this value as a vector of doubles
     *
     * This method will fail if the node is not an array or object.  For each
     * child, it will attempt to convert it to a double.  If it cannot, it will
     * use the default value instead.
     *
     * @param defaultValue  The value to use if a child is not a double
     *
     * @return the children of this value as a vector of doubles
     */
    std::vector<double> asDoubleArray(double defaultValue=0.0) const;
    
    /**
     * Returns the children of this value as a vector of longs
     *
     * This method will fail if the node is not an array or object.  For each
     * child, it will attempt to convert it to a long.  If it cannot, it will
     * use the default value instead.
     *
     * @param defaultValue  The value to use if a child is not a long
     *
     * @return the children of this value as a vector of longs
     */
    std::vector<long> asLongArray(long defaultValue=0L) const;
    
    /**
     * Returns the children of this value as a vector of ints
     *
     * This method will fail if the node is not an array or object.  For each
     * child, it will attempt to convert it to an int.  If it cannot, it will
     * use the default value instead.
     *
     * @param defaultValue  The value to use if a child is not an int
     *
     * @return the children of this value as a vector of ints
     */
    std::vector<int> asIntArray(int defaultValue=0) const;
    
    /**
     * Returns the children of this value as a vector of bools
     *
     * This method will fail if the node is not an array or object.  For each
     * child, it will attempt to convert it to a bool.  If it cannot, it will
     * use the default value instead.
     *
     * @param defaultValue  The value to use if a child is not a bool
     *
     * @return the children of this value as a vector of bools
     */
    std::vector<bool> asBoolArray (bool defaultValue=false);
    
    
#pragma mark -
#pragma mark Value Modification
    /**
     * Sets the value of this node to the given string.
     *
     * This method will fail if the node is not a value type or NULL. Using
     * this method will set the type of the node to StringType.
     *
     * @param value The string value to assign
     */
    void set(const std::string& value);
    
    /**
     * Sets the value of this node to the given string.
     *
     * This method will fail if the node is not a value type or NULL. Using
     * this method will set the type of the node to StringType.
     *
     * @param value The string value to assign (may not be null)
     */
    void set(const char* value) {
        set(std::string(value));
    }
    
    /**
     * Sets the value of this node to the given number.
     *
     * This method will fail if the node is not a value type or NULL. Using
     * this method will set the type of the node to NumberType.
     *
     * @param value The numeric value to assign
     */
    void set(long value);

    /**
     * Sets the value of this node to the given number.
     *
     * This method will fail if the node is not a value type or NULL. Using
     * this method will set the type of the node to NumberType.
     *
     * @param value The numeric value to assign
     */
    void set(double value);
    
    /**
     * Sets the value of this node to the given boolean.
     *
     * This method will fail if the node is not a value type or NULL. Using
     * this method will set the type of the node to BoolType.
     *
     * @param value The boolean value to assign
     */
    void set(bool value);
    
    /**
     * Sets this node to NULL, erasing all values.
     *
     * This method will fail if the node is not a value type or NULL. Using
     * this method will set the type of the node to NullType.
     */
    void setNull();

    
#pragma mark -
#pragma mark Child Access
    /** 
     * Returns the key for this object value.
     *
     * This method fails if there is no parent or the parent type is not
     * ObjectType.
     *
     * @return the key for this object value.
     */
    const std::string& key() const;
    
    /**
     * Sets this key for this object value.
     *
     * This method fails if there is no parent or the parent type is not
     * ObjectType. It also fails if this choice of key is not unique.
     *
     * @param key   the key for this object value.
     */
    void setKey(const std::string& key);

    /**
     * Sets this key for this object value.
     *
     * This method fails if there is no parent or the parent type is not
     * ObjectType. It also fails if this choice of key is not unique.
     *
     * @param key   the key for this object value.
     */
    void setKey(const char* key) { setKey(std::string(key)); }

    /**
     * Returns the index for this array/object value.
     *
     * This method fails if there is no parent node.
     *
     * @return the index for this array/object value.
     */
    const int index() const;

    /**
     * Returns the number of children of this node
     *
     * @return the number of children of this node
     */
    size_t size() const { return _children.size(); }
    
    /** 
     * Returns true if a child with the specified name exists. 
     *
     * This method will always return false if the node is not an object type
     *
     * @param name  The key identifying the child
     *
     * @return true if a child with the specified name exists.
     */
    bool has(const std::string& name) const;
    
    /**
     * Returns true if a child with the specified name exists.
     *
     * This method will always return false if the node is not an object type
     *
     * @param name  The key identifying the child
     *
     * @return true if a child with the specified name exists.
     */
    bool has(const char* name) const {
        return has(std::string(name));
    }

    /**
     * Returns the child at the specified index. 
     *
     * This method will fail if the node is not an array or object type.
     * If the index is out of bounds, this method will return nullptr.
     * 
     * @param index The index into the child array.
     *
     * @return the child at the specified index.
     */
    std::shared_ptr<JsonValue> get(int index);
    
    /**
     * Returns the child at the specified index.
     *
     * This method will fail if the node is not an array or object type.
     * If the index is out of bounds, this method will return nullptr.
     *
     * @param index The index into the child array.
     *
     * @return the child at the specified index.
     */
    const std::shared_ptr<JsonValue> get(int index) const;
    
    /** 
     * Returns the child with the specified key.
     *
     * This method will fail if the node is not an object type. If there is no
     * child with this key, the method returns nullptr.  If the node is somehow
     * corrupted and there is more than one child of this name, it will return
     * the first one.
     *
     * @param name  The key identifying the child.
     *
     * @return the child with the specified key.
     */
    std::shared_ptr<JsonValue> get(const std::string& name);
    
    /**
     * Returns the child with the specified key.
     *
     * This method will fail if the node is not an object type. If there is no
     * child with this key, the method returns nullptr.  If the node is somehow
     * corrupted and there is more than one child of this name, it will return
     * the first one.
     *
     * @param name  The key identifying the child.
     *
     * @return the child with the specified key.
     */
    const std::shared_ptr<JsonValue> get(const std::string& name) const;

    /**
     * Returns the child with the specified key.
     *
     * This method will fail if the node is not an object type. If there is no
     * child with this key, the method returns nullptr.  If the node is somehow
     * corrupted and there is more than one child of this name, it will return
     * the first one.
     *
     * @param name  The key identifying the child.
     *
     * @return the child with the specified key.
     */
    std::shared_ptr<JsonValue> get(const char* name) {
        return get(std::string(name));
    }
    
    /**
     * Returns the child with the specified key.
     *
     * This method will fail if the node is not an object type. If there is no
     * child with this key, the method returns nullptr.  If the node is somehow
     * corrupted and there is more than one child of this name, it will return
     * the first one.
     *
     * @param name  The key identifying the child.
     *
     * @return the child with the specified key.
     */
    const std::shared_ptr<JsonValue> get(const char* name) const {
        return get(std::string(name));
    }
    
    
#pragma mark -
#pragma mark Child Values
    /** 
     * Returns the string value of the child with the specified key. 
     *
     * If there is no child with the given key, or if that child cannot be
     * represented as a string value, it returns the default value instead.
     *
     * Note this is not the same behavior as get(key).asString(defaultValue),
     * since it will not fail if the child is an array or object.
     *
     * @param key  			The key identifying the child.
     * @param defaultValue  The value to use if child does not exist or is not a string
     *
     * @return the string value of the child with the specified key.
     */
    const std::string getString (const std::string& key, const std::string& defaultValue) const;
    
    /**
     * Returns the string value of the child with the specified key.
     *
     * If there is no child with the given key, or if that child cannot be
     * represented as a string value, it returns the default value instead.
     *
     * Note this is not the same behavior as get(key).asString(defaultValue),
     * since it will not fail if the child is an array or object.
     *
     * @param key  			The key identifying the child.
     * @param defaultValue  The value to use if child does not exist or is not a string
     *
     * @return the string value of the child with the specified key.
     */
    const std::string getString (const char* key, const char* defaultValue="") const {
        return getString(std::string(key),std::string(defaultValue));
    }
    
    /**
     * Returns the string value of the child with the specified key.
     *
     * If there is no child with the given key, or if that child cannot be
     * represented as a string value, it returns the default value instead.
     *
     * Note this is not the same behavior as get(key).asString(defaultValue),
     * since it will not fail if the child is an array or object.
     *
     * @param key  			The key identifying the child.
     * @param defaultValue  The value to use if child does not exist or is not a string
     *
     * @return the string value of the child with the specified key.
     */
    const std::string getString (const std::string& key, const char* defaultValue="") const {
        return getString(key,std::string(defaultValue));
    }
    
    /**
     * Returns the string value of the child with the specified key.
     *
     * If there is no child with the given key, or if that child cannot be
     * represented as a string value, it returns the default value instead.
     *
     * Note this is not the same behavior as get(key).asString(defaultValue),
     * since it will not fail if the child is an array or object.
     *
     * @param key  			The key identifying the child.
     * @param defaultValue  The value to use if child does not exist or is not a string
     *
     * @return the string value of the child with the specified key.
     */
    const std::string getString (const char* key, const std::string& defaultValue) const {
        return getString(std::string(key),defaultValue);
    }
    
    /**
     * Returns the float value of the child with the specified key.
     *
     * If there is no child with the given key, or if that child cannot be
     * represented as a numeric value, it returns the default value instead.
     *
     * Note this is not the same behavior as get(key).asFloat(defaultValue),
     * since it will not fail if the child is an array or object.
     *
     * @param key  			The key identifying the child.
     * @param defaultValue  The value to use if child does not exist or is not a number
     *
     * @return the float value of the child with the specified key.
     */
    float getFloat(const std::string& key, float defaultValue=0.0f) const;
    
    /**
     * Returns the float value of the child with the specified key.
     *
     * If there is no child with the given key, or if that child cannot be
     * represented as a numeric value, it returns the default value instead.
     *
     * Note this is not the same behavior as get(key).asFloat(defaultValue),
     * since it will not fail if the child is an array or object.
     *
     * @param key  			The key identifying the child.
     * @param defaultValue  The value to use if child does not exist or is not a number
     *
     * @return the float value of the child with the specified key.
     */
    float getFloat(const char* key, float defaultValue=0.0f) const {
        return getFloat(std::string(key),defaultValue);
    }
    
    /**
     * Returns the double value of the child with the specified key.
     *
     * If there is no child with the given key, or if that child cannot be
     * represented as a numeric value, it returns the default value instead.
     *
     * Note this is not the same behavior as get(key).asDouble(defaultValue),
     * since it will not fail if the child is an array or object.
     *
     * @param key  			The key identifying the child.
     * @param defaultValue  The value to use if child does not exist or is not a number
     *
     * @return the double value of the child with the specified key.
     */
    double getDouble(const std::string& key, double defaultValue=0.0) const;
    
    /**
     * Returns the double value of the child with the specified key.
     *
     * If there is no child with the given key, or if that child cannot be
     * represented as a numeric value, it returns the default value instead.
     *
     * Note this is not the same behavior as get(key).asDouble(defaultValue),
     * since it will not fail if the child is an array or object.
     *
     * @param key  			The key identifying the child.
     * @param defaultValue  The value to use if child does not exist or is not a number
     *
     * @return the double value of the child with the specified key.
     */
    double getDouble(const char* key, double defaultValue=0.0) const {
        return getDouble(std::string(key),defaultValue);
    }
    
    /**
     * Returns the long value of the child with the specified key.
     *
     * If there is no child with the given key, or if that child cannot be
     * represented as a numeric value, it returns the default value instead.
     *
     * Note this is not the same behavior as get(key).asLong(defaultValue),
     * since it will not fail if the child is an array or object.
     *
     * @param key  			The key identifying the child.
     * @param defaultValue  The value to use if child does not exist or is not a number
     *
     * @return the long value of the child with the specified key.
     */
    long getLong(const std::string& key, long defaultValue=0L) const;
    
    /**
     * Returns the long value of the child with the specified key.
     *
     * If there is no child with the given key, or if that child cannot be
     * represented as a numeric value, it returns the default value instead.
     *
     * Note this is not the same behavior as get(key).asLong(defaultValue),
     * since it will not fail if the child is an array or object.
     *
     * @param key  			The key identifying the child.
     * @param defaultValue  The value to use if child does not exist or is not a number
     *
     * @return the long value of the child with the specified key.
     */
    long getLong(const char* key, long defaultValue=0L) const {
        return getLong(std::string(key),defaultValue);
    }
    
    /**
     * Returns the int value of the child with the specified key.
     *
     * If there is no child with the given key, or if that child cannot be
     * represented as a numeric value, it returns the default value instead.
     *
     * Note this is not the same behavior as get(key).asInt(defaultValue),
     * since it will not fail if the child is an array or object.
     *
     * @param key  			The key identifying the child.
     * @param defaultValue  The value to use if child does not exist or is not a number
     *
     * @return the int value of the child with the specified key.
     */
    int getInt(const std::string& key, int defaultValue=0) const;
    
    /**
     * Returns the int value of the child with the specified key.
     *
     * If there is no child with the given key, or if that child cannot be
     * represented as a numeric value, it returns the default value instead.
     *
     * Note this is not the same behavior as get(key).asInt(defaultValue),
     * since it will not fail if the child is an array or object.
     *
     * @param key  			The key identifying the child.
     * @param defaultValue  The value to use if child does not exist or is not a number
     *
     * @return the int value of the child with the specified key.
     */
    int getInt(const char* key, int defaultValue=0) const {
        return getInt(std::string(key),defaultValue);
    }
    
    /**
     * Returns the boolean value of the child with the specified key.
     *
     * If there is no child with the given key, or if that child cannot be
     * represented as a boolean value, it returns the default value instead.
     *
     * Note this is not the same behavior as get(key).asBool(defaultValue),
     * since it will not fail if the child is an array or object.
     *
     * @param key  			The key identifying the child.
     * @param defaultValue  The value to use if child does not exist or is not a boolean
     *
     * @return the boolean value of the child with the specified key.
     */
    bool getBool(const std::string& key, bool defaultValue=false) const;

    /**
     * Returns the boolean value of the child with the specified key.
     *
     * If there is no child with the given key, or if that child cannot be
     * represented as a boolean value, it returns the default value instead.
     *
     * Note this is not the same behavior as get(key).asBool(defaultValue),
     * since it will not fail if the child is an array or object.
     *
     * @param key  			The key identifying the child.
     * @param defaultValue  The value to use if child does not exist or is not a boolean
     *
     * @return the boolean value of the child with the specified key.
     */
    bool getBool(const char* key, bool defaultValue=false) const {
        return getBool(std::string(key),defaultValue);
    }
    
#pragma mark -
#pragma mark Child Deletion
    /** 
     * Returns the child with the specified index and removes it from this node.
     *
     * All other children will be shifted to the right.  Returning the node 
     * allows the user to acquire ownership before it is deleted.
     *
     * If the index is out of bounds, this method returns nullptr.
     *
     * @param index The index of the child to remove
     *
     * Returns the child with the specified index and removes it from this node.
     */
    std::shared_ptr<JsonValue> removeChild(int index);
    
    /**
     * Returns the child with the specified key and removes it from this node.
     *
     * Returning the node allows the user to acquire ownership before it is 
     * deleted.
     *
     * If there is no child with this key, this method returns nullptr.
     *
     * @param name  The key of the child to remove
     *
     * Returns the child with the specified key and removes it from this node.
     */
    std::shared_ptr<JsonValue> removeChild(const std::string& name);
    
    /**
     * Returns the child with the specified key and removes it from this node.
     *
     * Returning the node allows the user to acquire ownership before it is
     * deleted.
     *
     * If there is no child with this key, this method returns nullptr.
     *
     * @param name  The key of the child to remove
     *
     * Returns the child with the specified key and removes it from this node.
     */
    std::shared_ptr<JsonValue> removeChild(const char* name) {
        return removeChild(std::string(name));
    }
    
#pragma mark -
#pragma mark Child Addition
    /**
     * Appends the given child to the end of this array or object.
     *
     * The child will be added to the next available position in the array.
     * If it is an object, it will use the current key of this object.
     *
     * This method will fail if this node is not an array or object type. If
     * is an object type, it will fail if the key to the child is not unique.
     *
     * This node will acquire ownership of the child, preventing it from being
     * deleted.
     *
     * @param child The child node to add
     */
    void appendChild(const std::shared_ptr<JsonValue>& child);
    
    /**
     * Appends the given child to the end of this object.
     *
     * The child will be added to the next available position in the array.
     * It will also use the provided key to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the 
     * child is not unique.
     *
     * This node will acquire ownership of the child, preventing it from being
     * deleted.
     *
     * @param key   The key to identify the child
     * @param child The child node to add
     */
    void appendChild(const std::string& key, const std::shared_ptr<JsonValue>& child);

    /**
     * Appends the given child to the end of this object.
     *
     * The child will be added to the next available position in the array.  
     * It will also use the provided key to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * This node will acquire ownership of the child, preventing it from being
     * deleted.
     *
     * @param key   The key to identify the child
     * @param child The child node to add
     */
    void appendChild(const char* key, const std::shared_ptr<JsonValue>& child) {
        return appendChild(std::string(key),child);
    }
    
    /**
     * Inserts the given child at the given position in this array or object.
     *
     * The child will be added to the given position in the array, and all other
     * children will be shifted to the right.  If it is an object, it will use 
     * the current key of this object.
     *
     * This method will fail if this node is not an array or object type. If it
     * is an object type, it will fail if the key to the child is not unique.
     *
     * This node will acquire ownership of the child, preventing it from being
     * deleted.
     *
     * @param index The position to add the child at
     * @param child The child node to add
     */
    void insertChild(unsigned int index, const std::shared_ptr<JsonValue>& child);

    /**
     * Inserts the given child at the given position in this object.
     *
     * The child will be added to the given position in the array, and all other
     * children will be shifted to the right. It will also use the provided key 
     * to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * This node will acquire ownership of the child, preventing it from being
     * deleted.
     *
     * @param index The position to add the child at
     * @param key   The key to identify the child
     * @param child The child node to add
     */
    void insertChild(unsigned int index, const std::string& key, const std::shared_ptr<JsonValue>& child);

    /**
     * Inserts the given child at the given position in this object.
     *
     * The child will be added to the given position in the array, and all other
     * children will be shifted to the right. It will also use the provided key
     * to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * This node will acquire ownership of the child, preventing it from being
     * deleted.
     *
     * @param index The position to add the child at
     * @param key   The key to identify the child
     * @param child The child node to add
     */
    void insertChild(unsigned int index, const char* key, const std::shared_ptr<JsonValue>& child) {
        insertChild(index,std::string(key),child);
    }

    /**
     * Allocates a new child with a boolean value and appends it to the end.
     *
     * The child will be added to the next available position in the array.
     *
     * This method will fail if this node is not an array (e.g. it will even
     * fail if it is an object, as the child will have no key).
     *
     * @param value The boolean value for the child
     */
    void appendValue(bool value) {
        appendChild(JsonValue::alloc(value));
    }
    
    /**
     * Allocates a new child with a boolean value and appends with the given key.
     *
     * The child will be added to the next available position in the array.
     * It will also use the provided key to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param key   The key to identify the child
     * @param value The boolean value for the child
     */
    void appendValue(const std::string& key, bool value) {
        appendChild(key,JsonValue::alloc(value));
    }
    
    /**
     * Allocates a new child with a boolean value and appends with the given key.
     *
     * The child will be added to the next available position in the array.
     * It will also use the provided key to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param key   The key to identify the child
     * @param value The boolean value for the child
     */
    void appendValue(const char* key, bool value) {
        appendChild(key,JsonValue::alloc(value));
    }
    
    /**
     * Allocates a new child with a boolean value and inserts it in place.
     *
     * The child will be added to the given position in the array, and all other
     * children will be shifted to the right.
     *
     * This method will fail if this node is not an array (e.g. it will even
     * fail if it is an object, as the child will have no key).
     *
     * @param index The position to insert the child
     * @param value The boolean value for the child
     */
    void insertValue(unsigned int index, bool value) {
        insertChild(index,JsonValue::alloc(value));
    }

    /**
     * Allocates a new child with a boolean value and inserts it in place.
     *
     * The child will be added to the given position in the array, and all other
     * children will be shifted to the right. It will also use the provided key 
     * to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param index The position to insert the child
     * @param key   The key to identify the child
     * @param value The boolean value for the child
     */
    void insertValue(unsigned int index, const std::string& key, bool value) {
        insertChild(index,JsonValue::alloc(value));
    }

    /**
     * Allocates a new child with a boolean value and inserts it in place.
     *
     * The child will be added to the given position in the array, and all other
     * children will be shifted to the right. It will also use the provided key
     * to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param index The position to insert the child
     * @param key   The key to identify the child
     * @param value The boolean value for the child
     */
    void insertValue(unsigned int index, const char* key, bool value) {
        insertChild(index,JsonValue::alloc(value));
    }
    
    /**
     * Allocates a new child with a long value and appends it to the end.
     *
     * The child will be added to the next available position in the array.
     *
     * This method will fail if this node is not an array (e.g. it will even
     * fail if it is an object, as the child will have no key).
     *
     * @param value The long value for the child
     */
    void appendValue(long value) {
        appendChild(JsonValue::alloc(value));
    }
    
    /**
     * Allocates a new child with a long value and appends with the given key.
     *
     * The child will be added to the next available position in the array.
     * It will also use the provided key to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param key   The key to identify the child
     * @param value The long value for the child
     */
    void appendValue(const std::string& key, long value)  {
        appendChild(key,JsonValue::alloc(value));
    }
    
    /**
     * Allocates a new child with a long value and appends with the given key.
     *
     * The child will be added to the next available position in the array.
     * It will also use the provided key to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param key   The key to identify the child
     * @param value The long value for the child
     */
    void appendValue(const char* key, long value)  {
        appendChild(key,JsonValue::alloc(value));
    }

    /**
     * Allocates a new child with a long value and inserts it in place.
     *
     * The child will be added to the given position in the array, and all other
     * children will be shifted to the right.
     *
     * This method will fail if this node is not an array (e.g. it will even
     * fail if it is an object, as the child will have no key).
     *
     * @param index The position to insert the child
     * @param value The long value for the child
     */
    void insertValue(unsigned int index, long value) {
        insertChild(index,JsonValue::alloc(value));
    }

    /**
     * Allocates a new child with a long value and inserts it in place.
     *
     * The child will be added to the given position in the array, and all other
     * children will be shifted to the right. It will also use the provided key
     * to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param index The position to insert the child
     * @param key   The key to identify the child
     * @param value The long value for the child
     */
    void insertValue(unsigned int index, const std::string& key, long value) {
        insertChild(index,JsonValue::alloc(value));
    }

    /**
     * Allocates a new child with a long value and inserts it in place.
     *
     * The child will be added to the given position in the array, and all other
     * children will be shifted to the right. It will also use the provided key
     * to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param index The position to insert the child
     * @param key   The key to identify the child
     * @param value The long value for the child
     */
    void insertValue(unsigned int index, const char* key, long value) {
        insertChild(index,JsonValue::alloc(value));
    }

    /**
     * Allocates a new child with a double value and appends it to the end.
     *
     * The child will be added to the next available position in the array.
     *
     * This method will fail if this node is not an array (e.g. it will even
     * fail if it is an object, as the child will have no key).
     *
     * @param value The double value for the child
     */
    void appendValue(double value)  {
        appendChild(JsonValue::alloc(value));
    }
    
    /**
     * Allocates a new child with a double value and appends with the given key.
     *
     * The child will be added to the next available position in the array.
     * It will also use the provided key to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param key   The key to identify the child
     * @param value The double value for the child
     */
    void appendValue(const std::string& key, double value) {
        appendChild(key,JsonValue::alloc(value));
    }
    
    /**
     * Allocates a new child with a double value and appends with the given key.
     *
     * The child will be added to the next available position in the array.
     * It will also use the provided key to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param key   The key to identify the child
     * @param value The double value for the child
     */
    void appendValue(const char* key, double value) {
        appendChild(key,JsonValue::alloc(value));
    }
    
    /**
     * Allocates a new child with a double value and inserts it in place.
     *
     * The child will be added to the given position in the array, and all other
     * children will be shifted to the right.
     *
     * This method will fail if this node is not an array (e.g. it will even
     * fail if it is an object, as the child will have no key).
     *
     * @param index The position to insert the child
     * @param value The double value for the child
     */
    void insertValue(unsigned int index, double value) {
        insertChild(index,JsonValue::alloc(value));
    }

    /**
     * Allocates a new child with a double value and inserts it in place.
     *
     * The child will be added to the given position in the array, and all other
     * children will be shifted to the right. It will also use the provided key
     * to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param index The position to insert the child
     * @param key   The key to identify the child
     * @param value The double value for the child
     */
    void insertValue(unsigned int index, const std::string& key, double value) {
        insertChild(index,JsonValue::alloc(value));
    }

    /**
     * Allocates a new child with a double value and inserts it in place.
     *
     * The child will be added to the given position in the array, and all other
     * children will be shifted to the right. It will also use the provided key
     * to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param index The position to insert the child
     * @param key   The key to identify the child
     * @param value The double value for the child
     */
    void insertValue(unsigned int index, const char* key, double value) {
        insertChild(index,JsonValue::alloc(value));
    }
    
    /**
     * Allocates a new child with a string value and appends it to the end.
     *
     * The child will be added to the next available position in the array.
     *
     * This method will fail if this node is not an array (e.g. it will even
     * fail if it is an object, as the child will have no key).
     *
     * @param value The string value for the child
     */
    void appendValue(const std::string& value) {
        appendChild(JsonValue::alloc(value));
    }
    
    /**
     * Allocates a new child with a string value and appends it to the end.
     *
     * The child will be added to the next available position in the array.
     *
     * This method will fail if this node is not an array (e.g. it will even
     * fail if it is an object, as the child will have no key).
     *
     * @param value The string value for the child
     */
    void appendValue(const char* value) {
        appendChild(JsonValue::alloc(value));
    }
    
    /**
     * Allocates a new child with a string value and appends with the given key.
     *
     * The child will be added to the next available position in the array.
     * It will also use the provided key to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param key   The key to identify the child
     * @param value The string value for the child
     */
    void appendValue(const std::string& key, const std::string& value) {
        appendChild(key,JsonValue::alloc(value));
    }
    
    /**
     * Allocates a new child with a string value and appends with the given key.
     *
     * The child will be added to the next available position in the array.
     * It will also use the provided key to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param key   The key to identify the child
     * @param value The string value for the child
     */
    void appendValue(const std::string& key, const char* value) {
        appendChild(key,JsonValue::alloc(value));
    }
    
    /**
     * Allocates a new child with a string value and appends with the given key.
     *
     * The child will be added to the next available position in the array.
     * It will also use the provided key to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param key   The key to identify the child
     * @param value The string value for the child
     */
    void appendValue(const char* key, const std::string& value) {
        appendChild(key,JsonValue::alloc(value));
    }
    
    /**
     * Allocates a new child with a string value and appends with the given key.
     *
     * The child will be added to the next available position in the array.
     * It will also use the provided key to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param key   The key to identify the child
     * @param value The string value for the child
     */
    void appendValue(const char* key, const char* value) {
        appendChild(key,JsonValue::alloc(value));
    }
    
    /**
     * Allocates a new child with a string value and inserts it in place.
     *
     * The child will be added to the given position in the array, and all other
     * children will be shifted to the right.
     *
     * This method will fail if this node is not an array (e.g. it will even
     * fail if it is an object, as the child will have no key).
     *
     * @param index The position to insert the child
     * @param value The string value for the child
     */
    void insertValue(unsigned int index, const std::string& value) {
        insertChild(index,JsonValue::alloc(value));
    }

    /**
     * Allocates a new child with a string value and inserts it in place.
     *
     * The child will be added to the given position in the array, and all other
     * children will be shifted to the right.
     *
     * This method will fail if this node is not an array (e.g. it will even
     * fail if it is an object, as the child will have no key).
     *
     * @param index The position to insert the child
     * @param value The string value for the child
     */
    void insertValue(unsigned int index, const char* value) {
        insertChild(index,JsonValue::alloc(value));
    }
    
    /**
     * Allocates a new child with a string value and inserts it in place.
     *
     * The child will be added to the given position in the array, and all other
     * children will be shifted to the right. It will also use the provided key
     * to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param index The position to insert the child
     * @param key   The key to identify the child
     * @param value The string value for the child
     */
    void insertValue(unsigned int index, const std::string& key, const std::string& value) {
        insertChild(index,JsonValue::alloc(value));
    }

    /**
     * Allocates a new child with a string value and inserts it in place.
     *
     * The child will be added to the given position in the array, and all other
     * children will be shifted to the right. It will also use the provided key
     * to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param index The position to insert the child
     * @param key   The key to identify the child
     * @param value The string value for the child
     */
    void insertValue(unsigned int index, const std::string& key, const char* value) {
        insertChild(index,JsonValue::alloc(value));
    }
    
    /**
     * Allocates a new child with a string value and inserts it in place.
     *
     * The child will be added to the given position in the array, and all other
     * children will be shifted to the right. It will also use the provided key
     * to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param index The position to insert the child
     * @param key   The key to identify the child
     * @param value The string value for the child
     */
    void insertValue(unsigned int index, const char* key, const std::string& value) {
        insertChild(index,JsonValue::alloc(value));
    }
    
    /**
     * Allocates a new child with a string value and inserts it in place.
     *
     * The child will be added to the given position in the array, and all other
     * children will be shifted to the right. It will also use the provided key
     * to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param index The position to insert the child
     * @param key   The key to identify the child
     * @param value The string value for the child
     */
    void insertValue(unsigned int index, const char* key, const char* value) {
        insertChild(index,JsonValue::alloc(value));
    }
    
    
    /**
     * Allocates a new child with no value and appends it to the end.
     *
     * The child will be added to the next available position in the array.
     *
     * This method will fail if this node is not an array (e.g. it will even
     * fail if it is an object, as the child will have no key).
     */
    void appendNull()  {
        appendChild(JsonValue::allocNull());
    }
    
    /**
     * Allocates a new child with no value and appends with the given key.
     *
     * The child will be added to the next available position in the array.
     * It will also use the provided key to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param key   The key to identify the child
     */
    void appendNull(const std::string& key) {
        appendChild(key,JsonValue::allocNull());
    }
    
    /**
     * Allocates a new child with no value and appends with the given key.
     *
     * The child will be added to the next available position in the array.
     * It will also use the provided key to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param key   The key to identify the child
     */
    void appendNull(const char* key) {
        appendChild(key,JsonValue::allocNull());
    }
    
    /**
     * Allocates a new child with no value and inserts it in place.
     *
     * The child will be added to the given position in the array, and all other
     * children will be shifted to the right.
     *
     * This method will fail if this node is not an array (e.g. it will even
     * fail if it is an object, as the child will have no key).
     *
     * @param index The position to insert the child
     */
    void insertNull(unsigned int index) {
        insertChild(index,JsonValue::allocNull());
    }
    
    /**
     * Allocates a new child with no value and inserts it in place.
     *
     * The child will be added to the given position in the array, and all other
     * children will be shifted to the right. It will also use the provided key
     * to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param index The position to insert the child
     * @param key   The key to identify the child
     */
    void insertNull(unsigned int index, const std::string& key) {
        insertChild(index,JsonValue::allocNull());
    }
    
    /**
     * Allocates a new child with no value and inserts it in place.
     *
     * The child will be added to the given position in the array, and all other
     * children will be shifted to the right. It will also use the provided key
     * to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param index The position to insert the child
     * @param key   The key to identify the child
     */
    void insertNull(unsigned int index, const char* key) {
        insertChild(index,JsonValue::allocNull());
    }
    
    /**
     * Allocates a new (empty) array and appends it to the end.
     *
     * The child will be added to the next available position in the array.
     *
     * This method will fail if this node is not an array (e.g. it will even
     * fail if it is an object, as the child will have no key).
     */
    void appendArray() {
        appendChild(JsonValue::allocArray());
    }
    
    /**
     * Allocates a new (empty) array and appends with the given key.
     *
     * The child will be added to the next available position in the array.
     * It will also use the provided key to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param key   The key to identify the child
     */
    void appendArray(const std::string& key) {
        appendChild(key,JsonValue::allocArray());
    }

    /**
     * Allocates a new (empty) array and appends with the given key.
     *
     * The child will be added to the next available position in the array.
     * It will also use the provided key to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param key   The key to identify the child
     */
    void appendArray(const char* key) {
        appendChild(key,JsonValue::allocArray());
    }
    
    /**
     * Allocates a new (empty) array and inserts it in place.
     *
     * The child will be added to the given position in the array, and all other
     * children will be shifted to the right.
     *
     * This method will fail if this node is not an array (e.g. it will even
     * fail if it is an object, as the child will have no key).
     *
     * @param index The position to insert the child
     */
    void insertArray(unsigned int index) {
        insertChild(index,JsonValue::allocArray());
    }

    /**
     * Allocates a new (empty) array and inserts it in place.
     *
     * The child will be added to the given position in the array, and all other
     * children will be shifted to the right. It will also use the provided key
     * to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param index The position to insert the child
     * @param key   The key to identify the child
     */
    void insertArray(unsigned int index, const std::string& key) {
        insertChild(index,key,JsonValue::allocArray());
    }

    /**
     * Allocates a new (empty) array and inserts it in place.
     *
     * The child will be added to the given position in the array, and all other
     * children will be shifted to the right. It will also use the provided key
     * to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param index The position to insert the child
     * @param key   The key to identify the child
     */
    void insertArray(unsigned int index, const char* key) {
        insertChild(index,key,JsonValue::allocArray());
    }
    
    /**
     * Allocates a new (empty) object and appends it to the end.
     *
     * The child will be added to the next available position in the array.
     *
     * This method will fail if this node is not an array (e.g. it will even
     * fail if it is an object, as the child will have no key).
     */
    void appendObject() {
        appendChild(JsonValue::allocObject());
    }
    
    /**
     * Allocates a new (empty) object and appends with the given key.
     *
     * The child will be added to the next available position in the array.
     * It will also use the provided key to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param key   The key to identify the child
     */
    void appendObject(const std::string& key) {
        appendChild(key,JsonValue::allocObject());
    }
    
    /**
     * Allocates a new (empty) object and appends with the given key.
     *
     * The child will be added to the next available position in the array.
     * It will also use the provided key to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param key   The key to identify the child
     */
    void appendObject(const char* key) {
        appendChild(key,JsonValue::allocObject());
    }
    
    /**
     * Allocates a new (empty) object and inserts it in place.
     *
     * The child will be added to the given position in the array, and all other
     * children will be shifted to the right.
     *
     * This method will fail if this node is not an array (e.g. it will even
     * fail if it is an object, as the child will have no key).
     *
     * @param index The position to insert the child
     */
    void insertObject(unsigned int index) {
        insertChild(index,JsonValue::allocObject());
    }
    
    /**
     * Allocates a new (empty) object and inserts it in place.
     *
     * The child will be added to the given position in the array, and all other
     * children will be shifted to the right. It will also use the provided key
     * to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param index The position to insert the child
     * @param key   The key to identify the child
     */
    void insertObject(unsigned int index, const std::string& key) {
        insertChild(index,key,JsonValue::allocObject());
    }

    /**
     * Allocates a new (empty) object and inserts it in place.
     *
     * The child will be added to the given position in the array, and all other
     * children will be shifted to the right. It will also use the provided key
     * to identify it.
     *
     * This method will fail if this node is not an object type (e.g. it will
     * even fail if the node is an array). It will also fail if the key to the
     * child is not unique.
     *
     * @param index The position to insert the child
     * @param key   The key to identify the child
     */
    void insertObject(unsigned int index, const char* key) {
        insertChild(index,key,JsonValue::allocObject());
    }


#pragma mark -
#pragma mark Encoding
    
    /**
     * Returns a string representation of this JSON.
     *
     * This method returns a proper string representation that can be written
     * to the file.  Providing this string to the {@link allocWithJson} constructor
     * is guaranteed to make a duplicate of this JSON tree.
     *
     * The JSON may either be pretty-printed or condensed depending on the
     * value of format.  By default, we pretty-print all JSON strings.
     *
     * @param format    Whether to pretty-print the JSON string
     *
     * @return a string representation of this JSON.
     */
    std::string toString(bool format=true) const;

};

}
#endif /* __CU_JSON_VALUE_H__ */

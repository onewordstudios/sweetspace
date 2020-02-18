//
//  CUJsonValue.cpp
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
//  Version: 1/7/18
//
#include <cugl/assets/CUJsonValue.h>
#include <cugl/util/CUDebug.h>
#include <cugl/util/CUStrings.h>

using namespace cugl;

/**
 * Returns the line of JSON with the offending error.
 *
 * Error messages in this library correspond to the tail of the JSON after the error is encounterd.  These
 * can be hard to read if the error happens early.  This function truncates the error to only include the
 * first line of the error.  It also indicates (via the reference variable) the line on which the error occurred.
 *
 * @param data      The JSON value being parsed
 * @param error     The tail of the JSON after encountering an error
 * @param lineno    The variable to store the line number
 *
 * @return the line of JSON with the offending error.
 */
std::string isolate_error(const char* data, const char* error, int& lineno) {
    lineno = 1;
    const char* pos = data;
    while (pos != error) {
        if (*pos == '\n') {
            lineno++;
        }
        pos++;
    }
    size_t len = 0;
    bool goon = true;
    pos = error;
    while (*pos && goon) {
        if (*pos == '\n') {
            goon = false;
        } else {
            len++;
        }
        pos++;
    }
    return std::string(error,len);
}

#pragma mark -
#pragma mark JSON Conversions
/**
 * Returns the JsonValue::Type appropriate for this cJSON node
 *
 * cJSON types are slightly different from JsonValue ones, particular for
 * boolean and strings.  We need this function to convert.
 *
 * @param node  The cJSON node
 *
 * @return the JsonValue::Type appropriate for this cJSON node
 */
JsonValue::Type JsonValueType(const cJSON* node) {
    if (node->type & cJSON_False || node->type & cJSON_True) {
        return JsonValue::Type::BoolType;
    } else if (node->type & cJSON_Number) {
        return JsonValue::Type::NumberType;
    } else if (node->type & cJSON_String) {
        return JsonValue::Type::StringType;
    } else if (node->type & cJSON_NULL) {
        return JsonValue::Type::NullType;
    } else if (node->type & cJSON_Array) {
        return JsonValue::Type::ArrayType;
    } else if (node->type & cJSON_Object) {
        return JsonValue::Type::ObjectType;
    }
    CUAssertLog(false,"Unknown JSON type %d",node->type);
	return JsonValue::Type::NullType;
}

/**
 * Returns the cJSON type appropriate for this JsonValue
 *
 * cJSON types are slightly different from JsonValue ones, particular for
 * boolean and strings.  We need this function to convert.
 *
 * @param value  The JsonValue node
 *
 * @return the cJSON type appropriate for this JsonValue
 */
int cJsonType(const JsonValue* value) {
    switch (value->type()) {
        case JsonValue::Type::NullType:
            return cJSON_NULL;
        case JsonValue::Type::BoolType:
            return (value->asBool() ? cJSON_True : cJSON_False);
        case JsonValue::Type::NumberType:
            return cJSON_Number;
        case JsonValue::Type::StringType:
            return cJSON_String;
        case JsonValue::Type::ArrayType:
            return cJSON_Array;
        case JsonValue::Type::ObjectType:
            return cJSON_Object;
    }
    CUAssertLog(false,"Unknown JSON type %d",value->type());
	return cJSON_NULL;
}

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
std::shared_ptr<JsonValue> JsonValue::toJsonValue(const cJSON* node) {
    std::shared_ptr<JsonValue> result = JsonValue::alloc(JsonValueType(node));

    if (result->_type == Type::BoolType) {
        result->_longValue = node->type & cJSON_True;
    } else {
        result->_longValue = node->valueint;
    }
    result->_doubleValue = node->valuedouble;
    if (node->valuestring) {
        result->_stringValue = node->valuestring;
    }
    if (node->string) {
        result->_key = node->string;
    }
    
    std::vector<std::shared_ptr<JsonValue>> items;
    if (node->child) {
        cJSON* current = node->child;
        while (current) {
            items.push_back(toJsonValue(current));
            items.back()->_parent = result.get();
            current = current->next;
        }
    }
    result->_children.assign(items.begin(),items.end());
    
    return result;
}

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
void JsonValue::toJsonValue(JsonValue* value, const cJSON* node) {
    value->_type = JsonValueType(node);
    if (value->_type == Type::BoolType) {
        value->_longValue = node->type & cJSON_True;
    } else {
        value->_longValue = node->valueint;
    }
    value->_doubleValue = node->valuedouble;
    if (node->valuestring) {
        value->_stringValue = node->valuestring;
    }
    if (node->string) {
        value->_key = node->string;
    }
    
    std::vector<std::shared_ptr<JsonValue>> items;
    if (node->child) {
        cJSON* current = node->child;
        while (current) {
            items.push_back(toJsonValue(current));
            items.back()->_parent = value;
            current = current->next;
        }
    }
    value->_children.assign(items.begin(),items.end());
}

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
cJSON* JsonValue::toCJSON(const JsonValue* value) {
    cJSON* result;
    switch (value->type()) {
        case JsonValue::Type::NullType:
            result = cJSON_CreateNull();
            break;
        case JsonValue::Type::BoolType:
            result = cJSON_CreateBool(value->asBool());
            break;
        case JsonValue::Type::NumberType:
            result = cJSON_CreateNumber(value->asDouble());
            break;
        case JsonValue::Type::StringType:
            result = cJSON_CreateString(value->asString().c_str());
            break;
        case JsonValue::Type::ArrayType:
            result = cJSON_CreateArray();
            break;
        case JsonValue::Type::ObjectType:
            result = cJSON_CreateObject();
            break;
        default:
            CUAssertLog(false,"Unknown JSON type %d",value->type());
    }
    result->type = result->type | cJSON_StringIsConst;
    result->string = (char*)(value->_key.c_str()); // Unsafe, but StringIsConst makes okay.
    
    bool first = true;
    cJSON* prev  = nullptr;
    for(auto it = value->_children.begin(); it != value->_children.end(); ++it) {
        cJSON* current = toCJSON(it->get());
        if (first) {
            result->child = current;
            first = false;
        } else {
            current->prev = prev;
            prev->next = current;
        }
        prev = current;
    }

    return result;
}

#pragma mark -
#pragma mark Constructors
/**
 * Creates a null JsonValue.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
 * the heap, use one of the static constructors instead.
 */
JsonValue::JsonValue() :
_type(Type::NullType),
_parent(nullptr),
_key(""),
_stringValue(""),
_longValue(0L),
_doubleValue(0.0) {
}

/**
 * Deletes this JsonValue and all of its resources.
 *
 * If no other references own the descendants of this node, they will all
 * be recursively deleted as well.
 */
JsonValue::~JsonValue() {
    _children.clear();
    _parent = nullptr;
    _type = Type::NullType;
}

/**
 * Initializes a new JsonValue of the given type.
 *
 * The value of this node will be the default value of the type.
 *
 * @param type  The type of the JSON node.
 *
 * @return  true if the JSON node is initialized properly, false otherwise.
 */
bool JsonValue::init(Type type) {
    _type = type;
    return true;
}

/**
 * Initializes a new JsonValue with the given string.
 *
 * The node will have type StringType.
 *
 * @param value The (string) value of this JSON node.
 *
 * @return  true if the JSON node is initialized properly, false otherwise.
 */
bool JsonValue::init(const std::string& value) {
    _type = Type::StringType;
    _stringValue = value;
    return true;
}

/**
 * Initializes a new JsonValue with the given boolean.
 *
 * The node will have type BoolType.
 *
 * @param value The (boolean) value of this JSON node.
 *
 * @return  true if the JSON node is initialized properly, false otherwise.
 */
bool JsonValue::init(bool value) {
    _type = Type::BoolType;
    _longValue = value;
    return true;
}

/**
 * Initializes a new JsonValue with the given number.
 *
 * The node will have type NumberType.
 *
 * @param value The (numeric) value of this JSON node.
 *
 * @return  true if the JSON node is initialized properly, false otherwise.
 */
bool JsonValue::init(long value) {
    _type = Type::NumberType;
    _longValue = value;
    _doubleValue = (double)value;
    return true;
}

/**
 * Initializes a new JsonValue with the given number.
 *
 * The node will have type NumberType.
 *
 * @param value The (numeric) value of this JSON node.
 *
 * @return  true if the JSON node is initialized properly, false otherwise.
 */
bool JsonValue::init(double value) {
    _type = Type::NumberType;
    _longValue = (long)value;
    _doubleValue = value;
    return true;
}

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
bool JsonValue::initWithJson(const char* json) {
    const char *error = NULL;
    cJSON* node = cJSON_ParseWithOpts(json, &error, 0);
    if (node) {
        toJsonValue(this,node);
        cJSON_Delete(node);
        return true;
    }
    if (error) {
        int line = 0;
        std::string source = isolate_error(json,error,line);
        CUAssertLog(false, "Invalid token at line %d:\n  %s",line,source.c_str());
    } else {
        CUAssertLog(false, "Invalid JSON");
    }
    return false; // If asserts turned off
}


#pragma mark -
#pragma mark Type
/**
 * Returns true if this node is not NULL nor an array or object.
 *
 * @return true if this node is not NULL nor an array or object.
 */
bool JsonValue::isValue() const {
    switch (_type) {
        case Type::StringType:
        case Type::NumberType:
        case Type::BoolType:
            return true;
        default:
            break;
    }
    return false;
}


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
const std::string JsonValue::asString(const char* defaultValue) const {
    CUAssertLog(isValue() || isNull(), "JSON node is not a value type");
    switch (_type) {
        case Type::NullType:
            return std::string("NULL");
        case Type::BoolType:
            return std::string(_longValue ? "true" : "false");
        case Type::NumberType:
            if (_longValue == _doubleValue) {
                return cugl::to_string((Uint64)_longValue);
            } else {
                return cugl::to_string(_doubleValue);
            }
        case Type::StringType:
            return _stringValue;
        default:
            return std::string(defaultValue);
    }
    return std::string(defaultValue);
}

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
float JsonValue::asFloat(float defaultValue) const {
    CUAssertLog(isValue() || isNull(), "JSON node is not a value type");
    if (_type == Type::NumberType) {
        return (float)_doubleValue;
    }
    return defaultValue;
}

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
double JsonValue::asDouble(double defaultValue) const {
    CUAssertLog(isValue() || isNull(), "JSON node is not a value type");
    if (_type == Type::NumberType) {
        return _doubleValue;
    }
    return defaultValue;
}

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
long JsonValue::asLong(long defaultValue) const {
    CUAssertLog(isValue() || isNull(), "JSON node is not a value type");
    if (_type == Type::NumberType) {
        return _longValue;
    }
    return defaultValue;
}

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
int JsonValue::asInt(int defaultValue) const {
    CUAssertLog(isValue() || isNull(), "JSON node is not a value type");
    if (_type == Type::NumberType) {
        return (int)_longValue;
    }
    return defaultValue;
}

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
bool JsonValue::asBool(bool defaultValue) const {
    CUAssertLog(isValue() || isNull(), "JSON node is not a value type");
    if (_type == Type::BoolType) {
        return (bool)_longValue;
    }
    return defaultValue;
}

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
std::vector<std::string> JsonValue::asStringArray(const std::string& defaultValue) const {
	CUAssertLog(_type == Type::ArrayType || _type == Type::ObjectType,
		"Value type cannot be converted to array: %d", _type);
	std::vector<std::string> result;
	for (auto it = _children.begin(); it != _children.end(); it++) {
		std::string value = (*it)->asString();
		result.push_back(value);
	}
	return result;
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
std::vector<float> JsonValue::asFloatArray(float defaultValue) const {
    CUAssertLog(_type == Type::ArrayType || _type == Type::ObjectType,
                "Value type cannot be converted to array: %d", _type);
    std::vector<float> result;
    for(auto it = _children.begin(); it != _children.end(); it++) {
        float value = defaultValue;
        if ((*it)->_type == Type::NumberType) {
            value = (float)(*it)->_doubleValue;
        }
        result.push_back(value);
    }
    return result;
}

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
std::vector<double> JsonValue::asDoubleArray(double defaultValue) const {
    CUAssertLog(_type == Type::ArrayType || _type == Type::ObjectType,
                "Value type cannot be converted to array: %d", _type);
    std::vector<double> result;
    for(auto it = _children.begin(); it != _children.end(); it++) {
        double value = defaultValue;
        if ((*it)->_type == Type::NumberType) {
            value = (*it)->_doubleValue;
        }
        result.push_back(value);        result.push_back((*it)->asDouble(defaultValue));
    }
    return result;
}

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
std::vector<long> JsonValue::asLongArray(long defaultValue) const {
    CUAssertLog(_type == Type::ArrayType || _type == Type::ObjectType,
                "Value type cannot be converted to array: %d", _type);
    std::vector<long> result;
    for(auto it = _children.begin(); it != _children.end(); it++) {
        long value = defaultValue;
        if ((*it)->_type == Type::NumberType) {
            value = (*it)->_longValue;
        }
        result.push_back(value);
    }
    return result;
}

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
std::vector<int> JsonValue::asIntArray(int defaultValue) const {
    CUAssertLog(_type == Type::ArrayType || _type == Type::ObjectType,
                "Value type cannot be converted to array: %d", _type);
    std::vector<int> result;
    for(auto it = _children.begin(); it != _children.end(); it++) {
        int value = defaultValue;
        if ((*it)->_type == Type::NumberType) {
            value = (int)(*it)->_longValue;
        }
        result.push_back(value);
    }
    return result;
}

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
std::vector<bool> JsonValue::asBoolArray (bool defaultValue) {
    CUAssertLog(_type == Type::ArrayType || _type == Type::ObjectType,
                "Value type cannot be converted to array: %d", _type);
    std::vector<bool> result;
    for(auto it = _children.begin(); it != _children.end(); it++) {
        bool value = defaultValue;
        if ((*it)->_type == Type::BoolType) {
            value = (bool)(*it)->_longValue;
        }
        result.push_back(value);
    }
    return result;
}

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
void JsonValue::set(const std::string& value) {
    CUAssertLog(isValue() || isNull(), "JSON node is not a value type");
    _stringValue = value;
    _type = Type::StringType;
}

/**
 * Sets the value of this node to the given number.
 *
 * This method will fail if the node is not a value type or NULL. Using
 * this method will set the type of the node to NumberType.
 *
 * @param value The numeric value to assign
 */
void JsonValue::set(long value) {
    CUAssertLog(isValue() || isNull(), "JSON node is not a value type");
    _doubleValue = (double)value;
    _longValue = value;
    _type = Type::NumberType;
}

/**
 * Sets the value of this node to the given number.
 *
 * This method will fail if the node is not a value type or NULL. Using
 * this method will set the type of the node to NumberType.
 *
 * @param value The numeric value to assign
 */
void JsonValue::set(double value) {
    CUAssertLog(isValue() || isNull(), "JSON node is not a value type");
    _doubleValue = value;
    _longValue = (long)value;
    _type = Type::NumberType;
}

/**
 * Sets the value of this node to the given boolean.
 *
 * This method will fail if the node is not a value type or NULL. Using
 * this method will set the type of the node to BoolType.
 *
 * @param value The boolean value to assign
 */
void JsonValue::set(bool value) {
    CUAssertLog(isValue() || isNull(), "JSON node is not a value type");
    _longValue = value;
    _type = Type::BoolType;
}

/**
 * Sets this node to NULL, erasing all values.
 *
 * This method will fail if the node is not a value type or NULL. Using
 * this method will set the type of the node to NullType.
 */
void JsonValue::setNull() {
    CUAssertLog(isValue() || isNull(), "JSON node is not a value type");
    _type = Type::NullType;
}


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
const std::string& JsonValue::key() const {
    CUAssertLog(_parent, "This node is not part of an object");
    return _key;
}

/**
 * Sets this key for this object value.
 *
 * This method fails if there is no parent or the parent type is not
 * ObjectType. It also fails if this choice of key is not unique.
 *
 * @param key   the key for this object value.
 */
void JsonValue::setKey(const std::string& key) {
    CUAssertLog(_parent, "This node is not part of an object");
    if (_parent) {
        CUAssertLog(!_parent->has(key), "The key %s is already in use", key.c_str());
        _key = key;
    }
}

/**
 * Returns the index for this array/object value.
 *
 * This method fails if there is no parent node.
 *
 * @return the index for this array/object value.
 */
const int JsonValue::index() const {
    CUAssertLog(_parent, "This node is not part of an array");
    int pos = 0;
    for(auto it = _parent->_children.begin(); it != _parent->_children.end(); it++) {
        if ((*it).get() == this) {
            return pos;
        }
        pos++;
    }
    return -1;
}

/**
 * Returns true if a child with the specified name exists.
 *
 * This method will always return false if the node is not an object type
 *
 * @param name  The key identifying the child
 *
 * @return true if a child with the specified name exists.
 */
bool JsonValue::has(const std::string& key) const {
    CUAssertLog(isObject(), "Node is not an object type");
    for(auto it = _children.begin(); it != _children.end(); it++) {
        if ((*it)->_key == key) {
            return true;
        }
    }
    return false;
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
std::shared_ptr<JsonValue> JsonValue::get(int index) {
    CUAssertLog(isArray() || isObject(), "Node is a value type");
    CUAssertLog(0 <= index && index < _children.size(), "Index %d out of range", index);
    return _children[index];
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
const std::shared_ptr<JsonValue> JsonValue::get(int index) const {
    CUAssertLog(isArray() || isObject(), "Node is a value type");
    CUAssertLog(0 <= index && index < _children.size(), "Index %d out of range", index);
    return _children.at(index);
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
std::shared_ptr<JsonValue> JsonValue::get(const std::string& key) {
    CUAssertLog(isObject(), "Node is not an object type");
    for(auto it = _children.begin(); it != _children.end(); it++) {
        if ((*it)->_key == key) {
            return *it;
        }
    }
    return nullptr;
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
const std::shared_ptr<JsonValue> JsonValue::get(const std::string& key) const {
    CUAssertLog(isObject(), "Node is not an object type");
    for(auto it = _children.begin(); it != _children.end(); it++) {
        if ((*it)->_key == key) {
            return *it;
        }
    }
    return nullptr;
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
 * @param defaultValue  The value to use if child does not exist or is not a string
 *
 * @return the string value of the child with the specified key.
 */
const std::string JsonValue::getString (const std::string& key, const std::string& defaultValue) const {
    JsonValue* child = get(key).get();
    bool astr = (child != nullptr && child->isValue());
    return astr ? child->asString(defaultValue) : std::string(defaultValue);
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
 * @param defaultValue  The value to use if child does not exist or is not a number
 *
 * @return the float value of the child with the specified key.
 */
float JsonValue::getFloat(const std::string& key, float defaultValue) const {
    JsonValue* child = get(key).get();
    bool astr = (child != nullptr && child->isNumber());
    return astr ? child->asFloat(defaultValue) : defaultValue;
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
 * @param defaultValue  The value to use if child does not exist or is not a number
 *
 * @return the double value of the child with the specified key.
 */
double JsonValue::getDouble(const std::string& key, double defaultValue) const {
    JsonValue* child = get(key).get();
    bool astr = (child != nullptr && child->isNumber());
    return astr ? child->asFloat(defaultValue) : defaultValue;
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
 * @param defaultValue  The value to use if child does not exist or is not a number
 *
 * @return the long value of the child with the specified key.
 */
long JsonValue::getLong(const std::string& key, long defaultValue) const {
    JsonValue* child = get(key).get();
    bool astr = (child != nullptr && child->isNumber());
    return astr ? child->asLong(defaultValue) : defaultValue;
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
 * @param defaultValue  The value to use if child does not exist or is not a number
 *
 * @return the int value of the child with the specified key.
 */
int JsonValue::getInt (const std::string& key, int defaultValue) const {
    JsonValue* child = get(key).get();
    bool astr = (child != nullptr && child->isNumber());
    return astr ? child->asInt(defaultValue) : defaultValue;
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
 * @param defaultValue  The value to use if child does not exist or is not a boolean
 *
 * @return the boolean value of the child with the specified key.
 */
bool JsonValue::getBool(const std::string& key, bool defaultValue) const {
    JsonValue* child = get(key).get();
    bool astr = (child != nullptr && child->isBool());
    return astr ? child->asBool(defaultValue) : defaultValue;
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
std::shared_ptr<JsonValue> JsonValue::removeChild(int index) {
    CUAssertLog(0 <= index && index < _children.size(), "Index %d out of range", index);
    std::shared_ptr<JsonValue> result = _children[index];
    _children.erase(_children.begin() + index);
    result->_parent = nullptr;
    return result;
}

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
std::shared_ptr<JsonValue> JsonValue::removeChild(const std::string& key) {
    auto jt = _children.end();
    for(auto it = _children.begin(); jt == _children.end() && it != _children.end(); it++) {
        if ((*it)->_key == key) {
            jt = it;
        }
    }
    if (jt != _children.end()) {
        std::shared_ptr<JsonValue> result = *jt;
        _children.erase(jt);
        result->_parent = nullptr;
        return result;
    }
    return nullptr;
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
void JsonValue::appendChild(const std::shared_ptr<JsonValue>& child) {
    CUAssertLog(!child->_parent, "This child already has a parent");
    CUAssertLog(isArray() || isObject(), "This node is a value type");
    CUAssertLog(!has(child->key()), "The key %s is already in use", child->key().c_str());
    _children.push_back(child);
    child->_parent = this;
}

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
void JsonValue::appendChild(const std::string& key, const std::shared_ptr<JsonValue>& child) {
    CUAssertLog(!child->_parent, "This child already has a parent");
    CUAssertLog(isObject(), "Node is not an object type");
    CUAssertLog(!has(key), "The key %s is already in use", key.c_str());
    child->_key = key;
    _children.push_back(child);
    child->_parent = this;
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
void JsonValue::insertChild(unsigned int index, const std::shared_ptr<JsonValue>& child) {
    CUAssertLog(0 <= index && index <= _children.size(), "Index %d out of range", index);
    CUAssertLog(!child->_parent, "This child already has a parent");
    CUAssertLog(isArray() || isObject(), "This node is a value type");
    _children.insert(_children.begin()+index,child);
    child->_parent = this;
}

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
void JsonValue::insertChild(unsigned int index, const std::string& key,
                            const std::shared_ptr<JsonValue>& child) {
    CUAssertLog(0 <= index && index <= _children.size(), "Index %d out of range", index);
    CUAssertLog(!child->_parent, "This child already has a parent");
    CUAssertLog(isObject(), "Node is not an object type");
    CUAssertLog(!has(key), "The key %s is already in use", key.c_str());
    child->_key = key;
    _children.insert(_children.begin()+index,child);
    child->_parent = this;
}


#pragma mark -
#pragma mark Encoding
/**
 * Returns a string representation of this JSON.
 *
 * This method returns a proper string representation that can be written
 * to the file.  Providing this string to the {@link allocJson} constructor
 * is guaranteed to make a duplicate of this JSON tree.
 *
 * The JSON may either be pretty-printed or condensed depending on the
 * value of format.  By default, we pretty-print all JSON strings.
 *
 * @param format    Whether to pretty-print the JSON string
 *
 * @return a string representation of this JSON.
 */
std::string JsonValue::toString(bool format) const {
    cJSON* ast = toCJSON(this);
    char* data = nullptr;
    if (format) {
        data = cJSON_Print(ast);
    } else {
        data = cJSON_PrintUnformatted(ast);
    }
    if (data) {
        std::string result(data);
        
        free(data);
        cJSON_Delete(ast);
        return result;
    }
    return "";
}

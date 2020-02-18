//
//  CUJsonReader.h
//  Cornell University Game Library (CUGL)
//
//  This module provides extends the basic TextReader to support JSON decoding.
//  It does not require that the entire file conform to JSON standards; it can
//  read a JSON string embedded in a larger text file.
//
//  By default, this module (and every module in the io package) accesses the
//  application save directory.  If you want to access another directory, you
//  will need to specify an absolute path for the file name.  Keep in mind that
//  absolute paths are very dangerous on mobile devices, because they do not
//  have proper file systems.  You should confine all files to either the asset
//  or the save directory.
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
//  Author: Walker White
//  Version: 11/28/16
//
#ifndef __CU_JSON_READER_H__
#define __CU_JSON_READER_H__
#include <cugl/io/CUTextReader.h>
#include <cugl/assets/CUJsonValue.h>

namespace  cugl {

/**
 * Simple JSON extension to {@link TextReader}.
 *
 * This class not require that the entire file conform to JSON standards; it
 * can read a JSON string embedded in a larger text file.  This allows for
 * maximum flexibility in encoding/decoding JSON data.
 *
 * By default, this class (and every class in the io package) accesses the
 * application save directory {@see Application#getSaveDirectory()}.  If you
 * want to access another directory, you will need to specify an absolute path
 * for the file name.  Keep in mind that absolute paths are very dangerous on
 * mobile devices, because they do not have proper file systems.  You should
 * confine all files to either the asset or the save directory.
 */
class JsonReader : public TextReader {
    
#pragma mark -
#pragma mark Static Constructors
public:
    /**
     * Returns a newly allocated reader for the given file.
     *
     * The reader will have the default buffer capacity for reading chunks from
     * the file.
     *
     * If the file is a relative path, this reader will look for the file in
     * the application save directory {@see Application#getSaveDirectory()}.
     * If you wish to read a file in any other directory, you must provide
     * an absolute path.
     *
     * @param file  the path (absolute or relative) to the file
     *
     * @return a newly allocated reader for the given file.
     */
    static std::shared_ptr<JsonReader> alloc(const std::string& file) {
        std::shared_ptr<JsonReader> result = std::make_shared<JsonReader>();
        return (result->init(file) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated reader for the given file.
     *
     * The reader will have the default buffer capacity for reading chunks from
     * the file.
     *
     * If the file is a relative path, this reader will look for the file in
     * the application save directory {@see Application#getSaveDirectory()}.
     * If you wish to read a file in any other directory, you must provide
     * an absolute path.
     *
     * @param file  the path (absolute or relative) to the file
     *
     * @return a newly allocated reader for the given file.
     */
    static std::shared_ptr<JsonReader> alloc(const char* file) {
        std::shared_ptr<JsonReader> result = std::make_shared<JsonReader>();
        return (result->init(file) ? result : nullptr);
    }

    /**
     * Returns a newly allocated reader for the given file.
     *
     * The reader will have the default buffer capacity for reading chunks from
     * the file.
     *
     * If the file is a relative path, this reader will look for the file in
     * the application save directory {@see Application#getSaveDirectory()}.
     * If you wish to read a file in any other directory, you must provide
     * an absolute path.
     *
     * @param file  the path (absolute or relative) to the file
     *
     * @return a newly allocated reader for the given file.
     */
    static std::shared_ptr<JsonReader> alloc(const Pathname& file) {
        std::shared_ptr<JsonReader> result = std::make_shared<JsonReader>();
        return (result->init(file) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated reader for the given file with the specified capacity.
     *
     * If the file is a relative path, this reader will look for the file in
     * the application save directory {@see Application#getSaveDirectory()}.
     * If you wish to read a file in any other directory, you must provide
     * an absolute path.
     *
     * @param file      the path (absolute or relative) to the file
     * @param capacity  the buffer capacity for reading chunks
     *
     * @return a newly allocated reader for the given file with the specified capacity.
     */
    static std::shared_ptr<JsonReader> alloc(const std::string& file, unsigned int capacity) {
        std::shared_ptr<JsonReader> result = std::make_shared<JsonReader>();
        return (result->init(file,capacity) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated reader for the given file with the specified capacity.
     *
     * If the file is a relative path, this reader will look for the file in
     * the application save directory {@see Application#getSaveDirectory()}.
     * If you wish to read a file in any other directory, you must provide
     * an absolute path.
     *
     * @param file      the path (absolute or relative) to the file
     * @param capacity  the buffer capacity for reading chunks
     *
     * @return a newly allocated reader for the given file with the specified capacity.
     */
    static std::shared_ptr<JsonReader> alloc(const char* file, unsigned int capacity) {
        std::shared_ptr<JsonReader> result = std::make_shared<JsonReader>();
        return (result->init(file,capacity) ? result : nullptr);
    }

    /**
     * Returns a newly allocated reader for the given file with the specified capacity.
     *
     * If the file is a relative path, this reader will look for the file in
     * the application save directory {@see Application#getSaveDirectory()}.
     * If you wish to read a file in any other directory, you must provide
     * an absolute path.
     *
     * @param file      the path (absolute or relative) to the file
     * @param capacity  the buffer capacity for reading chunks
     *
     * @return a newly allocated reader for the given file with the specified capacity.
     */
    static std::shared_ptr<JsonReader> alloc(const Pathname& file, unsigned int capacity) {
        std::shared_ptr<JsonReader> result = std::make_shared<JsonReader>();
        return (result->init(file,capacity) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated reader for the given file.
     *
     * The reader will have the default buffer capacity for reading chunks from
     * the file.
     *
     * This initializer assumes that the file name is a relative path. It will
     * search the application assert directory {@see Application#getAssetDirectory()}
     * for the file and return false if it cannot find it there.
     *
     * @param file  the relative path to the file
     *
     * @return a newly allocated reader for the given file.
     */
    static std::shared_ptr<JsonReader> allocWithAsset(const std::string& file) {
        std::shared_ptr<JsonReader> result = std::make_shared<JsonReader>();
        return (result->initWithAsset(file) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated reader for the given file.
     *
     * The reader will have the default buffer capacity for reading chunks from
     * the file.
     *
     * This initializer assumes that the file name is a relative path. It will
     * search the application assert directory {@see Application#getAssetDirectory()}
     * for the file and return false if it cannot find it there.
     *
     * @param file  the relative path to the file
     *
     * @return a newly allocated reader for the given file.
     */
    static std::shared_ptr<JsonReader> allocWithAsset(const char* file) {
        std::shared_ptr<JsonReader> result = std::make_shared<JsonReader>();
        return (result->initWithAsset(file) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated reader for the given file with the specified capacity.
     *
     * This initializer assumes that the file name is a relative path. It will
     * search the application assert directory {@see Application#getAssetDirectory()}
     * for the file and return false if it cannot find it there.
     *
     * @param file      the relative path to the file
     
     * @param capacity  the buffer capacity for reading chunks
     *
     * @return a newly allocated reader for the given file with the specified capacity.
     */
    static std::shared_ptr<JsonReader> allocWithAsset(const std::string& file, unsigned int capacity) {
        std::shared_ptr<JsonReader> result = std::make_shared<JsonReader>();
        return (result->initWithAsset(file,capacity) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated reader for the given file with the specified capacity.
     *
     * This initializer assumes that the file name is a relative path. It will
     * search the application assert directory {@see Application#getAssetDirectory()}
     * for the file and return false if it cannot find it there.
     *
     * @param file      the relative path to the file
     
     * @param capacity  the buffer capacity for reading chunks
     *
     * @return a newly allocated reader for the given file with the specified capacity.
     */
    static std::shared_ptr<JsonReader> allocWithAsset(const char* file, unsigned int capacity) {
        std::shared_ptr<JsonReader> result = std::make_shared<JsonReader>();
        return (result->initWithAsset(file,capacity) ? result : nullptr);
    }
    
    
#pragma mark -
#pragma mark Read Methods
    /**
     * Returns the next available JSON string
     *
     * A JSON string is defined to be any string within matching braces {}. This
     * method will skip over any whitespace to find the first brace.  If the
     * first non-whitespace character is not a brace, this method will fail.
     *
     * If the first non-whitespace character is a brace, it will advance until
     * it reaches the matching brace, or the end of the file, whichever is first.
     * If it finds no matching brace, it will fail.
     *
     * @return the next available JSON string
     */
    std::string readJsonString();

    /**
     * Returns a newly allocated JsonValue for the next available JSON string.
     * 
     * This method uses {@link readJsonString()} to extract the next available
     * JSON string and constructs a JsonValue from that.
     *
     * If there is a parsing error, this  method will return nullptr.  Detailed
     * information about the parsing error will be passed to an assert.  Hence
     * error messages are suppressed if asserts are turned off.
     *
     * @return a newly allocated JsonValue for the next available JSON string.
     */
    std::shared_ptr<JsonValue> readJson();
    
};

}
#endif /* __CU_JSON_READER_H__ */

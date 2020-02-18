//
//  CUTextReader.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a simple Java-style reader for reading from text files.
//  It supports both ASCII and UTF8 encoding. No other encodings are supported
//  (nor should they be since they are not cross-platform).
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
//  Version: 11/22/16
//
#ifndef __CU_TEXT_READER_H__
#define __CU_TEXT_READER_H__
#include <cugl/base/CUBase.h>
#include <SDL/SDL.h>
#include <string>
#include "CUPathname.h"

namespace  cugl {

#pragma mark -
#pragma mark TextReader

/**
 * Simple text-based reader for ASCII or UTF8 files.
 *
 * This class provides a simple Java-style reader for reading from text files.
 * It supports both ASCII and UTF8 encoding. No other encodings are supported
 * (nor should they be since they are not cross-platform).
 *
 * By default, this class (and every class in the io package) accesses the
 * application save directory {@see Application#getSaveDirectory()}.  If you
 * want to access another directory, you will need to specify an absolute path 
 * for the file name.  Keep in mind that absolute paths are very dangerous on 
 * mobile devices, because they do not have proper file systems.  You should 
 * confine all files to either the asset or the save directory.
 */
class TextReader {
protected:
    /** The (full) path for the file */
    std::string _name;
    /** The SDL I/O stream for reading */
    SDL_RWops*  _stream;
    /** The SDL I/O stream size */
    Sint64      _ssize;
    /** The cursor into the SDL I/O stream */
    Sint64      _scursor;
    
    /** The buffer for storing data read from the stream */
    std::string _sbuffer;
    /** The temporary transfer buffer */
    char*       _cbuffer;
    /** The buffer capacity */
    Uint32      _capacity;
    /** The current offset in the read buffer */
    Sint32      _bufoff;

#pragma mark -
#pragma mark Internal Methods
    /**
     * Fills the storage buffer to capacity
     *
     * This cuts down on the number of reads to the file by allowing us
     * to read from the file in predefined chunks.
     */
    void fill();
    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates a text reader with no assigned file.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    TextReader() : _name(""), _stream(nullptr), _ssize(-1), _scursor(-1),
                   _sbuffer(""), _cbuffer(nullptr), _capacity(0), _bufoff(-1) {}
    
    /**
     * Deletes this reader and all of its resources.
     *
     * Calls to the destructor will close the file if it is not already closed.
     */
    ~TextReader() { close(); }
    
    /**
     * Initializes a reader for the given file.
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
     * @return true if the reader is initialized properly, false otherwise.
     */
    bool init(const std::string& file) {
        return init(Pathname(file));
    }

    /**
     * Initializes a reader for the given file.
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
     * @return true if the reader is initialized properly, false otherwise.
     */
    bool init(const char* file)  {
        return init(Pathname(file));
    }

    /**
     * Initializes a reader for the given file.
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
     * @return true if the reader is initialized properly, false otherwise.
     */
    bool init(const Pathname& file);

    /**
     * Initializes a reader for the given file with the specified capacity.
     *
     * If the file is a relative path, this reader will look for the file in
     * the application save directory {@see Application#getSaveDirectory()}.
     * If you wish to read a file in any other directory, you must provide
     * an absolute path.
     *
     * @param file      the path (absolute or relative) to the file
     * @param capacity  the buffer capacity for reading chunks
     *
     * @return true if the reader is initialized properly, false otherwise.
     */
    bool init(const std::string& file, unsigned int capacity) {
        return init(Pathname(file),capacity);
    }
    
    /**
     * Initializes a reader for the given file with the specified capacity.
     *
     * If the file is a relative path, this reader will look for the file in
     * the application save directory {@see Application#getSaveDirectory()}.
     * If you wish to read a file in any other directory, you must provide
     * an absolute path.
     *
     * @param file      the path (absolute or relative) to the file
     * @param capacity  the buffer capacity for reading chunks
     *
     * @return true if the reader is initialized properly, false otherwise.
     */
    bool init(const char* file, unsigned int capacity)  {
        return init(Pathname(file),capacity);
    }

    /**
     * Initializes a reader for the given file with the specified capacity.
     *
     * If the file is a relative path, this reader will look for the file in
     * the application save directory {@see Application#getSaveDirectory()}.
     * If you wish to read a file in any other directory, you must provide
     * an absolute path.
     *
     * @param file      the path (absolute or relative) to the file
     * @param capacity  the buffer capacity for reading chunks
     *
     * @return true if the reader is initialized properly, false otherwise.
     */
    bool init(const Pathname& file, unsigned int capacity);

    /**
     * Initializes a reader for the given file.
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
     * @return true if the reader is initialized properly, false otherwise.
     */
    bool initWithAsset(const std::string& file) {
        return initWithAsset(file.c_str());
    }
    
    /**
     * Initializes a reader for the given file.
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
     * @return true if the reader is initialized properly, false otherwise.
     */
    bool initWithAsset(const char* file);
    
    /**
     * Initializes a reader for the given file with the specified capacity.
     *
     * This initializer assumes that the file name is a relative path. It will
     * search the application assert directory {@see Application#getAssetDirectory()}
     * for the file and return false if it cannot find it there.
     *
     * @param file      the relative path to the file
     * @param capacity  the buffer capacity for reading chunks
     *
     * @return true if the reader is initialized properly, false otherwise.
     */
    bool initWithAsset(const std::string& file, unsigned int capacity) {
        return initWithAsset(file.c_str(),capacity);
    }
    
    /**
     * Initializes a reader for the given file with the specified capacity.
     *
     * This initializer assumes that the file name is a relative path. It will
     * search the application assert directory {@see Application#getAssetDirectory()}
     * for the file and return false if it cannot find it there.
     *
     * @param file      the relative path to the file
     * @param capacity  the buffer capacity for reading chunks
     *
     * @return true if the reader is initialized properly, false otherwise.
     */
    bool initWithAsset(const char* file, unsigned int capacity);
    
    
#pragma mark -
#pragma mark Static Constructors
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
    static std::shared_ptr<TextReader> alloc(const std::string& file) {
        std::shared_ptr<TextReader> result = std::make_shared<TextReader>();
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
    static std::shared_ptr<TextReader> alloc(const char* file) {
        std::shared_ptr<TextReader> result = std::make_shared<TextReader>();
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
    static std::shared_ptr<TextReader> alloc(const Pathname& file) {
        std::shared_ptr<TextReader> result = std::make_shared<TextReader>();
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
    static std::shared_ptr<TextReader> alloc(const std::string& file, unsigned int capacity) {
        std::shared_ptr<TextReader> result = std::make_shared<TextReader>();
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
    static std::shared_ptr<TextReader> alloc(const char* file, unsigned int capacity) {
        std::shared_ptr<TextReader> result = std::make_shared<TextReader>();
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
    static std::shared_ptr<TextReader> alloc(const Pathname& file, unsigned int capacity) {
        std::shared_ptr<TextReader> result = std::make_shared<TextReader>();
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
    static std::shared_ptr<TextReader> allocWithAsset(const std::string& file) {
        std::shared_ptr<TextReader> result = std::make_shared<TextReader>();
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
    static std::shared_ptr<TextReader> allocWithAsset(const char* file) {
        std::shared_ptr<TextReader> result = std::make_shared<TextReader>();
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
    static std::shared_ptr<TextReader> allocWithAsset(const std::string& file, unsigned int capacity) {
        std::shared_ptr<TextReader> result = std::make_shared<TextReader>();
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
    static std::shared_ptr<TextReader> allocWithAsset(const char* file, unsigned int capacity) {
        std::shared_ptr<TextReader> result = std::make_shared<TextReader>();
        return (result->initWithAsset(file,capacity) ? result : nullptr);
    }

    
#pragma mark -
#pragma mark Stream Management
    /**
     * Resets the stream back to the beginning
     *
     * This allows the stream to be read a second time.  It may even be called
     * if the stream has been closed.
     */
    void reset();
    
    /**
     * Closes the stream, releasing all resources
     *
     * Any attempts to read from a closed stream will fail.  Calling this method
     * on a previously closed stream has no effect.
     */
    void close();
    
    /**
     * Returns true if there is still data to read.
     *
     * This method will return false if the stream is closed.
     *
     * @return true if there is still data to read
     */
    bool ready() const { return _bufoff < _sbuffer.size() || _scursor < _ssize; }
    
    
#pragma mark -
#pragma mark Read Methods
    /**
     * Returns a single ASCII character from the stream
     *
     * The value returned is a single byte character.  This means that it is
     * not safe to call this method on UTF8 files, as the value returned may
     * be a control point and not a complete character.
     *
     * @return a single ASCII character from the stream
     */
    char read();

    /**
     * Returns the argument with an ASCII character appended from the stream.
     *
     * The argument is modified to receive the newly read data at the end.
     *
     * The value appended is a single byte character.  This means that it is
     * not safe to call this method on UTF8 files, as the value returned may
     * be a control point and not a complete character.
     *
     * @param data  the argument to modify
     *
     * @return the argument with an ASCII character appended from the stream.
     */
    std::string& read(std::string& data);

    /**
     * Returns a single UTF8 character from the stream
     *
     * Because of the way UTF8 values are encoded, the resulting value may be
     * anywhere from 1 to 4 bytes.  That is why the result is returned as a
     * variable length string.
     *
     * @return a single UTF8 character from the stream
     */
    std::string readUTF8();
    
    /**
     * Returns the argument with a UTF8 character appended from the stream.
     *
     * The argument is modified to receive the newly read data at the end.
     *
     * Because of the way UTF8 values are encoded, the resulting value may be
     * anywhere from 1 to 4 bytes.
     *
     * @param data  the argument to modify
     *
     * @return the argument with a UTF8 character appended from the stream.
     */
    std::string& readUTF8(std::string& data);
    
    /**
     * Returns a single line for text from the stream
     *
     * A line of text is indicated by the newline character '\n', regardless 
     * of the platform.  Therefore, this class should not be used to read
     * files with Windows-style line feeds.  If the reader reaches the end
     * of the file without encountering a newline, it will return the remainder
     * of the file.
     *
     * @return a single line for text from the stream
     */
    std::string readLine();

    /**
     * Returns the argument with a single line appended from the stream.
     *
     * The argument is modified to receive the newly read data at the end.
     *
     * A line of text is indicated by the newline character '\n', regardless
     * of the platform.  Therefore, this class should not be used to read
     * files with Windows-style line feeds.  If the reader reaches the end
     * of the file without encountering a newline, it will append the remainder
     * of the file.
     *
     * @param data  the argument to modify
     *
     * @return the argument with a single line appended from the stream.
     */
    std::string& readLine(std::string& data);
    
    /**
     * Returns the unread remainder of the stream
     *
     * This method will exhaust the stream of all characters, but will not
     * close it.  Future attempts to read the stream will fail.
     *
     * @return the unread remainder of the stream
     */
    std::string readAll();
    
    /**
     * Returns the argument with the remainder of the stream appended.
     *
     * The argument is modified to receive the newly read data at the end.
     *
     * This method will exhaust the stream of all characters, but will not
     * close it.  Future attempts to read the stream will fail.
     *
     * @param data  the argument to modify
     *
     * @return the argument with the remainder of the stream appended.
     */
    std::string& readAll(std::string& data);

    /**
     * Skips over any whitespace in the stream.
     *
     * This method will move the read head until it reaches a non-whitespace
     * character or the end of the file, which ever comes first.
     */
    void skip();
};

}
#endif /* __CU_TEXT_READER_H__ */

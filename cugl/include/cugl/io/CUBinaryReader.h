//
//  CUBinaryReader.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a simple Java-style reader for decoding binary files.
//  All data is marshalled from network order, ensuring that the files are
//  supported across multiple platforms.
//
//  Note that this reader does not refer to the integral types as short, int,
//  long, etc.  Those types are NOT cross-platform.  For example, a long is
//  8 bytes on Unix/OS X, but 4 bytes on Win32 platforms.
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
#ifndef __CU_BINARY_READER_H__
#define __CU_BINARY_READER_H__
#include <cugl/base/CUBase.h>
#include <SDL/SDL.h>
#include <cugl/io/CUPathname.h>
#include <string>

namespace cugl {

/**
 * Simple cross-platform reader for binary files.
 *
 * This class provides a simple Java-style reader for decoding binary files.
 * All data is marshalled from network order, ensuring that the files are
 * supported across multiple platforms.
 *
 * Note that this reader does not refer to the integral types as short, int,
 * long, etc.  Those types are NOT cross-platform.  For example, a long is
 * 8 bytes on Unix/OS X, but 4 bytes on Win32 platforms.
 *
 * By default, this class (and every class in the io package) accesses the
 * application save directory {@see Application#getSaveDirectory()}.  If you
 * want to access another directory, you will need to specify an absolute path
 * for the file name.  Keep in mind that absolute paths are very dangerous on
 * mobile devices, because they do not have proper file systems.  You should
 * confine all files to either the asset or the save directory.
 */
class BinaryReader {
protected:
    /** The (full) path for the file */
    std::string _name;
    /** The SDL I/O stream for reading */
    SDL_RWops*  _stream;
    /** The SDL I/O stream size */
    Sint64      _ssize;
    /** The cursor into the SDL I/O stream */
    Sint64      _scursor;
    
    /** The temporary transfer buffer */
    char*       _buffer;
    /** The buffer capacity */
    Uint32      _capacity;
    /** The buffer capacity */
    Uint32      _bufsize;
    /** The current offset in the read buffer */
    Sint32      _bufoff;
    
#pragma mark -
#pragma mark Internal Methods
    /**
     * Fills the storage buffer to capacity
     *
     * This cuts down on the number of reads to the file by allowing us
     * to read from the file in predefined chunks.
     *
     * @param bytes The minimum number of bytes to ensure in the stream
     */
    void fill(unsigned int bytes=1);
    
    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates a binary reader with no assigned file.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    BinaryReader() : _name(""), _stream(nullptr), _ssize(-1), _scursor(-1),
                     _buffer(nullptr), _capacity(0), _bufoff(-1), _bufsize(0) {}
    
    /**
     * Deletes this reader and all of its resources.
     *
     * Calls to the destructor will close the file if it is not already closed.
     */
    ~BinaryReader() { close(); }
    
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
    bool init(const char* file) {
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
    bool init(const char* file, unsigned int capacity) {
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
        return init(file.c_str());
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
        return init(file.c_str(),capacity);
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
    static std::shared_ptr<BinaryReader> alloc(const std::string& file) {
        std::shared_ptr<BinaryReader> result = std::make_shared<BinaryReader>();
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
    static std::shared_ptr<BinaryReader> alloc(const char* file) {
        std::shared_ptr<BinaryReader> result = std::make_shared<BinaryReader>();
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
    static std::shared_ptr<BinaryReader> alloc(const std::string& file, unsigned int capacity) {
        std::shared_ptr<BinaryReader> result = std::make_shared<BinaryReader>();
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
    static std::shared_ptr<BinaryReader> alloc(const char* file, unsigned int capacity) {
        std::shared_ptr<BinaryReader> result = std::make_shared<BinaryReader>();
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
    static std::shared_ptr<BinaryReader> allocWithAsset(const std::string& file) {
        std::shared_ptr<BinaryReader> result = std::make_shared<BinaryReader>();
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
    static std::shared_ptr<BinaryReader> allocWithAsset(const char* file) {
        std::shared_ptr<BinaryReader> result = std::make_shared<BinaryReader>();
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
    static std::shared_ptr<BinaryReader> allocWithAsset(const std::string& file, unsigned int capacity) {
        std::shared_ptr<BinaryReader> result = std::make_shared<BinaryReader>();
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
    static std::shared_ptr<BinaryReader> allocWithAsset(const char* file, unsigned int capacity) {
        std::shared_ptr<BinaryReader> result = std::make_shared<BinaryReader>();
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
     * This method will return false if the stream is closed, or if there are
     * too few bytes remaining.
     *
     * @param bytes The number of bytes required
     *
     * @return true if there is enough data left to read
     */
    bool ready(unsigned int bytes=1) const;
    
    
#pragma mark -
#pragma mark Single Element Reads
    /**
     * Returns a single character from the stream
     *
     * @return a single character from the stream
     */
    char readChar();

    /**
     * Returns a single byte from the stream
     *
     * @return a single byte from the stream
     */
    Uint8 readByte();
    
    /**
     * Returns a single 16 bit signed integer from the stream
     *
     * The value is marshalled from network order, ensuring that the binary file
     * is compatible against all platforms.
     *
     * @return a single 16 bit signed integer from the stream
     */
    Sint16 readSint16();

    /**
     * Returns a single 16 bit unsigned integer from the stream
     *
     * The value is marshalled from network order, ensuring that the binary file
     * is compatible against all platforms.
     *
     * @return a single 16 bit unsigned integer from the stream
     */
    Uint16 readUint16();

    /**
     * Returns a single 32 bit signed integer from the stream
     *
     * The value is marshalled from network order, ensuring that the binary file
     * is compatible against all platforms.
     *
     * @return a single 32 bit signed integer from the stream
     */
    Sint32 readSint32();
    
    /**
     * Returns a single 32 bit unsigned integer from the stream
     *
     * The value is marshalled from network order, ensuring that the binary file
     * is compatible against all platforms.
     *
     * @return a single 32 bit unsigned integer from the stream
     */
    Uint32 readUint32();
    
    /**
     * Returns a single 32 bit signed integer from the stream
     *
     * The value is marshalled from network order, ensuring that the binary file
     * is compatible against all platforms.
     *
     * @return a single 32 bit signed integer from the stream
     */
    Sint64 readSint64();
    
    /**
     * Returns a single 32 bit unsigned integer from the stream
     *
     * The value is marshalled from network order, ensuring that the binary file
     * is compatible against all platforms.
     *
     * @return a single 32 bit unsigned integer from the stream
     */
    Uint64 readUint64();
    
    /**
     * Returns a single float from the stream
     *
     * The value is marshalled from network order, ensuring that the binary file
     * is compatible against all platforms.
     *
     * @return a single float from the stream
     */
    float readFloat();
    
    /**
     * Returns a single double from the stream
     *
     * The value is marshalled from network order, ensuring that the binary file
     * is compatible against all platforms.
     *
     * @return a single double from the stream
     */
    double readDouble();
    
    
#pragma mark -
#pragma mark Array Reads
    /**
     * Reads a sequence of characters from the stream.
     *
     * The function will attempt to read up to maximum number of elements.
     * It will return the actual number of elements read (which may be 0).
     *
     * @param buffer    The array to store the data when read
     * @param maximum   The maximum number of elements to read from the stream
     * @param offset    The offset to start in the buffer array
     *
     * @return the number of characters read from the stream
     */
    size_t read(char* buffer, size_t maximum, size_t offset=0);

    /**
     * Reads a sequence of bytes from the stream.
     *
     * The function will attempt to read up to maximum number of elements.
     * It will return the actual number of elements read (which may be 0).
     *
     * @param buffer    The array to store the data when read
     * @param maximum   The maximum number of elements to read from the stream
     * @param offset    The offset to start in the buffer array
     *
     * @return the number of bytes read from the stream
     */
    size_t read(Uint8* buffer, size_t maximum, size_t offset=0);

    /**
     * Reads a sequence of 16 bit signed integers from the stream.
     *
     * The function will attempt to read up to maximum number of elements.
     * It will return the actual number of elements read (which may be 0).
     *
     * The values are marshalled from network order, ensuring that the binary 
     * file is compatible against all platforms.
     *
     * @param buffer    The array to store the data when read
     * @param maximum   The maximum number of elements to read from the stream
     * @param offset    The offset to start in the buffer array
     *
     * @return the number of 16 bit signed integers read from the stream
     */
    size_t read(Sint16* buffer, size_t maximum, size_t offset=0);

    /**
     * Reads a sequence of 16 bit unsigned integers from the stream.
     *
     * The function will attempt to read up to maximum number of elements.
     * It will return the actual number of elements read (which may be 0).
     *
     * The values are marshalled from network order, ensuring that the binary
     * file is compatible against all platforms.
     *
     * @param buffer    The array to store the data when read
     * @param maximum   The maximum number of elements to read from the stream
     * @param offset    The offset to start in the buffer array
     *
     * @return the number of 16 bit unsigned integers read from the stream
     */
    size_t read(Uint16* buffer, size_t maximum, size_t offset=0);

    /**
     * Reads a sequence of 32 bit signed integers from the stream.
     *
     * The function will attempt to read up to maximum number of elements.
     * It will return the actual number of elements read (which may be 0).
     *
     * The values are marshalled from network order, ensuring that the binary
     * file is compatible against all platforms.
     *
     * @param buffer    The array to store the data when read
     * @param maximum   The maximum number of elements to read from the stream
     * @param offset    The offset to start in the buffer array
     *
     * @return the number of 32 bit signed integers read from the stream
     */
    size_t read(Sint32* buffer, size_t maximum, size_t offset=0);
    
    /**
     * Reads a sequence of 32 bit unsigned integers from the stream.
     *
     * The function will attempt to read up to maximum number of elements.
     * It will return the actual number of elements read (which may be 0).
     *
     * The values are marshalled from network order, ensuring that the binary
     * file is compatible against all platforms.
     *
     * @param buffer    The array to store the data when read
     * @param maximum   The maximum number of elements to read from the stream
     * @param offset    The offset to start in the buffer array
     *
     * @return the number of 32 bit unsigned integers read from the stream
     */
    size_t read(Uint32* buffer, size_t maximum, size_t offset=0);
    
    /**
     * Reads a sequence of 32 bit signed integers from the stream.
     *
     * The function will attempt to read up to maximum number of elements.
     * It will return the actual number of elements read (which may be 0).
     *
     * The values are marshalled from network order, ensuring that the binary
     * file is compatible against all platforms.
     *
     * @param buffer    The array to store the data when read
     * @param maximum   The maximum number of elements to read from the stream
     * @param offset    The offset to start in the buffer array
     *
     * @return the number of 32 bit signed integers read from the stream
     */
    size_t read(Sint64* buffer, size_t maximum, size_t offset=0);
    
    /**
     * Reads a sequence of 32 bit unsigned integers from the stream.
     *
     * The function will attempt to read up to maximum number of elements.
     * It will return the actual number of elements read (which may be 0).
     *
     * The values are marshalled from network order, ensuring that the binary
     * file is compatible against all platforms.
     *
     * @param buffer    The array to store the data when read
     * @param maximum   The maximum number of elements to read from the stream
     * @param offset    The offset to start in the buffer array
     *
     * @return the number of 32 bit unsigned integers read from the stream
     */
    size_t read(Uint64* buffer, size_t maximum, size_t offset=0);

    /**
     * Reads a sequence of floats from the stream.
     *
     * The function will attempt to read up to maximum number of elements.
     * It will return the actual number of elements read (which may be 0).
     *
     * The values are marshalled from network order, ensuring that the binary
     * file is compatible against all platforms.
     *
     * @param buffer    The array to store the data when read
     * @param maximum   The maximum number of elements to read from the stream
     * @param offset    The offset to start in the buffer array
     *
     * @return the number of floats read from the stream
     */
    size_t read(float* buffer, size_t maximum, size_t offset=0);
    
    /**
     * Reads a sequence of doubles from the stream.
     *
     * The function will attempt to read up to maximum number of elements.
     * It will return the actual number of elements read (which may be 0).
     *
     * The values are marshalled from network order, ensuring that the binary
     * file is compatible against all platforms.
     *
     * @param buffer    The array to store the data when read
     * @param maximum   The maximum number of elements to read from the stream
     * @param offset    The offset to start in the buffer array
     *
     * @return the number of doubles read from the stream
     */
    size_t read(double* buffer, size_t maximum, size_t offset=0);
};

}
#endif /* __CU_BINARY_READER_H__ */

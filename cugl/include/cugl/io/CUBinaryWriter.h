//
//  CUBinaryWriter.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a simple Java-style writer for encoding binary files.
//  All data is marshalled to network order, ensuring that the files are the
//  same across multiple platforms.
//
//  Note that this writer does not refer to the integral types as short, int,
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
#ifndef __CU_BINARY_WRITER_H__
#define __CU_BINARY_WRITER_H__
#include <cugl/base/CUBase.h>
#include <SDL/SDL.h>
#include <cugl/io/CUPathname.h>
#include <string>

namespace cugl {
    
/**
 * Simple cross-platform writer for binary files.
 *
 * This class provides a simple Java-style writer for encoding binary files.
 * All data is marshalled to network order, ensuring that the files are the
 * same across multiple platforms.
 *
 * Note that this writer does not refer to the integral types as short, int,
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
class BinaryWriter {
protected:
    /** The (full) path for the file */
    std::string _name;
    /** The SDL I/O stream for writing */
    SDL_RWops*  _stream;
    
    /** The buffer for cutting down on file access */
    char*       _cbuffer;
    /** The buffer capacity */
    Uint32      _capacity;
    /** The current offset in the writer buffer */
    Sint32      _bufoff;

    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates a binary writer with no assigned file.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    BinaryWriter() : _name(""), _stream(nullptr), _cbuffer(nullptr), _bufoff(-1) {}
    
    /**
     * Deletes this writer and all of its resources.
     *
     * Calls to the destructor will close the file if it is not already closed.
     */
    ~BinaryWriter() { close(); }
    
    /**
     * Initializes a writer for the given file.
     *
     * The writer will have the default buffer capacity for writing chunks to
     * the file.
     *
     * If the file is a relative path, this reader will look for the file in
     * the application save directory {@see Application#getSaveDirectory()}.
     * If you wish to write a file in any other directory, you must provide
     * an absolute path. Be warned, however, that write priviledges are
     * heavily restricted on mobile platforms.
     *
     * @param file  the path (absolute or relative) to the file
     *
     * @return true if the writer is initialized properly, false otherwise.
     */
    bool init(const std::string& file) {
        return init(Pathname(file));
    }
    
    /**
     * Initializes a writer for the given file.
     *
     * The writer will have the default buffer capacity for writing chunks to
     * the file.
     *
     * If the file is a relative path, this reader will look for the file in
     * the application save directory {@see Application#getSaveDirectory()}.
     * If you wish to write a file in any other directory, you must provide
     * an absolute path. Be warned, however, that write priviledges are
     * heavily restricted on mobile platforms.
     *
     * @param file  the path (absolute or relative) to the file
     *
     * @return true if the writer is initialized properly, false otherwise.
     */
    bool init(const char* file) {
        return init(Pathname(file));
    }

    /**
     * Initializes a writer for the given file.
     *
     * The writer will have the default buffer capacity for writing chunks to
     * the file.
     *
     * If the file is a relative path, this reader will look for the file in
     * the application save directory {@see Application#getSaveDirectory()}.
     * If you wish to write a file in any other directory, you must provide
     * an absolute path. Be warned, however, that write priviledges are
     * heavily restricted on mobile platforms.
     *
     * @param file  the path (absolute or relative) to the file
     *
     * @return true if the writer is initialized properly, false otherwise.
     */
    bool init(const Pathname& file);

    /**
     * Initializes a writer for the given file with the specified capacity.
     *
     * If the file is a relative path, this reader will look for the file in
     * the application save directory {@see Application#getSaveDirectory()}.
     * If you wish to write a file in any other directory, you must provide
     * an absolute path. Be warned, however, that write priviledges are
     * heavily restricted on mobile platforms.
     *
     * @param file      the path (absolute or relative) to the file
     * @param capacity  the buffer capacity for reading chunks
     *
     * @return true if the writer is initialized properly, false otherwise.
     */
    bool init(const std::string& file, unsigned int capacity) {
        return init(Pathname(file),capacity);
    }
    
    /**
     * Initializes a writer for the given file with the specified capacity.
     *
     * If the file is a relative path, this reader will look for the file in
     * the application save directory {@see Application#getSaveDirectory()}.
     * If you wish to write a file in any other directory, you must provide
     * an absolute path. Be warned, however, that write priviledges are
     * heavily restricted on mobile platforms.
     *
     * @param file      the path (absolute or relative) to the file
     * @param capacity  the buffer capacity for reading chunks
     *
     * @return true if the writer is initialized properly, false otherwise.
     */
    bool init(const char* file, unsigned int capacity) {
        return init(Pathname(file),capacity);
    }

    /**
     * Initializes a writer for the given file with the specified capacity.
     *
     * If the file is a relative path, this reader will look for the file in
     * the application save directory {@see Application#getSaveDirectory()}.
     * If you wish to write a file in any other directory, you must provide
     * an absolute path. Be warned, however, that write priviledges are
     * heavily restricted on mobile platforms.
     *
     * @param file      the path (absolute or relative) to the file
     * @param capacity  the buffer capacity for reading chunks
     *
     * @return true if the writer is initialized properly, false otherwise.
     */
    bool init(const Pathname& file, unsigned int capacity);
    
    
#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a newly allocated writer for the given file.
     *
     * The writer will have the default buffer capacity for writing chunks to
     * the file.
     *
     * If the file is a relative path, this reader will look for the file in
     * the application save directory {@see Application#getSaveDirectory()}.
     * If you wish to write a file in any other directory, you must provide
     * an absolute path. Be warned, however, that write priviledges are
     * heavily restricted on mobile platforms.
     *
     * @param file  the path (absolute or relative) to the file
     *
     * @return a newly allocated writer for the given file.
     */
    static std::shared_ptr<BinaryWriter> alloc(const std::string& file) {
        std::shared_ptr<BinaryWriter> result = std::make_shared<BinaryWriter>();
        return (result->init(file) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated writer for the given file.
     *
     * The writer will have the default buffer capacity for writing chunks to
     * the file.
     *
     * If the file is a relative path, this reader will look for the file in
     * the application save directory {@see Application#getSaveDirectory()}.
     * If you wish to write a file in any other directory, you must provide
     * an absolute path. Be warned, however, that write priviledges are
     * heavily restricted on mobile platforms.
     *
     * @param file  the path (absolute or relative) to the file
     *
     * @return a newly allocated writer for the given file.
     */
    static std::shared_ptr<BinaryWriter> alloc(const char* file) {
        std::shared_ptr<BinaryWriter> result = std::make_shared<BinaryWriter>();
        return (result->init(file) ? result : nullptr);
    }

    /**
     * Returns a newly allocated writer for the given file.
     *
     * The writer will have the default buffer capacity for writing chunks to
     * the file.
     *
     * If the file is a relative path, this reader will look for the file in
     * the application save directory {@see Application#getSaveDirectory()}.
     * If you wish to write a file in any other directory, you must provide
     * an absolute path. Be warned, however, that write priviledges are
     * heavily restricted on mobile platforms.
     *
     * @param file  the path (absolute or relative) to the file
     *
     * @return a newly allocated writer for the given file.
     */
    static std::shared_ptr<BinaryWriter> alloc(const Pathname& file) {
        std::shared_ptr<BinaryWriter> result = std::make_shared<BinaryWriter>();
        return (result->init(file) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated writer for the given file with the specified capacity.
     *
     * If the file is a relative path, this reader will look for the file in
     * the application save directory {@see Application#getSaveDirectory()}.
     * If you wish to write a file in any other directory, you must provide
     * an absolute path. Be warned, however, that write priviledges are
     * heavily restricted on mobile platforms.
     *
     * @param file      the path (absolute or relative) to the file
     * @param capacity  the buffer capacity for reading chunks
     *
     * @return a newly allocated writer for the given file with the specified capacity.
     */
    static std::shared_ptr<BinaryWriter> alloc(const std::string& file, unsigned int capacity) {
        std::shared_ptr<BinaryWriter> result = std::make_shared<BinaryWriter>();
        return (result->init(file,capacity) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated writer for the given file with the specified capacity.
     *
     * If the file is a relative path, this reader will look for the file in
     * the application save directory {@see Application#getSaveDirectory()}.
     * If you wish to write a file in any other directory, you must provide
     * an absolute path. Be warned, however, that write priviledges are
     * heavily restricted on mobile platforms.
     *
     * @param file      the path (absolute or relative) to the file
     * @param capacity  the buffer capacity for reading chunks
     *
     * @return a newly allocated writer for the given file with the specified capacity.
     */
    static std::shared_ptr<BinaryWriter> alloc(const char* file, unsigned int capacity) {
        std::shared_ptr<BinaryWriter> result = std::make_shared<BinaryWriter>();
        return (result->init(file,capacity) ? result : nullptr);
    }

    /**
     * Returns a newly allocated writer for the given file with the specified capacity.
     *
     * If the file is a relative path, this reader will look for the file in
     * the application save directory {@see Application#getSaveDirectory()}.
     * If you wish to write a file in any other directory, you must provide
     * an absolute path. Be warned, however, that write priviledges are
     * heavily restricted on mobile platforms.
     *
     * @param file      the path (absolute or relative) to the file
     * @param capacity  the buffer capacity for reading chunks
     *
     * @return a newly allocated writer for the given file with the specified capacity.
     */
    static std::shared_ptr<BinaryWriter> alloc(const Pathname& file, unsigned int capacity) {
        std::shared_ptr<BinaryWriter> result = std::make_shared<BinaryWriter>();
        return (result->init(file,capacity) ? result : nullptr);
    }
    
    
#pragma mark -
#pragma mark Stream Management
    /**
     * Flushes the contents of the write buffer to the file.
     *
     * It is usually unnecessary to call this method. It is called automatically
     * when the buffer fills, or just before the file is closed.
     */
    void flush();
    
    /**
     * Closes the stream, releasing all resources
     *
     * The contents of the buffer are flushed before the file is closed.  Any
     * attempts to write to a closed stream will fail.  Calling this method
     * on a previously closed stream has no effect.
     */
    void close();


#pragma mark -
#pragma mark Single Element Writes
    /**
     * Writes a character to the binary file.
     *
     * The value is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param c  the character to write
     */
    void write(char c);

    /**
     * Writes a byte to the binary file.
     *
     * The value is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param c  the byte to write
     */
    void writeUint8(Uint8 c);
    
    /**
     * Writes a single 16 bit signed integer to the binary file.
     *
     * The value is marshalled to network order, ensuring that the binary file
     * is compatible against all platforms.
     *
     * The value is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param n  the 16 bit signed integer to write
     */
    void writeSint16(Sint16 n);

    /**
     * Writes a single 16 bit unsigned integer to the binary file.
     *
     * The value is marshalled to network order, ensuring that the binary file
     * is compatible against all platforms.
     *
     * The value is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param n  the 16 bit unsigned integer to write
     */
    void writeUint16(Uint16 n);

    /**
     * Writes a single 32 bit signed integer to the binary file.
     *
     * The value is marshalled to network order, ensuring that the binary file
     * is compatible against all platforms.
     *
     * The value is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param n  the 32 bit signed integer to write
     */
    void writeSint32(Sint32 n);
    
    /**
     * Writes a single 32 bit unsigned integer to the binary file.
     *
     * The value is marshalled to network order, ensuring that the binary file
     * is compatible against all platforms.
     *
     * The value is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param n  the 32 bit unsigned integer to write
     */
    void writeUint32(Uint32 n);

    /**
     * Writes a single 64 bit signed integer to the binary file.
     *
     * The value is marshalled to network order, ensuring that the binary file
     * is compatible against all platforms.
     *
     * The value is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param n  the 64 bit signed integer to write
     */
    void writeSint64(Sint64 n);
    
    /**
     * Writes a single 64 bit unsigned integer to the binary file.
     *
     * The value is marshalled to network order, ensuring that the binary file
     * is compatible against all platforms.
     *
     * The value is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param n  the 64 bit unsigned integer to write
     */
    void writeUint64(Uint64 n);
    
    /**
     * Writes a float to the binary file.
     *
     * The value is marshalled to network order, ensuring that the binary file
     * is compatible against all platforms.
     *
     * The value is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param n  the float to write
     */
    void writeFloat(float n);
    
    /**
     * Writes a double to the binary file.
     *
     * The value is marshalled to network order, ensuring that the binary file
     * is compatible against all platforms.
     *
     * The value is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param n  the double to write
     */
    void writeDouble(double n);

    
#pragma mark -
#pragma mark Array Writes
    /**
     * Writes an array of characters to the binary file.
     *
     * The array is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param array  the array of characters to write
     * @param length the number of characters to write
     * @param offset the initial offset into the array
     */
    void write(const char* array, size_t length, size_t offset=0);
    
    /**
     * Writes an array of bytes to the binary file.
     *
     * The array is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param array  the array of bytes to write
     * @param length the number of bytes to write
     * @param offset the initial offset into the array
     */
    void write(const Uint8* array, size_t length, size_t offset=0);
    
    /**
     * Writes an array of 16 bit signed integers to the binary file.
     *
     * The values are marshalled to network order, ensuring that the binary
     * file is compatible against all platforms.
     *
     * The array is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param array  the array of 16 bit signed integers to write
     * @param length the number of 16 bit signed integers to write
     * @param offset the initial offset into the array
     */
    void write(const Sint16* array, size_t length, size_t offset=0);
    
    /**
     * Writes an array of 16 bit unsigned integers to the binary file.
     *
     * The values are marshalled to network order, ensuring that the binary
     * file is compatible against all platforms.
     *
     * The array is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param array  the array of 16 bit unsigned integers shorts to write
     * @param length the number of 16 bit unsigned integers shorts to write
     * @param offset the initial offset into the array
     */
    void write(const Uint16* array, size_t length, size_t offset=0);
    
    /**
     * Writes an array of 32 bit signed integers to the binary file.
     *
     * The values are marshalled to network order, ensuring that the binary
     * file is compatible against all platforms.
     *
     * The array is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param array  the array of 32 bit signed integers to write
     * @param length the number of 32 bit signed integers to write
     * @param offset the initial offset into the array
     */
    void write(const Sint32* array, size_t length, size_t offset=0);
    
    /**
     * Writes an array of 32 bit unsigned integers to the binary file.
     *
     * The values are marshalled to network order, ensuring that the binary
     * file is compatible against all platforms.
     *
     * The array is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param array  the array of 32 bit unsigned integers shorts to write
     * @param length the number of 32 bit unsigned integers shorts to write
     * @param offset the initial offset into the array
     */
    void write(const Uint32* array, size_t length, size_t offset=0);
    
    /**
     * Writes an array of 64 bit signed integers to the binary file.
     *
     * The values are marshalled to network order, ensuring that the binary
     * file is compatible against all platforms.
     *
     * The array is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param array  the array of 64 bit signed integers to write
     * @param length the number of 64 bit signed integers to write
     * @param offset the initial offset into the array
     */
    void write(const Sint64* array, size_t length, size_t offset=0);
    
    /**
     * Writes an array of 64 bit unsigned integers to the binary file.
     *
     * The values are marshalled to network order, ensuring that the binary
     * file is compatible against all platforms.
     *
     * The array is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param array  the array of 64 bit unsigned integers shorts to write
     * @param length the number of 64 bit unsigned integers shorts to write
     * @param offset the initial offset into the array
     */
    void write(const Uint64* array, size_t length, size_t offset=0);
    
    /**
     * Writes an array of floats to the binary file.
     *
     * The values are marshalled to network order, ensuring that the binary
     * file is compatible against all platforms.
     *
     * The array is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param array  the array of floats to write
     * @param length the number of floats to write
     * @param offset the initial offset into the array
     */
    void write(const float* array, size_t length, size_t offset=0);
    
    /**
     * Writes an array of doubles to the binary file.
     *
     * The values are marshalled to network order, ensuring that the binary
     * file is compatible against all platforms.
     *
     * The array is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param array  the array of doubles to write
     * @param length the number of doubles to write
     * @param offset the initial offset into the array
     */
    void write(const double* array, size_t length, size_t offset=0);
    
};

}

#endif /* __CU_BINARY_WRITER_H__ */

//
//  CUTextWriter.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a simple Java-style writer for writing to text files.
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
#ifndef __CU_TEXT_WRITER_H__
#define __CU_TEXT_WRITER_H__
#include <cugl/base/CUBase.h>
#include <cugl/util/CUStrings.h>
#include <cugl/io/CUPathname.h>
#include <SDL/SDL.h>
#include <string>

namespace cugl {

/**
 * Simple text-based writer for ASCII or UTF8 files.
 *
 * This class provides a simple Java-style writer for writing to text files.
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
class TextWriter {
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
     * Creates a text writer with no assigned file.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    TextWriter() : _name(""), _stream(nullptr), _cbuffer(nullptr), _bufoff(-1) {}

    /**
     * Deletes this writer and all of its resources.
     *
     * Calls to the destructor will close the file if it is not already closed.
     */
    ~TextWriter() { close(); }
    
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
    static std::shared_ptr<TextWriter> alloc(const std::string& file) {
        std::shared_ptr<TextWriter> result = std::make_shared<TextWriter>();
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
    static std::shared_ptr<TextWriter> alloc(const char* file) {
        std::shared_ptr<TextWriter> result = std::make_shared<TextWriter>();
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
    static std::shared_ptr<TextWriter> alloc(const Pathname& file) {
        std::shared_ptr<TextWriter> result = std::make_shared<TextWriter>();
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
    static std::shared_ptr<TextWriter> alloc(const std::string& file, unsigned int capacity) {
        std::shared_ptr<TextWriter> result = std::make_shared<TextWriter>();
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
    static std::shared_ptr<TextWriter> alloc(const char* file, unsigned int capacity) {
        std::shared_ptr<TextWriter> result = std::make_shared<TextWriter>();
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
    static std::shared_ptr<TextWriter> alloc(const Pathname& file, unsigned int capacity) {
        std::shared_ptr<TextWriter> result = std::make_shared<TextWriter>();
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
#pragma mark Primitive Methods
    /**
     * Writes a single character to the file.
     *
     * The value is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param c  the character to write
     */
    void write(char c);

    /**
     * Writes a byte value to the file.
     *
     * The value is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param b  the byte value to write
     */
    void write(Uint8 b)                 { write(cugl::to_string(b)); }

    /**
     * Writes a signed 16 bit integer to the file.
     *
     * The value is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param n  the signed 16 bit integer to write
     */
    void write(Sint16 n)                { write(cugl::to_string(n)); }

    /**
     * Writes a unsigned 16 bit integer to the file.
     *
     * The value is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param n  the unsigned 16 bit integer to write
     */
    void write(Uint16 n)                { write(cugl::to_string(n)); }
    
    /**
     * Writes a signed 32 bit integer to the file.
     *
     * The value is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param n  the signed 32 bit integer to write
     */
    void write(Sint32 n)                { write(cugl::to_string(n)); }
    
    /**
     * Writes a unsigned 32 bit integer to the file.
     *
     * The value is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param n  the unsigned 32 bit integer to write
     */
    void write(Uint32 n)                { write(cugl::to_string(n)); }

    /**
     * Writes a signed 64 bit integer to the file.
     *
     * The value is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param n  the signed 64 bit integer to write
     */
    void write(Sint64 n)                { write(cugl::to_string(n)); }
    
    /**
     * Writes a unsigned 64 bit integer to the file.
     *
     * The value is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param n  the unsigned 64 bit integer to write
     */
    void write(Uint64 n)                { write(cugl::to_string(n)); }
    
    /**
     * Writes a boolean value to the file.
     *
     * The boolean will be written as the string "false" or "true".
     *
     * The value is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param b  the boolean value to write
     */
    void write(bool b) {
        if (b) { write("true"); } else { write("false"); }
    }
    
    /**
     * Writes a float value to the file.
     *
     * The value will be written with full precision.
     *
     * The value is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param n  the float value to write
     */
    void write(float n)                 { write(cugl::to_string(n)); }

    /**
     * Writes a double value to the file.
     *
     * The value will be written with full precision.
     *
     * The value is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param n  the double value to write
     */
    void write(double n)                { write(cugl::to_string(n)); }

    
#pragma mark -
#pragma mark String Methods
    /**
     * Writes a string (ASCII or UTF8) to the file
     *
     * The value is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param s  the string to write
     */
    void write(const char* s);
    
    /**
     * Writes a string (ASCII or UTF8) to the file
     *
     * The value is written to the internal buffer, but is not necessarily
     * flushed automatically.  It will be written when the buffer reaches
     * capacity or the file is closed.
     *
     * @param s  the string to write
     */
    void write(const std::string& s);
    
    /**
     * Writes a string (ASCII or UTF8) to the file, followed by a newline
     *
     * The newline used is a standard Unix newline '\n'. You should not expect
     * Windows-style carriage returns (e.g. '\r'). This method automatically
     * flushes the buffer when done.
     *
     * @param s  the string to write
     */
    void writeLine(const char* s);

    /**
     * Writes a string (ASCII or UTF8) to the file, followed by a newline
     *
     * The newline used is a standard Unix newline '\n'. You should not expect
     * Windows-style carriage returns (e.g. '\r'). This method automatically
     * flushes the buffer when done.
     *
     * @param s  the string to write
     */
    void writeLine(const std::string& s);
    
};

}

#endif /* __CU_TEXT_WRITER_H__ */

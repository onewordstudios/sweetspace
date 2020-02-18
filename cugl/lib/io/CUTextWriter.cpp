//
//  CUTextWriter.cpp
//  Cornell University Game Library (CUGL)
//
//  This class provides a simple Java-style writer for writing to text files.
//  It supports both ASCII and UTF8 encoding. No other encodings are supported
//  (nor should they be since they are not cross-platform).
//
//  By default, this class (and every class in the io package) accesses the
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
#include <cugl/io/CUTextWriter.h>
#include <cugl/base/CUApplication.h>
#include <cugl/util/CUDebug.h>
#include <cstring>

using namespace cugl;

#define BUFFSIZE 1024

#pragma mark -
#pragma mark Constructors

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
bool TextWriter::init(const Pathname& file) {
    return init(file,BUFFSIZE);
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
bool TextWriter::init(const Pathname& file, unsigned int capacity) {
    CUAssertLog(capacity, "The buffer capacity must be positive");
    _name = file.getAbsoluteName();
    _stream = SDL_RWFromFile(_name.c_str(), "w");
    if (!_stream) {
        CULogError("%s", SDL_GetError());
        return false;
    }
    
    _capacity = capacity;
    _cbuffer = new char[_capacity];
    _bufoff = 0;
    
    return (bool)_cbuffer;
}


#pragma mark -
#pragma mark Stream Management
/**
 * Flushes the contents of the write buffer to the file.
 *
 * It is usually unnecessary to call this method. It is called automatically
 * when the buffer fills, or just before the file is closed.
 */
void TextWriter::flush() {
    size_t amt = SDL_RWwrite(_stream, _cbuffer, 1, _bufoff);
    CUAssertLog(amt == _bufoff, "Unable to fully flush the writer");
    _bufoff = 0;
}

/**
 * Closes the stream, releasing all resources
 *
 * The contents of the buffer are flushed before the file is closed.  Any
 * attempts to write to a closed stream will fail.  Calling this method
 * on a previously closed stream has no effect.
 */
void TextWriter::close() {
    if (_stream) {
        flush();
        SDL_RWclose(_stream);
        _stream  = nullptr;
    }
    if (_cbuffer) {
        delete[] _cbuffer;
        _cbuffer = nullptr;
    }
}


#pragma mark -
#pragma mark Write Methods
/**
 * Writes a single character to the file.
 *
 * The value is written to the internal buffer, but is not necessarily
 * flushed automatically.  It will be written when the buffer reaches
 * capacity or the file is closed.
 *
 * @param c  the character to write
 */
void TextWriter::write(char c) {
    CUAssertLog(_stream, "Attempt to write to a closed stream");
    if (_bufoff >= _capacity) {
        flush();
    }
    _cbuffer[_bufoff++] = c;
}

/**
 * Writes a string (ASCII or UTF8) to the file
 *
 * The value is written to the internal buffer, but is not necessarily
 * flushed automatically.  It will be written when the buffer reaches
 * capacity or the file is closed.
 *
 * @param s  the string to write
 */
void TextWriter::write(const char* s) {
    CUAssertLog(_stream, "Attempt to write to a closed stream");
    size_t pos = 0;
    size_t len = strlen(s);

    if (_bufoff+len >  _capacity) {
        flush();
    }
    while (len-pos > _capacity-_bufoff) {
        memcpy(&(_cbuffer[_bufoff]), &(s[pos]),_capacity-_bufoff);
        pos += _capacity-_bufoff;
        _bufoff = _capacity;
        flush();
    }

    memcpy(&(_cbuffer[_bufoff]), &(s[pos]), len-pos);
    _bufoff += (Sint32)(len-pos);
}

/**
 * Writes a string (ASCII or UTF8) to the file
 *
 * The value is written to the internal buffer, but is not necessarily
 * flushed automatically.  It will be written when the buffer reaches
 * capacity or the file is closed.
 *
 * @param s  the string to write
 */
void TextWriter::write(const std::string& s) {
    CUAssertLog(_stream, "Attempt to write to a closed stream");
    size_t pos = 0;
    if (_bufoff+s.size() > _capacity) {
        flush();
    }
    while (s.size()-pos > _capacity-_bufoff) {
        memcpy(&(_cbuffer[_bufoff]), &(s.c_str()[pos]), _capacity-_bufoff);
        pos += _capacity-_bufoff;
        _bufoff = _capacity;
        flush();
    }

    memcpy(&(_cbuffer[_bufoff]), &(s.c_str()[pos]), s.size()-pos);
    _bufoff += (Sint32)(s.size()-pos);
}

/**
 * Writes a string (ASCII or UTF8) to the file, followed by a newline
 *
 * The newline used is a standard Unix newline '\n'. You should not expect
 * Windows-style carriage returns (e.g. '\r'). This method automatically
 * flushes the buffer when done.
 *
 * @param s  the string to write
 */
void TextWriter::writeLine(const std::string& s) {
    write(s);
    write('\n');
    flush();
}

/**
 * Writes a string (ASCII or UTF8) to the file, followed by a newline
 *
 * The newline used is a standard Unix newline '\n'. You should not expect
 * Windows-style carriage returns (e.g. '\r'). This method automatically
 * flushes the buffer when done.
 *
 * @param s  the string to write
 */
void TextWriter::writeLine(const char* s) {
    write(s);
    write('\n');
    flush();
}


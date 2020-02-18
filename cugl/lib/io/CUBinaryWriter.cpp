//
//  CUBinaryWriter.cpp
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
#include <cugl/io/CUBinaryWriter.h>
#include <cugl/base/CUApplication.h>
#include <cugl/base/CUEndian.h>
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
bool BinaryWriter::init(const Pathname& file) {
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
bool BinaryWriter::init(const Pathname& file, unsigned int capacity) {
    CUAssertLog(capacity >= 8, "Buffer capacity is too small: %d", capacity);
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
void BinaryWriter::flush() {
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
void BinaryWriter::close() {
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
#pragma mark Single Element Writes
/**
 * Writes a single character to the binary file.
 *
 * The value is written to the internal buffer, but is not necessarily
 * flushed automatically.  It will be written when the buffer reaches
 * capacity or the file is closed.
 *
 * @param c  the character to write
 */
void BinaryWriter::write(char c) {
    CUAssertLog(_stream, "Attempt to write to a closed stream");
    if (_bufoff >= _capacity) {
        flush();
    }
    _cbuffer[_bufoff++] = c;
}

/**
 * Writes a single byte to the binary file.
 *
 * The value is written to the internal buffer, but is not necessarily
 * flushed automatically.  It will be written when the buffer reaches
 * capacity or the file is closed.
 *
 * @param c  the byte to write
 */
void BinaryWriter::writeUint8(Uint8 c) {
    CUAssertLog(_stream, "Attempt to write to a closed stream");
    if (_bufoff >= _capacity) {
        flush();
    }
    _cbuffer[_bufoff++] = c;
}

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
void BinaryWriter::writeSint16(Sint16 n) {
    CUAssertLog(_stream, "Attempt to write to a closed stream");
    if (_bufoff+2 > _capacity) {
        flush();
    }
    
    Sint16* pointer = (Sint16*)(&_cbuffer[_bufoff]);
    *pointer = marshall(n);
    _bufoff += 2;
}

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
void BinaryWriter::writeUint16(Uint16 n) {
    CUAssertLog(_stream, "Attempt to write to a closed stream");
    if (_bufoff+2 > _capacity) {
        flush();
    }
    
    Uint16* pointer = (Uint16*)(&_cbuffer[_bufoff]);
    *pointer = marshall(n);
    _bufoff += 2;
}

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
void BinaryWriter::writeSint32(Sint32 n) {
    CUAssertLog(_stream, "Attempt to write to a closed stream");
    if (_bufoff+4 > _capacity) {
        flush();
    }
    
    Sint32* pointer = (Sint32*)(&_cbuffer[_bufoff]);
    *pointer = marshall(n);
    _bufoff += 4;
}

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
void BinaryWriter::writeUint32(Uint32 n) {
    CUAssertLog(_stream, "Attempt to write to a closed stream");
    if (_bufoff+4 > _capacity) {
        flush();
    }
    
    Uint32* pointer = (Uint32*)(&_cbuffer[_bufoff]);
    *pointer = marshall(n);
    _bufoff += 4;
}

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
void BinaryWriter::writeSint64(Sint64 n) {
    CUAssertLog(_stream, "Attempt to write to a closed stream");
    if (_bufoff+8 > _capacity) {
        flush();
    }
    
    Sint64* pointer = (Sint64*)(&_cbuffer[_bufoff]);
    *pointer = marshall(n);
    _bufoff += 8;
}

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
void BinaryWriter::writeUint64(Uint64 n) {
    CUAssertLog(_stream, "Attempt to write to a closed stream");
    if (_bufoff+8 > _capacity) {
        flush();
    }
    
    Uint64* pointer = (Uint64*)(&_cbuffer[_bufoff]);
    *pointer = marshall(n);
    _bufoff += 8;
}


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
void BinaryWriter::writeFloat(float n) {
    CUAssertLog(_stream, "Attempt to write to a closed stream");
    if (_bufoff+4 > _capacity) {
        flush();
    }
    
    float* pointer = (float*)(&_cbuffer[_bufoff]);
    *pointer = marshall(n);
    _bufoff += 4;
}


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
void BinaryWriter::writeDouble(double n) {
    CUAssertLog(_stream, "Attempt to write to a closed stream");
    if (_bufoff+8 > _capacity) {
        flush();
    }
    
    double* pointer = (double*)(&_cbuffer[_bufoff]);
    *pointer = marshall(n);
    _bufoff += 8;
}



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
void BinaryWriter::write(const char* array, size_t length, size_t offset) {
    CUAssertLog(_stream, "Attempt to write to a closed stream");
    size_t pos = 0;
    
    if (_bufoff+length > _capacity) {
        flush();
    }
    while (length-pos > _capacity-_bufoff) {
        memcpy(&(_cbuffer[_bufoff]), &(array[pos+offset]),_capacity-_bufoff);
        pos += _capacity-_bufoff;
        _bufoff = _capacity;
        flush();
    }
    
    memcpy(&(_cbuffer[_bufoff]), &(array[pos+offset]), length-pos);
    _bufoff += (Sint32)(length-pos);
}

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
void BinaryWriter::write(const Uint8* array, size_t length, size_t offset) {
    CUAssertLog(_stream, "Attempt to write to a closed stream");
    size_t pos = 0;
        
    if (_bufoff+length > _capacity) {
        flush();
    }
    while (length-pos > _capacity-_bufoff) {
        memcpy(&(_cbuffer[_bufoff]), &(array[pos+offset]),_capacity-_bufoff);
        pos += _capacity-_bufoff;
        _bufoff = _capacity;
        flush();
    }
        
    memcpy(&(_cbuffer[_bufoff]), &(array[pos+offset]), length-pos);
    _bufoff += (Sint32)(length-pos);
}

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
void BinaryWriter::write(const Sint16* array, size_t length, size_t offset) {
    CUAssertLog(_stream, "Attempt to write to a closed stream");
    size_t pos = 0;
    
    unsigned int bytes = 2;
    if (_bufoff+length*bytes > _capacity) {
        flush();
    }
    while ((length-pos)*bytes > _capacity-_bufoff) {
        unsigned int skip = bytes*((_capacity-_bufoff)/bytes);
        memcpy(&(_cbuffer[_bufoff]), &(array[pos]),skip);
        
        for(int ii = 0; ii < skip; ii += bytes) {
            Sint16* ref = (Sint16*)(&_cbuffer[_bufoff+ii]);
            *ref = marshall(*ref);
        }
        
        pos += skip/bytes;
        _bufoff += skip;
        flush();
    }
    
    unsigned int skip = (unsigned int)((length-pos)*bytes);
    memcpy(&(_cbuffer[_bufoff]), &(array[pos+offset]), skip);
    for(int ii = 0; ii < skip; ii += bytes) {
        Sint16* ref = (Sint16*)(&_cbuffer[_bufoff+ii]);
        *ref = marshall(*ref);
    }

    _bufoff += skip;
}

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
void BinaryWriter::write(const Uint16* array, size_t length, size_t offset) {
    CUAssertLog(_stream, "Attempt to write to a closed stream");
    size_t pos = 0;
    
    unsigned int bytes = 2;
    if (_bufoff+length*bytes > _capacity) {
        flush();
    }
    while ((length-pos)*bytes > _capacity-_bufoff) {
        unsigned int skip = bytes*((_capacity-_bufoff)/bytes);
        memcpy(&(_cbuffer[_bufoff]), &(array[pos]),skip);
        
        for(int ii = 0; ii < skip; ii += bytes) {
            Uint16* ref = (Uint16*)(&_cbuffer[_bufoff+ii]);
            *ref = marshall(*ref);
        }
        
        pos += skip/bytes;
        _bufoff += skip;
        flush();
    }
    
    unsigned int skip = (unsigned int)((length-pos)*bytes);
    memcpy(&(_cbuffer[_bufoff]), &(array[pos+offset]), skip);
    for(int ii = 0; ii < skip; ii += bytes) {
        Uint16* ref = (Uint16*)(&_cbuffer[_bufoff+ii]);
        *ref = marshall(*ref);
    }
    
    _bufoff += skip;
}

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
void BinaryWriter::write(const Sint32* array, size_t length, size_t offset) {
    CUAssertLog(_stream, "Attempt to write to a closed stream");
    size_t pos = 0;
    
    unsigned int bytes = 4;
    if (_bufoff+length*bytes > _capacity) {
        flush();
    }
    while ((length-pos)*bytes > _capacity-_bufoff) {
        unsigned int skip = bytes*((_capacity-_bufoff)/bytes);
        memcpy(&(_cbuffer[_bufoff]), &(array[pos]),skip);
        
        for(int ii = 0; ii < skip; ii += bytes) {
            Sint32* ref = (Sint32*)(&_cbuffer[_bufoff+ii]);
            *ref = marshall(*ref);
        }
        
        pos += skip/bytes;
        _bufoff += skip;
        flush();
    }
    
    unsigned int skip = (unsigned int)((length-pos)*bytes);
    memcpy(&(_cbuffer[_bufoff]), &(array[pos+offset]), skip);
    for(int ii = 0; ii < skip; ii += bytes) {
        Sint32* ref = (Sint32*)(&_cbuffer[_bufoff+ii]);
        *ref = marshall(*ref);
    }
    
    _bufoff += skip;
}


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
void BinaryWriter::write(const Uint32* array, size_t length, size_t offset) {
    CUAssertLog(_stream, "Attempt to write to a closed stream");
    size_t pos = 0;
    
    unsigned int bytes = 4;
    if (_bufoff+length*bytes > _capacity) {
        flush();
    }
    while ((length-pos)*bytes > _capacity-_bufoff) {
        unsigned int skip = bytes*((_capacity-_bufoff)/bytes);
        memcpy(&(_cbuffer[_bufoff]), &(array[pos]),skip);
        
        for(int ii = 0; ii < skip; ii += bytes) {
            Uint32* ref = (Uint32*)(&_cbuffer[_bufoff+ii]);
            *ref = marshall(*ref);
        }
        
        pos += skip/bytes;
        _bufoff += skip;
        flush();
    }
    
    unsigned int skip = (unsigned int)((length-pos)*bytes);
    memcpy(&(_cbuffer[_bufoff]), &(array[pos+offset]), skip);
    for(int ii = 0; ii < skip; ii += bytes) {
        Uint32* ref = (Uint32*)(&_cbuffer[_bufoff+ii]);
        *ref = marshall(*ref);
    }
    
    _bufoff += skip;
}


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
void BinaryWriter::write(const Sint64* array, size_t length, size_t offset) {
    CUAssertLog(_stream, "Attempt to write to a closed stream");
    size_t pos = 0;
    
    unsigned int bytes = 8;
    if (_bufoff+length*bytes > _capacity) {
        flush();
    }
    while ((length-pos)*bytes > _capacity-_bufoff) {
        unsigned int skip = bytes*((_capacity-_bufoff)/bytes);
        memcpy(&(_cbuffer[_bufoff]), &(array[pos]),skip);
        
        for(int ii = 0; ii < skip; ii += bytes) {
            Sint64* ref = (Sint64*)(&_cbuffer[_bufoff+ii]);
            *ref = marshall(*ref);
        }
        
        pos += skip/bytes;
        _bufoff += skip;
        flush();
    }
    
    unsigned int skip = (unsigned int)((length-pos)*bytes);
    memcpy(&(_cbuffer[_bufoff]), &(array[pos+offset]), skip);
    for(int ii = 0; ii < skip; ii += bytes) {
        Sint64* ref = (Sint64*)(&_cbuffer[_bufoff+ii]);
        *ref = marshall(*ref);
    }
    
    _bufoff += skip;
}


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
void BinaryWriter::write(const Uint64* array, size_t length, size_t offset) {
    CUAssertLog(_stream, "Attempt to write to a closed stream");
    size_t pos = 0;
    
    unsigned int bytes = 8;
    if (_bufoff+length*bytes > _capacity) {
        flush();
    }
    while ((length-pos)*bytes > _capacity-_bufoff) {
        unsigned int skip = bytes*((_capacity-_bufoff)/bytes);
        memcpy(&(_cbuffer[_bufoff]), &(array[pos]),skip);
        
        for(int ii = 0; ii < skip; ii += bytes) {
            Uint64* ref = (Uint64*)(&_cbuffer[_bufoff+ii]);
            *ref = marshall(*ref);
        }
        
        pos += skip/bytes;
        _bufoff += skip;
        flush();
    }
    
    unsigned int skip = (unsigned int)((length-pos)*bytes);
    memcpy(&(_cbuffer[_bufoff]), &(array[pos+offset]), skip);
    for(int ii = 0; ii < skip; ii += bytes) {
        Uint64* ref = (Uint64*)(&_cbuffer[_bufoff+ii]);
        *ref = marshall(*ref);
    }
    
    _bufoff += skip;
}


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
void BinaryWriter::write(const float* array, size_t length, size_t offset) {
    CUAssertLog(_stream, "Attempt to write to a closed stream");
    size_t pos = 0;
    
    unsigned int bytes = 4;
    if (_bufoff+length*bytes > _capacity) {
        flush();
    }
    while ((length-pos)*bytes > _capacity-_bufoff) {
        unsigned int skip = bytes*((_capacity-_bufoff)/bytes);
        memcpy(&(_cbuffer[_bufoff]), &(array[pos]),skip);
        
        for(int ii = 0; ii < skip; ii += bytes) {
            float* ref = (float*)(&_cbuffer[_bufoff+ii]);
            *ref = marshall(*ref);
        }
        
        pos += skip/bytes;
        _bufoff += skip;
        flush();
    }
    
    unsigned int skip = (unsigned int)((length-pos)*bytes);
    memcpy(&(_cbuffer[_bufoff]), &(array[pos+offset]), skip);
    for(int ii = 0; ii < skip; ii += bytes) {
        float* ref = (float*)(&_cbuffer[_bufoff+ii]);
        *ref = marshall(*ref);
    }
    
    _bufoff += skip;
}

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
void BinaryWriter::write(const double* array, size_t length, size_t offset) {
    CUAssertLog(_stream, "Attempt to write to a closed stream");
    size_t pos = 0;
    
    unsigned int bytes = 8;
    if (_bufoff+length*bytes > _capacity) {
        flush();
    }
    while ((length-pos)*bytes > _capacity-_bufoff) {
        unsigned int skip = bytes*((_capacity-_bufoff)/bytes);
        memcpy(&(_cbuffer[_bufoff]), &(array[pos]),skip);
        
        for(int ii = 0; ii < skip; ii += bytes) {
            double* ref = (double*)(&_cbuffer[_bufoff+ii]);
            *ref = marshall(*ref);
        }
        
        pos += skip/bytes;
        _bufoff += skip;
        flush();
    }
    
    unsigned int skip = (unsigned int)((length-pos)*bytes);
    memcpy(&(_cbuffer[_bufoff]), &(array[pos+offset]), skip);
    for(int ii = 0; ii < skip; ii += bytes) {
        double* ref = (double*)(&_cbuffer[_bufoff+ii]);
        *ref = marshall(*ref);
    }
    
    _bufoff += skip;
}

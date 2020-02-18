//
//  CUBinaryReader.cpp
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
#include <cugl/io/CUBinaryReader.h>
#include <cugl/util/CUDebug.h>
#include <cugl/base/CUApplication.h>
#include <cugl/base/CUEndian.h>

using namespace cugl;

#define BUFFSIZE 1024

#pragma mark -
#pragma mark Constructors

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
bool BinaryReader::init(const Pathname& file) {
    return init(file,BUFFSIZE);
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
bool BinaryReader::init(const Pathname& file, unsigned int capacity) {
    CUAssertLog(capacity, "The buffer capacity must be positive");
    _name = file.getAbsoluteName();
    _stream = SDL_RWFromFile(_name.c_str(), "rb");
    if (!_stream) {
        return false;
    }
    
    _ssize = SDL_RWsize(_stream);
    _scursor = 0;
    _capacity = capacity;
    _buffer = new char[_capacity];
    _bufsize = 0;
    fill();
    
    return _ssize >= 0;
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
bool BinaryReader::initWithAsset(const char* file) {
    return initWithAsset(file,BUFFSIZE);
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
bool BinaryReader::initWithAsset(const char* file, unsigned int capacity) {
    CUAssertLog(capacity, "The buffer capacity must be positive");
    
    // Check if the path is absolute
#if defined (__WINDOWS__)
    bool absolute = (bool)strstr(file,":");
#else
    bool absolute = file[0] == '/';
#endif
    CUAssertLog(!absolute, "This initializer does not accept absolute paths");
    
    _name = Application::get()->getAssetDirectory();
    _name.append(file);
    _stream = SDL_RWFromFile(_name.c_str(), "rb");
    if (!_stream) {
        return false;
    }
    
    _ssize = SDL_RWsize(_stream);
    _scursor = 0;
    _capacity = capacity;
    _buffer = new char[_capacity];
    _bufsize = 0;
    fill();
    
    return _ssize >= 0;
}


#pragma mark -
#pragma mark Stream Management
/**
 * Resets the stream back to the beginning
 *
 * This allows the stream to be read a second time.  It may even be called
 * if the stream has been closed.
 */
void BinaryReader::reset() {
    if (_stream) {
        close();
    }
    _stream = SDL_RWFromFile(_name.c_str(), "rb");
    _ssize  = SDL_RWsize(_stream);
    _buffer = new char[_capacity];
    _bufsize = 0;
    _bufoff  = 0;
    _scursor = 0;
}

/**
 * Closes the stream, releasing all resources
 *
 * Any attempts to read from a closed stream will fail.  Calling this method
 * on a previously closed stream has no effect.
 */
void BinaryReader::close() {
    if (_stream) {
        SDL_RWclose(_stream);
        _stream  = nullptr;
        _scursor = 0;
    }
    if (_buffer) {
        delete[] _buffer;
        _buffer  = nullptr;
        _bufsize = 0;
    }
}

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
bool BinaryReader::ready(unsigned int bytes) const {
    unsigned int remain = (unsigned int)(_bufsize-_bufoff);
    if (remain < bytes) {
        remain += (unsigned int)(_ssize-_scursor);
        return remain >= bytes;
    }
    return true;
}

/**
 * Fills the storage buffer to capacity
 *
 * This cuts down on the number of reads to the file by allowing us
 * to read from the file in predefined chunks.
 *
 * @param bytes The minimum number of bytes to ensure in the stream
 */
void BinaryReader::fill(unsigned int bytes) {
    if (!_bufoff || !_stream || _scursor == _ssize) {
        return;
    }
    
    if (_bufoff == -1 || _bufoff+bytes > _bufsize) {
        if (_bufoff < _bufsize) {
            memcpy(_buffer, &(_buffer[_bufoff]), _bufsize-_bufoff);
            _bufsize -= _bufoff;
        } else {
            _bufsize = 0;
        }
        _bufoff   = 0;
    }
    
    size_t amt = SDL_RWread(_stream, &_buffer[_bufsize], 1, _capacity-_bufsize);
    _bufsize += (Uint32)amt;
    _scursor += amt;
}

#pragma mark -
#pragma mark Single Element Reads
/**
 * Returns a single character from the stream
 *
 * @return a single character from the stream
 */
char BinaryReader::readChar() {
    CUAssertLog(ready(), "Attempt to read a finished stream");
    if (_bufoff >= _bufsize) {
        fill(1);
    }
    return _buffer[_bufoff++];
}

/**
 * Returns a single byte from the stream
 *
 * @return a single byte from the stream
 */
Uint8 BinaryReader::readByte() {
    CUAssertLog(ready(), "Attempt to read a finished stream");
    if (_bufoff >= _bufsize) {
        fill(1);
    }
    return (Uint8)_buffer[_bufoff++];
}

/**
 * Returns a single 16 bit signed integer from the stream
 *
 * The value is marshalled from network order, ensuring that the binary file
 * is compatible against all platforms.
 *
 * @return a single 16 bit signed integer from the stream
 */
Sint16 BinaryReader::readSint16() {
    CUAssertLog(ready(), "Attempt to read a finished stream");
    if (_bufoff+2 > _bufsize) {
        fill(2);
    }
    CUAssertLog(_bufsize - _bufoff >= 2, "Too few elements remaining in stream");
    Sint16* ref = (Sint16*)(&_buffer[_bufoff]);
    _bufoff += 2;
    return marshall(*ref);
}

/**
 * Returns a single 16 bit unsigned integer from the stream
 *
 * The value is marshalled from network order, ensuring that the binary file
 * is compatible against all platforms.
 *
 * @return a single 16 bit unsigned integer from the stream
 */
Uint16 BinaryReader::readUint16() {
    CUAssertLog(ready(), "Attempt to read a finished stream");
    if (_bufoff+2 > _bufsize) {
        fill(2);
    }
    CUAssertLog(_bufsize - _bufoff >= 2, "Too few elements remaining in stream");
    Uint16* ref = (Uint16*)(&_buffer[_bufoff]);
    _bufoff += 2;
    return marshall(*ref);
}

/**
 * Returns a single 32 bit signed integer from the stream
 *
 * The value is marshalled from network order, ensuring that the binary file
 * is compatible against all platforms.
 *
 * @return a single 32 bit signed integer from the stream
 */
Sint32 BinaryReader::readSint32() {
    CUAssertLog(ready(), "Attempt to read a finished stream");
    if (_bufoff+4 > _bufsize) {
        fill(4);
    }
    CUAssertLog(_bufsize - _bufoff >= 4, "Too few elements remaining in stream");
    Sint32* ref = (Sint32*)(&_buffer[_bufoff]);
    _bufoff += 4;
    return marshall(*ref);
}


/**
 * Returns a single 32 bit unsigned integer from the stream
 *
 * The value is marshalled from network order, ensuring that the binary file
 * is compatible against all platforms.
 *
 * @return a single 32 bit unsigned integer from the stream
 */
Uint32 BinaryReader::readUint32() {
    CUAssertLog(ready(), "Attempt to read a finished stream");
    if (_bufoff+4 > _bufsize) {
        fill(4);
    }
    CUAssertLog(_bufsize - _bufoff >= 4, "Too few elements remaining in stream");
    Uint32* ref = (Uint32*)(&_buffer[_bufoff]);
    _bufoff += 4;
    return marshall(*ref);
}


/**
 * Returns a single 32 bit signed integer from the stream
 *
 * The value is marshalled from network order, ensuring that the binary file
 * is compatible against all platforms.
 *
 * @return a single 32 bit signed integer from the stream
 */
Sint64 BinaryReader::readSint64() {
    CUAssertLog(ready(), "Attempt to read a finished stream");
    if (_bufoff+8 > _bufsize) {
        fill(8);
    }
    CUAssertLog(_bufsize - _bufoff >= 8, "Too few elements remaining in stream");
    Sint64* ref = (Sint64*)(&_buffer[_bufoff]);
    _bufoff += 8;
    return marshall(*ref);
}

/**
 * Returns a single 32 bit unsigned integer from the stream
 *
 * The value is marshalled from network order, ensuring that the binary file
 * is compatible against all platforms.
 *
 * @return a single 32 bit unsigned integer from the stream
 */
Uint64 BinaryReader::readUint64() {
    CUAssertLog(ready(), "Attempt to read a finished stream");
    if (_bufoff+8 > _bufsize) {
        fill(8);
    }
    CUAssertLog(_bufsize - _bufoff >= 8, "Too few elements remaining in stream");
    Uint64* ref = (Uint64*)(&_buffer[_bufoff]);
    _bufoff += 8;
    return marshall(*ref);
}


/**
 * Returns a single float from the stream
 *
 * The value is marshalled from network order, ensuring that the binary file
 * is compatible against all platforms.
 *
 * @return a single float from the stream
 */
float BinaryReader::readFloat() {
    CUAssertLog(ready(), "Attempt to read a finished stream");
    if (_bufoff+4 > _bufsize) {
        fill(4);
    }
    CUAssertLog(_bufsize - _bufoff >= 4, "Too few elements remaining in stream");
    float* ref = (float*)(&_buffer[_bufoff]);
    _bufoff += 4;
    return marshall(*ref);
}


/**
 * Returns a single double from the stream
 *
 * The value is marshalled from network order, ensuring that the binary file
 * is compatible against all platforms.
 *
 * @return a single double from the stream
 */
double BinaryReader::readDouble()  {
    CUAssertLog(ready(), "Attempt to read a finished stream");
    if (_bufoff+8 > _bufsize) {
        fill(8);
    }
    CUAssertLog(_bufsize - _bufoff >= 8, "Too few elements remaining in stream");
    double* ref = (double*)(&_buffer[_bufoff]);
    _bufoff += 8;
    return marshall(*ref);
}


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
size_t BinaryReader::read(char* buffer, size_t maximum, size_t offset) {
    CUAssertLog(ready(), "Attempt to read a finished stream");
    unsigned int pos = (unsigned int)offset;
    while (ready(1) && pos-offset < maximum) {
        size_t available = _bufsize-_bufoff;
        size_t wanted = maximum-(pos-offset);
        wanted = wanted < available ? wanted : available;
        memcpy(&(buffer[pos]),&(_buffer[_bufoff]),wanted);
        _bufoff += (Sint32)wanted;
        pos += (unsigned int)wanted;
    }
    return pos-offset;
}

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
size_t BinaryReader::read(Uint8* buffer, size_t maximum, size_t offset)  {
    CUAssertLog(ready(), "Attempt to read a finished stream");
    unsigned int pos = (unsigned int)offset;
    while (ready(1) && pos-offset < maximum) {
        size_t available = _bufsize-_bufoff;
        size_t wanted = maximum-(pos-offset);
        wanted = wanted < available ? wanted : available;
        memcpy(&(buffer[pos]),&(_buffer[_bufoff]),wanted);
        _bufoff += (Sint32)wanted;
        pos += (unsigned int)wanted;
    }
    return pos-offset;
}

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
size_t BinaryReader::read(Sint16* buffer, size_t maximum, size_t offset) {
    CUAssertLog(ready(), "Attempt to read a finished stream");
    unsigned int pos = (unsigned int)offset;
    unsigned int bytes = 2;
    while (ready(bytes) && pos-offset < maximum) {
        size_t available = bytes*((_bufsize-_bufoff)/bytes);
        size_t wanted = (maximum-(pos-offset))*bytes;
        wanted = wanted < available ? wanted : available;
        memcpy(&(buffer[pos]),&(_buffer[_bufoff]),wanted);
        _bufoff += (Sint32)wanted;
        pos += (unsigned int)(wanted/bytes);
    }
    for(int ii = (int)offset; ii < pos; ii++) {
        buffer[ii] = marshall(buffer[ii]);
    }
    
    return pos-offset;
}

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
size_t BinaryReader::read(Uint16* buffer, size_t maximum, size_t offset)  {
    CUAssertLog(ready(), "Attempt to read a finished stream");
    unsigned int pos = (unsigned int)offset;
    unsigned int bytes = 2;
    while (ready(bytes) && pos-offset < maximum) {
        size_t available = bytes*((_bufsize-_bufoff)/bytes);
        size_t wanted = (maximum-(pos-offset))*bytes;
        wanted = wanted < available ? wanted : available;
        memcpy(&(buffer[pos]),&(_buffer[_bufoff]),wanted);
        _bufoff += (Sint32)wanted;
        pos += (unsigned int)(wanted/bytes);
    }
    for(int ii = (int)offset; ii < pos; ii++) {
        buffer[ii] = marshall(buffer[ii]);
    }
    
    return pos-offset;
}


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
size_t BinaryReader::read(Sint32* buffer, size_t maximum, size_t offset) {
    CUAssertLog(ready(), "Attempt to read a finished stream");
    unsigned int pos = (unsigned int)offset;
    unsigned int bytes = 4;
    while (ready(bytes) && pos-offset < maximum) {
        size_t available = bytes*((_bufsize-_bufoff)/bytes);
        size_t wanted = (maximum-(pos-offset))*bytes;
        wanted = wanted < available ? wanted : available;
        memcpy(&(buffer[pos]),&(_buffer[_bufoff]),wanted);
        _bufoff += (Sint32)wanted;
        pos += (unsigned int)(wanted/bytes);
    }
    for(int ii = (int)offset; ii < pos; ii++) {
        buffer[ii] = marshall(buffer[ii]);
    }
    
    return pos-offset;
}

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
size_t BinaryReader::read(Uint32* buffer, size_t maximum, size_t offset) {
    CUAssertLog(ready(), "Attempt to read a finished stream");
    unsigned int pos = (unsigned int)offset;
    unsigned int bytes = 4;
    while (ready(bytes) && pos-offset < maximum) {
        size_t available = bytes*((_bufsize-_bufoff)/bytes);
        size_t wanted = (maximum-(pos-offset))*bytes;
        wanted = wanted < available ? wanted : available;
        memcpy(&(buffer[pos]),&(_buffer[_bufoff]),wanted);
        _bufoff += (Sint32)wanted;
        pos += (unsigned int)(wanted/bytes);
    }
    for(int ii = (int)offset; ii < pos; ii++) {
        buffer[ii] = marshall(buffer[ii]);
    }
    
    return pos-offset;
}

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
size_t BinaryReader::read(Sint64* buffer, size_t maximum, size_t offset) {
    CUAssertLog(ready(), "Attempt to read a finished stream");
    unsigned int pos = (unsigned int)offset;
    unsigned int bytes = 8;
    while (ready(bytes) && pos-offset < maximum) {
        size_t available = bytes*((_bufsize-_bufoff)/bytes);
        size_t wanted = (maximum-(pos-offset))*bytes;
        wanted = wanted < available ? wanted : available;
        memcpy(&(buffer[pos]),&(_buffer[_bufoff]),wanted);
        _bufoff += (Sint32)wanted;
        pos += (unsigned int)(wanted/bytes);
    }
    for(int ii = (int)offset; ii < pos; ii++) {
        buffer[ii] = marshall(buffer[ii]);
    }
    
    return pos-offset;
}

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
size_t BinaryReader::read(Uint64* buffer, size_t maximum, size_t offset) {
    CUAssertLog(ready(), "Attempt to read a finished stream");
    unsigned int pos = (unsigned int)offset;
    unsigned int bytes = 8;
    while (ready(bytes) && pos-offset < maximum) {
        size_t available = bytes*((_bufsize-_bufoff)/bytes);
        size_t wanted = (maximum-(pos-offset))*bytes;
        wanted = wanted < available ? wanted : available;
        memcpy(&(buffer[pos]),&(_buffer[_bufoff]),wanted);
        _bufoff += (Sint32)wanted;
        pos += (unsigned int)(wanted/bytes);
    }
    for(int ii = (int)offset; ii < pos; ii++) {
        buffer[ii] = marshall(buffer[ii]);
    }
    
    return pos-offset;
}

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
size_t BinaryReader::read(float* buffer, size_t maximum, size_t offset) {
    CUAssertLog(ready(), "Attempt to read a finished stream");
    unsigned int pos = (unsigned int)offset;
    unsigned int bytes = 4;
    while (ready(bytes) && pos-offset < maximum) {
        size_t available = bytes*((_bufsize-_bufoff)/bytes);
        size_t wanted = (maximum-(pos-offset))*bytes;
        wanted = wanted < available ? wanted : available;
        memcpy(&(buffer[pos]),&(_buffer[_bufoff]),wanted);
        _bufoff += (Sint32)wanted;
        pos += (unsigned int)(wanted/bytes);
    }
    for(int ii = (int)offset; ii < pos; ii++) {
        buffer[ii] = marshall(buffer[ii]);
    }
    
    return pos-offset;
}

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
size_t BinaryReader::read(double* buffer, size_t maximum, size_t offset) {
    CUAssertLog(ready(), "Attempt to read a finished stream");
    unsigned int pos = (unsigned int)offset;
    unsigned int bytes = 8;
    while (ready(bytes) && pos-offset < maximum) {
        size_t available = bytes*((_bufsize-_bufoff)/bytes);
        size_t wanted = (maximum-(pos-offset))*bytes;
        wanted = wanted < available ? wanted : available;
        memcpy(&(buffer[pos]),&(_buffer[_bufoff]),wanted);
        _bufoff += (Sint32)wanted;
        pos += (unsigned int)(wanted/bytes);
    }
    for(int ii = (int)offset; ii < pos; ii++) {
        buffer[ii] = marshall(buffer[ii]);
    }
    
    return pos-offset;
}


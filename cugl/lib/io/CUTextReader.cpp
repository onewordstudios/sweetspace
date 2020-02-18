//
//  CUTextReader.cpp
//  Cornell University Game Library (CUGL)
//
//  This class provides a simple Java-style reader for reading from text files.
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
#include <cugl/io/CUTextReader.h>
#include <cugl/util/CUDebug.h>
#include <cugl/base/CUApplication.h>
#include <utf8/utf8.h>
#include <cctype>

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
bool TextReader::init(const Pathname& file) {
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
bool TextReader::init(const Pathname& file, unsigned int capacity) {
    CUAssertLog(capacity, "The buffer capacity must be positive");
    _name = file.getAbsoluteName();
    _stream = SDL_RWFromFile(_name.c_str(), "r");
    if (!_stream) {
        return false;
    }
    
    _ssize = SDL_RWsize(_stream);
    _scursor = 0;
    _capacity = capacity;
    _sbuffer.reserve(_capacity);
    _cbuffer = new char[_capacity];
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
bool TextReader::initWithAsset(const char* file) {
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
bool TextReader::initWithAsset(const char* file, unsigned int capacity) {
    CUAssertLog(capacity, "The buffer capacity must be positive");

    // Check if the path is absolute
#if defined (__WINDOWS__)
    bool absolute = (bool)strstr(file,":") || file[0] == '\\';
#else
    bool absolute = file[0] == '/';
#endif
    CUAssertLog(!absolute, "This initializer does not accept absolute paths");

    _name = Application::get()->getAssetDirectory();
    _name.append(file);
#if defined (__WINDOWS__)
	for (int ii = 0; ii < _name.size(); ii++) {
		if (_name[ii] == '/') {
			_name[ii] = '\\';
		}
	}
#endif

    _stream = SDL_RWFromFile(_name.c_str(), "r");
    if (!_stream) {
        return false;
    }
    
    _ssize = SDL_RWsize(_stream);
	_scursor = 0;
    _capacity = capacity;
    _sbuffer.reserve(_capacity);
    _cbuffer = new char[_capacity];
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
void TextReader::reset() {
    if (_stream) {
        close();
    }
    _stream = SDL_RWFromFile(_name.c_str(), "r");
    _ssize  = SDL_RWsize(_stream);
    _cbuffer = new char[_capacity];
    _sbuffer.clear();
    _bufoff  = -1;
    _scursor = 0;
}

/**
 * Closes the stream, releasing all resources
 *
 * Any attempts to read from a closed stream will fail.  Calling this method
 * on a previously closed stream has no effect.
 */
void TextReader::close() {
    if (_stream) {
        SDL_RWclose(_stream);
        _stream  = nullptr;
        _scursor = 0;
    }
    if (_cbuffer) {
        delete[] _cbuffer;
        _cbuffer = nullptr;
    }
}

/**
 * Fills the storage buffer to capacity
 *
 * This cuts down on the number of reads to the file by allowing us
 * to read from the file in predefined chunks.
 */
void TextReader::fill() {
    if (!_bufoff || !_stream || _scursor == _ssize) {
        return;
    } else if (_bufoff > 0) {
		_sbuffer.erase(_sbuffer.begin(), _sbuffer.begin() + _bufoff);
	}

    _bufoff = 0;
    size_t amt = SDL_RWread(_stream, _cbuffer, 1, _capacity-_sbuffer.size());
    _sbuffer.append(_cbuffer,amt);
    _scursor += amt;
}

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
char TextReader::read() {
    std::string result;
    read(result);
    return result[0];
}

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
std::string& TextReader::read(std::string& data) {
    CUAssertLog(ready(), "Attempt to read a finished stream");
    if (_bufoff >= _sbuffer.size()) {
        fill();
    }

    data.push_back(_sbuffer[_bufoff++]);
    return data;
}

/**
 * Returns a single UTF8 character from the stream
 *
 * Because of the way UTF8 values are encoded, the resulting value may be
 * anywhere from 1 to 4 bytes.  That is why the result is returned as a
 * variable length string.
 *
 * @return a single UTF8 character from the stream
 */
std::string TextReader::readUTF8() {
    std::string result;
    readUTF8(result);
    return result;
}

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
std::string& TextReader::readUTF8(std::string& data) {
    CUAssertLog(ready(), "Attempt to read a finished stream");
    if (_bufoff+3 >= _sbuffer.size()) { // Need a full UTF8 sequence
        fill();
    }
    
    size_t orig = data.size();
    
    std::string::iterator start = _sbuffer.begin()+_bufoff;
    utf8::next(start,_sbuffer.end());
    
    data.append(_sbuffer.begin()+_bufoff,start);
    _bufoff += (Sint32)(data.size()-orig);
    
    return data;
}

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
std::string TextReader::readLine() {
    std::string result;
    readLine(result);
    return result;
}

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
std::string& TextReader::readLine(std::string& data) {
    CUAssertLog(ready(), "Attempt to read a finished stream");
    if (_bufoff >= _sbuffer.size()) {
        fill();
    }
    
    bool found = false;
    while (!found) {
        size_t pos = _sbuffer.find('\n',_bufoff);
        if (pos != std::string::npos) {
            data.append(_sbuffer.begin()+_bufoff,_sbuffer.begin()+pos);
            _bufoff = (Sint32)(pos+1);
            found = true;
        } else {
            data.append(_sbuffer.begin()+_bufoff,_sbuffer.end());
            _bufoff = (Sint32)_sbuffer.size();
            fill();
        }
    }
    return data;
}

/**
 * Returns the unread remainder of the stream
 *
 * This method will exhaust the stream of all characters, but will not
 * close it.  Future attempts to read the stream will fail.
 *
 * @return the unread remainder of the stream
 */
std::string TextReader::readAll() {
    std::string result;
    readAll(result);
    return result;
}

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
std::string& TextReader::readAll(std::string& data) {
    CUAssertLog(ready(), "Attempt to read a finished stream");
    if (_bufoff >= _sbuffer.size()) {
        fill();
    }
    
    while (ready()) {
        data.append(_sbuffer.begin()+_bufoff,_sbuffer.end());
        _bufoff = (Sint32)_sbuffer.size();
        fill();
    }
    return data;
}

/**
 * Skips over any whitespace in the stream.
 *
 * This method will move the read head until it reaches a non-whitespace
 * character or the end of the file, which ever comes first.
 */
void TextReader::skip() {
    CUAssertLog(ready(), "Attempt to read a finished stream");
    if (_bufoff >= _sbuffer.size()) {
        fill();
    }
    
    bool found = false;
    while (isspace(_sbuffer[_bufoff]) && !found) {
        _bufoff++;
        if (_bufoff >= _sbuffer.size()) {
            if (ready()) {
                fill();
            } else {
                found = true;
            }
        }
    }
}


//
//  CUJsonReader.cpp
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
#include <cugl/io/CUJsonReader.h>
#include <cugl/util/CUDebug.h>

using namespace cugl;

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
std::string JsonReader::readJsonString() {
    CUAssertLog(ready(), "Attempt to read a finished stream");
    skip();
    
    // Make sure first character a bracket
    CUAssertLog(_sbuffer[_bufoff] == '{', "JSON is missing initial {");
    
    int depth = 0;
    std::string data;
    // Go until close brace.
    while (ready()) {
        fill();
        int pos = 0;
        for(auto it = _sbuffer.begin()+_bufoff; it != _sbuffer.end(); ++it) {
            if (*it == '{') {
                depth++;
            } else if (*it == '}') {
                depth--;
            }
            if (depth == 0) {
                data.append(_sbuffer.begin()+_bufoff,it+1);
                _bufoff += pos+1;
                return data;
            }
            pos++;
        }
        data.append(_sbuffer.begin()+_bufoff,_sbuffer.end());
        _bufoff =  (int)_sbuffer.size();
    }
    CUAssertLog(false, "JSON is missing closing }");
    return "";
}

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
std::shared_ptr<JsonValue> JsonReader::readJson() {
    std::string data = readJsonString();
    if (!data.empty()) {
        return JsonValue::allocWithJson(data);
    }
    return nullptr;
}

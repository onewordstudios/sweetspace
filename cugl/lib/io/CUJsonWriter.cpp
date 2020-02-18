//
//  CUJsonWriter.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides extends the basic TextWriter to support JSON encoding.
//  It does not require that the entire file conform to JSON standards; it can
//  write a JSON string embedded in a larger text file.
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
#include <cugl/io/CUJsonWriter.h>
#include <cugl/util/CUDebug.h>

using namespace cugl;

/**
 * Writes a JsonValue to the file, appending a newline at the end.
 *
 * The JSON may either be pretty-printed or condensed depending on the
 * value of format.  By default, we pretty-print all JSON strings.
 *
 * This method automatically flushes the buffer when done.
 *
 * @param json      The JSON value to write
 * @param format    Whether to pretty-print the JSON string
 */
void JsonWriter::writeJson(const JsonValue* json, bool format) {
    CUAssertLog(json, "Attempt to write a nullptr JSON");
    std::string data = json->toString(format);
    writeLine(data);
}

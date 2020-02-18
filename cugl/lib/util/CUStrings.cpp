//
//  CUStrings.cpp
//  Cornell University Game Library (CUGL)
//
//  Android does not support a lot of the built-in string methods.  Therefore,
//  we need alternate definitions that are platform agnostic.  Note that these
//  functions have names that are very similar to those in the std namespace,
//  but all live in the cocos2d namespace.
//
//  Note that this module does not refer to the integral types as short, int,
//  long, etc.  Those types are NOT cross-platform.  For example, a long is
//  8 bytes on Unix/OS X, but 4 bytes on some Win32 platforms.
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
//  Version: 2/10/16
//
#include <cugl/util/CUStrings.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>

#if defined (__ANDROID__)
#include <cstdlib>
#endif

namespace cugl {

#pragma mark NUMBER TO STRING FUNCTIONS
/**
 * Returns a string equivalent to the given byte
 *
 * The value is displayed as a number, not a character.
 *
 * @param  value    the numeric value to convert
 *
 * @return a string equivalent to the given byte
 */
std::string to_string(Uint8 value) {
#if defined (__ANDROID__)
    std::stringstream ss;
    ss << (Uint32)value;
    return ss.str();
#else
    return std::to_string((Uint32)value);
#endif
}

/**
 * Returns a string equivalent to the given signed 16 bit integer
 *
 * @param  value    the numeric value to convert
 *
 * @return a string equivalent to the given signed 16 bit integer
 */
std::string to_string(Sint16 value) {
#if defined (__ANDROID__)
    std::stringstream ss;
    ss << (Sint32)value;
    return ss.str();
#else
    return std::to_string((Sint32)value);
#endif
}

/**
 * Returns a string equivalent to the given unsigned 16 bit integer
 *
 * @param  value    the numeric value to convert
 *
 * @return a string equivalent to the given unsigned 16 bit integer
 */
std::string to_string(Uint16 value) {
#if defined (__ANDROID__)
    std::stringstream ss;
    ss << (Uint32)value;
    return ss.str();
#else
    return std::to_string((Uint32)value);
#endif
}

/**
 * Returns a string equivalent to the given signed 32 bit integer
 *
 * @param  value    the numeric value to convert
 *
 * @return a string equivalent to the given signed 32 bit integer
 */
std::string to_string(Sint32 value) {
#if defined (__ANDROID__)
    std::stringstream ss;
    ss << value;
    return ss.str();
#else
    return std::to_string(value);
#endif
}

/**
 * Returns a string equivalent to the given unsigned 32 bit integer
 *
 * @param  value    the numeric value to convert
 *
 * @return a string equivalent to the given unsigned 32 bit integer
 */
std::string to_string(Uint32 value ) {
#if defined (__ANDROID__)
    std::stringstream ss;
    ss << value;
    return ss.str();
#else
    return std::to_string(value);
#endif
}

/**
 * Returns a string equivalent to the given signed 64 bit integer
 *
 * @param  value    the numeric value to convert
 *
 * @return a string equivalent to the given signed 64 bit integer
 */
std::string to_string(Sint64 value) {
#if defined (__ANDROID__)
    std::stringstream ss;
    ss << value;
    return ss.str();
#else
    return std::to_string(value);
#endif
}

/**
 * Returns a string equivalent to the given unsigned 64 bit integer
 *
 * @param  value    the numeric value to convert
 *
 * @return a string equivalent to the given unsigned 64 bit integer
 */
std::string to_string(Uint64 value ) {
#if defined (__ANDROID__)
    std::stringstream ss;
    ss << value;
    return ss.str();
#else
    return std::to_string(value);
#endif
}

/**
 * Returns a string equivalent to the given float value
 *
 * This function differs from std::to_string(float) in that it allows us
 * to specify a precision (the number of digits to display after the decimal
 * point).  If precision is negative, then maximum precision will be used.
 *
 * @param  value        the numeric value to convert
 * @param  precision    the number of digits to display after the decimal
 *
 * @return a string equivalent to the given float value
 */
std::string to_string(float value, int precision) {
    int width = (precision >= 0 ? precision : std::numeric_limits<long double>::digits10 + 1);
    std::stringstream ss;
    ss << std::fixed << std::setprecision(width) << value;
    return ss.str();
}

/**
 * Returns a string equivalent to the given double value
 *
 * This function differs from std::to_string(double) in that it allows us
 * to specify a precision (the number of digits to display after the decimal
 * point).  If precision is negative, then maximum precision will be used.
 *
 * @param  value        the numeric value to convert
 * @param  precision    the number of digits to display after the decimal
 *
 * @return a string equivalent to the given double value
 */
std::string to_string(double value, int precision) {
    int width = (precision >= 0 ? precision : std::numeric_limits<long double>::digits10 + 1);
    std::stringstream ss;
    ss << std::fixed << std::setprecision(width) << value;
    return ss.str();
}


#pragma mark -
#pragma mark ARRAY TO STRING FUNCTIONS
/**
 * Returns a string equivalent to the given byte array
 *
 * The value is display as a python-style list in brackets.
 *
 * @param array     the array to convert
 * @param length    the array length
 * @param offset    the starting position in the array
 *
 * @return a string equivalent to the given byte array
 */
std::string to_string(Uint8* array, size_t length, size_t offset) {
    std::stringstream ss;
    ss << "[";
    for(int ii = 0; ii < length; ii++) {
        ss << (Uint32)array[ii+offset];
        if (ii != length-1) {
            ss << ", ";
        }
    }
    ss << "]";
    return ss.str();
}

/**
 * Returns a string equivalent to the signed 16 bit integer array
 *
 * The value is display as a python-style list in brackets.
 *
 * @param array     the array to convert
 * @param length    the array length
 * @param offset    the starting position in the array
 *
 * @return a string equivalent to the signed 16 bit integer array
 */
std::string to_string(Sint16* array, size_t length, size_t offset) {
    std::stringstream ss;
    ss << "[";
    for(int ii = 0; ii < length; ii++) {
        ss << array[ii+offset];
        if (ii != length-1) {
            ss << ", ";
        }
    }
    ss << "]";
    return ss.str();
}

/**
 * Returns a string equivalent to the unsigned 16 bit integer array
 *
 * The value is display as a python-style list in brackets.
 *
 * @param array     the array to convert
 * @param length    the array length
 * @param offset    the starting position in the array
 *
 * @return a string equivalent to the unsigned 16 bit integer array
 */
std::string to_string(Uint16* array, size_t length, size_t offset) {
    std::stringstream ss;
    ss << "[";
    for(int ii = 0; ii < length; ii++) {
        ss << array[ii+offset];
        if (ii != length-1) {
            ss << ", ";
        }
    }
    ss << "]";
    return ss.str();
}


/**
 * Returns a string equivalent to the signed 32 bit integer array
 *
 * The value is display as a python-style list in brackets.
 *
 * @param array     the array to convert
 * @param length    the array length
 * @param offset    the starting position in the array
 *
 * @return a string equivalent to the signed 32 bit integer array
 */
std::string to_string(Sint32* array, size_t length, size_t offset) {
    std::stringstream ss;
    ss << "[";
    for(int ii = 0; ii < length; ii++) {
        ss << array[ii+offset];
        if (ii != length-1) {
            ss << ", ";
        }
    }
    ss << "]";
    return ss.str();
}


/**
 * Returns a string equivalent to the unsigned 32 bit integer array
 *
 * The value is display as a python-style list in brackets.
 *
 * @param array     the array to convert
 * @param length    the array length
 * @param offset    the starting position in the array
 *
 * @return a string equivalent to the unsigned 32 bit integer array
 */
std::string to_string(Uint32* array, size_t length, size_t offset) {
    std::stringstream ss;
    ss << "[";
    for(int ii = 0; ii < length; ii++) {
        ss << array[ii+offset];
        if (ii != length-1) {
            ss << ", ";
        }
    }
    ss << "]";
    return ss.str();
}

/**
 * Returns a string equivalent to the signed 64 bit integer array
 *
 * The value is display as a python-style list in brackets.
 *
 * @param array     the array to convert
 * @param length    the array length
 * @param offset    the starting position in the array
 *
 * @return a string equivalent to the signed 64 bit integer array
 */
std::string to_string(Sint64* array, size_t length, size_t offset) {
    std::stringstream ss;
    ss << "[";
    for(int ii = 0; ii < length; ii++) {
        ss << array[ii+offset];
        if (ii != length-1) {
            ss << ", ";
        }
    }
    ss << "]";
    return ss.str();
}


/**
 * Returns a string equivalent to the unsigned 64 bit integer array
 *
 * The value is display as a python-style list in brackets.
 *
 * @param array     the array to convert
 * @param length    the array length
 * @param offset    the starting position in the array
 *
 * @return a string equivalent to the unsigned 64 bit integer array
 */
std::string to_string(Uint64* array, size_t length, size_t offset) {
    std::stringstream ss;
    ss << "[";
    for(int ii = 0; ii < length; ii++) {
        ss << array[ii+offset];
        if (ii != length-1) {
            ss << ", ";
        }
    }
    ss << "]";
    return ss.str();
}


/**
 * Returns a string equivalent to the given float array
 *
 * The value is display as a python-style list in brackets.
 *
 * As with to_string(float), this function allows us to specify a precision
 * (the number of digits to display after the decimal point).  If precision is
 * negative, then maximum precision will be used.
 *
 * @param array     the array to convert
 * @param length    the array length
 * @param offset    the starting position in the array
 *
 * @return a string equivalent to the given float array
 */
std::string to_string(float* array, size_t length, size_t offset, int precision) {
    int width = (precision >= 0 ? precision : std::numeric_limits<long double>::digits10 + 1);
    std::stringstream ss;
    ss << "[" << std::fixed << std::setprecision(width);
    for(int ii = 0; ii < length; ii++) {
        ss << array[ii+offset] << 'f';
        if (ii != length-1) {
            ss << ", ";
        }
    }
    ss << "]";
    return ss.str();
}

/**
 * Returns a string equivalent to the given double array
 *
 * The value is display as a python-style list in brackets.
 *
 * As with to_string(double), this function allows us to specify a precision
 * (the number of digits to display after the decimal point).  If precision is
 * negative, then maximum precision will be used.
 *
 * @param array     the array to convert
 * @param length    the array length
 * @param offset    the starting position in the array
 *
 * @return a string equivalent to the given double array
 */
std::string to_string(double* array, size_t length, size_t offset, int precision) {
    int width = (precision >= 0 ? precision : std::numeric_limits<long double>::digits10 + 1);
    std::stringstream ss;
    ss << "[" << std::fixed << std::setprecision(width);
    for(int ii = 0; ii < length; ii++) {
        ss << array[ii+offset];
        if (ii != length-1) {
            ss << ", ";
        }
    }
    ss << "]";
    return ss.str();
}


#pragma mark -
#pragma mark STRING TO NUMBER FUNCTIONS

/**
 * Returns the byte equivalent to the given string
 *
 * This function discards any whitespace characters (as identified by calling isspace())
 * until the first non-whitespace character is found, then takes as many characters
 * as possible to form a valid base-n (where n=base) integer number representation
 * and converts them to an integer value.
 *
 * @param  str  the string to convert
 * @param  pos  address of an integer to store the number of characters processed
 * @param  base the number base
 *
 * @return the byte equivalent to the given string
 */
Uint8 stou8(const std::string& str, std::size_t* pos, int base) {
#if defined (__ANDROID__)
    const char* start = str.c_str();
    char* end;
    int result = (Uint8)std::strtol(start, &end, base);
    *pos = (std::size_t)(end-start); // Bad but no alternative on android
    return result;
#else
    return (Uint8)std::stoi(str,pos,base);
#endif
}

/**
 * Returns the signed 16 bit integer equivalent to the given string
 *
 * This function discards any whitespace characters (as identified by calling isspace())
 * until the first non-whitespace character is found, then takes as many characters
 * as possible to form a valid base-n (where n=base) integer number representation
 * and converts them to a long value.
 *
 * @param  str  the string to convert
 * @param  pos  address of an integer to store the number of characters processed
 * @param  base the number base
 *
 * @return the signed 16 bit integer equivalent to the given string
 */
Sint16 stos16(const std::string& str, std::size_t* pos, int base)  {
#if defined (__ANDROID__)
    const char* start = str.c_str();
    char* end;
    int result = (Sint16)std::strtol(start, &end, base);
    *pos = (std::size_t)(end-start); // Bad but no alternative on android
    return result;
#else
    return (Sint16)std::stoi(str,pos,base);
#endif
}

/**
 * Returns the unsigned 16 bit integer equivalent to the given string
 *
 * This function discards any whitespace characters (as identified by calling isspace())
 * until the first non-whitespace character is found, then takes as many characters
 * as possible to form a valid base-n (where n=base) integer number representation
 * and converts them to a long value.
 *
 * @param  str  the string to convert
 * @param  pos  address of an integer to store the number of characters processed
 * @param  base the number base
 *
 * @return the unsigned 16 bit integer equivalent to the given string
 */
Uint16 stou16(const std::string& str, std::size_t* pos, int base) {
#if defined (__ANDROID__)
    const char* start = str.c_str();
    char* end;
    int result = (Uint16)std::strtol(start, &end, base);
    *pos = (std::size_t)(end-start); // Bad but no alternative on android
    return result;
#else
    return (Uint16)std::stol(str,pos,base);
#endif
}

/**
 * Returns the signed 32 bit integer equivalent to the given string
 *
 * This function discards any whitespace characters (as identified by calling isspace())
 * until the first non-whitespace character is found, then takes as many characters
 * as possible to form a valid base-n (where n=base) integer number representation
 * and converts them to a long value.
 *
 * @param  str  the string to convert
 * @param  pos  address of an integer to store the number of characters processed
 * @param  base the number base
 *
 * @return the signed 32 bit integer equivalent to the given string
 */
Sint32 stos32(const std::string& str, std::size_t* pos, int base) {
#if defined (__ANDROID__)
    const char* start = str.c_str();
    char* end;
    int result = (Sint32)std::strtol(start, &end, base);
    *pos = (std::size_t)(end-start); // Bad but no alternative on android
    return result;
#else
    return (Sint32)std::stol(str,pos,base);
#endif
}

/**
 * Returns the unsigned 32 bit integer equivalent to the given string
 *
 * This function discards any whitespace characters (as identified by calling isspace())
 * until the first non-whitespace character is found, then takes as many characters
 * as possible to form a valid base-n (where n=base) integer number representation
 * and converts them to a long value.
 *
 * @param  str  the string to convert
 * @param  pos  address of an integer to store the number of characters processed
 * @param  base the number base
 *
 * @return the unsigned 32 bit integer equivalent to the given string
 */
Uint32 stou32(const std::string& str, std::size_t* pos, int base) {
#if defined (__ANDROID__)
    const char* start = str.c_str();
    char* end;
    int result = (Uint32)std::strtoul(start, &end, base);
    *pos = (std::size_t)(end-start); // Bad but no alternative on android
    return result;
#else
    return (Uint32)std::stoul(str,pos,base);
#endif
}

/**
 * Returns the signed 64 bit integer equivalent to the given string
 *
 * This function discards any whitespace characters (as identified by calling isspace())
 * until the first non-whitespace character is found, then takes as many characters
 * as possible to form a valid base-n (where n=base) integer number representation
 * and converts them to a long value.
 *
 * @param  str  the string to convert
 * @param  pos  address of an integer to store the number of characters processed
 * @param  base the number base
 *
 * @return the signed 64 bit integer equivalent to the given string
 */
Sint64 stos64(const std::string& str, std::size_t* pos, int base) {
#if defined (__ANDROID__)
    const char* start = str.c_str();
    char* end;
    int result = (Sint64)std::strtoll(start, &end, base);
    *pos = (std::size_t)(end-start); // Bad but no alternative on android
    return result;
#else
    return (Sint64)std::stoll(str,pos,base);
#endif
}


/**
 * Returns the unsigned 64 bit integer equivalent to the given string
 *
 * This function discards any whitespace characters (as identified by calling isspace())
 * until the first non-whitespace character is found, then takes as many characters
 * as possible to form a valid base-n (where n=base) integer number representation
 * and converts them to a long value.
 *
 * @param  str  the string to convert
 * @param  pos  address of an integer to store the number of characters processed
 * @param  base the number base
 *
 * @return the unsigned 64 bit integer equivalent to the given string
 */
Uint64 stou64(const std::string& str, std::size_t* pos, int base) {
#if defined (__ANDROID__)
    const char* start = str.c_str();
    char* end;
    int result = (Uint64)std::strtoull(start, &end, base);
    *pos = (std::size_t)(end-start); // Bad but no alternative on android
    return result;
#else
    return (Uint64)std::stoull(str,pos,base);
#endif
}

/**
 * Returns the float equivalent to the given string
 *
 * This function discards any whitespace characters (as determined by std::isspace())
 * until first non-whitespace character is found. Then it takes as many characters as
 * possible to form a valid floating point representation and converts them to a floating
 * point value.
 *
 * @param  str  the string to convert
 * @param  pos  address of an integer to store the number of characters processed
 *
 * @return the float equivalent to the given string
 */
float  stof(const std::string& str, std::size_t* pos) {
#if defined (__ANDROID__)
    const char* start = str.c_str();
    char* end;
    float result = (float)std::strtod(start, &end);
    *pos = (std::size_t)(end-start); // Bad but no alternative on android
    return result;
#else
    return std::stof(str,pos);
#endif
}

/**
 * Returns the double equivalent to the given string
 *
 * This function discards any whitespace characters (as determined by std::isspace())
 * until first non-whitespace character is found. Then it takes as many characters as
 * possible to form a valid floating point representation and converts them to a floating
 * point value.
 *
 * @param  str  the string to convert
 * @param  pos  address of an integer to store the number of characters processed
 *
 * @return the double equivalent to the given string
 */
double stod(const std::string& str, std::size_t* pos) {
#if defined (__ANDROID__)
    const char* start = str.c_str();
    char* end;
    double result = std::strtod(start, &end);
    *pos = (std::size_t)(end-start); // Bad but no alternative on android
    return result;
#else
    return std::stod(str,pos);
#endif
}

#pragma mark -
#pragma mark UTILITY FUNCTIONS
/**
 * Returns a lower case copy of str.
 *
 * This function uses the current C++ locale.
 *
 * @param str   The string to convert
 *
 * @return a lower case copy of str.
 */
std::string to_lower(const std::string& str) {
    std::locale loc;
    std::string result;
    for(auto it = str.begin(); it != str.end(); ++it) {
        result.push_back(std::tolower(*it, loc));
    }
    return result;
}

/**
 * Returns an upper case copy of str.
 *
 * This function uses the current C++ locale.
 *
 * @param str   The string to convert
 *
 * @return an upper case copy of str.
 */
std::string to_upper(const std::string& str) {
    std::locale loc;
    std::string result;
    for(auto it = str.begin(); it != str.end(); ++it) {
        result.push_back(std::tolower(*it, loc));
    }
    return result;
}

/**
 * Returns a copy of str with any leading and trailing whitespace removed.
 *
 * This function uses the current C++ locale.
 *
 * @param str   The string to trim
 *
 * @return a copy of str with any leading and trailing whitespace removed.
 */
std::string trim(const std::string& str) {
    std::locale loc;
    std::string::size_type pos1 = std::string::npos;
    for(std::string::size_type ii = 0; pos1 == std::string::npos && ii < str.size(); ii++) {
        if (!std::isspace(str[ii], loc)) {
            pos1 = ii;
        }
    }
    if (pos1 == std::string::npos) {
        return std::string("");
    }
    std::string::size_type pos2 = std::string::npos;
    for(std::string::size_type ii = str.size()-1; pos2 == std::string::npos && ii > 0; ii--) {
        if (!std::isspace(str[ii], loc)) {
            pos2 = ii;
        }
    }
    if (pos2 == std::string::npos && !std::isspace(str[0], loc)) {
        pos2 = 0;
    }
    return str.substr(pos1, pos2-pos1+1);
}
    

/**
 * Returns a list of substrings separate by the given separator
 *
 * The separator is interpretted exactly; no whitespace is remove around
 * the separator.  If the separator is the empty string, this function
 * will return a list of the characters in str.
 *
 * @param str   The string to split
 * @param sep   The splitting delimeter
 *
 * @return a list of substrings separate by the given separator
 */
std::vector<std::string> split(const std::string& str, const std::string& sep) {
    std::vector<std::string> result;
    std::string::size_type start = 0;
    std::string::size_type end = str.find(sep,start);
    while (end != std::string::npos) {
        result.push_back(str.substr(start,end-start));
        start = end+sep.size();
        end = str.find(sep,start);
    }
    result.push_back(str.substr(start,end-start));
    return result;
}

/**
 * Returns a list of substrings separate by the given separator
 *
 * The separator is interpretted exactly; no whitespace is remove around
 * the separator.  If the separator is the empty string, this function
 * will return a list of the characters in str.
 *
 * @param str   The string to split
 * @param sep   The splitting delimeter
 *
 * @return a list of substrings separate by the given separator
 */
std::vector<std::string> split(const std::string& str, const char* sep) {
    return split(str,std::string(sep));
}

/**
 * Returns true if the string only contains alphabetic characters.
 *
 * This function uses the current C++ locale.
 *
 * @param str   The string to check
 *
 * @return true if the string only contains alphabetic characters.
 */
bool isalpha(const std::string& str) {
    std::locale loc;
    bool result = true;
    for(auto it = str.begin(); result && it != str.end(); ++it) {
        result = result && std::isalpha(*it, loc);
    }
    return result;
}

/**
 * Returns true if the string only contains alphabetic and numeric characters.
 *
 * This function uses the current C++ locale.
 *
 * @param str   The string to check
 *
 * @return true if the string only contains alphabetic and numeric characters.
 */
bool isalphanum(const std::string& str) {
    std::locale loc;
    bool result = true;
    for(auto it = str.begin(); result && it != str.end(); ++it) {
        result = result && std::isalnum(*it, loc);
    }
    return result;
}

/**
 * Returns true if the string only contains numeric characters.
 *
 * This function uses the current C++ locale.
 *
 * @param str   The string to check
 *
 * @return true if the string only contains numeric characters.
 */
bool isnumeric(const std::string& str) {
    std::locale loc;
    bool result = true;
    for(auto it = str.begin(); result && it != str.end(); ++it) {
        result = result && std::isdigit(*it, loc);
    }
    return result;
}

/**
 * Returns true if the string can safely be converted to a number (double)
 *
 * This function uses the current C++ locale.
 *
 * @param str   The string to check
 *
 * @return true if the string can safely be converted to a number (double)
 */
bool isnumber(const std::string& str) {
    size_t p;
    cugl::stod(str, &p);
    return p != 0;
}

}

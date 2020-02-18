//
//  CUColor4.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a standard colors.  It provides both a
//  float based color solution and a byte-based color solution.  The former
//  is better for blending and calculations.  The later is better for storage.
//
//  Because math objects are intended to be on the stack, we do not provide
//  any shared pointer support in this class.
//
//  This module is based on an original file from GamePlay3D: http://gameplay3d.org.
//  It has been modified to support the CUGL framework.
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
//  Version: 5/30/16
#include <SDL/SDL.h>
#include <algorithm>
#include <iostream>
#include <sstream>

#include <cugl/util/CUDebug.h>
#include <cugl/util/CUStrings.h>
#include <cugl/math/CUColor4.h>
#include <cugl/math/CUVec3.h>
#include <cugl/math/CUVec4.h>

using namespace cugl;

#pragma mark -
#pragma mark Color with Float Attributes

/**
 * Constructs a new color initialized to the specified values.
 *
 * The color values must all in the range 0..1.
 *
 * @param r The red color.
 * @param g The green color.
 * @param b The blue color.
 * @param a The alpha value (optional).
 */
Color4f::Color4f(float r, float g, float b, float a) {
    CUAssertLog(0 <= r && r <= 1, "Red value out of range: %.3f", r);
    CUAssertLog(0 <= g && g <= 1, "Green value out of range: %.3f", g);
    CUAssertLog(0 <= b && b <= 1, "Blue value out of range: %.3f", b);
    CUAssertLog(0 <= a && a <= 1, "Alpha value out of range: %.3f", a);
    this->r = r; this->g = g; this->b = b; this->a = a;
}

/**
 * Constructs a new color from the values in the specified array.
 *
 * The color values must all in the range 0..1.
 *
 * @param array An array containing the color values in the order r, g, b, a.
 */
Color4f::Color4f(const float* array) {
    CUAssertLog(0 <= array[0] && array[0] <= 1, "Red value out of range: %.3f", array[0]);
    CUAssertLog(0 <= array[1] && array[1] <= 1, "Green value out of range: %.3f", array[1]);
    CUAssertLog(0 <= array[2] && array[2] <= 1, "Blue value out of range: %.3f", array[2]);
    CUAssertLog(0 <= array[3] && array[3] <= 1, "Alpha value out of range: %.3f", array[3]);
    this->r = array[0]; this->g = array[1]; this->b = array[2]; this->a = array[3];
}

/**
 * Creates a new color from an integer interpreted as an RGBA value.
 *
 * This method converts the RGBA value to a Color4 and then converts the
 * result to a Color4f. This representation is endian dependent. Do not
 * serialize this value.
 *
 * @param color The integer to interpret as an RGBA value.
 */
Color4f::Color4f(unsigned int color) {
    set(color);
}

/**
 * Sets the elements of this color to the specified values.
 *
 * The color values must all in the range 0..1.
 *
 * @param r The red color.
 * @param g The green color.
 * @param b The blue color.
 * @param a The alpha value (optional).
 *
 * @return A reference to this (modified) Color4f for chaining.
 */
Color4f& Color4f::set(float r, float g, float b, float a) {
    CUAssertLog(0 <= r && r <= 1, "Red value out of range: %.3f", r);
    CUAssertLog(0 <= g && g <= 1, "Green value out of range: %.3f", g);
    CUAssertLog(0 <= b && b <= 1, "Blue value out of range: %.3f", b);
    CUAssertLog(0 <= a && a <= 1, "Alpha value out of range: %.3f", a);
    this->r = r; this->g = g; this->b = b; this->a = a;
    return *this;
}

/**
 * Sets the elements of this color from the values in the specified array.
 *
 * The color values must all in the range 0..1.
 *
 * @param array An array containing the color values in the order r, g, b, a.
 *
 * @return A reference to this (modified) Color4f for chaining.
 */
Color4f& Color4f::set(const float* array) {
    CUAssertLog(0 <= array[0] && array[0] <= 1, "Red value out of range: %.3f", array[0]);
    CUAssertLog(0 <= array[1] && array[1] <= 1, "Green value out of range: %.3f", array[1]);
    CUAssertLog(0 <= array[2] && array[2] <= 1, "Blue value out of range: %.3f", array[2]);
    CUAssertLog(0 <= array[3] && array[3] <= 1, "Alpha value out of range: %.3f", array[3]);
    this->r = array[0]; this->g = array[1]; this->b = array[2]; this->a = array[3];
    return *this;
}

/**
 * Sets this color to an integer interpreted as an RGBA value.
 *
 * This method converts the RGBA value to a Color4 and then converts the
 * result to a Color4f. This representation is endian dependent. Do not
 * serialize this value.
 *
 * @param color The integer to interpret as an RGBA value.
 *
 * @return A reference to this (modified) Color4f for chaining.
 */
Color4f& Color4f::set(unsigned int color) {
    Color4 temp(color);
    r = COLOR_BYTE_TO_FLOAT(temp.r);
    g = COLOR_BYTE_TO_FLOAT(temp.g);
    b = COLOR_BYTE_TO_FLOAT(temp.b);
    a = COLOR_BYTE_TO_FLOAT(temp.a);
    return *this;
}

/**
 * Returns the packed integer representation of this color
 *
 * This method converts the color to a Color4 and returns the packed color
 * of that result.  The answer is endian dependent.  Do not serialize this
 * value.
 *
 * @return the packed integer representation of this color
 */
GLuint Color4f::getRGBA() const {
    return Color4(*this).getRGBA();
}

#pragma mark Comparisons
/**
 * Returns true if this color is less than the given color.
 *
 * This comparison uses lexicographical order of rgba.  To test if all
 * components in this color are less than those of c, use the method
 * darkerThan().
 *
 * @param c The color to compare against.
 *
 * @return True if this color is less than the given color.
 */
bool Color4f::operator<(const Color4f& c) const {
    bool result = r < c.r || (r == c.r && g < c.g);
    result = result ||  (r == c.r && g == c.g && b < c.b);
    result = result ||  (r == c.r && g == c.g && b == c.b && a < c.a);
    return result;
}

/**
 * Returns true if this color is greater than the given color.
 *
 * This comparison uses lexicographical order of rgba.  To test if all
 * components in this color are greater than those of c, use the method
 * lighterThan().
 *
 * @param c The color to compare against.
 *
 * @return True if this color is greater than the given color.
 */
bool Color4f::operator>(const Color4f& c) const {
    bool result = r > c.r || (r == c.r && g > c.g);
    result = result ||  (r == c.r && g == c.g && b > c.b);
    result = result ||  (r == c.r && g == c.g && b == c.b && a > c.a);
    return result;
}

#pragma mark Arithmetic

/**
 * Clamps this color within the given range.
 *
 * @param min The minimum value.
 * @param max The maximum value.
 *
 * @return A reference to this (modified) Color4f for chaining.
 */
Color4f& Color4f::clamp(const Color4f& min, const Color4f& max) {
    r = clampf(r,min.r,max.r);
    g = clampf(g,min.g,max.g);
    b = clampf(b,min.b,max.b);
    a = clampf(a,min.a,max.a);
    return *this;
}

/**
 * Interpolates the two colors c1 and c2, and stores the result in dst.
 *
 * If alpha is 0, the result is c1.  If alpha is 1, the color is c2.
 * Otherwise it is a value in c1..c2.  If alpha is outside of the
 * range 0 to 1, it is clamped to the nearest value.
 *
 * This method just implements standard linear interpolation.  It does
 * not attempt to give any blending semantics to it.
 *
 * @param c1    The first color.
 * @param c2    The second color.
 * @param alpha The interpolation value in 0..1
 * @param dst   The color to store the result in
 *
 * @return A reference to dst for chaining
 */
Color4f* Color4f::lerp(const Color4f& c1, const Color4f& c2, float alpha, Color4f* dst) {
    if (dst == nullptr) { return nullptr; }
    float x = clampf(alpha,0,1);
    *dst = c1 * (1.f - x) + c2 * x;
    return dst;
}

/**
 * Blends the two colors c1 and c2, assuming they are not premultiplied.
 *
 * The blending is the standard over operation with color c1 as the source
 * and c2 as the destination. It assumes that the color values are not
 * premultiplied.
 *
 * @param c1    The source color.
 * @param c2    The destination color.
 * @param dst   The color to store the result in
 *
 * @return A reference to dst for chaining
 */
Color4f* Color4f::blend(const Color4f& c1, const Color4f& c2, Color4f* dst) {
    float a1 = c2.a*(1-c1.a);
    float a2 = c1.a+a1;
    dst->r = (c1.r*c1.a+c2.r*a1)/a2;
    dst->g = (c1.g*c1.a+c2.g*a1)/a2;
    dst->b = (c1.b*c1.a+c2.b*a1)/a2;
    dst->a = a2;
    return dst;
}

/**
 * Blends the two colors c1 and c2, assuming they are premultiplied.
 *
 * The blending is the standard over operation with color c1 as the source
 * and c2 as the destination. It assumes that the color values are
 * premultiplied.
 *
 * @param c1    The source color.
 * @param c2    The destination color.
 * @param dst   The color to store the result in
 *
 * @return A reference to dst for chaining
 */
Color4f* Color4f::blendPre(const Color4f& c1, const Color4f& c2, Color4f* dst) {
    float a = c1.a+c2.a*(1-c1.a);
    dst->r = (c1.r+c2.r*(1-c1.a));
    dst->g = (c1.g+c2.g*(1-c1.a));
    dst->b = (c1.b+c2.b*(1-c1.a));
    dst->a = a;
    return dst;
}

#pragma mark Conversions

/**
 * Returns a string representation of this vector for debuggging purposes.
 *
 * If verbose is true, the string will include class information.  This
 * allows us to unambiguously identify the class.
 *
 * @param verbose Whether to include class information
 *
 * @return a string representation of this vector for debuggging purposes.
 */
std::string Color4f::toString(bool verbose) const {
    std::stringstream ss;
    ss << (verbose ? "cugl::Color4f[r=" : "[r=");
    ss <<  to_string(r);
    ss << ",g=";
    ss << to_string(g);
    ss << ",b=";
    ss << to_string(b);
    ss << ",a=";
    ss << to_string(a);
    ss << "]";
    return ss.str();
}

/** Cast from Color4f to a vector. */
Color4f::operator Vec4() const {
    return Vec4(r,g,b,a);
}

/**
 * Creates a color from the given vector.
 *
 * The attributes are read in the order x,y,z,w.
 *
 * @param vector    The vector to convert
 */
Color4f::Color4f(const Vec4& vector) {
    CUAssertLog(0 <= vector.x && vector.x <= 1, "Red value out of range: %.3f", vector.x);
    CUAssertLog(0 <= vector.y && vector.y <= 1, "Green value out of range: %.3f", vector.y);
    CUAssertLog(0 <= vector.z && vector.z <= 1, "Blue value out of range: %.3f", vector.z);
    CUAssertLog(0 <= vector.w && vector.w <= 1, "Alpha value out of range: %.3f", vector.w);
    r = vector.x; g = vector.y; b = vector.z; a = vector.w;
}

/**
 * Sets the coordinates of this color to those of the given vector.
 *
 * The attributes are read in the order x,y,z,w.
 *
 * @param vector    The vector to convert
 *
 * @return A reference to this (modified) Color4f for chaining.
 */
Color4f& Color4f::operator= (const Vec4& vector) {
    CUAssertLog(0 <= vector.x && vector.x <= 1, "Red value out of range: %.3f", vector.x);
    CUAssertLog(0 <= vector.y && vector.y <= 1, "Green value out of range: %.3f", vector.y);
    CUAssertLog(0 <= vector.z && vector.z <= 1, "Blue value out of range: %.3f", vector.z);
    CUAssertLog(0 <= vector.w && vector.w <= 1, "Alpha value out of range: %.3f", vector.w);
    r = vector.x; g = vector.y; b = vector.z; a = vector.w;
    return *this;
}

/** Cast from Color4f to a vector. */
Color4f::operator Vec3() const {
    return Vec3(r,g,b);
}

/**
 * Creates a color from the given vector.
 *
 * The attributes are read in the order x,y,z. The alpha value is 1.
 *
 * @param vector    The vector to convert
 */
Color4f::Color4f(const Vec3& vector) {
    CUAssertLog(0 <= vector.x && vector.x <= 1, "Red value out of range: %.3f", vector.x);
    CUAssertLog(0 <= vector.y && vector.y <= 1, "Green value out of range: %.3f", vector.y);
    CUAssertLog(0 <= vector.z && vector.z <= 1, "Blue value out of range: %.3f", vector.z);
    r = vector.x; g = vector.y; b = vector.z; a = 1;
}

/**
 * Sets the coordinates of this color to those of the given vector.
 *
 * The attributes are read in the order x,y,z. The alpha value is 1.
 *
 * @param vector    The vector to convert
 *
 * @return A reference to this (modified) Color4f for chaining.
 */
Color4f& Color4f::operator= (const Vec3& vector) {
    CUAssertLog(0 <= vector.x && vector.x <= 1, "Red value out of range: %.3f", vector.x);
    CUAssertLog(0 <= vector.y && vector.y <= 1, "Green value out of range: %.3f", vector.y);
    CUAssertLog(0 <= vector.z && vector.z <= 1, "Blue value out of range: %.3f", vector.z);
    r = vector.x; g = vector.y; b = vector.z; a = 1;
    return *this;
}

/** Cast from Color4f to a byte-based Color4. */
Color4f::operator Color4() const {
    return Color4(COLOR_FLOAT_TO_BYTE(r),COLOR_FLOAT_TO_BYTE(g),
                 COLOR_FLOAT_TO_BYTE(b),COLOR_FLOAT_TO_BYTE(a));
}

/**
 * Creates a float-based color from the given byte-based color.
 *
 * The attributes are read in the order r,g,b,a. They are all divided by
 * 255.0f before assignment.
 *
 * @param color The color to convert
 */
Color4f::Color4f(Color4 color) {
    r = COLOR_BYTE_TO_FLOAT(color.r);
    g = COLOR_BYTE_TO_FLOAT(color.g);
    b = COLOR_BYTE_TO_FLOAT(color.b);
    a = COLOR_BYTE_TO_FLOAT(color.a);
}

/**
 * Sets the attributes of this color from the given byte-based color.
 *
 * The attributes are read in the order r,g,b,a. They are all divided by
 * 255.0f before assignment.
 *
 * @param color The color to convert.
 *
 * @return A reference to this (modified) Color4f for chaining.
 */
Color4f& Color4f::operator= (Color4 color) {
    r = COLOR_BYTE_TO_FLOAT(color.r);
    g = COLOR_BYTE_TO_FLOAT(color.g);
    b = COLOR_BYTE_TO_FLOAT(color.b);
    a = COLOR_BYTE_TO_FLOAT(color.a);
    return *this;
}

#pragma mark Constants

/** The Clear color (0,0,0,0) */
const Color4f Color4f::CLEAR(0.0f,0.0f,0.0f,0.0f);
/** The White color (1,1,1,1) */
const Color4f Color4f::WHITE(1.0f,1.0f,1.0f,1.0f);
/** The Black color (0,0,0,1) */
const Color4f Color4f::BLACK(0.0f,0.0f,0.0f,1.0f);
/** The Yellow color (1,1,0,1) */
const Color4f Color4f::YELLOW(1.0f,1.0f,0.0f,1.0f);
/** The Blue color (0,0,1,1) */
const Color4f Color4f::BLUE(0.0f,0.0f,1.0f,1.0f);
/** The Green color (0,1,0,1) */
const Color4f Color4f::GREEN(0.0f,1.0f,0.0f,1.0f);
/** The Red color (1,0,0,1) */
const Color4f Color4f::RED(1.0f,0.0f,0.0f,1.0f);
/** The Magenta color (1,0,1,1) */
const Color4f Color4f::MAGENTA(1.0f,0.0f,1.0f,1.0f);
/** The Cyan color (0,1,1,1) */
const Color4f Color4f::CYAN(0.0f,1.0f,1.0f,1.0f);
/** The Orange color (1,0.5,0,1) */
const Color4f Color4f::ORANGE(1.0f,0.5f,0.0f,1.0f);
/** The Gray color (0.65,0.65,0.65,1) */
const Color4f Color4f::GRAY(0.65f,0.65f,0.65f,1.0f);
/** The classic XNA color (0.392f,0.584f,0.93f,1,0f) */
const Color4f Color4f::CORNFLOWER(0.392f,0.584f,0.93f,1.0f);
/** The Playing Fields color (0.8f,0.8f,0.5f,1.0f) */
const Color4f Color4f::PAPYRUS(0.8f,0.8f,0.5f,1.0f);



#pragma mark -
#pragma mark Color with Byte Attributes

/**
 * Constructs a new color from the values in the specified array.
 *
 * The color values must all be in the range 0..1. They are multiplied
 * by 255.0 and rounded up
 *
 * @param array An array containing the color values in the order r, g, b, a.
 */
Color4::Color4(const float* array) {
    CUAssertLog(0 <= array[0] && array[0] <= 1, "Red value out of range: %.3f", array[0]);
    CUAssertLog(0 <= array[1] && array[1] <= 1, "Green value out of range: %.3f", array[1]);
    CUAssertLog(0 <= array[2] && array[2] <= 1, "Blue value out of range: %.3f", array[2]);
    CUAssertLog(0 <= array[3] && array[3] <= 1, "Alpha value out of range: %.3f", array[3]);
    this->r = COLOR_FLOAT_TO_BYTE(array[0]);
    this->g = COLOR_FLOAT_TO_BYTE(array[1]);
    this->b = COLOR_FLOAT_TO_BYTE(array[2]);
    this->a = COLOR_FLOAT_TO_BYTE(array[3]);
}

/**
 * Sets the elements of this color from the values in the specified array.
 *
 * The color values must all be in the range 0..1. They are multiplied
 * by 255.0 and rounded up
 *
 * @param array An array containing the color values in the order r, g, b, a.
 *
 * @return A reference to this (modified) Color4f for chaining.
 */
Color4& Color4::set(const float* array) {
    CUAssertLog(0 <= array[0] && array[0] <= 1, "Red value out of range: %.3f", array[0]);
    CUAssertLog(0 <= array[1] && array[1] <= 1, "Green value out of range: %.3f", array[1]);
    CUAssertLog(0 <= array[2] && array[2] <= 1, "Blue value out of range: %.3f", array[2]);
    CUAssertLog(0 <= array[3] && array[3] <= 1, "Alpha value out of range: %.3f", array[3]);
    this->r = COLOR_FLOAT_TO_BYTE(array[0]);
    this->g = COLOR_FLOAT_TO_BYTE(array[1]);
    this->b = COLOR_FLOAT_TO_BYTE(array[2]);
    this->a = COLOR_FLOAT_TO_BYTE(array[3]);
    return *this;
}

#pragma mark Comparisons
/**
 * Returns true if this color is less than the given color.
 *
 * This comparison uses lexicographical order of rgba.  To test if all
 * components in this color are less than those of c, use the method
 * darkerThan().
 *
 * @param c The color to compare against.
 *
 * @return True if this color is less than the given color.
 */
bool Color4::operator<(Color4 c) const {
    bool result = r < c.r || (r == c.r && g < c.g);
    result = result ||  (r == c.r && g == c.g && b < c.b);
    result = result ||  (r == c.r && g == c.g && b == c.b && a < c.a);
    return result;
}

/**
 * Returns true if this color is greater than the given color.
 *
 * This comparison uses lexicographical order of rgba.  To test if all
 * components in this color are greater than those of c, use the method
 * lighterThan().
 *
 * @param c The color to compare against.
 *
 * @return True if this color is greater than the given color.
 */
bool Color4::operator>(Color4 c) const {
    bool result = r > c.r || (r == c.r && g > c.g);
    result = result ||  (r == c.r && g == c.g && b > c.b);
    result = result ||  (r == c.r && g == c.g && b == c.b && a > c.a);
    return result;
}

#pragma mark Arithmetic

/**
 * Clamps this color within the given range.
 *
 * @param min The minimum value.
 * @param max The maximum value.
 *
 * @return A reference to this (modified) Color4f for chaining.
 */
Color4& Color4::clamp(Color4 min, Color4 max) {
    r = clampb(r,min.r,max.r);
    g = clampb(g,min.g,max.g);
    b = clampb(b,min.b,max.b);
    a = clampb(a,min.a,max.a);
    return *this;
}

/**
 * Interpolates the two colors c1 and c2, and stores the result in dst.
 *
 * If alpha is 0, the result is c1.  If alpha is 1, the color is c2.
 * Otherwise it is a value in c1..c2.  If alpha is outside of the
 * range 0 to 1, it is clamped to the nearest value.
 *
 * This method just implements standard linear interpolation.  It does
 * not attempt to give any blending semantics to it.
 *
 * @param c1    The first color.
 * @param c2    The second color.
 * @param alpha The interpolation value in 0..1
 * @param dst   The color to store the result in
 *
 * @return A reference to dst for chaining
 */
Color4* Color4::lerp(Color4 c1, Color4 c2, float alpha, Color4* dst) {
    if (dst == nullptr) { return nullptr; }
    float x = clampf(alpha,0,1);
    *dst = c1 * (1.f - x) + c2 * x;
    return dst;
}

/**
 * Blends the two colors c1 and c2, assuming they are not premultiplied.
 *
 * The blending is the standard over operation with color c1 as the source
 * and c2 as the destination. It assumes that the color values are not
 * premultiplied.
 *
 * @param c1    The source color.
 * @param c2    The destination color.
 * @param dst   The color to store the result in
 *
 * @return A reference to dst for chaining
 */
Color4* Color4::blend(Color4 c1, Color4 c2, Color4* dst) {
    float srca = COLOR_BYTE_TO_FLOAT(c1.a);
    float a1 = COLOR_BYTE_TO_FLOAT(c2.a)*(1-srca);
    float a2 = srca+a1;
    dst->r = clampb((GLubyte)((c1.r*srca+c2.r*a1)/a2),0,255);
    dst->g = clampb((GLubyte)((c1.g*srca+c2.g*a1)/a2),0,255);
    dst->b = clampb((GLubyte)((c1.b*srca+c2.b*a1)/a2),0,255);
    dst->a = COLOR_FLOAT_TO_BYTE(a2);
    return dst;
}

/**
 * Blends the two colors c1 and c2, assuming they are premultiplied.
 *
 * The blending is the standard over operation with color c1 as the source
 * and c2 as the destination. It assumes that the color values are
 * premultiplied.
 *
 * @param c1    The source color.
 * @param c2    The destination color.
 * @param dst   The color to store the result in
 *
 * @return A reference to dst for chaining
 */
Color4* Color4::blendPre(Color4 c1, Color4 c2, Color4* dst) {
    float srca = COLOR_BYTE_TO_FLOAT(c1.a);
    float dsta = COLOR_BYTE_TO_FLOAT(c2.a);
    float a = srca+dsta*(1-srca);
    dst->r = clampb((GLubyte)(c1.r+c2.r*(1-srca)),0,255);
    dst->g = clampb((GLubyte)(c1.g+c2.g*(1-srca)),0,255);
    dst->b = clampb((GLubyte)(c1.b+c2.b*(1-srca)),0,255);
    dst->a = COLOR_FLOAT_TO_BYTE(a);
    return dst;
}

#pragma mark Conversions

/**
 * Returns a string representation of this vector for debuggging purposes.
 *
 * If verbose is true, the string will include class information.  This
 * allows us to unambiguously identify the class.
 *
 * @param verbose Whether to include class information
 *
 * @return a string representation of this vector for debuggging purposes.
 */
std::string Color4::toString(bool verbose) const {
    std::stringstream ss;
    ss << (verbose ? "cugl::Color4[r=" : "[r=");
    ss <<  to_string(r);
    ss << ",g=";
    ss << to_string(g);
    ss << ",b=";
    ss << to_string(b);
    ss << ",a=";
    ss << to_string(a);
    ss << "]";
    return ss.str();
}

/** Cast from Color4f to a vector. */
Color4::operator Vec4() const {
    return Vec4(r,g,b,a);
}

/**
 * Creates a color from the given vector.
 *
 * The attributes are read in the order x,y,z,w.  They are all multiplied
 * by 255.0f and rounded up before assignment.
 *
 * @param vector    The vector to convert
 */
Color4::Color4(const Vec4& vector) {
    CUAssertLog(0 <= vector.x && vector.x <= 1, "Red value out of range: %.3f", vector.x);
    CUAssertLog(0 <= vector.y && vector.y <= 1, "Green value out of range: %.3f", vector.y);
    CUAssertLog(0 <= vector.z && vector.z <= 1, "Blue value out of range: %.3f", vector.z);
    CUAssertLog(0 <= vector.z && vector.z <= 1, "Alpha value out of range: %.3f", vector.z);
    r = COLOR_FLOAT_TO_BYTE(vector.x);
    g = COLOR_FLOAT_TO_BYTE(vector.y);
    b = COLOR_FLOAT_TO_BYTE(vector.z);
    a = COLOR_FLOAT_TO_BYTE(vector.w);
}

/**
 * Sets the coordinates of this color to those of the given vector.
 *
 * The attributes are read in the order x,y,z,w.  They are all multiplied
 * by 255.0f and rounded up before assignment.
 *
 * @param vector    The vector to convert
 *
 * @return A reference to this (modified) Color4f for chaining.
 */
Color4& Color4::operator= (const Vec4& vector) {
    CUAssertLog(0 <= vector.x && vector.x <= 1, "Red value out of range: %.3f", vector.x);
    CUAssertLog(0 <= vector.y && vector.y <= 1, "Green value out of range: %.3f", vector.y);
    CUAssertLog(0 <= vector.z && vector.z <= 1, "Blue value out of range: %.3f", vector.z);
    CUAssertLog(0 <= vector.z && vector.z <= 1, "Alpha value out of range: %.3f", vector.z);
    r = COLOR_FLOAT_TO_BYTE(vector.x);
    g = COLOR_FLOAT_TO_BYTE(vector.y);
    b = COLOR_FLOAT_TO_BYTE(vector.z);
    a = COLOR_FLOAT_TO_BYTE(vector.w);
    return *this;
}

/**
 * Cast from Color4f to a vector.
 *
 * The attributes are all divided by 255.0.  The alpha value is dropped.
 */
Color4::operator Vec3() const {
    return Vec3(COLOR_BYTE_TO_FLOAT(r),COLOR_BYTE_TO_FLOAT(g),COLOR_BYTE_TO_FLOAT(b));
}

/**
 * Creates a color from the given vector.
 *
 * The attributes are read in the order x,y,z. The alpha value is 1.
 *
 * @param vector    The vector to convert
 */
Color4::Color4(const Vec3& vector) {
    CUAssertLog(0 <= vector.x && vector.x <= 1, "Red value out of range: %.3f", vector.x);
    CUAssertLog(0 <= vector.y && vector.y <= 1, "Green value out of range: %.3f", vector.y);
    CUAssertLog(0 <= vector.z && vector.z <= 1, "Blue value out of range: %.3f", vector.z);
    r = COLOR_FLOAT_TO_BYTE(vector.x);
    g = COLOR_FLOAT_TO_BYTE(vector.y);
    b = COLOR_FLOAT_TO_BYTE(vector.z);
    a = 255;
}

/**
 * Sets the coordinates of this color to those of the given vector.
 *
 * The attributes are read in the order x,y,z. The alpha value is 1.
 *
 * @param vector    The vector to convert
 *
 * @return A reference to this (modified) Color4f for chaining.
 */
Color4& Color4::operator= (const Vec3& vector) {
    CUAssertLog(0 <= vector.x && vector.x <= 1, "Red value out of range: %.3f", vector.x);
    CUAssertLog(0 <= vector.y && vector.y <= 1, "Green value out of range: %.3f", vector.y);
    CUAssertLog(0 <= vector.z && vector.z <= 1, "Blue value out of range: %.3f", vector.z);
    r = COLOR_FLOAT_TO_BYTE(vector.x);
    g = COLOR_FLOAT_TO_BYTE(vector.y);
    b = COLOR_FLOAT_TO_BYTE(vector.z);
    a = 255;
    return *this;
}

/** Cast from Color4f to a byte-based Color4. */
Color4::operator Color4f() const {
    return Color4f(COLOR_BYTE_TO_FLOAT(r),COLOR_BYTE_TO_FLOAT(g),
                  COLOR_BYTE_TO_FLOAT(b),COLOR_BYTE_TO_FLOAT(a));
}

/**
 * Creates a float-based color from the given byte-based color.
 *
 * The attributes are read in the order r,g,b,a. They are all multiplied by
 * 255.0f and rounded up before assignment.
 *
 * @param color The color to convert
 */
Color4::Color4(const Color4f& color) {
    r = COLOR_FLOAT_TO_BYTE(color.r);
    g = COLOR_FLOAT_TO_BYTE(color.g);
    b = COLOR_FLOAT_TO_BYTE(color.b);
    a = COLOR_FLOAT_TO_BYTE(color.a);
}

/**
 * Sets the attributes of this color from the given float-based color.
 *
 * The attributes are read in the order r,g,b,a. They are all multiplied by
 * 255.0f and rounded up before assignment.
 *
 * @param color The color to convert.
 *
 * @return A reference to this (modified) Color4f for chaining.
 */
Color4& Color4::operator= (const Color4f& color) {
    r = COLOR_FLOAT_TO_BYTE(color.r);
    g = COLOR_FLOAT_TO_BYTE(color.g);
    b = COLOR_FLOAT_TO_BYTE(color.b);
    a = COLOR_FLOAT_TO_BYTE(color.a);
    return *this;
}

#pragma mark Constants

/** The Clear color (0,0,0,0) */
const Color4 Color4::CLEAR(0,0,0,0);
/** The White color (1,1,1,1) */
const Color4 Color4::WHITE(255,255,255,255);
/** The Black color (0,0,0,1) */
const Color4 Color4::BLACK(0,0,0,255);
/** The Yellow color (1,1,0,1) */
const Color4 Color4::YELLOW(255,255,0,255);
/** The Blue color (0,0,1,1) */
const Color4 Color4::BLUE(0,0,255,255);
/** The Green color (0,1,0,1) */
const Color4 Color4::GREEN(0,255,0,255);
/** The Red color (1,0,0,1) */
const Color4 Color4::RED(255,0,0,255);
/** The Magenta color (1,0,1,1) */
const Color4 Color4::MAGENTA(255,0,255,255);
/** The Cyan color (0,1,1,1) */
const Color4 Color4::CYAN(0,255,255,255);
/** The Orange color (1,0.5,0,1) */
const Color4 Color4::ORANGE(255,128,0,255);
/** The Gray color (0.65,0.65,0.65,1) */
const Color4 Color4::GRAY(166,166,166,255);
/** The classic XNA color (0.392f,0.584f,0.93f,1,0f) */
const Color4 Color4::CORNFLOWER(100,149,237,255);
/** The Playing Fields color (0.8f,0.8f,0.5f,1.0f) */
const Color4 Color4::PAPYRUS(204,204,128,255);

//
//  CUSize.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides support for representing the size of a 2d rectangle.
//  This is critical for screen dimensions and scene graphs.
//
//  Because math objects are intended to be on the stack, we do not provide
//  any shared pointer support in this class.
//
//  This module is based on an original file from Cocos2d: http://cocos2d-x.org
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
#include <iostream>
#include <sstream>

#include <cugl/util/CUDebug.h>
#include <cugl/util/CUStrings.h>
#include <cugl/math/CUSize.h>
#include <cugl/math/CUVec2.h>

using namespace cugl;

#pragma mark -
#pragma mark Operators
/**
 * Divides this size in place by the given factor.
 *
 * @param a The scalar to divide by
 *
 * @return A reference to this (modified) Size for chaining.
 */
Size& Size::operator/=(float a) {
    SDL_assert(a != 0);
    width /= a; height /= a;
    return *this;
}

/**
 * Divides this size in place by the given size.
 *
 * This method is provided to support non-uniform scaling.
 *
 * @param right The size to divide by
 *
 * @return A reference to this (modified) Size for chaining.
 */
Size& Size::operator/=(const Size& right) {
    SDL_assert(right.width != 0 && right.height != 0);
    width /= right.width; height /= right.height;
    return *this;
}


#pragma mark -
#pragma mark Conversion Methods

/**
 * Returns a string representation of this Size for debugging purposes.
 *
 * If verbose is true, the string will include class information.  This
 * allows us to unambiguously identify the class.
 *
 * @param verbose Whether to include class information
 *
 * @return a string representation of this Size for debugging purposes.
 */
std::string Size::toString(bool verbose) const {
    std::stringstream ss;
    ss << (verbose ? "cugl::Size(w=" : "(w=");
    ss <<  to_string(width);
    ss << ",h=";
    ss << to_string(height);
    ss << ")";
    return ss.str();
}

/** Cast from from Size to Vec2. */
Size::operator Vec2() const {
    return Vec2(width,height);
}

/**
 * Creates a size from the given Vec2
 *
 * The x coordinate is converted to width and y to height.
 *
 * @param point The vector to convert
 */
Size::Size(const Vec2& point) {
    width = point.x; height = point.y;
}

/**
 * Creates the smallest size containing the two points.
 *
 * @param p1 The first point.
 * @param p2 The second point.
 */
Size::Size(const Vec2& p1, const Vec2& p2) {
    width  = fabsf(p1.x-p2.x);
    height = fabsf(p1.y-p2.y);
}

/**
 * Sets this size to the smallest one containing the two points.
 *
 * @param p1 The first point.
 * @param p2 The second point.
 *
 * @return A reference to this (modified) Vec2 for chaining.
 */
Size& Size::set(const Vec2& p1, const Vec2& p2) {
    width  = fabsf(p1.x-p2.x);
    height = fabsf(p1.y-p2.y);
    return *this;
}

/**
 * Sets the dimensions of this size to those of the given vector.
 *
 * The x coordinate is converted to width and y to height.
 *
 * @param point The vector to convert.
 *
 * @return A reference to this (modified) Size for chaining.
 */
Size& Size::operator= (const Vec2& point) {
    width = point.x; height = point.y;
    return *this;
}



#pragma mark -
#pragma mark Constants

/** The zero dimension Size(0,0) */
const Size Size::ZERO(0.0f, 0.0f);

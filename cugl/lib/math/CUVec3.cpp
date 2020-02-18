//
//  CUVec3.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a 3d vector.  It has support for basic
//  arithmetic, as well as conversions to color formats.
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
#include <cugl/math/CUVec2.h>
#include <cugl/math/CUVec3.h>
#include <cugl/math/CUVec4.h>
#include <cugl/math/CUColor4.h>

using namespace cugl;

#pragma mark -
#pragma mark Static Arithmetic

/**
 * Clamps the specified vector within the given range and returns it in dst.
 *
 * @param v     The vector to clamp.
 * @param min   The minimum value.
 * @param max   The maximum value.
 * @param dst   A vector to store the result in.
 *
 * @return A reference to dst for chaining
 */
Vec3* Vec3::clamp(const Vec3& v, const Vec3& min, const Vec3& max, Vec3* dst) {
    CUAssertLog(dst, "Assignment vector is null");
    dst->x = clampf(v.x,min.x,max.x);
    dst->y = clampf(v.y,min.y,max.y);
    dst->z = clampf(v.z,min.z,max.z);
    return dst;
}

/**
 * Returns the angle (in radians) between the specified vectors.
 *
 * The resulting angle is signed.  It uses the (optional) up direction to
 * determine the relative plane for signing the angle.  If either vector
 * is zero, the result is undefined.
 *
 * @param v1    The first vector.
 * @param v2    The second vector.
 * @param up    The up direction (defaults to Vec3::UNIT_Z)
 *
 * @return The angle between the two vectors (in radians).
 */
float Vec3::angle(const Vec3& v1, const Vec3& v2, const Vec3& up) {
    float dx = v1.y * v2.z - v1.z * v2.y;
    float dy = v1.z * v2.x - v1.x * v2.z;
    float dz = v1.x * v2.y - v1.y * v2.x;
    float dc = sqrt(dx * dx + dy * dy + dz * dz);
    
    float angle = (dc < CU_MATH_EPSILON ? 0.0f : atan2f(dc, dot(v1, v2)));
    if (up.x*dx+up.y*dy+up.z*dz < 0) {
        angle = -angle;
    }
    return angle;
}

/**
 * Adds the specified vectors and stores the result in dst.
 *
 * @param v1    The first vector.
 * @param v2    The second vector.
 * @param dst   A vector to store the result in
 *
 * @return A reference to dst for chaining
 */
Vec3* Vec3::add(const Vec3& v1, const Vec3& v2, Vec3* dst) {
    CUAssertLog(dst, "Assignment vector is null");
    dst->x = v1.x+v2.x;
    dst->y = v1.y+v2.y;
    dst->z = v1.z+v2.z;
    return dst;
}

/**
 * Subtracts the specified vectors and stores the result in dst.
 * The resulting vector is computed as (v1 - v2).
 *
 * @param v1    The first vector.
 * @param v2    The second vector.
 * @param dst   The destination vector.
 *
 * @return A reference to dst for chaining
 */
Vec3* Vec3::subtract(const Vec3& v1, const Vec3& v2, Vec3* dst) {
    CUAssertLog(dst, "Assignment vector is null");
    dst->x = v1.x-v2.x;
    dst->y = v1.y-v2.y;
    dst->z = v1.z-v2.z;
    return dst;
}

/**
 * Scales the specified vector and stores the result in dst.
 *
 * The scale is uniform by the constant s.
 *
 * @param v     The vector to scale.
 * @param s     The uniform scaling factor.
 * @param dst   The destination vector.
 *
 * @return A reference to dst for chaining
 */
Vec3* Vec3::scale(const Vec3& v, float s, Vec3* dst) {
    CUAssertLog(dst, "Assignment vector is null");
    dst->x = v.x*s;
    dst->y = v.y*s;
    dst->z = v.z*s;
    return dst;
}

/**
 * Scales the specified vector and stores the result in dst.
 *
 * The scale is nonuniform by the attributes in v2.
 *
 * @param v1    The vector to scale.
 * @param v2    The nonuniform scaling factor.
 * @param dst   The destination vector.
 *
 * @return A reference to dst for chaining
 */
Vec3* Vec3::scale(const Vec3& v1, const Vec3& v2, Vec3* dst) {
    CUAssertLog(dst, "Assignment vector is null");
    dst->x = v1.x*v2.x;
    dst->y = v1.y*v2.y;
    dst->z = v1.z*v2.z;
    return dst;
}

/**
 * Divides the specified vector and stores the result in dst.
 *
 * The division is uniform by the constant s.
 *
 * @param v     The vector to divide.
 * @param s     The uniform division factor.
 * @param dst   The destination vector.
 *
 * @return A reference to dst for chaining
 */
Vec3* Vec3::divide(const Vec3& v, float s, Vec3* dst) {
    CUAssertLog(dst, "Assignment vector is null");
    dst->x = v.x/s;
    dst->y = v.y/s;
    dst->z = v.z/s;
    return dst;
}

/**
 * Divides the specified vector and stores the result in dst.
 *
 * The division is nonuniform by the attributes in v2.
 *
 * @param v1    The vector to divide.
 * @param v2    The nonuniform division factor.
 * @param dst   The destination vector.
 *
 * @return A reference to dst for chaining
 */
Vec3* Vec3::divide(const Vec3& v1, const Vec3& v2, Vec3* dst) {
    CUAssertLog(dst, "Assignment vector is null");
    dst->x = v1.x/v2.x;
    dst->y = v1.y/v2.y;
    dst->z = v1.z/v2.z;
    return dst;
}

/**
 * Reciprocates the specified quaternion and stores the result in dst.
 *
 * Reciprocation is applied to each element invidiually.  This method
 * does not check that all elements are non-zero.  If any element is
 * zero, the result will be system-dependent.
 *
 * @param v     The vector to reciprocate.
 * @param dst   The destination vector.
 *
 * @return A reference to dst for chaining
 */
Vec3* Vec3::reciprocate(const Vec3& v, Vec3* dst) {
    CUAssertLog(dst, "Assignment vector is null");
    CUAssertLog(v.x != 0 && v.y != 0 && v.z != 0, "Reciprocating zero value");
    dst->x = 1.0f/v.x; dst->y = 1.0f/v.y; dst->z = 1.0f/v.z;
    return dst;
}

/**
 * Negates the specified vector and stores the result in dst.
 *
 * @param v     The vector to negate.
 * @param dst   The destination vector.
 *
 * @return A reference to dst for chaining
 */
Vec3* Vec3::negate(const Vec3& v, Vec3* dst) {
    CUAssertLog(dst, "Assignment vector is null");
    dst->x = -v.x; dst->y = -v.y; dst->z = -v.z;
    return dst;
}


#pragma mark -
#pragma mark Arithmetic
/**
 * Divides this vector in place by the given factor.
 *
 * @param s The scalar to divide by
 *
 * @return A reference to this (modified) Vec3 for chaining.
 */
Vec3& Vec3::divide(float s) {
    CUAssertLog(s != 0,"Division by zero");
    x /= s; y /= s; z /= s;
    return *this;
}

/**
 * Divides this vector nonuniformly by the given factors.
 *
 * @param sx The scalar to divide the x-axis
 * @param sy The scalar to divide the y-axis
 * @param sz The scalar to divide the z-axis
 *
 * @return A reference to this (modified) Vec3 for chaining.
 */
Vec3& Vec3::divide(float sx, float sy, float sz) {
    CUAssertLog(sx != 0 && sy != 0 && sz != 0,"Division by zero");
    x /= sx; y /= sy; z /= sz;
    return *this;
}

/**
 * Divides this vector in place by the given vector.
 *
 * This method is provided to support non-uniform scaling.
 *
 * @param s The vector to divide by
 *
 * @return A reference to this (modified) Vec3 for chaining.
 */
Vec3& Vec3::divide(const Vec3& v) {
    CUAssertLog(v.x != 0 && v.y != 0 && v.z != 0,"Division by zero");
    x /= v.x; y /= v.y; z /= v.z;
    return *this;
}


#pragma mark -
#pragma mark Linear Algebra
/**
 * Returns the angle between this vector and other.
 *
 * The resulting angle is signed.  It uses the (optional) up direction to
 * determine the relative plane for signing the angle.  If either vector
 * is zero, the result is undefined.
 *
 * @param other The vector to sweep towards.
 * @param up    The up direction (defaults to Vec3::UNIT_Z)
 *
 * @return the angle between this vector and other.
 */
float Vec3::getAngle(const Vec3& other, const Vec3& up) const {
    float dx = y * other.z - z * other.y;
    float dy = z * other.x - x * other.z;
    float dz = x * other.y - y * other.x;
    float dc = sqrt(dx * dx + dy * dy + dz * dz);
    
    float angle = (dc < CU_MATH_EPSILON ? 0.0f : atan2f(dc, dot(other)));
    if (up.x*dx+up.y*dy+up.z*dz < 0) {
        angle = -angle;
    }
    return angle;
}

/**
 * Normalizes this vector.
 *
 * This method normalizes Vec3 so that it is of unit length (i.e. the
 * length of the vector after calling this method will be 1.0f). If the
 * vector already has unit length or if the length of the vector is zero,
 * this method does nothing.
 *
 * @return This vector, after the normalization occurs.
 */
Vec3& Vec3::normalize() {
    float n = x * x + y * y + z * z;
    // Already normalized.
    if (n == 1.0f) {
        return *this;
    }
    
    n = sqrt(n);
    
    // Too close to zero.
    if (n < CU_MATH_FLOAT_SMALL) {
        return *this;
    }
    
    n = 1.0f / n;
    x *= n;
    y *= n;
    z *= n;
    
    return *this;
}

/**
 * Updates this vector towards the given target using a smoothing function.
 *
 * The given response time determines the amount of smoothing (lag). A
 * longer response time yields a smoother result and more lag. To force
 * this vector to follow the target closely, provide a response time that
 * is very small relative to the given elapsed time.
 *
 * @param target    The target value.
 * @param elapsed   The elapsed time between calls.
 * @param response  The response time (in the same units as elapsed).
 *
 * @return This vector, after smoothing
 */
Vec3& Vec3::smooth(const Vec3& target, float elapsed, float response) {
    if (elapsed > 0) {
        *this += (target - *this) * (elapsed / (elapsed + response));
    }
    return *this;
}


#pragma mark -
#pragma mark Static Linear Algebra
/**
 * Returns the dot product between the specified vectors.
 *
 * @param v1 The first vector.
 * @param v2 The second vector.
 *
 * @return The dot product between the vectors.
 */
float Vec3::dot(const Vec3& v1, const Vec3& v2) {
    return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
}

/**
 * Computes the cross product of the vectors and stores the result in dst.
 *
 * @param v1    The first vector.
 * @param v2    The second vector.
 * @param dst   The destination vector.
 *
 * @return A reference to dst for chaining
 */
Vec3* Vec3::cross(const Vec3& v1, const Vec3& v2, Vec3* dst) {
    CUAssertLog(dst, "Assignment vector is null");
    float x = (v1.y * v2.z) - (v1.z * v2.y);
    float y = (v1.z * v2.x) - (v1.x * v2.z);
    float z = (v1.x * v2.y) - (v1.y * v2.x);
    dst->set(x,y,z);
    return dst;
}

/**
 * Normalizes the specified vector and stores the result in dst.
 *
 * If the vector already has unit length or if the length of the
 *  vector is zero, this method copies v into dst.
 *
 * @param v     The vector to normalize.
 * @param dst   The destination vector.
 *
 * @return A reference to dst for chaining
 */
Vec3* Vec3::normalize(const Vec3& v, Vec3* dst) {
    float n = v.x * v.x + v.y * v.y + v.z * v.z;
    // Already normalized.
    if (n == 1.0f) {
        *dst = v;
        return dst;
    }
    
    n = sqrt(n);
    
    // Do nothing if too close to zero.
    if (n >= CU_MATH_FLOAT_SMALL) {
        n = 1.0f / n;
        dst->x = v.x*n;
        dst->y = v.y*n;
        dst->z = v.z*n;
    }

    return dst;
}


/**
 * Computes the midpoint between two points and stores it in dst.
 *
 * @param v1    The first point.
 * @param v2    The second point.
 * @param dst   The destination vector.
 *
 * @return A reference to dst for chaining
 */
Vec3* Vec3::midpoint(const Vec3& v1, const Vec3& v2, Vec3* dst) {
    CUAssertLog(dst, "Assignment vector is null");
    dst->x = (v1.x + v2.x) / 2.0f;
    dst->y = (v1.y + v2.y) / 2.0f;
    dst->z = (v1.z + v2.z) / 2.0f;
    return dst;
}

/**
 * Computes the projection of one vector on to another and stores it in dst.
 *
 * @param v1    The original vector.
 * @param v2    The vector to project on.
 * @param dst   The destination vector.
 *
 * @return A reference to dst for chaining
 */
Vec3* Vec3::project(const Vec3& v1, const Vec3& v2, Vec3* dst) {
    CUAssertLog(dst, "Assignment vector is null");
    *dst = v2 * (v1.dot(v2)/v2.dot(v2));
    return dst;
}

/**
 * Computes the linear interpolation of two vectors and stores it in dst.
 *
 * If alpha is 0, the vector is a copy of v1.  If alpha is 1, the vector is
 * a copy of v2.  Otherwise it is a value on the line v1v2.  This method 
 * supports alpha outside of the range 0..1.
 *
 * @param v1    The first point.
 * @param v2    The second point.
 * @param alpha The interpolation value.
 * @param dst   The destination vector.
 *
 * @return The linear interpolation of this vector with other.
 */
Vec3* Vec3::lerp(const Vec3& v1, const Vec3& v2, float alpha, Vec3* dst) {
    CUAssertLog(dst, "Assignment vector is null");
    *dst = v1 * (1.f - alpha) + v2 * alpha;
    return dst;
}


#pragma mark -
#pragma mark Conversion Methods
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
std::string Vec3::toString(bool verbose) const {
    std::stringstream ss;
    ss << (verbose ? "cugl::Vec3(" : "(");
    ss <<  to_string(x);
    ss << ",";
    ss << to_string(y);
    ss << ",";
    ss << to_string(z);
    ss << ")";
    return ss.str();
}

/** Cast from Vec3 to a Color4. */
Vec3::operator Color4() const {
    return Color4(COLOR_FLOAT_TO_BYTE(x),COLOR_FLOAT_TO_BYTE(y),COLOR_FLOAT_TO_BYTE(z));
}

/**
 * Creates a vector from the given color.
 *
 * The attributes are read in the order r,g,b.
 *
 * @param color The color to convert
 */
Vec3::Vec3(Color4 color) {
    x = COLOR_BYTE_TO_FLOAT(color.r);
    y = COLOR_BYTE_TO_FLOAT(color.g);
    z = COLOR_BYTE_TO_FLOAT(color.b);
}

/**
 * Sets the coordinates of this vector to those of the given color.
 *
 * The attributes are read in the order r,g,b.
 *
 * @param color The color to convert.
 *
 * @return A reference to this (modified) Vec3 for chaining.
 */
Vec3& Vec3::operator= (Color4 color) {
    x = COLOR_BYTE_TO_FLOAT(color.r);
    y = COLOR_BYTE_TO_FLOAT(color.g);
    z = COLOR_BYTE_TO_FLOAT(color.b);
    return *this;
}

/** Cast from Vec3 to a Color4f. */
Vec3::operator Color4f() const {
    return Color4f(x,y,z);
}

/**
 * Creates a vector from the given color.
 *
 * The attributes are read in the order r,g,b.
 *
 * @param color The color to convert
 */
Vec3::Vec3(const Color4f& color) {
    x = color.r; y = color.g; z = color.b;
}

/**
 * Sets the coordinates of this vector to those of the given color.
 *
 * The attributes are read in the order r,g,b.
 *
 * @param color The color to convert.
 *
 * @return A reference to this (modified) Vec3 for chaining.
 */
Vec3& Vec3::operator= (const Color4f& color) {
    x = color.r; y = color.g; z = color.b;
    return *this;
}

/**
 * Casts from Vec2 to Vec3.
 *
 * The z-value is dropped.
 */
Vec3::operator Vec2() const {
    return Vec2(x,y);
}

/**
 * Creates a 3d vector from the given 2d one.
 *
 * The z-value is set to 0.
 *
 * @param v The vector to convert
 */
Vec3::Vec3(const Vec2& v) {
    x = v.x; y = v.y; z = 0;
}

/**
 * Sets the coordinates of this vector to those of the given 3d vector.
 *
 * The z-value is set to 0.
 *
 * @param v The vector to convert
 *
 * @return A reference to this (modified) Vec3 for chaining.
 */
Vec3& Vec3::operator= (const Vec2& v) {
    x = v.x; y = v.y; z = 0;
    return *this;
}

/**
 * Casts from Vec3 to Vec4.
 *
 * The w-value is set to 1.  That is because the standard usage of Vec4
 * objects is homogenous coords.
 */
Vec3::operator Vec4() const {
    return Vec4(x,y,z,1);
}

/**
 * Creates a 2d vector from the given 4d one.
 *
 * All coordinates are divided by the w-coordinate (assuming it is not
 * zero) before this conversion. Afterwards, w is dropped.
 *
 * @param v The vector to convert
 */
Vec3::Vec3(const Vec4& v) {
    float d = v.w == 0 ? 1 : 1/v.w;
    x = v.x*d; y = v.y*d; z = v.z*d;
}

/**
 * Sets the coordinates of this vector to those of the given 4d vector.
 *
 * All coordinates are divided by the w-coordinate (assuming it is not
 * zero) before this conversion. Afterwards, w is dropped.
 *
 * @param v The vector to convert
 *
 * @return A reference to this (modified) Vec3 for chaining.
 */
Vec3& Vec3::operator= (const Vec4& v) {
    float d = v.w == 0 ? 1 : 1/v.w;
    x = v.x*d; y = v.y*d; z = v.z*d;
    return *this;
}

#pragma mark -
#pragma mark Constants

/** The zero vector Vec3(0,0,0) */
const Vec3 Vec3::ZERO(0,0,0);
/** The unit vector Vec3(1,1,1) */
const Vec3 Vec3::ONE(1,1,1);
/** The x-axis Vec3(1,0,0) */
const Vec3 Vec3::UNIT_X(1,0,0);
/** The y-axis Vec3(0,1,0) */
const Vec3 Vec3::UNIT_Y(0,1,0);
/** The z-axis Vec3(0,0,1) */
const Vec3 Vec3::UNIT_Z(0,0,1);

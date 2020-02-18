//
//  CUVec3.h
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
#ifndef __CU_VEC3_H__
#define __CU_VEC3_H__

#include <math.h>
#include <functional>
#include <string>
#include "CUMathBase.h"

namespace cugl {

// Forward reference for conversions
class Color4;
class Color4f;
class Vec2;
class Vec4;
    
/**
 * This class defines a 3-element floating point vector.
 *
 * This class may be used to represent either a normal, a direction or
 * a point interchangeably without casting. In addition, instances of this
 * class may be freely cast to {@link Color4} and vectors of other sizes.
 *
 * This class is in standard layout with fields of uniform type.  This means
 * that it is safe to reinterpret_cast objects to float arrays.
 */
class  Vec3 {

#pragma mark Values
public:
    /** The x-coordinate. */
    float x;
    /** The y-coordinate. */
    float y;
    /** The z-coordinate. */
    float z;
    
    /** The zero vector Vec3(0,0,0) */
    static const Vec3 ZERO;
    /** The ones vector Vec3(1,1,1) */
    static const Vec3 ONE;
    /** The x-axis Vec3(1,0,0) */
    static const Vec3 UNIT_X;
    /** The y-axis Vec3(0,1,0) */
    static const Vec3 UNIT_Y;
    /** The z-axis Vec3(0,0,1) */
    static const Vec3 UNIT_Z;
    
    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Constructs a new vector initialized to all zeros.
     */
    Vec3() : x(0), y(0), z(0) {}
    
    /**
     * Constructs a new vector initialized to the specified values.
     *
     * @param x The x coordinate.
     * @param y The y coordinate.
     * @param z The z coordinate.
     */
    Vec3(float x, float y, float z) { this->x = x; this->y = y; this->z = z; }
    
    /**
     * Constructs a new vector from the values in the specified array.
     *
     * @param array An array containing the elements of the vector in the order x, y, z.
     */
    Vec3(const float* array) { x = array[0]; y = array[1]; z = array[2]; }
    
    /**
     * Constructs a vector that describes the direction between the specified points.
     *
     * @param p1 The first point.
     * @param p2 The second point.
     */
    Vec3(const Vec3& p1, const Vec3& p2) {
        x = p2.x-p1.x; y = p2.y-p1.y; z = p2.z-p1.z;
    }

    
#pragma mark -
#pragma mark Setters
    /**
     * Sets the elements of this vector from the values in the specified array.
     *
     * @param array An array containing the elements of the vector in the order x, y, z.
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& operator=(const float* array) {
        return set(array);
    }
    
    /**
     * Sets the elements of this vector to the specified values.
     *
     * @param x The new x coordinate.
     * @param y The new y coordinate.
     * @param z The new z coordinate.
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& set(float x, float y, float z) {
        this->x = x; this->y = y; this->z = z;
        return *this;
    }

    /**
     * Sets the elements of this vector from the values in the specified array.
     *
     * @param array An array containing the elements of the vector in the order x, y, z.
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& set(const float* array) {
        x = array[0]; y = array[1]; z = array[2];
        return *this;
    }
    
    /**
     * Sets the elements of this vector to those in the specified vector.
     *
     * @param v The vector to copy.
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& set(const Vec3& v) {
        x = v.x; y = v.y; z = v.z;
        return *this;
    }
    
    /**
     * Sets this vector to the directional vector between the specified points.
     *
     * @param p1 The initial point of the vector.
     * @param p2 The terminal point of the vector.
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& set(const Vec3& p1, const Vec3& p2) {
        x = p2.x-p1.x; y = p2.y-p1.y; z = p2.z-p1.z;
        return *this;
    }
    
    /**
     * Sets the elements of this vector to zero.
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& setZero() {
        x = y = z = 0;
        return *this;
    }
    
    
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
    static Vec3* clamp(const Vec3& v, const Vec3& min, const Vec3& max, Vec3* dst);
    
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
    static float angle(const Vec3& v1, const Vec3& v2, const Vec3& up = Vec3::UNIT_Z);
    
    /**
     * Adds the specified vectors and stores the result in dst.
     *
     * @param v1    The first vector.
     * @param v2    The second vector.
     * @param dst   A vector to store the result in
     *
     * @return A reference to dst for chaining
     */
    static Vec3* add(const Vec3& v1, const Vec3& v2, Vec3* dst);
    
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
    static Vec3* subtract(const Vec3& v1, const Vec3& v2, Vec3* dst);
    
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
    static Vec3* scale(const Vec3& v, float s, Vec3* dst);
    
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
    static Vec3* scale(const Vec3& v1, const Vec3& v2, Vec3* dst);
    
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
    static Vec3* divide(const Vec3& v, float s, Vec3* dst);
    
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
    static Vec3* divide(const Vec3& v1, const Vec3& v2, Vec3* dst);
    
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
    static Vec3* reciprocate(const Vec3& v, Vec3* dst);
    
    /**
     * Negates the specified vector and stores the result in dst.
     *
     * @param v     The vector to negate.
     * @param dst   The destination vector.
     *
     * @return A reference to dst for chaining
     */
    static Vec3* negate(const Vec3& v, Vec3* dst);
    
#pragma mark -
#pragma mark Arithmetic
    /**
     * Clamps this vector within the given range.
     *
     * @param min The minimum value.
     * @param max The maximum value.
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& clamp(const Vec3& min, const Vec3& max) {
        x = clampf(x, min.x, max.x);
        y = clampf(y, min.y, max.y);
        z = clampf(z, min.z, max.z);
        return *this;
    }
    
    /**
     * Returns a copy of this vector clamped within the given range.
     *
     * Note: this does not modify this vector.
     *
     * @param min The minimum value.
     * @param max The maximum value.
     *
     * @return A copy of this vector clamped within the given range.
     */
    Vec3 getClamp(const Vec3& min, const Vec3& max) const {
        return Vec3(clampf(x,min.x,max.x), clampf(y, min.y, max.y),clampf(z, min.z, max.z));
    }
    
    /**
     * Adds the given vector to this one in place.
     *
     * @param v The vector to add
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& add(const Vec3& v) {
        x += v.x; y += v.y; z += v.z;
        return *this;
    }
    
    /**
     * Adds the given values to this vector.
     *
     * @param x The x coordinate to add.
     * @param y The y coordinate to add.
     * @param z The z coordinate to add.
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& add(float x, float y, float z) {
        this->x += x; this->y += y; this->z += z;
        return *this;
    }
    
    /**
     * Subtracts the given vector from this one in place.
     *
     * @param v The vector to subtract
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& subtract(const Vec3& v) {
        x -= v.x; y -= v.y; z -= v.z;
        return *this;
    }
    
    /**
     * Subtracts the given values from this vector.
     *
     * @param x The x coordinate to subtract.
     * @param y The y coordinate to subtract.
     * @param z The z coordinate to subtract.
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& subtract(float x, float y, float z) {
        this->x -= x; this->y -= y; this->z -= z;
        return *this;
    }
    
    /**
     * Scales this vector in place by the given factor.
     *
     * @param s The scalar to multiply by
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& scale(float s) {
        x *= s; y *= s; z *= s;
        return *this;
    }
    
    /**
     * Scales this vector nonuniformly by the given factors.
     *
     * @param sx The scalar to multiply the x-axis
     * @param sy The scalar to multiply the y-axis
     * @param sz The scalar to multiply the z-axis
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& scale(float sx, float sy, float sz) {
        x *= sx; y *= sy; z *= sz;
        return *this;
    }
    
    /**
     * Scales this vector nonuniformly by the given vector.
     *
     * @param v The vector to scale by
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& scale(const Vec3& v) {
        x *= v.x; y *= v.y; z *= v.z;
        return *this;
    }
    
    /**
     * Divides this vector in place by the given factor.
     *
     * @param s The scalar to divide by
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& divide(float s);
    
    /**
     * Divides this vector nonuniformly by the given factors.
     *
     * @param sx The scalar to divide the x-axis
     * @param sy The scalar to divide the y-axis
     * @param sz The scalar to divide the z-axis
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& divide(float sx, float sy, float sz);
    
    /**
     * Divides this vector in place by the given vector.
     *
     * This method is provided to support non-uniform scaling.
     *
     * @param v The vector to divide by
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& divide(const Vec3& v);
    
    /**
     * Negates this vector.
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& negate() {
        x = -x; y = -y; z = -z;
        return *this;
    }
    
    /**
     * Reciprovates this vector in place.
     *
     * The reciprocal is computed for each element invidiually.  This method
     * does not check that all elements are non-zero.  If any element is
     * zero, the result will be system-dependent.
     *
     * @return A reference to this (modified) Vec4 for chaining.
     */
    Vec3& reciprocate() {
        x = 1.0f/x; y = 1.0f/y; z = 1.0f/z;
        return *this;
    }
    
    /**
     * Returns a negated copy of this vector.
     *
     * Note: This method does not modify the vector
     *
     * @return a negated copy of this vector.
     */
    Vec3 getNegation() const {
        Vec3 result(*this);
        return result.negate();
    }
    
    /**
     * Returns a reciprocated copy of this vector.
     *
     * The reciprocal is computed for each element invidiually.  This method
     * does not check that all elements are non-zero.  If any element is
     * zero, the result will be system-dependent.
     *
     * Note: This method does not modify the vector
     *
     * @return a reciprocated copy of this vector.
     */
    Vec3 getReciprocal() const {
        Vec3 result(*this);
        return result.reciprocate();
    }
        
    /**
     * Maps the given function to the vector coordinates in place.
     *
     * This method supports any function that has the signature float func(float);
     * This includes many of the functions in math.h.
     *
     * @param func The function to map on the coordinates.
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& map(std::function<float(float)> func) {
        x = func(x); y = func(y); z = func(z);
        return *this;
    }
        
    /**
     * Returns a copy of this vector with func applied to each component.
     *
     * This method supports any function that has the signature float func(float);
     * This includes many of the functions in math.h.
     *
     * @param func The function to map on the coordinates.
     *
     * @return A copy of this vector with func applied to each component.
     */
    Vec3 getMap(std::function<float(float)> func) const {
        return Vec3(func(x), func(y), func(z));
    }
    
    
#pragma mark -
#pragma mark Operators
    /**
     * Adds the given vector to this one in place.
     *
     * @param v The vector to add
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& operator+=(const Vec3& v) {
        return add(v);
    }
    
    /**
     * Subtracts the given vector from this one in place.
     *
     * @param v The vector to subtract
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& operator-=(const Vec3& v) {
        return subtract(v);
    }
    
    /**
     * Scales this vector in place by the given factor.
     *
     * @param s The value to scale by
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& operator*=(float s) {
        return scale(s);
    }
    
    /**
     * Scales this vector nonuniformly by the given vector.
     *
     * @param v The vector to scale by
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& operator*=(const Vec3& v) {
        return scale(v);
    }
    
    /**
     * Divides this vector in place by the given factor.
     *
     * @param s The scalar to divide by
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& operator/=(float s) {
        return divide(s);
    }
    
    /**
     * Divides this vector in place by the given vector.
     *
     * This method is provided to support non-uniform scaling.
     *
     * @param v The vector to divide by
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& operator/=(const Vec3& v) {
        return divide(v);
    }
    
    /**
     * Returns the sum of this vector with the given vector.
     *
     * Note: this does not modify this vector.
     *
     * @param v The vector to add.
     *
     * @return The sum of this vector with the given vector.
     */
    const Vec3 operator+(const Vec3& v) const {
        Vec3 result(*this);
        return result.add(v);
    }
    
    /**
     * Returns the difference of this vector with the given vector.
     *
     * Note: this does not modify this vector.
     *
     * @param v The vector to subtract.
     *
     * @return The difference of this vector with the given vector.
     */
    const Vec3 operator-(const Vec3& v) const  {
        Vec3 result(*this);
        return result.subtract(v);
    }
    
    /**
     * Returns the negation of this vector.
     *
     * Note: this does not modify this vector.
     *
     * @return The negation of this vector.
     */
    const Vec3 operator-() const {
        Vec3 result(*this);
        return result.negate();
    }
    
    /**
     * Returns the scalar product of this vector with the given value.
     *
     * Note: this does not modify this vector.
     *
     * @param s The value to scale by.
     *
     * @return The scalar product of this vector with the given value.
     */
    const Vec3 operator*(float s) const {
        Vec3 result(*this);
        return result.scale(s);
    }
    
    /**
     * Returns the scalar product of this vector with the given vector.
     *
     * This method is provided to support non-uniform scaling.
     * Note: this does not modify this vector.
     *
     * @param v The vector to scale by.
     *
     * @return The scalar product of this vector with the given vector.
     */
    const Vec3 operator*(const Vec3& v) const {
        Vec3 result(*this);
        return result.scale(v);
    }
    
    /**
     * Returns a copy of this vector divided by the given constant
     *
     * Note: this does not modify this vector.
     *
     * @param s the constant to divide this vector with
     *
     * @return A copy of this vector divided by the given constant
     */
    const Vec3 operator/(float s) const {
        Vec3 result(*this);
        return result.divide(s);
    }
    
    /**
     * Returns a copy of this vector divided by the given vector.
     *
     * This method is provided to support non-uniform scaling.
     * Note: this does not modify this vector.
     *
     * @param v the vector to divide this vector with
     *
     * @return A copy of this vector divided by the given vector
     */
    const Vec3 operator/(const Vec3& v) const {
        Vec3 result(*this);
        return result.divide(v);
    }
    
    
#pragma mark -
#pragma mark Comparisons
    /**
     * Returns true if this vector is less than the given vector.
     *
     * This comparison uses the lexicographical order.  To test if all
     * components in this vector are less than those of v, use the method
     * under().
     *
     * @param v The vector to compare against.
     *
     * @return True if this vector is less than the given vector.
     */
    bool operator<(const Vec3& v) const {
        return (x == v.x ? (y == v.y ? z < v.z : y < v.y) : x < v.x);
    }
    
    /**
     * Returns true if this vector is less than or equal the given vector.
     *
     * This comparison uses the lexicographical order.  To test if all
     * components in this vector are less than those of v, use the method
     * under().
     *
     * @param v The vector to compare against.
     *
     * @return True if this vector is less than or equal the given vector.
     */
    bool operator<=(const Vec3& v) const {
        return (x == v.x ? (y == v.y ? z <= v.z : y <= v.y) : x <= v.x);
    }
    
    /**
     * Returns true if this vector is greater than the given vector.
     *
     * This comparison uses the lexicographical order.  To test if all
     * components in this vector are greater than those of v, use the method
     * over().
     *
     * @param v The vector to compare against.
     *
     * @return True if this vector is greater than the given vector.
     */
    bool operator>(const Vec3& v) const {
        return (x == v.x ? (y == v.y ? z > v.z : y > v.y) : x > v.x);
    }
    
    /**
     * Returns true if this vector is greater than or equal the given vector.
     *
     * This comparison uses the lexicographical order.  To test if all
     * components in this vector are greater than those of v, use the method
     * over().
     *
     * @param v The vector to compare against.
     *
     * @return True if this vector is greater than or equal the given vector.
     */
    bool operator>=(const Vec3& v) const {
        return (x == v.x ? (y == v.y ? z >= v.z : y >= v.y) : x >= v.x);
    }
    
    /**
     * Returns true if this vector is equal to the given vector.
     *
     * Comparison is exact, which may be unreliable given that the attributes
     * are floats.
     *
     * @param v The vector to compare against.
     *
     * @return True if this vector is equal to the given vector.
     */
    bool operator==(const Vec3& v) const {
        return x == v.x && y == v.y && z == v.z;
    }
    
    /**
     * Returns true if this vector is not equal to the given vector.
     *
     * Comparison is exact, which may be unreliable given that the attributes
     * are floats.
     *
     * @param v The vector to compare against.
     *
     * @return True if this vector is not equal to the given vector.
     */
    bool operator!=(const Vec3& v) const {
        return x != v.x || y != v.y || z != v.z;
    }
    
    /**
     * Returns true if this vector is dominated by the given vector.
     *
     * Domination means that all components of the given vector are greater than
     * or equal to the components of this one.
     *
     * @param v The vector to compare against.
     *
     * @return True if this vector is dominated by the given vector.
     */
    bool under(const Vec3& v) const {
        return x <= v.x && y <= v.y && z <= v.z;
    }
    
    /**
     * Returns true if this vector dominates the given vector.
     *
     * Domination means that all components of this vector are greater than
     * or equal to the components of the given vector.
     *
     * @param v The vector to compare against.
     *
     * @return True if this vector is dominated by the given vector.
     */
    bool over(const Vec3& v) const {
        return x >= v.x && y >= v.y && z >= v.z;
    }
    
    /**
     * Returns true if the vectors are within tolerance of each other.
     *
     * The tolerance bounds the traditional Euclidean difference between
     * the two vectors (treated as points)
     *
     * @param v         The vector to compare against.
     * @param variance  The comparison tolerance.
     *
     * @return true if the vectors are within tolerance of each other.
     */
    bool equals(const Vec3& v, float variance=CU_MATH_EPSILON) const {
        return distanceSquared(v) <= variance*variance;
    }
    
    
#pragma mark -
#pragma mark Linear Attributes
    /**
     * Returns true this vector contains all zeros.
     *
     * @return true if this vector contains all zeros, false otherwise.
     */
    bool isZero() const {
        return x == 0.0f && y == 0.0f && z == 0.0f;
    }
    
    /**
     * Returns true if this vector is with tolerance of the origin.
     *
     * @param variance The comparison tolerance
     *
     * @return true if this vector is with tolerance of the origin.
     */
    bool isNearZero(float variance=CU_MATH_EPSILON) const {
        return lengthSquared() < variance*variance;
    }
    
    /**
     * Returns true if this vector contains all ones.
     *
     * @return true if this vector contains all ones, false otherwise.
     */
    bool isOne() const {
        return x == 1.0f && y == 1.0f && z == 1.0f;
    }
    
    /**
     * Returns true if this vector contains no zeroes.
     *
     * @return true if this vector contains no zeroes, false otherwise.
     */
    bool isInvertible() const {
        return x != 0.0f && y != 0.0f && z != 0.0f;
    }
    
    /**
     * Returns true if this vector is a unit vector.
     *
     * @param variance The comparison tolerance
     *
     * @return true if this vector is a unit vector.
     */
    bool isUnit(float variance=CU_MATH_EPSILON) const {
        float dot = lengthSquared()-1;
        return dot < variance && dot > -variance;
    }
    
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
    float getAngle(const Vec3& other, const Vec3& up = Vec3::UNIT_Z) const;
    
    /**
     * Returns the distance between this vector and v.
     *
     * @param v The other vector.
     *
     * @return The distance between this vector and v.
     *
     * {@see distanceSquared}
     */
    float distance(const Vec3& v) const {
        return sqrt(distanceSquared(v));
    }
    
    /**
     * Returns the squared distance between this vector and v.
     *
     * This method is faster than distance because it does not need to compute
     * a square root.  Hence it is best to us this method when it is not
     * necessary to get the exact distance between two vectors (e.g. when
     * simply comparing the distance between different vectors).
     *
     * @param v The other vector.
     *
     * @return The squared distance between this vector and v.
     *
     * {@see distance}
     */
    float distanceSquared(const Vec3& v) const {
        return (x-v.x)*(x-v.x)+(y-v.y)*(y-v.y)+(z-v.z)*(z-v.z);
    }

    /**
     * Returns the length of this vector.
     *
     * @return The length of the vector.
     *
     * {@see lengthSquared}
     */
    float length() const {
        return sqrt(lengthSquared());
    }
    
    /**
     * Returns the squared length of this vector.
     *
     * This method is faster than length because it does not need to compute
     * a square root.  Hence it is best to us this method when it is not
     * necessary to get the exact length of a vectors (e.g. when simply
     * comparing the length to a threshold value).
     *
     * @return The squared length of the vector.
     *
     * {@see length}
     */
    float lengthSquared() const {
        return x*x+y*y+z*z;
    }

    
#pragma mark -
#pragma mark Linear Algebra
    /**
     * Returns the dot product of this vector and the specified vector.
     *
     * @param v The vector to compute the dot product with.
     *
     * @return The dot product.
     */
    float dot(const Vec3& v) const { return (x * v.x + y * v.y + z * v.z); }

    /**
     * Sets this vector to the cross product between itself and the specified one.
     *
     * @param v The vector to cross with
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& cross(const Vec3& v) {
        return *(cross(*this,v,this));
    }
    
    /**
     * Returns the cross product of this vector with another
     *
     * Note: this does not modify this vector.
     *
     * @param other The vector to cross with
     *
     * @return the cross product of this vector with another
     */
    Vec3 getCross(const Vec3& other) const {
        Vec3 result(*this);
        return result.cross(other);
    }
    
    /**
     * Normalizes this vector.
     *
     * This method normalizes the vector so that it is of unit length (i.e.
     * the length of the vector after calling this method will be 1.0f). If the
     * vector already has unit length or if the length of the vector is zero,
     * this method does nothing.
     *
     * @return This vector, after the normalization occurs.
     */
    Vec3& normalize();
    
    /**
     * Returns a normalized copy of this vector.
     *
     * This method creates a copy of this vector that is of unit length (i.e.
     * the length of the vector after calling this method will be 1.0f). If the
     * vector already has unit length or if the length of the vector is zero,
     * this method does nothing.
     * Note: this does not modify this vector.
     *
     * @return A normalized copy of this vector.
     */
    Vec3 getNormalization() const {
        Vec3 result(*this);
        return result.normalize();
    }

    /**
     * Returns the midpoint between this point and another.
     *
     * Note: this does not modify this vector.
     *
     * @param other The other end of the line segment.
     *
     * @return the midpoint between this point and another.
     */
    Vec3 getMidpoint(const Vec3& other) const {
        return Vec3((x + other.x) / 2.0f, (y + other.y) / 2.0f, (z + other.z) / 2.0f);
    }
    
    /**
     * Modifies this vector to be its projection on to the other one.
     *
     * @param other The vector to project on.
     *
     * @return This vector, after the projection.
     */
    Vec3& project(const Vec3& other) {
        *this = getProjection(other);
        return *this;
    }
    
    /**
     * Returns the projection of this vector on to the other one.
     *
     * Note: this does not modify this vector.
     *
     * @param other The vector to project on.
     *
     * @return the projection of this vector on to the other one.
     */
    Vec3 getProjection(const Vec3& other) const {
        return other * (dot(other)/other.dot(other));
    }

    /**
     * Modifies this vector to be the linear interpolation with other.
     *
     * If alpha is 0, the vector is unchanged.  If alpha is 1, the vector is
     * other.  Otherwise it is a value on the line ab.  This method supports
     * alpha outside of the range 0..1.
     *
     * @param other The other end of the line segment.
     * @param alpha The interpolation value in 0..1
     *
     * @return This vector, after the interpolation.
     */
    Vec3& lerp(const Vec3& other, float alpha) {
        *this *= (1.f - alpha);
        return *this += other * alpha;
    }
    
    /**
     * Returns the linear interpolation of this vector with other.
     *
     * If alpha is 0, the vector is unchanged.  If alpha is 1, the vector is
     * other.   Otherwise it is a value on the line ab.  This method supports
     * alpha outside of the range 0..1.
     *
     * @param other The other end of the line segment.
     * @param alpha The interpolation value in 0..1
     *
     * @return The linear interpolation of this vector with other.
     */
    Vec3 getLerp(const Vec3& other, float alpha) {
        return *this * (1.f - alpha) + other * alpha;
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
    Vec3& smooth(const Vec3& target, float elapsed, float response);

    
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
    static float dot(const Vec3& v1, const Vec3& v2);
    
    /**
     * Computes the cross product of the vectors and stores the result in dst.
     *
     * @param v1    The first vector.
     * @param v2    The second vector.
     * @param dst   The destination vector.
     *
     * @return A reference to dst for chaining
     */
    static Vec3* cross(const Vec3& v1, const Vec3& v2, Vec3* dst);

    /**
     * Normalizes the specified vector and stores the result in dst.
     *
     * If the quaternion already has unit length or if the length of the
     *  quaternion is zero, this method copies v into dst.
     *
     * @param v     The vector to normalize.
     * @param dst   The destination vector.
     *
     * @return A reference to dst for chaining
     */
    static Vec3* normalize(const Vec3& v, Vec3* dst);
    
    /**
     * Computes the midpoint between two points and stores it in dst.
     *
     * @param v1    The first point.
     * @param v2    The second point.
     * @param dst   The destination vector.
     *
     * @return A reference to dst for chaining
     */
    static Vec3* midpoint(const Vec3& v1, const Vec3& v2, Vec3* dst);
    
    /**
     * Computes the projection of one vector on to another and stores it in dst.
     *
     * @param v1    The original vector.
     * @param v2    The vector to project on.
     * @param dst   The destination vector.
     *
     * @return A reference to dst for chaining
     */
    static Vec3* project(const Vec3& v1, const Vec3& v2, Vec3* dst);
    
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
    static Vec3* lerp(const Vec3& v1, const Vec3& v2, float alpha, Vec3* dst);


#pragma mark -
#pragma mark Conversion Methods
public:
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
    std::string toString(bool verbose = false) const;
    
    /** Cast from Vec3 to a string. */
    operator std::string() const { return toString(); }
    
    /** Cast from Vec3 to a Color4. */
    operator Color4() const;
    
    /**
     * Creates a vector from the given color.
     *
     * The attributes are read in the order r,g,b. The alpha value is dropped.
     *
     * @param color The color to convert
     */
    explicit Vec3(Color4 color);
    
    /**
     * Sets the coordinates of this vector to those of the given color.
     *
     * The attributes are read in the order r,g,b. The alpha value is dropped.
     *
     * @param color The color to convert.
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& operator= (Color4 color);

    /** Cast from Vec3 to a Color4f. */
    operator Color4f() const;
    
    /**
     * Creates a vector from the given color.
     *
     * The attributes are read in the order r,g,b. The alpha value is dropped.
     *
     * @param color The color to convert
     */
    explicit Vec3(const Color4f& color);
    
    /**
     * Sets the coordinates of this vector to those of the given color.
     *
     * The attributes are read in the order r,g,b. The alpha value is dropped.
     *
     * @param color The color to convert.
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& operator= (const Color4f& color);
    
    /**
     * Casts from Vec2 to Vec3.
     *
     * The z-value is dropped.
     */
    operator Vec2() const;
    
    /**
     * Creates a 3d vector from the given 2d one.
     *
     * The z-value is set to 0.
     *
     * @param v The vector to convert
     */
    explicit Vec3(const Vec2& v);
    
    /**
     * Sets the coordinates of this vector to those of the given 2d vector.
     *
     * The z-value is set to 0.
     *
     * @param v The vector to convert
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& operator= (const Vec2& v);
    
    /**
     * Casts from Vec3 to Vec4.
     *
     * The w-value is set to 1. That is because the standard usage of Vec4
     * objects is homogenous coords.
     */
    operator Vec4() const;
    
    /**
     * Creates a 3d vector from the given homogenous one.
     *
     * All coordinates are divided by the w-coordinate (assuming it is not
     * zero) before this conversion. Afterwards, w is dropped.
     *
     * @param v The vector to convert
     */
    explicit Vec3(const Vec4& v);
    
    /**
     * Sets the coordinates of this vector to those of the given homogenous vector.
     *
     * All coordinates are divided by the w-coordinate (assuming it is not
     * zero) before this conversion. Afterwards, w is dropped.
     *
     * @param size The vector to convert
     *
     * @return A reference to this (modified) Vec3 for chaining.
     */
    Vec3& operator= (const Vec4& size);

};

#pragma mark -
#pragma mark Friend Operations

/**
 * Returns the scalar product of the given vector with the given value.
 *
 * @param x The value to scale by.
 * @param v The vector to scale.
 *
 * @return The scaled vector.
 */
inline const Vec3 operator*(float x, const Vec3& v) {
    Vec3 result(v);
    return result.scale(x);
}

/** Provide an alternative name for Vec3 */
typedef Vec3 Point3;

}
#endif /* __CU_VEC3_H__ */

//
//  CUVec2.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a 2d vector.  It has support for basic
//  arithmetic, as well as some common line intersection properties.
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

#ifndef __CU_VEC2_H__
#define __CU_VEC2_H__

#include <math.h>
#include <string>
#include <functional>
#include "CUMathBase.h"

namespace cugl {

// Forward references for conversions
class Size;
class Vec3;
class Vec4;
    
/**
 * This class defines a 2-element floating point vector.
 *
 * This class may be used to represent either a normal, a direction or
 * a point interchangeably without casting.  In addition, instances of this 
 * class may be freely cast to {@link Size} and vectors of other sizes.
 *
 * This class is in standard layout with fields of uniform type.  This means 
 * that it is safe to reinterpret_cast objects to float arrays.
 */
class Vec2 {
    
#pragma mark Values
public:
    /** The x coordinate. */
    float x;
    /** The x coordinate. */
    float y;
    
    /** The zero vector Vec2(0,0) */
    static const Vec2 ZERO;
    /** The unit vector Vec2(1,1) */
    static const Vec2 ONE;
    /** The x-axis Vec2(1,0) */
    static const Vec2 UNIT_X;
    /** The y-axis Vec2(0,1) */
    static const Vec2 UNIT_Y;
    
    /** The relative anchor Vec2(0.5, 0.5) in the unit square */
    static const Vec2 ANCHOR_CENTER;
    /** The relative anchor Vec2(0, 0) in the unit square */
    static const Vec2 ANCHOR_BOTTOM_LEFT;
    /** The relative anchor Vec2(0, 1) in the unit square */
    static const Vec2 ANCHOR_TOP_LEFT;
    /** The relative anchor Vec2(1, 0) in the unit square */
    static const Vec2 ANCHOR_BOTTOM_RIGHT;
    /** The relative anchor Vec2(1, 1) in the unit square */
    static const Vec2 ANCHOR_TOP_RIGHT;
    /** The relative anchor Vec2(1, 0.5) in the unit square */
    static const Vec2 ANCHOR_MIDDLE_RIGHT;
    /** The relative anchor Vec2(0, 0.5) in the unit square */
    static const Vec2 ANCHOR_MIDDLE_LEFT;
    /** The relative anchor Vec2(0.5, 1) in the unit square */
    static const Vec2 ANCHOR_TOP_CENTER;
    /** The relative anchor Vec2(0.5, 0) in the unit square */
    static const Vec2 ANCHOR_BOTTOM_CENTER;

#pragma mark -
#pragma mark Constructors
public:
    /**
     * Constructs a new vector initialized to all zeros.
     */
    Vec2() : x(0), y(0) { }

    /**
     * Constructs a new vector initialized to the specified values.
     *
     * @param x The x coordinate.
     * @param y The y coordinate.
     */
    Vec2(float x, float y) { this->x = x; this->y = y; }
    
    /**
     * Constructs a new vector from the values in the specified array.
     *
     * @param array An array containing the elements of the vector in the order x, y.
     */
    Vec2(const float* array) { x = array[0]; y = array[1]; }
    
    /**
     * Constructs a vector that describes the direction between the specified points.
     *
     * @param p1 The first point.
     * @param p2 The second point.
     */
    Vec2(const Vec2& p1, const Vec2& p2) {
        x = p2.x-p1.x; y = p2.y-p1.y;
    }
    

#pragma mark -
#pragma mark Setters
    /**
     * Sets the elements of this vector from the values in the specified array.
     *
     * @param array An array containing the elements of the vector in the order x, y.
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& operator=(const float* array) {
        return set(array);
    }
    
    /**
     * Sets the elements of this vector to the specified values.
     *
     * @param x The new x coordinate.
     * @param y The new y coordinate.
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& set(float x, float y) {
        this->x = x; this->y = y;
        return *this;
    }
    
    /**
     * Sets the elements of this vector from the values in the specified array.
     *
     * @param array An array containing the elements of the vector in the order x, y.
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& set(const float* array) {
        x = array[0]; y = array[1];
        return *this;
    }
    
    /**
     * Sets the elements of this vector to those in the specified vector.
     *
     * @param v The vector to copy.
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& set(const Vec2& v) {
        x = v.x; y = v.y;
        return *this;
    }
    
    /**
     * Sets this vector to the directional vector between the specified points.
     *
     * @param p1 The initial point of the vector.
     * @param p2 The terminal point of the vector.
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& set(const Vec2& p1, const Vec2& p2) {
        x = p2.x-p1.x; y = p2.y-p1.y;
        return *this;
    }
    
    /**
     * Sets the elements of this vector to zero.
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& setZero() {
        x = y = 0;
        return *this;
    }
    
    
#pragma mark -
#pragma mark Static Arithmetic
    /**
     * Returns the unit vector for the given angle (in radians)
     *
     * @param a The defining angle in radians.
     *
     * @return the unit vector for the given angle (in radians)
     */
    static Vec2 forAngle(const float a) {
        return Vec2(cosf(a), sinf(a));
    }
    
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
    static Vec2* clamp(const Vec2& v, const Vec2& min, const Vec2& max, Vec2* dst);
    
    /**
     * Returns the angle (in radians) between the specified vectors.
     *
     * @param v1 The first vector.
     * @param v2 The second vector.
     *
     * @return The angle between the two vectors (in radians).
     */
    static float angle(const Vec2& v1, const Vec2& v2);
    
    /**
     * Adds the specified vectors and stores the result in dst.
     *
     * @param v1    The first vector.
     * @param v2    The second vector.
     * @param dst   A vector to store the result in
     *
     * @return A reference to dst for chaining
     */
    static Vec2* add(const Vec2& v1, const Vec2& v2, Vec2* dst);

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
    static Vec2* subtract(const Vec2& v1, const Vec2& v2, Vec2* dst);

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
    static Vec2* scale(const Vec2& v, float s, Vec2* dst);
    
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
    static Vec2* scale(const Vec2& v1, const Vec2& v2, Vec2* dst);
    
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
    static Vec2* divide(const Vec2& v, float s, Vec2* dst);
    
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
    static Vec2* divide(const Vec2& v1, const Vec2& v2, Vec2* dst);
    
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
    static Vec2* reciprocate(const Vec2& v, Vec2* dst);
    
    /**
     * Negates the specified vector and stores the result in dst.
     *
     * @param v     The vector to negate.
     * @param dst   The destination vector.
     *
     * @return A reference to dst for chaining
     */
    static Vec2* negate(const Vec2& v, Vec2* dst);
    
#pragma mark -
#pragma mark Arithmetic
    /**
     * Clamps this vector within the given range.
     *
     * @param min The minimum value.
     * @param max The maximum value.
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& clamp(const Vec2& min, const Vec2& max) {
        x = clampf(x, min.x, max.x);
        y = clampf(y, min.y, max.y);
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
    Vec2 getClamp(const Vec2& min, const Vec2& max) const {
        return Vec2(clampf(x,min.x,max.x), clampf(y, min.y, max.y));
    }

    /**
     * Adds the given vector to this one in place.
     *
     * @param v The vector to add
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& add(const Vec2& v) {
        x += v.x; y += v.y;
        return *this;
    }
    
    /**
     * Adds the given values to this vector.
     *
     * @param x The x coordinate to add.
     * @param y The y coordinate to add.
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& add(float x, float y) {
        this->x += x; this->y += y;
        return *this;
    }
    
    /**
     * Subtracts the given vector from this one in place.
     *
     * @param v The vector to subtract
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& subtract(const Vec2& v) {
        x -= v.x; y -= v.y;
        return *this;
    }
    
    /**
     * Subtracts the given values from this vector.
     *
     * @param x The x coordinate to subtract.
     * @param y The y coordinate to subtract.
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& subtract(float x, float y) {
        this->x -= x; this->y -= y;
        return *this;
    }
    
    /**
     * Scales this vector in place by the given factor.
     *
     * @param s The scalar to multiply by
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& scale(float s) {
        x *= s; y *= s;
        return *this;
    }
    
    /**
     * Scales this vector nonuniformly by the given factors.
     *
     * @param sx The scalar to multiply the x-axis
     * @param sy The scalar to multiply the y-axis
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& scale(float sx, float sy) {
        x *= sx; y *= sy;
        return *this;
    }
    
    /**
     * Scales this vector nonuniformly by the given vector.
     *
     * @param v The vector to scale by
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& scale(const Vec2& v) {
        x *= v.x; y *= v.y;
        return *this;
    }
    
    /**
     * Divides this vector in place by the given factor.
     *
     * @param s The scalar to divide by
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& divide(float s);
    
    /**
     * Divides this vector nonuniformly by the given factors.
     *
     * @param sx The scalar to divide the x-axis
     * @param sy The scalar to divide the y-axis
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& divide(float sx, float sy);
    
    /**
     * Divides this vector in place by the given vector.
     *
     * This method is provided to support non-uniform scaling.
     *
     * @param v The vector to divide by
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& divide(const Vec2& v);
    
    /**
     * Negates this vector.
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& negate() {
        x = -x; y = -y;
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
    Vec2& reciprocate() {
        x = 1.0f/x; y = 1.0f/y;
        return *this;
    }
    
    /**
     * Returns a negated copy of this vector.
     *
     * Note: This method does not modify the vector
     *
     * @return a negated copy of this vector.
     */
    Vec2 getNegation() const {
        Vec2 result(*this);
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
    Vec2 getReciprocal() const {
        Vec2 result(*this);
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
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& map(std::function<float(float)> func) {
        x = func(x); y = func(y);
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
    Vec2 getMap(std::function<float(float)> func) const {
        return Vec2(func(x), func(y));
    }

    
#pragma mark -
#pragma mark Operators
    /**
     * Adds the given vector to this one in place. 
     *
     * @param v The vector to add
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& operator+=(const Vec2& v) {
        return add(v);
    }

    /**
     * Subtracts the given vector from this one in place.
     *
     * @param v The vector to subtract
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& operator-=(const Vec2& v) {
        return subtract(v);
    }

    /**
     * Scales this vector in place by the given factor.
     *
     * @param s The value to scale by
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& operator*=(float s) {
        return scale(s);
    }

    /**
     * Scales this vector nonuniformly by the given vector.
     *
     * @param v The vector to scale by
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& operator*=(const Vec2& v) {
        return scale(v);
    }

    /**
     * Divides this vector in place by the given factor.
     *
     * @param s The scalar to divide by
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& operator/=(float s) {
        return divide(s);
    }

    /**
     * Divides this vector in place by the given vector.
     *
     * This method is provided to support non-uniform scaling.
     *
     * @param v The vector to divide by
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& operator/=(const Vec2& v) {
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
    const Vec2 operator+(const Vec2& v) const {
        Vec2 result(*this);
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
    const Vec2 operator-(const Vec2& v) const  {
        Vec2 result(*this);
        return result.subtract(v);
    }
    
    /**
     * Returns the negation of this vector.
     *
     * Note: this does not modify this vector.
     *
     * @return The negation of this vector.
     */
    const Vec2 operator-() const {
        Vec2 result(*this);
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
    const Vec2 operator*(float s) const {
        Vec2 result(*this);
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
    const Vec2 operator*(const Vec2& v) const {
        Vec2 result(*this);
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
    const Vec2 operator/(float s) const {
        Vec2 result(*this);
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
    const Vec2 operator/(const Vec2& v) const {
        Vec2 result(*this);
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
    bool operator<(const Vec2& v) const {
        return (x == v.x ? y < v.y : x < v.x);
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
    bool operator<=(const Vec2& v) const {
        return (x == v.x ? y <= v.y : x <= v.x);
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
    bool operator>(const Vec2& v) const {
        return (x == v.x ? y > v.y : x > v.x);
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
    bool operator>=(const Vec2& v) const {
        return (x == v.x ? y >= v.y : x >= v.x);
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
    bool operator==(const Vec2& v) const {
        return x == v.x && y == v.y;
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
    bool operator!=(const Vec2& v) const {
        return x != v.x || y != v.y;
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
    bool under(const Vec2& v) const {
        return x <= v.x && y <= v.y;
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
    bool over(const Vec2& v) const {
        return x >= v.x && y >= v.y;
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
    bool equals(const Vec2& v, float variance=CU_MATH_EPSILON) const {
        return distanceSquared(v) <= variance*variance;
    }


#pragma mark -
#pragma mark Linear Attributes
    /** 
     * Returns the angle in radians between this vector and the x axis.
     *
     * If the vector is zero, the result is undefined.
     *
     * @return the angle in radians between this vector and the x axis.
     */
    float getAngle() const {
        return atan2f(y, x);
    }
    
    /**
     * Returns the angle between this vector and other.
     *
     * The angle is measured starting at this one.  If either vector is zero,
     * the result is undefined.
     *
     * @param other The vector to sweep towards.
     *
     * @return the angle between this vector and other.
     */
    float getAngle(const Vec2& other) const;
    
    /**
     * Returns true this vector contains all zeros.
     *
     * @return true if this vector contains all zeros, false otherwise.
     */
    bool isZero() const {
        return x == 0.0f && y == 0.0f;
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
        return x == 1.0f && y == 1.0f;
    }
    
    /**
     * Returns true if this vector contains no zeroes.
     *
     * @return true if this vector contains no zeroes, false otherwise.
     */
    bool isInvertible() const {
        return x != 0.0f && y != 0.0f;
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
     * Returns the distance between this vector and v.
     *
     * @param v The other vector.
     *
     * @return The distance between this vector and v.
     *
     * {@see distanceSquared}
     */
    float distance(const Vec2& v) const {
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
    float distanceSquared(const Vec2& v) const {
        return (x-v.x)*(x-v.x)+(y-v.y)*(y-v.y);
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
        return x*x+y*y;
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
    float dot(const Vec2& v) const { return (x * v.x + y * v.y); }

    /** 
     * Returns the cross product of this vector with another
     *
     * The cross-product of any two vectors in the plane is perpendicular to
     * the plane.  This method returns the magnitude of that z-vector.
     *
     * @param other The vector to cross with
     *
     * @return the cross product of this vector with another
     */
    float cross(const Vec2& other) const {
        return x*other.y - y*other.x;
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
    Vec2& normalize();
    
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
    Vec2 getNormalization() const {
        Vec2 result(*this);
        return result.normalize();
    }

    /**
     * Rotates this vector by the angle (in radians) around the origin.
     *
     * @param angle The angle to rotate by (in radians).
     *
     * @return This vector, after the rotation occurs.
     */
    Vec2& rotate(float angle);
    
    /**
     * Rotates this vector by the angle (in radians) around the given point.
     *
     * @param point The point to rotate around.
     * @param angle The angle to rotate by (in radians).
     *
     * @return This vector, after the rotation occurs.
     */
    Vec2& rotate(float angle, const Vec2& point);
    
    /** 
     * Rotates this vector so that its angle is increased by that of the other.
     *
     * This rotation uses complex multiplication to add the angles of the two
     * vectors together.  This method is faster than rotating by an angle.
     *
     * @param other The vector representing the angle to rotate by
     *
     * @return This vector, after the rotation occurs.
     */
    Vec2& rotate(const Vec2& other) {
        // The const is more of a guideline than a rule.  Must bullet-proof.
        float tx = x; float ty = y;
        float ox = other.x; float oy = other.y;
        x = tx*ox - ty*oy; y = tx*oy + ty*ox;
        return *this;
    }
    
    /**
     * Rotates this vector so that its angle is decreased by that of the other.
     *
     * This rotation uses complex multiplication to add the angles of the two
     * vectors together.  This method is faster than rotating by an angle.
     *
     * @param other The vector representing the angle to unrotate by
     *
     * @return This vector, after the rotation occurs.
     */
    Vec2& unrotate(const Vec2& other) {
        float tx = x; float ty = y;
        float ox = other.x; float oy = other.y;
        x = tx*ox + ty*oy; y = -tx*oy + ty*ox;
        return *this;
    }
    
    /**
     * Returns a copy of this vector rotated by the angle around the origin.
     *
     * The angle is measured in radians.
     * Note: this does not modify this vector.
     *
     * @param angle The angle to rotate by (in radians).
     *
     * @return a copy of this vector rotated by the angle around the origin.
     */
    Vec2 getRotation(float angle);
    
    /**
     * Returns a copy of this vector rotated by the angle around the given point.
     *
     * The angle is measured in radians.
     * Note: this does not modify this vector.
     *
     * @param point The point to rotate around.
     * @param angle The angle to rotate by (in radians).
     *
     * @return a copy of this vector rotated by the angle around the given point.
     */
    Vec2 getRotation(float angle, const Vec2& point);
    
    /**
     * Returns a rotated copy of this vector using the angle of the other.
     *
     * This rotation uses complex multiplication to add the angles of the two
     * vectors together.  This method is faster than rotating by an angle.
     *
     * Note: this does not modify this vector.
     *
     * @param other The vector representing the angle to rotate by
     *
     * @return a rotated copy of this vector using the angle of the other.
     */
    Vec2 getRotation(const Vec2& other) {
        return Vec2(x*other.x - y*other.y, x*other.y + y*other.x);
    }
    
    /**
     * Returns an unrotated copy of this vector using the angle of the other.
     *
     * This rotation uses complex multiplication to add the angles of the two
     * vectors together.  This method is faster than rotating by an angle.
     *
     * Note: this does not modify this vector.
     *
     * @param other The vector representing the angle to unrotate by
     *
     * @return an unrotated copy of this vector using the angle of the other.
     */
    Vec2 getUnrotation(const Vec2& other) {
        return Vec2(x*other.x + y*other.y, y*other.x - x*other.y);
    }
    
    /**
     * Converts this vector to its perpendicular, rotated 90 degrees counter-clockwise.
     *
     * The result guarantees cross(original, v) >= 0
     *
     * @return This vector, after the transformation.
     */
    Vec2& perp() {
        float a = x; x = -y; y = a;
        return *this;
    }

    /**
     * Converts this vector to its perpendicular, rotated 90 degrees clockwise.
     *
     * The result guarantees cross(original, v) <= 0
     *
     * @return This vector, after the transformation.
     */
    Vec2& rperp() {
        float a = x; x = y; y = -a;
        return *this;
    }
    
    /**
     * Returns a perpendicular of v, rotated 90 degrees counter-clockwise.
     *
     * The result guarantees cross(v, perp(v)) >= 0
     *
     * Note: this does not modify this vector.
     *
     * @return a perpendicular of v, rotated 90 degrees counter-clockwise.
     */
    Vec2 getPerp() const {
        return Vec2(-y, x);
    }
    
    /**
     * Returns a perpendicular of v, rotated 90 degrees clockwise.
     *
     * The result guarantees cross(v, perp(v)) <= 0
     *
     * Note: this does not modify this vector.
     *
     * @return a perpendicular of v, rotated 90 degrees clockwise.
     */
    Vec2 getRPerp() const {
        return Vec2(y, -x);
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
    Vec2 getMidpoint(const Vec2& other) const {
        return Vec2((x + other.x) / 2.0f, (y + other.y) / 2.0f);
    }
 
    /** 
     * Modifies this vector to be its projection on to the other one.
     *
     * @param other The vector to project on.
     *
     * @return This vector, after the projection.
     */
    Vec2& project(const Vec2& other) {
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
    Vec2 getProjection(const Vec2& other) const {
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
     * @param alpha The interpolation value
     *
     * @return This vector, after the interpolation.
     */
    Vec2& lerp(const Vec2& other, float alpha) {
        *this *= (1.f - alpha);
        return *this += other * alpha;
    }

    /**
     * Returns the linear interpolation of this vector with other.
     *
     * If alpha is 0, the vector is unchanged.  If alpha is 1, the vector is
     * other.  Otherwise it is a value on the line ab.  This method supports
     * alpha outside of the range 0..1.
     *
     * @param other The other end of the line segment.
     * @param alpha The interpolation value
     *
     * @return The linear interpolation of this vector with other.
     */
    Vec2 getLerp(const Vec2& other, float alpha) {
        return *this * (1.f - alpha) + other * alpha;
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
    static float dot(const Vec2& v1, const Vec2& v2);
    
    /**
     * Returns the cross product of the specified vectors.
     *
     * The cross-product of any two vectors in the plane is perpendicular to
     * the plane.  This method returns the magnitude of that z-vector.
     *
     * @param v1 The first vector.
     * @param v2 The second vector.
     *
     * @return the cross product between the vectors.
     */
    static float cross(const Vec2& v1, const Vec2& v2);
    
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
    static Vec2* normalize(const Vec2& v, Vec2* dst);
    
    /**
     * Computes the midpoint between two points and stores it in dst.
     *
     * @param v1    The first point.
     * @param v2    The second point.
     * @param dst   The destination vector.
     *
     * @return A reference to dst for chaining
     */
    static Vec2* midpoint(const Vec2& v1, const Vec2& v2, Vec2* dst);
    
    /**
     * Computes the projection of one vector on to another and stores it in dst.
     *
     * @param v1    The original vector.
     * @param v2    The vector to project on.
     * @param dst   The destination vector.
     *
     * @return A reference to dst for chaining
     */
    static Vec2* project(const Vec2& v1, const Vec2& v2, Vec2* dst);
    
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
    static Vec2* lerp(const Vec2& v1, const Vec2& v2, float alpha, Vec2* dst);
    
    /** 
     * Returns true if lines AB and CD intersect
     *
     * The line segment parameters of the hit-points are stored in S and T.
     * These values are left unchanged if there is no intersection.
     *
     * The hit point is given by C + T * (D - C) [ or alternatively by
     * A + S * (B - A)]
     *
     * The return value for this function only tests for lines.  To test 
     * intersection for segments, you must verify that both S & T lie in 
     * [0..1] .  For rays, we have to make sure S & T > 0.
     * 
     * This method returns false if the lines overlap.
     *
     * @param A   the startpoint for the first line L1 = (A - B)
     * @param B   the endpoint for the first line L1 = (A - B)
     * @param C   the startpoint for the second line L2 = (C - D)
     * @param D   the endpoint for the second line L2 = (C - D)
     * @param S   the range for a hitpoint in L1 (p = A + S*(B - A))
     * @param T   the range for a hitpoint in L2 (p = C + T*(D - C))
     *
     * @return true if lines AB and CD intersect
     */
    static bool doesLineIntersect(const Vec2& A, const Vec2& B,
                                  const Vec2& C, const Vec2& D,
                                  float *S = nullptr, float *T = nullptr);
    
    /**
     * Returns true if line AB overlaps segment CD.
     * 
     * This result means that AB and CD are both parallel and are on top
     * of each other.  AB and CD are treated as lines for this function.
     *
     * @param A   the startpoint for the first line L1 = (A - B)
     * @param B   the endpoint for the first line L1 = (A - B)
     * @param C   the startpoint for the second line L2 = (C - D)
     * @param D   the endpoint for the second line L2 = (C - D)
     *
     * @return true if line AB overlaps segment CD.
     */
    static bool doesLineOverlap(const Vec2& A, const Vec2& B,
                                const Vec2& C, const Vec2& D);
    
    /**
     * Returns true if line AB is non-trivially parallel with segment CD.
     *
     * This result means that AB and CD are parallel and are NOT overlappping.
     * AB and CD are treated as lines for this function.
     *
     * @param A   the startpoint for the first line L1 = (A - B)
     * @param B   the endpoint for the first line L1 = (A - B)
     * @param C   the startpoint for the second line L2 = (C - D)
     * @param D   the endpoint for the second line L2 = (C - D)
     *
     * @return true if line AB is non-trivially parallel with segment CD.
     */
    static bool isLineParallel(const Vec2& A, const Vec2& B,
                               const Vec2& C, const Vec2& D);
    
    /**
     * Returns true if segment AB intersects with segment CD
     *
     * This method returns false if the segments overlap.
     *
     * @param A   the startpoint for the first line L1 = (A - B)
     * @param B   the endpoint for the first line L1 = (A - B)
     * @param C   the startpoint for the second line L2 = (C - D)
     * @param D   the endpoint for the second line L2 = (C - D)
     *
     * @return true if segment AB intersects with segment CD
     */
    static bool doesSegmentIntersect(const Vec2& A, const Vec2& B, const Vec2& C, const Vec2& D);
    
    /**
     * Returns true if Segment AB overlaps with segment CD
     *
     * This result means that AB and CD are both parallel and are on top
     * of each other.  AB and CD are treated as segments for this function.
     *
     * The bounds of the overlap are stored in S and E.  If there is no
     * overlap, these values may or may not be modified.
     *
     * @param A   the startpoint for the first line L1 = (A - B)
     * @param B   the endpoint for the first line L1 = (A - B)
     * @param C   the startpoint for the second line L2 = (C - D)
     * @param D   the endpoint for the second line L2 = (C - D)
     * @param S   the initial overlap position
     * @param E   the terminal overlap position
     */
    static bool doesSegmentOverlap(const Vec2& A, const Vec2& B,
                                   const Vec2& C, const Vec2& D,
                                   Vec2* S = nullptr, Vec2* E = nullptr);
    
    /**
     * Returns the intersection point of lines AB and CD.
     *
     * This function treats AB and CD as lines, not segments.  To get finer
     * control over the intersection point, use doesLineIntersect.
     *
     * @param A   the startpoint for the first line L1 = (A - B)
     * @param B   the endpoint for the first line L1 = (A - B)
     * @param C   the startpoint for the second line L2 = (C - D)
     * @param D   the endpoint for the second line L2 = (C - D)
     *
     * @return the intersection point of lines AB and CD.
     */
    static Vec2 getIntersection(const Vec2& A, const Vec2& B, const Vec2& C, const Vec2& D);

    
#pragma mark -
#pragma mark Conversion Methods
public:
    /**
     * Returns a string representation of this vector for debugging purposes.
     *
     * If verbose is true, the string will include class information.  This
     * allows us to unambiguously identify the class.
     *
     * @param verbose Whether to include class information
     *
     * @return a string representation of this vector for debuggging purposes.
     */
    std::string toString(bool verbose = false) const;
    
    /** Cast from Vec2 to a string. */
    operator std::string() const { return toString(); }
    
    /** Cast from Vec2 to Size. */
    operator Size() const;
    
    /**
     * Creates a vector from the given size.
     *
     * The width is converted to the x coordinate and height to y.
     *
     * @param size The size to convert
     */
    explicit Vec2(const Size& size);
    
    /**
     * Sets the coordinates of this vector to those of the given size.
     *
     * The width is converted to the x coordinate and height to y.
     *
     * @param size The size to convert.
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& operator= (const Size& size);

    /**
     * Adds the given size to this vector in place.
     *
     * @param right The Size to add
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& operator+=(const Size& right);
    
    /**
     * Subtracts the given size from this vector in place.
     *
     * @param right The Size to subtract
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& operator-=(const Size& right);

    /**
     * Returns the sum of this vector with the given size.
     *
     * Note: this does not modify this vector.
     *
     * @param right The suze to add.
     *
     * @return The sum of this vector with the given size.
     */
    const Vec2 operator+(const Size& right) const {
        Vec2 result(*this);
        return result += right;
    }
    
    /**
     * Returns the difference of this vector with the given size.
     *
     * Note: this does not modify this vector.
     *
     * @param right The size to subtract.
     *
     * @return The difference of this vector with the given size.
     */
    const Vec2 operator-(const Size& right) const {
        Vec2 result(*this);
        return result -= right;
    }
    
    /** 
     * Casts from Vec2 to Vec3. 
     *
     * The z-value is set to 0.
     */
    operator Vec3() const;
    
    /**
     * Creates a 2d vector from the given 3d one.
     *
     * The z-value is dropped.
     *
     * @param v The vector to convert
     */
    explicit Vec2(const Vec3& v);
    
    /**
     * Sets the coordinates of this vector to those of the given 3d vector.
     *
     * The z-value is dropped.
     *
     * @param size The vector to convert
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& operator= (const Vec3& size);

    /**
     * Casts from Vec2 to Vec4.
     *
     * The z-value is set to 0, but the w-value is set to 1.  That is
     * because the standard usage of Vec4 objects is homogenous coords.
     */
    operator Vec4() const;
    
    /**
     * Creates a 2d vector from the given 4d one.
     *
     * All coordinates are divided by the w-coordinate (assuming it is not
     * zero) before this conversion. Afterwards, both z and w are dropped.
     *
     * @param v The vector to convert
     */
    explicit Vec2(const Vec4& v);
    
    /**
     * Sets the coordinates of this vector to those of the given 4d vector.
     *
     * All coordinates are divided by the w-coordinate (assuming it is not
     * zero) before this conversion. Afterwards, both z and w are dropped.
     *
     * @param size The vector to convert
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Vec2& operator= (const Vec4& size);
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
inline const Vec2 operator*(float x, const Vec2& v) {
    Vec2 result(v);
    return result.scale(x);
}

/** Provide an alternative name for Vec2 */
typedef Vec2 Point2;

}

#endif /* __CU_VEC2_H__ */

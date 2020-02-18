//
//  CUSize.h
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

#ifndef CU_SIZE_H
#define CU_SIZE_H

#include <string>
#include <math.h>
#include "CUMathBase.h"

namespace cugl {

/** Forward reference to Vec2 */
class Vec2;
    
/**
 * This class defines a the size of a two dimensional box
 *
 * Instances of this class may be freely cast to {@link Vec2} and
 * vice-versa.  
 *
 * This class is in standard layout with fields of uniform type.  This means
 * that it is also safe to reinterpret_cast objects to float arrays.
 */
class Size {
#pragma mark Values
public:
    /** Width of the Size object.*/
    float width;
    /** Height of the Size object.*/
    float height;

    /** The degnerate Size(0,0).*/
    static const Size ZERO;

#pragma mark -
#pragma mark Constructors
public:
    
    /**
     * Creates a new degenerate size 
     * 
     * The width and height of the size are zero.
     */
    Size() : width(0), height(0) {}

    /**
     * Creates a new size with the given dimensions
     *
     * @param width     The width value
     * @param height    The height value
     *
     */
    Size(float width, float height) {
        this->width = width; this->height = height;
    }
    
    /**
     * Constructs a new size from the values in the specified array.
     *
     * The array is read in the order width and height
     *
     * @param  array    An array containing at least two elements.
     */
    Size(const float* array) { width = array[0]; height = array[1]; }

    
#pragma mark -
#pragma mark Setters
    /**
     * Sets the dimensions of this size to the contents of the given array.
     *
     * The array is read in the order width and height
     *
     * @param  array    An array containing at least two elements.
     *
     * @return A reference to this (modified) Size for chaining.
     */
    Size& operator= (const float* array) {
        return set(array);
    }

    /**
     * Sets the dimensions of this size to the specified values.
     *
     * @param width     The new width value
     * @param height    The new height value
     *
     * @return A reference to this (modified) Size for chaining.
     */
    Size& set(float width, float height) {
        this->width = width; this->height = height;
        return *this;
    }
    
    /**
     * Sets the dimensions of this size from the values in the specified array.
     *
     * The array is read in the order width and height
     *
     * @param  array    An array containing at least two elements.
     *
     * @return A reference to this (modified) Siz for chaining.
     */
    Size& set(const float* array) {
        this->width = array[0]; this->height = array[1];
        return *this;
    }
    
    /**
     * Sets the dimensions of this size to those in the specified size.
     *
     * @param other The size to copy.
     *
     * @return A reference to this (modified) Size for chaining.
     */
    Size& set(const Size& other) {
        this->width = other.width; this->height = other.height;
        return *this;
    }

    
#pragma mark -
#pragma mark Integer Access
    
    /** 
     * Returns the Size width as an integer.
     *
     * The value is always rounded up.
     *
     * @return the Size width as an integer.
     */
    int getIWidth() const { return (int)ceilf(width); }

    /**
     * Returns the Size height as an integer.
     *
     * The value is always rounded up.
     *
     * @return the Size height as an integer.
     */
    int getIHeight() const { return (int)ceilf(height); }

    
#pragma mark -
#pragma mark Comparisons
    /**
     * Returns true if this size is less than the given size.
     *
     * This comparison uses the lexicographical order.  To test if this size
     * can properly fit inside theother, use the method inside().
     *
     * @param v The size to compare against.
     *
     * @return True if this size is less than the given size.
     */
    bool operator<(const Size& v) const {
        return (width == v.width ? height < v.height : width < v.width);
    }
    
    /**
     * Returns true if this Size is less than or equal the given Size.
     *
     * This comparison uses the lexicographical order.  To test if this Size
     * can properly fit inside theother, use the method inside().
     *
     * @param v The size to compare against.
     *
     * @return True if this size is less than or equal the given size.
     */
    bool operator<=(const Size& v) const {
        return (width == v.width ? height <= v.height : width <= v.width);
    }
    
    /**
     * Determines if this size is greater than the given size.
     *
     * This comparison uses the lexicographical order.  To test if this size
     * can properly hold the other vector, use the method contains().
     *
     * @param v The vector to compare against.
     *
     * @return True if this size is greater than the given size.
     */
    bool operator>(const Size& v) const {
        return (width == v.width ? height > v.height : width > v.width);
    }
    
    /**
     * Determines if this size is greater than or equal to the given size.
     *
     * This comparison uses the lexicographical order.  To test if this size
     * can properly hold the other vector, use the method contains().
     *
     * @param v The vector to compare against.
     *
     * @return True if this size is greater than or equal to the given size.
     */
    bool operator>=(const Size& v) const {
        return (width == v.width ? height >= v.height : width >= v.width);
    }
    
    /**
     * Returns true if this size is equal to the given size.
     *
     * Comparison is exact, which may be unreliable given that the attributes
     * are floats.
     *
     * @param other The vector to compare against.
     *
     * @return True if this size is equal to the given size.
     */
    bool operator==(const Size& other) const {
        return width == other.width && height == other.height;
    }
    
    /**
     * Returns true if this size is not equal to the given size.
     *
     * Comparison is exact, which may be unreliable given that the attributes
     * are floats.
     *
     * @param other The vector to compare against.
     *
     * @return True if this size is not equal to the given size.
     */
    bool operator!=(const Size& other) const {
        return width != other.width || height != other.height;
    }
    
    /**
     * Returns true if this size fits inside of the given size.
     *
     * The means that all dimensions of the current size are less than or
     * equal to the size parameter.
     *
     * @param other The potentially larger size
     *
     * @return True if this size fits inside of the given size.
     */
    bool inside(const Size& other) const {
        return width <= other.width && height <= other.height;
    }
    
    /**
     * Returns true if this size can hold the given size.
     *
     * The means that all dimensions of the current size are greater than or
     * equal to the size parameter.
     *
     * @param other The potentially smaller size
     *
     * @return True if this size can hold the given size.
     */
    bool contains(const Size& other) const {
        return width >= other.width && height >= other.height;
    }
    
    /**
     * Returns true if the sizes are within tolerance of each other.
     *
     * The tolerance bound is on each axis independently.
     *
     * @param other     The size to compare against.
     * @param variance  The comparison tolerance.
     *
     * @return true if the sizes are within tolerance of each other.
     */
    bool equals(const Size& other, float variance=CU_MATH_EPSILON) const {
        float dx = width-other.width;
        float dy = height-other.height;
        return ((-variance < dx && dx < variance) &&
                (-variance < dy && dy < variance));
    }
    
    
#pragma mark -
#pragma mark Operators

    /**
     * Adds the given size to this one in place.
     *
     * @param right The size to add
     *
     * @return A reference to this (modified) Size for chaining.
     */
    Size& operator+=(const Size& right) {
        width += right.width; height += right.height;
        return *this;
    }
    
    /**
     * Subtracts the given size from this one in place.
     *
     * @param right The size to subtract
     *
     * @return A reference to this (modified) Size for chaining.
     */
    Size& operator-=(const Size& right) {
        width -= right.width; height -= right.height;
        return *this;
    }
    
    /**
     * Scales this size in place by the given factor.
     *
     * @param a The value to scale by
     *
     * @return A reference to this (modified) Size for chaining.
     */
    Size& operator*=(float a) {
        width *= a; height *= a;
        return *this;
    }
    
    /**
     * Scales this size in place by the given vector.
     *
     * @param right The vector to scale by
     *
     * @return A reference to this (modified) Size for chaining.
     */
    Size& operator*=(const Size& right) {
        width *= right.width; height *= right.height;
        return *this;
    }

    /**
     * Divides this size in place by the given factor.
     *
     * @param a The scalar to divide by
     *
     * @return A reference to this (modified) Size for chaining.
     */
    Size& operator/=(float a);

    /**
     * Divides this size in place by the given size.
     *
     * This method is provided to support non-uniform scaling.
     *
     * @param right The size to divide by
     *
     * @return A reference to this (modified) Size for chaining.
     */
    Size& operator/=(const Size& right);

    /**
     * Returns the sum of this size with the given size.
     *
     * Note: this does not modify this size.
     *
     * @param right The size to add.
     *
     * @return The sum of this size with the given size.
     */
    Size operator+(const Size& right) const { return Size(*this) += right; }
    
    /**
     * Returns the difference of this size with the given size.
     *
     * Note: this does not modify this size.
     *
     * @param right The size to subtract.
     *
     * @return The difference of this size with the given size.
     */
    Size operator-(const Size& right) const { return Size(*this) -= right; }
    
    /**
     * Returns the scalar product of this size with the given size.
     *
     * Note: this does not modify this size.
     *
     * @param scalar The value to scale by.
     *
     * @return The scalar product of this size with the given value.
     */
    Size operator*(float scalar) const { return Size(*this) *= scalar; }

    /**
     * Returns the scalar product of this size with the given size.
     *
     * This method is provided to support non-uniform scaling.
     * Note: this does not modify this size.
     *
     * @param right The size to scale by.
     *
     * @return The scalar product of this size with the given size.
     */
    Size operator*(const Size& right) const { return Size(*this) *= right; }

    /**
     * Returns a copy of this size divided by the given constant
     *
     * Note: this does not modify this size.
     *
     * @param scalar the constant to divide this size with
     *
     * @return A copy of this size divided by the given constant
     */
    Size operator/(float scalar) const { return Size(*this) /= scalar; }
    
    /**
     * Returns a copy of this size divided by the given size.
     *
     * This method is provided to support non-uniform scaling.
     * Note: this does not modify this size.
     *
     * @param right the size to divide this size with
     *
     * @return A copy of this size divided by the given size
     */
    Size operator/(const Size& right) const { return Size(*this) /= right; }

    
#pragma mark -
#pragma mark Conversion Methods
public:
    /**
     * Returns a string representation of this size for debugging purposes.
     *
     * If verbose is true, the string will include class information.  This
     * allows us to unambiguously identify the class.
     *
     * @param verbose Whether to include class information
     *
     * @return a string representation of this size for debugging purposes.
     */
    std::string toString(bool verbose = false) const;
    
    /** Cast from Size to a string. */
    operator std::string() const { return toString(); }
    
    /** Cast from from Size to Vec2. */
    operator Vec2() const;
    
    /**
     * Creates a size from the given Vec2 
     *
     * The x coordinate is converted to width and y to height.
     *
     * @param point The vector to convert
     */
    explicit Size(const Vec2& point);

    /**
     * Creates the smallest size containing the two points.
     *
     * @param p1 The first point.
     * @param p2 The second point.
     */
    Size(const Vec2& p1, const Vec2& p2);
    
    /**
     * Sets this size to the smallest one containing the two points.
     *
     * @param p1 The first point.
     * @param p2 The second point.
     *
     * @return A reference to this (modified) Vec2 for chaining.
     */
    Size& set(const Vec2& p1, const Vec2& p2);
    
    /**
     * Sets the dimensions of this size to those of the given vector.
     *
     * The x coordinate is converted to width and y to height.
     *
     * @param point The vector to convert.
     *
     * @return A reference to this (modified) Size for chaining.
     */
    Size& operator= (const Vec2& point);
};
    
}

#endif /* CU_SIZE_H */

//
//  CURect.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a 2d bouding rectangle.  This rectangle
//  is not intended for drawing.  Use CUPoly2 instead for rectangle graphics.
//
//  Because math objects are intended to be on the stack, we do not provide
//  any shared pointer support in this class.
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
#ifndef __CU_RECT_H__
#define __CU_RECT_H__

#include "CUSize.h"
#include "CUVec2.h"

namespace cugl {

/**
 * This class represents a rectangle area in 2D space.
 *
 * It is generally safe to manipulate the fields directly.
 */
class Rect {
#pragma mark Values
public:
    /** The bottom left point of rect.      */
    Vec2 origin;
    /** The width and height of the rect.   */
    Size  size;
    
    /** The empty Rect.*/
    static const Rect ZERO;
    /** The unit square. */
    static const Rect UNIT;


#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates an empty Rect at the origin
     */
    Rect() : Rect(Vec2::ZERO,Size::ZERO) {}

    /**
     * Creates a rect with the given origin and dimensions.
     *
     * @param x      The x-coordinate of the bottom left corner
     * @param y      The y-coordinate of the bottom left corner
     * @param width  The width of the rect
     * @param height The width of the rect
     */
    Rect(float x, float y, float width, float height) :
        Rect(Vec2(x,y),Size(width,height)) {}

    /**
     * Creates a rect from the given float array.
     *
     * @param array An array containing the attributes in the order origin, size.
     */
    Rect(float* array) : Rect(Vec2(array),Size(&array[2])) {}

    
    /**
     * Creates a rect with the given origin and dimensions.
     *
     * @param pos   The position the bottom left corner
     * @param dimen The size of the rect
     */
    Rect(const Vec2& pos, const Size& dimen) {
        origin = pos; size = dimen;
    }

    
#pragma mark -
#pragma mark Setters
    /**
     * Sets the elements of this rect from the values in the specified array.
     *
     * @param array An array containing the elements in the order origin, size.
     *
     * @return This rectangle, after assignment
     */
    Rect& operator=(const float* array) {
        return set(array);
    }
    
    /**
     * Assigns this rect to have the given position and dimensions.
     *
     * @param x      The x-coordinate of the bottom left corner
     * @param y      The y-coordinate of the bottom left corner
     * @param width  The width of the rect
     * @param height The width of the rect
     *
     * @return This rectangle, after assignment
     */
    Rect& set(float x, float y, float width, float height) {
        origin.set(x,y); size.set(width,height);
        return *this;
    }

    /**
     * Sets the elements of this rect from the values in the specified array.
     *
     * @param array An array containing the elements in the order origin, size.
     *
     * @return This rectangle, after assignment
     */
    Rect& set(const float* array) {
        origin = &array[0]; size = &array[2];
        return *this;
    }
    /**
     * Assigns this rect to have the given position and dimensions.
     *
     * @param pos   The position the bottom left corner
     * @param dimen The size of the rect
     *
     * @return This rectangle, after assignment
     */
    Rect& set(const Vec2& pos, const Size& dimen) {
        origin.set(pos); size.set(dimen);
        return *this;
    }
    
    /**
     * Assigns this rect to be a copy of the given rectangle.
     *
     * @param other The rectangle to copy
     *
     * @return This rectangle, after assignment
     */
    Rect& set(const Rect& other) {
        origin.set(other.origin); size.set(other.size);
        return *this;
    }


#pragma mark -
#pragma mark Accessors
    
    /**
     * Returns the leftmost x-value of the rect.
     *
     * @return the leftmost x-value of the rect.
     */
    float getMinX() const { return (size.width < 0 ? origin.x+size.width : origin.x); }
    
    /**
     * Returns the center x-value of the rect.
     *
     * @return the center x-value of the rect.
     */
    float getMidX() const { return origin.x + size.width / 2.0f; }
    
    /**
     * Returns the rightmost x-value of the rect.
     *
     * @return the rightmost x-value of the rect.
     */
    float getMaxX() const { return (size.width < 0 ? origin.x : origin.x + size.width); }
    
    /**
     * Returns the bottom y-value of the rect.
     *
     * @return the bottom y-value of the rect.
     */
    float getMinY() const { return (size.height < 0 ? origin.y+size.height : origin.y); }
    
    /**
     * Returns the center y-value of the rect.
     *
     * @return the center y-value of the rect.
     */
    float getMidY() const { return origin.y + size.height / 2.0f; }
    
    /**
     * Returns the top y-value of the rect.
     *
     * @return the top y-value of the rect.
     */
    float getMaxY() const { return (size.height < 0 ?  origin.y : origin.y + size.height); }

    /**
     * Returns true if the rectangle has non-positive size.
     *
     * @return true if the rectangle has non-positive size.
     */
    float isDegenerate() const { return size.width <= 0 || size.height <= 0; }


#pragma mark -
#pragma mark Comparisons
    /**
     * Returns true if the rects are equal to each other.
     *
     * This operator uses exact equality and may fail due to round-off error.
     *
     * @param rect  The rect to compare against.
     *
     * @return true if the rects are equal to each other.
     */
    bool operator==(const Rect& rect) const {
        return origin == rect.origin && size == rect.size;
    }
    
    /**
     * Returns true if the rects are not equal to each other.
     *
     * This operator uses exact equality and may fail due to round-off error.
     *
     * @param rect  The rect to compare against.
     *
     * @return true if the rects are not equal to each other.
     */
    bool operator!=(const Rect& rect) const {
        return origin != rect.origin || size != rect.size;
    }

    /**
     * Returns true if the rects are within tolerance of each other.
     *
     * The tolerance bound is on attribute independently.
     *
     * @param rect      The rect to compare against.
     * @param variance  The comparison tolerance.
     *
     * @return true if the rects are within tolerance of each other.
     */
    bool equals(const Rect& rect, float variance=CU_MATH_EPSILON) const {
        return origin.equals(rect.origin,variance) && size.equals(rect.size,variance);
    }
    
    /**
     * Returns true if this rect fits inside of the given rect.
     *
     * This operator is provided for convenience.  However, this does not 
     * provide a total order, making it unsafe for std::sort.
     *
     * @param rect The potentially larger rect
     *
     * @return True if this rect fits inside of the given rect.
     */
    bool operator<=(const Rect& rect) const {
        return inside(rect);
    }
    
    /**
     * Returns true if this rect properly fits inside of the given rect.
     *
     * Proper containment means that no edges of the two rects touch.
     *
     * This operator is provided for convenience.  However, this does not
     * provide a total order, making it unsafe for std::sort.
     *
     * @param rect The potentially larger rect
     *
     * @return True if this rect properly fits inside of the given rect.
     */
    bool operator<(const Rect& rect) const;
    
    /**
     * Returns true if this rect can hold the given rect.
     *
     * This operator is provided for convenience.  However, this does not
     * provide a total order, making it unsafe for std::sort.
     *
     * @param rect The potentially smaller rect
     *
     * @return True if this rect can hold the given rect.
     */
    bool operator>=(const Rect& rect) const {
        return contains(rect);
    }

    /**
     * Returns true if this rect can properly hold the given rect.
     *
     * Proper containment means that no edges of the two rects touch.
     *
     * This operator is provided for convenience.  However, this does not
     * provide a total order, making it unsafe for std::sort.
     *
     * @param rect The potentially smaller rect
     *
     * @return True if this rect can properly hold the given rect.
     */
    bool operator>(const Rect& rect) const;
    
    /**
     * Returns true if this rect fits inside of the given rect.
     *
     * This method does not test for strict containment.  To test for 
     * strict containment, use the comparison operator <.
     *
     * @param rect The potentially larger rect
     *
     * @return True if this rect fits inside of the given rect.
     */
    bool inside(const Rect& rect) const;
    
    /**
     * Returns true if this rect can hold the given rect.
     *
     * This method does not test for strict containment.  To test for
     * strict containment, use the comparison operator >.
     *
     * @param rect The potentially smaller rect
     *
     * @return True if this rect can hold the given rect.
     */
    bool contains(const Rect& rect) const;

    /**
     * Returns true if the given point is on the boundar of this rect.
     *
     * Combining this method with contains() allows you to test for strict
     * containment.
     *
     * @param point The point to test
     *
     * @return True if the given point is on the boundar of this rect.
     */
    bool touches(const Vec2& point) const;
    
    /**
     * Returns true if this rect contains the given point.
     *
     * This method does not test for strict containment; it includes the
     * boundardy.  To test for strict containment, combine this with the
     * method touches().
     *
     * @param point The point to test
     *
     * @return True if this rect contains the given point.
     */
    bool contains(const Vec2& point) const;
    
    /**
     * Returns true if this rect contains the given circle.
     *
     * This method does not test for strict containment; it includes the
     * boundary of both the circle and the rectangle.
     *
     * @param center The center of the circle
     * @param radius The radius of the circle
     *
     * @return True if this rect contains the given circle.
     */
    bool contains(const Vec2& center, float radius) const;
    
    /**
     * Returns true if this rect intersects the other.
     *
     * This method allows for intersections where the edges of the rects
     * are touching.  In this case, the size of the intersection is empty.
     *
     * @param rect The rect to test
     *
     * @return true if this rect intersects the other.
     */
    bool doesIntersect(const Rect& rect) const;

    /**
     * Returns true if this rect intersects the given circle.
     *
     * This method allows for intersections where the edge of the rect
     * simply touches the boundary of the circle.
     *
     * @param center The center of the circle
     * @param radius The radius of the circle
     *
     * @return true if this rect intersects the given circle.
     */
    bool doesIntersect(const Vec2& center, float radius) const;

    
#pragma mark -
#pragma mark Rectangle Arithmetic
    /**
     * Computes the union of this rect and the other, assigning it in place.
     *
     * @param rect  The rect to union with this one.
     *
     * @return This rect, after the union operation.
     */
    Rect& merge(const Rect & rect);

    /**
     * Computes the intersection of this rect and the other, assigning it in place.
     *
     * If there is no intersection, this rect becomes the zero rectangle.
     *
     * @param rect  The rect to intersect with this one.
     *
     * @return This rect, after the intersection operation.
     */
    Rect& intersect(const Rect & rect);

    /**
     * Expands this rectangle uniformly from its center.
     *
     * Each edge of the rectangle is adjusted factor away from the center
     * point.  As a result, this method changes both origin and size.  The
     * value factor can be negative, in which case the rect shrinks in size.
     *
     * @param factor  The amount to expand each edge from the center.
     *
     * @return This rect, after the expansion.
     */
    Rect& expand(float factor);
    
    /**
     * Expands this rectangle to the minimal one containing the given point.
     *
     * If the rectangle already contains the point, it is unchanged.
     *
     * @param point  The point to envelop
     *
     * @return This rect, after the expansion.
     */
    Rect& expand(const Vec2& point);
    
    /**
     * Returns the union of this rect and the other.
     *
     * @param rect  The rect to union with this one.
     *
     * Note: This does not modify the rect.
     *
     * @return the union of this rect and the other.
     */
    Rect getMerge(const Rect & rect) const {
        Rect result(*this);
        return result.merge(rect);
    }
    
    /**
     * Returns the intersection of this rect and the other.
     *
     * If there is no intersection, this method returns the zero rectangle.
     *
     * @param rect  The rect to intersect with this one.
     *
     * Note: This does not modify the rect.
     *
     * @return the intersection of this rect and the other.
     */
    Rect getIntersection(const Rect & rect) const {
        Rect result(*this);
        return result.intersect(rect);
    }
    
    /**
     * Returns a copy of this rect, expanded uniformly from its center.
     *
     * Each edge of the rectangle is adjusted factor away from the center
     * point.  As a result, this method changes both origin and size.  The
     * value factor can be negative, in which case the rect shrinks in size.
     *
     * Note: This does not modify the rect.
     *
     * @param factor  The amount to expand each edge from the center.
     *
     * @return a copy of this rect, expanded uniformly from its center.
     */
    Rect getExpansion(float factor) const {
        Rect result(*this);
        return result.expand(factor);
    }
    
    /**
     * Returns a copy of this rectangle, expanded to contain the given point.
     *
     * If the rectangle already contains the point, the rect is the same as
     * the original.
     *
     * @param point  The point to envelop
     *
     * @return a copy of this rect, expanded to contain the given point.
     */
    Rect getExpansion(const Vec2& point) const {
        Rect result(*this);
        return result.expand(point);
    }



};

}

#endif /* __CU_RECT_H__ */

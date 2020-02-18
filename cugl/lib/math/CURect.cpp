//
//  CURect.cpp
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
#include <algorithm>
#include <cugl/math/CURect.h>

using namespace cugl;

#pragma mark -
#pragma mark Comparisons
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
bool Rect::operator<(const Rect& rect) const {
    bool horz = rect.getMinX() < getMinX() && getMaxX() < rect.getMaxX();
    bool vert = rect.getMinY() < getMinY() && getMaxY() < rect.getMaxY();
    return horz && vert;
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
bool Rect::operator>(const Rect& rect) const {
    bool horz = getMinX() < rect.getMinX() && rect.getMaxX() < getMaxX();
    bool vert = getMinY() < rect.getMinY() && rect.getMaxY() < getMaxY();
    return horz && vert;
}

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
bool Rect::inside(const Rect& rect) const {
    bool horz = rect.getMinX() <= getMinX() && getMaxX() <= rect.getMaxX();
    bool vert = rect.getMinY() <= getMinY() && getMaxY() <= rect.getMaxY();
    return horz && vert;
}

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
bool Rect::contains(const Rect& rect) const {
    bool horz = getMinX() <= rect.getMinX() && rect.getMaxX() <= getMaxX();
    bool vert = getMinY() <= rect.getMinY() && rect.getMaxY() <= getMaxY();
    return horz && vert;
}


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
bool Rect::touches(const Vec2& point) const {
    bool horz = (getMinX() == point.x || getMaxX() == point.x) && getMinY() <= point.y && point.y <= getMaxY();
    bool vert = (getMinY() == point.y || getMaxY() == point.y) && getMinX() <= point.x && point.x <= getMaxX();
    return horz || vert;
}

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
bool Rect::contains(const Vec2& point) const {
    return  (getMinX() <= point.x && point.x <= getMaxX() &&
             getMinY() <= point.y && point.y <= getMaxY());
}

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
bool Rect::contains(const Vec2& center, float radius) const {
    Rect bounds(center.x-radius,center.y-radius,2*radius,2*radius);
    return contains(bounds);
}

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
bool Rect::doesIntersect(const Rect& rect) const {
    return !(getMaxX() < rect.getMinX() || rect.getMaxX() < getMinX() ||
             getMaxY() < rect.getMinY() || rect.getMaxY() < getMinY());
}

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
bool Rect::doesIntersect(const Vec2& center, float radius) const {
    Vec2 rectangleCenter((origin.x + size.width / 2),
                         (origin.y + size.height / 2));
    
    float w = size.width / 2;
    float h = size.height / 2;
    
    float dx = fabs(center.x - rectangleCenter.x);
    float dy = fabs(center.y - rectangleCenter.y);
    
    if (dx > (radius + w) || dy > (radius + h)) {
        return false;
    }
    
    Vec2 circleDistance(fabs(center.x - origin.x - w),
                        fabs(center.y - origin.y - h));
    
    if (circleDistance.x <= (w)) {
        return true;
    }
    
    if (circleDistance.y <= (h)) {
        return true;
    }
    
    float cornerDistanceSq = powf(circleDistance.x - w, 2) +
    powf(circleDistance.y - h, 2);
    return (cornerDistanceSq <= (powf(radius, 2)));
}


#pragma mark -
#pragma mark Rectangle Arithmetic
/**
 * Computes the union of this rect and the other, assigning it in place.
 *
 * @param rect  The rect to union with this one.
 *
 * @return This rect, after the union operation.
 */
Rect& Rect::merge(const Rect & rect) {
    float minX = std::min(getMinX(), rect.getMinX());
    float minY = std::min(getMinY(), rect.getMinY());
    float maxX = std::max(getMaxX(), rect.getMaxX());
    float maxY = std::max(getMaxY(), rect.getMaxY());
    set(minX, minY, maxX - minX, maxY - minY);
    return *this;
}

/**
 * Computes the intersection of this rect and the other, assigning it in place.
 *
 * If there is no intersection, this rect becomes the zero rectangle.
 *
 * @param rect  The rect to intersect with this one.
 *
 * @return This rect, after the intersection operation.
 */
Rect& Rect::intersect(const Rect & rect) {
    float minX = std::max(getMinX(), rect.getMinX());
    float minY = std::max(getMinY(), rect.getMinY());
    float maxX = std::min(getMaxX(), rect.getMaxX());
    float maxY = std::min(getMaxY(), rect.getMaxY());
    if (maxX - minX  < 0 || maxY - minY < 0) {
        minX = maxX = minY = maxY = 0;
    }
    set(minX, minY, maxX - minX, maxY - minY);
    return *this;
}

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
Rect& Rect::expand(float factor) {
    origin.set(origin.x-factor,origin.y-factor);
    size.set(size.width+factor,size.height+factor);
    return *this;
}

/**
 * Expands this rectangle to the minimal one containing the given point.
 *
 * If the rectangle already contains the point, it is unchanged.
 *
 * @param point  The point to envelop
 *
 * @return This rect, after the expansion.
 */
Rect& Rect::expand(const Vec2& point) {
    if (point.x < origin.x) {
        size.width += origin.x - point.x;
        origin.x = point.x;
    } else if (point.x > origin.x+size.width) {
        size.width = point.x-origin.x;
    }
    if (point.y < origin.y) {
        size.height += origin.y - point.y;
        origin.y = point.y;
    } else if (point.y > origin.y+size.height) {
        size.height = point.y-origin.y;
    }
    return *this;
}

#pragma mark -
#pragma mark Constants

/** The zero vector Vec2(0,0) */
const Rect Rect::ZERO(Vec2::ZERO,Size::ZERO);
/** The unit vector Vec2(1,1) */
const Rect Rect::UNIT(Vec2::ZERO,Size(1.0f,1.0f));

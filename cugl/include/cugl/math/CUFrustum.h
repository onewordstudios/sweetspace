//
//  CUFrustum.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a 3d viewing frustrum.  This is a bit
//  heavy-weight for our cameras, so we pulled it out..
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
//  Version: 6/20/16

#ifndef __CU_FRUSTRUM_H__
#define __CU_FRUSTRUM_H__

#include <cugl/math/CUPlane.h>
#include <cugl/math/CUVec3.h>
#include <cugl/util/CUDebug.h>

namespace cugl {


/**
 * This class is a truncated rectangular pyramid. 
 *
 * A frustum is used to define the viewable region and its projection onto
 * the screen.  Normally, we would embed this into a camera class.  However,
 * we do not want our cameras any more heavy weight than they are, so we 
 * pull them out.
 */
class Frustum {
#pragma mark Values
public:
    /** The number of planes in a frustum */
    static const int PLANE_COUNT = 6;
    /** The number of corners in a frustum */
    static const int CORNER_COUNT = 8;
    
    /** The three frustum locations */
    enum class Region {
        /** The shape is fully inside the frustum */
        INSIDE,
        /** The shape is fully outside the frustum */
        OUTSIDE,
        /** The shape intersects the frustum */
        INTERSECT
    };
    
    
private:
    /** The six clipping planes, near, far, left, right, top, bottom **/
    Plane _planes[6];
    
    /**
     * The eight points making up the near and far clipping "rectangles".
     *
     * The order is counter clockwise, starting at bottom left
     */
    Vec3 _points[8];

    
public:
    /** This enum is used to index the six clipping planes */
    enum class Side : unsigned int {
        /** The near clipping plane */
        CLOSE = 0,
        /** The far clipping plane */
        AWAY = 1,
        /** The left clipping plane */
        LEFT = 2,
        /** The right clipping plane */
        RIGHT  = 3,
        /** The top clipping plane */
        TOP = 4,
        /** The bottom clipping plane */
        BOTTOM  = 5
    };
    
#pragma mark Constructors
    /** 
     * Creates a frustum for the identity matrix.
     */
    Frustum () {
        set(Mat4::IDENTITY);
    }

    /**
     * Creates a frustum for the inverse projection view matrix.
     *
     * @param inverseView   The inverse projection view matrix.
     */
    Frustum (const Mat4& inverseView) {
        set(inverseView);
    }
    
    /**
     * Creates a copy of the given frustum.
     *
     * @param frustum   The frustum to copy.
     */
    Frustum (const Frustum& frustum) {
        set(frustum);
    }
    
    /**
     * Destroys this frustum, releasing all resources
     */
    ~Frustum() {}

#pragma mark -
#pragma mark Setters
    /**
     * Sets this frustum's clipping planes to the given inverse projection view matrix.
     *
     * @param inverseView   The inverse projection view matrix.
     *
     * @return A reference to this (modified) Frustum for chaining.
     */
    Frustum& operator=(const Mat4& inverseView) {
        return set(inverseView);
    }

    /**
     * Sets this frustum to be a copy of the given frustum.
     *
     * @param frustum The frustum to copy,
     *
     * @return A reference to this (modified) Frustum for chaining.
     */
    Frustum& operator=(const Frustum& frustum) {
        return set(frustum);
    }
    
    /**
     * Sets this frustum's clipping planes to the given inverse projection view matrix.
     *
     * @param inverseView   The inverse projection view matrix.
     *
     * @return A reference to this (modified) Frustum for chaining.
     */
    Frustum& set(const Mat4& inverseView);
    
    /**
     * Sets this frustum to be a copy of the given frustum.
     *
     * @param frustum The frustum to copy,
     *
     * @return A reference to this (modified) Frustum for chaining.
     */
    Frustum& set(const Frustum& frustum);
    

#pragma mark -
#pragma mark Attributes
    
    /**
     * Returns the clipping plane for the given side.
     *
     * See the enum Side for the available options.
     *
     * @param side  The clipping plane side.
     *
     * @return the clipping plane for the given side.
     */
    const Plane& getPlane(Side side) {
        CUAssertLog((unsigned int)side < PLANE_COUNT, "Side is not valid");
        return _planes[(unsigned int)side];
    }

    /**
     * Returns the frustum corner for the given index.
     *
     * The order is counter clockwise, starting at bottom left
     *
     * @return the frustum corner for the given index.
     */
    const Vec3& getCorner(unsigned int index) {
        CUAssertLog(index < CORNER_COUNT, "Index is not valid");
        return _points[index];
    }

#pragma mark -
#pragma mark Containment Methods
    /** 
     * Returns the location of the point with respect to the frustum.
     *
     * @param point The point to check.
     *
     * @return the location of the point with respect to the frustum.
     */
    Region find(const Vec3 point);
    
    /**
     * Returns the location of the point with respect to the frustum.
     *
     * @param x The x-coordinate of the point
     * @param y The y-coordinate of the point
     * @param z The z-coordinate of the point
     *
     * @return the location of the point with respect to the frustum.
     */
    Region find(float x, float y, float z) {
        return find(Vec3(x,y,z));
    }
    
    /** 
     * Returns the location of the sphere with respect to the frustum.
     *
     * @param center    The center of the sphere
     * @param radius    The radius of the sphere
     *
     * @return the location of the sphere with respect to the frustum.  
     */
    Region findSphere(const Vec3& center, float radius);
    
    /**
     * Returns the location of the sphere with respect to the frustum.
     *
     * @param x         The x-coordinate of the center of the sphere
     * @param y         The y-coordinate of the center of the sphere
     * @param z         The z-coordinate of the center of the sphere
     * @param radius    The radius of the sphere
     *
     * @return the containment type of the sphere.
     */
    Region findSphere(float x, float y, float z, float radius) {
        return findSphere(Vec3(x,y,z),radius);
    }
    
    /**
     * Returns the location of the sphere with respect to the frustum.
     *
     * This method does not check whether is the behind the near or far
     * clipping planes.
     *
     * @param center    The center of the sphere
     * @param radius    The radius of the sphere
     *
     * @return the location of the sphere with respect to the frustum.
     */
    Region findSphereWithoutNearFar(const Vec3& center, float radius);
    
    /**
     * Returns the location of the sphere with respect to the frustum.
     *
     * This method does not check whether is the behind the near or far
     * clipping planes.
     *
     * @param x         The x-coordinate of the center of the sphere
     * @param y         The y-coordinate of the center of the sphere
     * @param z         The z-coordinate of the center of the sphere
     * @param radius    The radius of the sphere
     *
     * @return the location of the sphere with respect to the frustum.
     */
    Region findSphereWithoutNearFar(float x, float y, float z, float radius) {
        return findSphereWithoutNearFar(Vec3(x,y,z),radius);
    }
    
    /** 
     * Returns the location of the bounding box with respect to the frustum.
     *
     * The bounding box is a cube, defined by its center and dimenion along
     * each axis.
     *
     * @param center    The center of the bounding box
     * @param dimension The dimensions of the bounding box
     *
     * @return the location of the bounding box with respect to the frustum.
     */
    Region findBox(const Vec3& center, const Vec3& dimension) {
        return findBox(center.x, center.y, center.z, dimension.x / 2, dimension.y / 2, dimension.z / 2);
    }
    
    /**
     * Returns the location of the bounding box with respect to the frustum.
     *
     * The bounding box is a cube, defined by its center and dimenion along
     * each axis.
     *
     * @param x             The x-coordinate of the center of the bounding box
     * @param y             The y-coordinate of the center of the bounding box
     * @param z             The z-coordinate of the center of the bounding box
     * @param halfWidth     Half of the width (x-axis) of the bounding box
     * @param halfHeight    Half of the height (y-axis) of the bounding box
     * @param halfDepth     Half of the depth (z-axis) of the bounding box
     *
     * @return the location of the bounding box with respect to the frustum.
     */
    Region findBox(float x, float y, float z, float halfWidth, float halfHeight, float halfDepth);
    
};
    
}
#endif /* __CU_FRUSTRUM_H__ */

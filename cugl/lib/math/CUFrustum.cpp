//
//  CUFrustum.cpp
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

#include <cugl/math/CUFrustum.h>
#include <cugl/util/CUDebug.h>
#include <cstring>

using namespace cugl;

/** The seed points to define a new frustum */
static float CLIP_SPACE_POINTS[8*3] = {
    -1.0f,-1.0f,-1.0f,  1.0f,-1.0f,-1.0f,  1.0f, 1.0f,-1.0f,  -1.0f, 1.0f,-1.0f, // near clip
    -1.0f,-1.0f, 1.0f,  1.0f,-1.0f, 1.0f,  1.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 1.0f  // far clip
};

#pragma mark Setters

/**
 * Sets this frustum's clipping planes to the given inverse projection view matrix.
 *
 * @param inverseView   The inverse projection view matrix.
 *
 * @return A reference to this (modified) Frustum for chaining.
 */
Frustum& Frustum::set(const Mat4& inverseView) {
    std::memcpy(_points, CLIP_SPACE_POINTS, CORNER_COUNT*sizeof(Vec3));
    for (int ii = 0; ii <  CORNER_COUNT; ii++) {
        Mat4::transform(inverseView,_points[ii],&_points[ii]);
    }
    
    _planes[0].set(_points[1], _points[2], _points[0]);
    _planes[1].set(_points[4], _points[7], _points[5]);
    _planes[2].set(_points[0], _points[3], _points[4]);
    _planes[3].set(_points[5], _points[6], _points[1]);
    _planes[4].set(_points[2], _points[6], _points[3]);
    _planes[5].set(_points[4], _points[1], _points[0]);
    return *this;
}

/**
 * Sets this frustum to be a copy of the given frustum.
 *
 * @param frustum The frustum to copy,
 *
 * @return A reference to this (modified) Frustum for chaining.
 */
Frustum& Frustum::set(const Frustum& frustum) {
    std::memcpy(_points, frustum._points, CORNER_COUNT*sizeof(Vec3));
    for (int ii = 0; ii < PLANE_COUNT; ii++) {
        _planes[ii].set(frustum._planes[ii]);
    }
    return *this;
}

#pragma mark -
#pragma mark Containment Methods


/**
 * Returns true if the point is in the frustum.
 *
 * @param point The point to check.
 *
 * @return true if the point is in the frustum.
 */
Frustum::Region Frustum::find(const Vec3 point) {
    int totalIN = 0;
    for (int ii = 0; ii < PLANE_COUNT; ii++) {
        int planeIN = 1;
        Plane::Side result = _planes[ii].sideOf(point);
        if (result == Plane::Side::INCIDENT) {
            return Region::INTERSECT;
        } else if (result == Plane::Side::BACK) {
            planeIN = 0;
        }
        totalIN += planeIN;
    }
    return (totalIN == PLANE_COUNT ? Region::INSIDE : Region::OUTSIDE);
}

/**
 * Returns if the sphere is in the frustum.
 *
 * @param center    The center of the sphere
 * @param radius    The radius of the sphere
 *
 * @return true if the sphere is in the frustum.
 */
Frustum::Region Frustum::findSphere(const Vec3& center, float radius) {
    for (int ii = 0; ii < PLANE_COUNT; ii++) {
        float dist = _planes[ii].distance(center);
        if (dist < -radius) {
            return Region::OUTSIDE;
        }
        if(fabs(dist) < radius) {
            return Region::INTERSECT;
        }
    }
    return  Region::INSIDE;
}

/**
 * Returns true if the sphere is in the frustum.
 *
 * This method does not check whether is the behind the near or far
 * clipping planes.
 *
 * @param center    The center of the sphere
 * @param radius    The radius of the sphere
 *
 * @return true if the sphere is in the frustum.
 */
Frustum::Region Frustum::findSphereWithoutNearFar (const Vec3& center, float radius) {
    for (int ii = 2; ii < PLANE_COUNT; ii++) {
        float dist = _planes[ii].distance(center);
        if (dist < -radius) {
            return Region::OUTSIDE;
        }
        if(fabs(dist) < radius) {
            return Region::INTERSECT;
        }
    }
    return  Region::INSIDE;
}

/**
 * Returns the containment type of the bounding box.
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
 * @return the containment type of the bounding box.
 */
Frustum::Region Frustum::findBox(float x, float y, float z,
                                 float halfWidth, float halfHeight, float halfDepth) {
    Vec3 corner[8];
    corner[0].set(x + halfWidth, y + halfHeight, z + halfDepth);
    corner[1].set(x + halfWidth, y + halfHeight, z - halfDepth);
    corner[2].set(x + halfWidth, y - halfHeight, z + halfDepth);
    corner[3].set(x + halfWidth, y - halfHeight, z - halfDepth);
    corner[4].set(x - halfWidth, y + halfHeight, z + halfDepth);
    corner[5].set(x - halfWidth, y + halfHeight, z - halfDepth);
    corner[6].set(x - halfWidth, y - halfHeight, z + halfDepth);
    corner[7].set(x - halfWidth, y - halfHeight, z - halfDepth);

    int totalIN = 0;
    for (int ii = 0; ii < PLANE_COUNT; ii++) {
        int countIN = 8;
        int pointIN = 1;
        for(int jj = 0; jj < 8; ++jj) {
            if (_planes[ii].sideOf(corner[jj]) == Plane::Side::BACK) {
                pointIN = 0;
                --countIN;
            }
        }
            
        // Nothing was on the right side.
        if (countIN == 0) {
            return Region::OUTSIDE;
        }
            
        totalIN += pointIN;
    }
        
    // All points are inside
    if (totalIN == 6) {
        return Region::INSIDE;
    }
        
    return Region::INTERSECT;
}

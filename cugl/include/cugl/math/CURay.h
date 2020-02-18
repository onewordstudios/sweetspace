//
//  CURay.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a 3d ray.
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

#ifndef __CU_RAY_H__
#define __CU_RAY_H__

#include "CUVec3.h"
#include "CUMat4.h"

namespace cugl {
/** 
 * This class is a ray with a starting position and a unit length direction.
 *
 * While this class has a few convience methods, most of the access is 
 * intended to be through the individual attributes.
 */
class Ray {

#pragma mark Values
public:
    /** The anchoring origin of this ray. */
    Vec3 origin;
    /** The direction of this ray (a unit vector) */
    Vec3 direction;
    
    /** The ray anchored at the origin along the x-axis */
    static const Ray X_AXIS;
    /** The ray anchored at the origin along the y-axis */
    static const Ray Y_AXIS;
    /** The ray anchored at the origin along the z-axis */
    static const Ray Z_AXIS;

#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates a new ray along the x-axis
     */
    Ray() : direction(Vec3::UNIT_X) { }

    /** 
     * Creates a new ray with the given direction.
     * 
     * The origin of the ray is (0,0,0)
     *
     * @param direction The ray direction 
     */
    Ray (const Vec3& direction) {
        set(Vec3::ZERO,direction);
    }
    
    /** 
     * Creates a new ray with the given origin and the direction.
     *
     * @param origin    The starting position
     * @param direction The ray direction 
     */
    Ray (const Vec3&  origin, const Vec3& direction) {
        set(origin,direction);
    }
    
    /** 
     * Creates a copy of the given ray.
     *
     * @param ray   The ray to copy
     */
    Ray(const Ray& ray) {
        set(ray);
    }
    
    /**
     * Destroys this ray, releasing all resources
     */
    ~Ray() {}
    
    
#pragma mark -
#pragma mark Setters
    /**
     * Sets this ray to be a copy of the given ray.
     *
     * @param ray   The ray to copy
     *
     * @return A reference to this (modified) Ray for chaining.
     */
    Ray& operator=(const Ray& ray) {
        return set(ray);
    }

    /**
     * Sets this ray to go along the given direction from the origin.
     *
     * This will reset the origin to (0,0,0).
     *
     * @param direction The ray direction
     *
     * @return A reference to this (modified) Ray for chaining.
     */
    Ray& operator=(const Vec3& direction) {
        return set(direction);
    }

    /**
     * Sets this ray to be a copy of the given ray.
     *
     * @param ray   The ray to copy
     *
     * @return A reference to this (modified) Ray for chaining.
     */
    Ray& set(const Ray& ray) {
        origin = ray.origin;
        direction = ray.direction;
        return *this;
    }

    /**
     * Sets this ray to have the given origin and the direction.
     *
     * @param origin    The starting position
     * @param direction The ray direction
     *
     * @return A reference to this (modified) Ray for chaining.
     */
    Ray& set(const Vec3&  origin, const Vec3& direction) {
        this->origin = origin;
        this->direction  = direction.getNormalization();
        return *this;
    }
    
    /**
     * Sets this ray to go along the given direction from the origin.
     *
     * This will reset the origin to (0,0,0).
     *
     * @param direction The ray direction
     *
     * @return A reference to this (modified) Ray for chaining.
     */
    Ray& set(const Vec3& direction) {
        origin.setZero();
        this->direction  = direction.getNormalization();
        return *this;
    }

#pragma mark -
#pragma mark Static Arithmetic
    /** 
     * Computes the endpoint for the given distance and assigns it to dst.
     * 
     * This is calculated as startpoint + distance * direction.
     *
     * @param ray       The ray to calculate
     * @param distance  The distance value
     * @param dst       A vector to store the result in
     *
     * @return A reference to dst for chaining
     */
    static Vec3* endpoint(const Ray& ray, float distance, Vec3* dst);

    /**
     * Multiplies the ray by the given matrix and assigns it to dst.
     *
     * Use this to transform a ray into another coordinate system.
     *
     * @param ray   The ray to transform
     * @param mat   The transform matrix
     * @param dst   A ray to store the result in
     *
     * @return A reference to dst for chaining
     */
    static Ray* multiply(const Ray& ray, const Mat4& mat, Ray* dst);

#pragma mark -
#pragma mark Arithmetic

    /**
     * Returns the endpoint for the given distance.
     *
     * This is calculated as startpoint + distance * direction.
     *
     * @param distance  The distance value
     *
     * @return the endpoint for the given distance.
     */
    Vec3 getEndpoint(float distance) const {
        return direction*distance+origin;
    }
    
    /**
     * Multiplies this ray by the given matrix.
     *
     * Use this to transform the ray into another coordinate system.
     *
     * @param mat   The transform matrix
     *
     * @return A reference to this (modified) Ray for chaining.
     */
    Ray& multiply(const Mat4& mat) {
        return *(multiply(*this,mat,this));
    }
    
#pragma mark -
#pragma mark Operators
    
    /**
     * Multiplies this ray by the given matrix.
     *
     * Use this to transform the ray into another coordinate system.
     *
     * @param mat   The transform matrix
     *
     * @return A reference to this (modified) Ray for chaining.
     */
    Ray& operator*=(const Mat4& mat) {
        return multiply(mat);
    }
    
    /**
     * Returns the endpoint for the given distance.
     *
     * This is calculated as startpoint + distance * direction.
     *
     * @param distance  The distance value
     *
     * @return the endpoint for the given distance.
     */
    Vec3 operator*(float distance) const {
        return getEndpoint(distance);
    }

    /**
     * Returns a copy of this ray multiplied by the given matrix.
     *
     * Use this to transform the ray into another coordinate system.
     *
     * @param mat   The transform matrix
     *
     * @return a copy of this ray multiplied by the given matrix.
     */
    const Ray operator*(const Mat4& mat) const {
        Ray result(*this);
        return result.multiply(mat);
    }
    
#pragma mark -
#pragma mark Comparisons
    /**
     * Returns true if this ray is equal to the given ray.
     *
     * Comparison is exact, which may be unreliable given that the attributes
     * are floats.
     *
     * @param r The ray to compare against.
     *
     * @return True if this ray is equal to the given ray.
     */
    bool operator==(const Ray& r) const {
        return origin == r.origin && direction == r.direction;
    }
    
    /**
     * Returns true if this ray is not equal to the given ray.
     *
     * Comparison is exact, which may be unreliable given that the attributes
     * are floats.
     *
     * @param r The ray to compare against.
     *
     * @return True if this ray is not equal to the given ray.
     */
    bool operator!=(const Ray& r) const {
        return origin != r.origin || direction != r.direction;
    }
    
    /**
     * Returns true if the rays are within tolerance of each other.
     *
     * The tolerance bounds the origin and direction separately.
     *
     * @param r         The ray to compare against.
     * @param variance  The comparison tolerance.
     *
     * @return true if the rays are within tolerance of each other.
     */
    bool equals(const Ray& r, float variance=CU_MATH_EPSILON) const {
        return (origin.equals(r.origin,variance) &&
                direction.equals(r.direction,variance));
    }
    
#pragma mark -
#pragma mark Conversion Methods
    /**
     * Returns a string representation of this ray for debugging purposes.
     *
     * If verbose is true, the string will include class information.  This
     * allows us to unambiguously identify the class.
     *
     * @param verbose Whether to include class information
     *
     * @return a string representation of this ray for debugging purposes.
     */
    std::string toString(bool verbose = false) const;
    
    /** Cast from Ray to a string. */
    operator std::string() const { return toString(); }
};

}
#endif /* __CU_RAY_H__ */

//
//  CUPlane.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a 3d plane.
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

#ifndef __CU_PLANE_H__
#define __CU_PLANE_H__

#include "CUVec3.h"
#include "CUMat4.h"
#include "CURay.h"

namespace cugl {

/**
 * This class ia a plane defined via a normal and the distance from the origin.
 *
 * The normal must be a unit vector.  However, we allow direct access to the
 * normal and do not check this invariant.
*/
class Plane {
#pragma mark Values
public:
    /** 
     * This enum specifies on which side a point lies respective to the plane.
     */
    enum class Side {
        /** The point is on the plane */
        INCIDENT,
        /** The point is on the side opposite the normal */
        BACK,
        /** The point is on the side with the normal */
        FRONT
    };
    
    /** The plane normal (should be a unit vector) */
    Vec3 normal;
    /** The distance of the plane from the origin */
    float offset;
    
    /** The xy plane (normal is z-axis) */
    static const Plane XY;
    /** The xz plane (normal is y-axis) */
    static const Plane XZ;
    /** The yz plane (normal is x-axis) */
    static const Plane YZ;
    
#pragma mark -
#pragma mark Constructors
    /**
     * Creates the XY plane.
     */
    Plane () : normal(Vec3::UNIT_Z), offset(0) {}

    /** 
     * Creates a new plane with the given normal and distance to the origin.
     *
     * @param normal    The plane normal
     * @param dist      The distance to the origin 
     */
    Plane (const Vec3& normal, float dist=0) {
        set(normal,dist);
    }
    
    /** 
     * Creates a new plane based on the normal and a point on the plane.
     *
     * @param normal    The normal
     * @param point     The point on the plane
     */
    Plane (const Vec3& normal, const Vec3& point) {
        set(normal,point);
    }
    
    /** 
     * Creates a new plane containing the three given points.
     *
     * The normal is calculated via a cross product between 
     * (point1-point2)x(point2-point3)
     *
     * @param point1    The first point
     * @param point2    The second point
     * @param point3    The third point
     */
    Plane(const Vec3&  point1, const Vec3& point2, const Vec3& point3) {
        set(point1, point2, point3);
    }

    /**
     * Creates a new plane from the given equation coefficients.
     *
     * The equation of the plane is ax + by + cz = d.
     *
     * @param a    The x coefficient
     * @param b    The y coefficient
     * @param c    The z coefficient
     * @param d    The constant factor
     */
    Plane(float a, float b, float c, float d) {
        set(a,b,c,d);
    }
    
    /**
     * Creates a copy of the given plane.
     *
     * @param plane The plane to copy,
     */
    Plane(const Plane& plane) {
        set(plane);
    }
    
    /**
     * Destroys this plane, releasing all resources
     */
    ~Plane() {}
    
#pragma mark -
#pragma mark Setters
    /**
     * Sets this plane to be a copy of the given plane.
     *
     * @param plane The plane to copy,
     *
     * @return A reference to this (modified) Plane for chaining.
     */
    Plane& operator=(const Plane& plane) {
        return set(plane);
    }
    
    /**
     * Sets this plane to have the given normal.
     *
     * After assignment, this plane will intersect the origin.
     *
     * @param normal    The plane normal
     *
     * @return A reference to this (modified) Plane for chaining.
     */
    Plane& operator=(const Vec3& normal) {
        return set(normal,0);
    }
    
    /**
     * Sets this plane to have the given normal and distance to the origin.
     *
     * @param normal    The plane normal
     * @param dist      The distance to the origin
     *
     * @return A reference to this (modified) Plane for chaining.
     */
    Plane& set(const Vec3& normal, float dist=0);
    
    /**
     * Sets this plane to have the given normal and contain the given point.
     *
     * @param normal    The normal
     * @param point     The point on the plane
     *
     * @return A reference to this (modified) Plane for chaining.
     */
    Plane& set(const Vec3& normal, const Vec3& point);
    
    /**
     * Creates a new plane to one containing the three given points.
     *
     * The normal is calculated via a cross product between
     * (point1-point2)x(point2-point3)
     *
     * @param point1    The first point
     * @param point2    The second point
     * @param point3    The third point
     *
     * @return A reference to this (modified) Plane for chaining.
     */
    Plane& set(const Vec3&  point1, const Vec3& point2, const Vec3& point3);
    
    /**
     * Sets this plane to have the given equation coefficients.
     *
     * The equation of the plane is ax + by + cz = d.
     *
     * @param a    The x coefficient
     * @param b    The y coefficient
     * @param c    The z coefficient
     * @param d    The constant factor
     */
    Plane& set(float a, float b, float c, float d);
    
    /**
     * Sets this plane to be a copy of the given plane.
     *
     * @param plane The plane to copy,
     *
     * @return A reference to this (modified) Plane for chaining.
     */
    Plane& set(const Plane& plane) {
        normal = plane.normal;
        offset = plane.offset;
        return *this;
    }
    
#pragma mark -
#pragma mark Static Arithmetic
    /**
     * Multiplies the plane by the given matrix and assigns it to dst.
     *
     * The multiplication transforms the plane.  That is, if point p is
     * on the original plane, p*M is on the newly created plane.
     * To transform a plane by a matrix, we mutliply the vector (a,b,c,d)
     * by the inverse of mat, where ax + by + cz = d is the equation of
     * the plane.
     *
     * @param plane The plane to transform
     * @param mat   The transform matrix
     * @param dst   A plane to store the result in
     *
     * @return A reference to dst for chaining
     */
    static Plane* multiply(const Plane& plane, const Mat4& mat, Plane* dst);

    /**
     * Computes the intersection paramter the ray with the plane.
     *
     * If the value is negative, the intersection is in the reverse direction
     * of the ray.
     *
     * @param plane The plane to check
     * @param ray   The ray to check
     *
     * @return the intersection paramter the ray with the plane.
     */
    static float intersection(const Plane& plane, const Ray& ray);

#pragma mark -
#pragma mark Arithmetic
    /**
     * Multiplies this plane by the given matrix.
     *
     * The multiplication transforms the plane.  That is, if point p is
     * on the original plane, p*M is on the newly created plane.
     * To transform a plane by a matrix, we mutliply the vector (a,b,c,d)
     * by the inverse of mat, where ax + by + cz = d is the equation of
     * the plane.
     *
     * @param mat   The transform matrix
     *
     * @return A reference to this (modified) Plane for chaining.
     */
    Plane& multiply(const Mat4& mat) {
        return *(multiply(*this,mat,this));
    }

    /**
     * Multiplies this plane by the given matrix.
     *
     * The multiplication transforms the plane.  That is, if point p is
     * on the original plane, p*M is on the newly created plane.
     * To transform a plane by a matrix, we mutliply the vector (a,b,c,d)
     * by the inverse of mat, where ax + by + cz = d is the equation of
     * the plane.
     *
     * @param mat   The transform matrix
     *
     * @return A reference to this (modified) Plane for chaining.
     */
    Plane& operator*=(const Mat4& mat) {
        return *(multiply(*this,mat,this));
    }

    /**
     * Return a copy of this plane transformed by the given matrix.
     *
     * The multiplication transforms the plane.  That is, if point p is
     * on the original plane, p*M is on the newly created plane.
     * To transform a plane by a matrix, we mutliply the vector (a,b,c,d)
     * by the inverse of mat, where ax + by + cz = d is the equation of
     * the plane.
     *
     * @param mat   The transform matrix
     *
     * @return a copy of this plane transformed by the given matrix.
     */
    const Plane operator*(const Mat4& mat) const {
        Plane result;
        return *(multiply(*this,mat,&result));
    }

#pragma mark -
#pragma mark Plane Methods
    /** 
     * Return the signed distance between the plane and the given point.
     *
     * The distance is signed, so that a point on the same side as the normal
     * will have positive distance, while a point on the back side will have
     * negative distance.
     *
     * @return the signed distance between the plane and the given point.
     */
    float distance(const Vec3& point) const {
        return normal.dot(point) + offset;
    }
    
    /**
     * Returns the intersection parameter of the ray with this plane.
     *
     * If the value is negative, the intersection is in the reverse direction
     * of the ray.
     *
     * @param ray   The ray to check
     *
     * @return the intersection of the ray with this plane.
     */
    float getIntersection(const Ray& ray) const {
        return intersection(*this,ray);
    }
    
    /**
     * Returns true if this plane contains the given point within tolerance.
     *
     * The tolerance bounds the point distance.
     *
     * @param point The point to check
     * @param variance  The comparison tolerance.
     * @return true if this plane contains the given point within tolerance.
     */
    bool contains(const Vec3& point, float variance=CU_MATH_EPSILON) const {
        return fabsf(distance(point)) <= variance;
    }
    
    /** 
     * Returns which side the given point lies relative to normal. 
     *
     * Side::FRONT refers to the side the plane normal points to.
     *
     * @param point The point to check
     *
     * @return which side the given point lies relative to normal.
     */
    Side sideOf(const Vec3& point) const {
        float dist = normal.dot(point) + offset;
        return (dist == 0 ? Side::INCIDENT : (dist < 0 ? Side::BACK : Side::FRONT));
    }
    
    /**
     * Returns which side the given point lies relative to normal.
     *
     * Side::FRONT refers to the side the plane normal points to.
     *
     * @param x     The x-coordinate of the point
     * @param y     The x-coordinate of the point
     * @param z     The x-coordinate of the point
     *
     * @return which side the given point lies relative to normal.
     */
     Side sideOf(float x, float y, float z) const {
         return sideOf(Vec3(x,y,z));
     }
    
    
    /** 
     * Returns whether the plane is facing the direction vector.
     *
     * In this method, you should think of the direction vector as the 
     * direction a camera looks in. This method will return true if the 
     * front side of the plane determined by its normal faces the camera.
     *
     * @param direction The direction
     * @return whether the plane is facing the direction vector.
     */
    bool isFrontFacing(const Vec3& direction) const {
        float dot = normal.dot(direction);
        return dot <= 0;
    }
    
#pragma mark -
#pragma mark Comparisons
    /**
     * Returns true if this plane is equal to the given plane.
     *
     * Comparison is exact, which may be unreliable given that the attributes
     * are floats.
     *
     * @param p The plane to compare against.
     *
     * @return True if this plane is equal to the given plane.
     */
    bool operator==(const Plane& p) const {
        return normal == p.normal && offset == p.offset;
    }
    
    /**
     * Returns true if this plane is not equal to the given plane.
     *
     * Comparison is exact, which may be unreliable given that the attributes
     * are floats.
     *
     * @param p The plane to compare against.
     *
     * @return True if this plane is not equal to the given plane.
     */
    bool operator!=(const Plane& p) const {
        return normal != p.normal || offset != p.offset;
    }
    
    /**
     * Returns true if the planes are within tolerance of each other.
     *
     * The tolerance bounds the normal and distance separately.
     *
     * @param p         The plane to compare against.
     * @param variance  The comparison tolerance.
     *
     * @return true if the rays are within tolerance of each other.
     */
    bool equals(const Plane& p, float variance=CU_MATH_EPSILON) const {
        return (normal.equals(p.normal,variance) &&
                CU_MATH_APPROX(offset,p.offset,variance));
    }
    
#pragma mark -
#pragma mark Conversion Methods
    /**
     * Returns a string representation of this plane for debugging purposes.
     *
     * The plane will be represented by its equation ax+by+cz = d.
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
    operator std::string() const { return toString(); }};
}

#endif /* __CU_PLANE_H__ */

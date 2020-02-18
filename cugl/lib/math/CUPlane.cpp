//
//  CUPlane.cpp
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

#include <cugl/math/CUPlane.h>
#include <cugl/util/CUDebug.h>
#include <cugl/util/CUStrings.h>
#include <sstream>

using namespace cugl;

#pragma mark Methods
/**
 * Sets this plane to have the given normal and distance to the origin.
 *
 * @param normal    The plane normal
 * @param dist      The distance to the origin
 *
 * @return A reference to this (modified) Plane for chaining.
 */
Plane& Plane::set(const Vec3& normal, float dist) {
    this->normal = normal.getNormalization();
    CUAssertLog(!normal.isZero(), "Normal has zero length");
    offset = dist;
    return *this;
}

/**
 * Sets this plane to have the given normal and contain the given point.
 *
 * @param normal    The normal
 * @param point     The point on the plane
 *
 * @return A reference to this (modified) Plane for chaining.
 */
Plane& Plane::set(const Vec3& normal, const Vec3& point) {
    this->normal = normal.getNormalization();
    CUAssertLog(!normal.isZero(), "Normal has zero length");
    offset = -(this->normal.dot(point));
    return *this;
}

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
Plane& Plane::set(const Vec3&  point1, const Vec3& point2, const Vec3& point3) {
    Vec3::cross(point1-point2,point2-point3,&normal);
    normal.normalize();
    CUAssertLog(!normal.isZero(), "Points are colinear");
    offset = -point1.dot(normal);
    return *this;
}

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
Plane& Plane::set(float a, float b, float c, float d) {
    normal.set(a,b,c);
    float leng = normal.length();
    CUAssertLog(leng != 0, "Normal has zero length");
    offset = d/leng;
    normal /= leng;
    return *this;
}

/**
 * Multiplies the plane by the given matrix and assigns it to dst.
 *
 * The multiplication transforms the plane.  That is, if point p is
 * on the original plane, p*M is on the newly created plane.
 * To transform a plane by a matrix, we mutliply the vector (a,b,c,d)
 * by the inverse of mat, where ax + by + cz = d is the equation of
 * the plane.
 *
 * @param plane The ray to transform
 * @param mat   The transform matrix
 * @param dst   A plane to store the result in
 *
 * @return A reference to dst for chaining
 */
Plane* Plane::multiply(const Plane& plane, const Mat4& mat, Plane* dst) {
    CUAssertLog(dst, "Assignment plane is null");
    Mat4 inverse;
    Mat4::invert(mat,&inverse);
    CUAssertLog(inverse != Mat4::ZERO, "Matrix mat is not an invertible");
    Vec4 equation(plane.normal,plane.offset);
    equation *= inverse;
    dst->normal.set(equation.x,equation.y,equation.z);
    float leng = dst->normal.length();
    dst->offset = equation.w/leng;
    dst-> normal /= leng;
    return dst;
}

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
float Plane::intersection(const Plane& plane, const Ray& ray) {
    return -(ray.origin.dot(plane.normal)+plane.offset)/plane.normal.dot(ray.direction);
}


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
std::string Plane::toString(bool verbose) const {
    std::stringstream ss;
    ss << (verbose ? "cugl::Plane[" : "[");
    ss << to_string(normal.x) << "x+";
    ss << to_string(normal.y) << "y+";
    ss << to_string(normal.z) << "z = ";
    ss << to_string(offset);
    ss << "]";
    return ss.str();
}

#pragma mark -
#pragma mark Constants

/** The xy plane (normal is z-axis) */
const Plane Plane::XY(Vec3(0,0,1));
/** The xz plane (normal is y-axis) */
const Plane Plane::XZ(Vec3(0,1,0));
/** The yz plane (normal is x-axis) */
const Plane Plane::YZ(Vec3(1,0,0));

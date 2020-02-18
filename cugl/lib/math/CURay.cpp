///
//  CURay.cpp
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
#include <cugl/math/CURay.h>
#include <cugl/util/CUDebug.h>
#include <cugl/util/CUStrings.h>
#include <sstream>

using namespace cugl;

#pragma mark Methods

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
Vec3* Ray::endpoint(const Ray& ray, float distance, Vec3* dst) {
    CUAssertLog(dst, "Assignment vector is null");
    *dst = ray.direction*distance+ray.origin;
    return dst;
}

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
Ray* Ray::multiply(const Ray& ray, const Mat4& mat, Ray* dst) {
    CUAssertLog(dst, "Assignment vector is null");
    Vec3 tmp = ray.origin+ray.direction;
    tmp *= mat;
    dst->origin = ray.origin*mat;
    dst->direction = tmp-dst->origin;
    return dst;
}

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
std::string Ray::toString(bool verbose) const {
    std::stringstream ss;
    ss << (verbose ? "cugl::Ray(origin:" : "(origin:");
    ss <<  origin.toString();
    ss << ",direction:";
    ss << direction.toString();
    ss << ")";
    return ss.str();
}

#pragma mark -
#pragma mark Constants

/** The ray anchored at the origin along the x-axis */
const Ray Ray::X_AXIS(Vec3(1,0,0));
/** The ray anchored at the origin along the y-axis */
const Ray Ray::Y_AXIS(Vec3(0,1,0));
/** The ray anchored at the origin along the z-axis */
const Ray Ray::Z_AXIS(Vec3(0,0,1));

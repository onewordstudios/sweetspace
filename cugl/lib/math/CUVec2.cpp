//
//  CUVec2.cpp
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
#include <SDL/SDL.h>
#include <algorithm>
#include <iostream>
#include <sstream>

#include <cugl/util/CUDebug.h>
#include <cugl/util/CUStrings.h>
#include <cugl/math/CUVec2.h>
#include <cugl/math/CUVec3.h>
#include <cugl/math/CUVec4.h>
#include <cugl/math/CUSize.h>

using namespace cugl;

#pragma mark -
#pragma mark Static Arithmetic

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
Vec2* Vec2::clamp(const Vec2& v, const Vec2& min, const Vec2& max, Vec2* dst) {
    CUAssertLog(dst, "Assignment vector is null");
    dst->x = clampf(v.x,min.x,max.x);
    dst->y = clampf(v.y,min.y,max.y);
    return dst;
}

/**
 * Returns the angle (in radians) between the specified vectors.
 *
 * The angle is directional from v1 to v2.
 *
 * @param v1 The first vector.
 * @param v2 The second vector.
 *
 * @return The angle between the two vectors (in radians).
 */
float Vec2::angle(const Vec2& v1, const Vec2& v2) {
    float dz = v1.x * v2.y - v1.y * v2.x;
    return (dz < CU_MATH_EPSILON && dz > -CU_MATH_EPSILON ? 0.0f : atan2f(dz, dot(v1, v2)));
}

/**
 * Adds the specified vectors and stores the result in dst.
 *
 * @param v1    The first vector.
 * @param v2    The second vector.
 * @param dst   A vector to store the result in
 *
 * @return A reference to dst for chaining
 */
Vec2* Vec2::add(const Vec2& v1, const Vec2& v2, Vec2* dst) {
    CUAssertLog(dst, "Assignment vector is null");
    dst->x = v1.x+v2.x;
    dst->y = v1.y+v2.y;
    return dst;

}

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
Vec2* Vec2::subtract(const Vec2& v1, const Vec2& v2, Vec2* dst) {
    CUAssertLog(dst, "Assignment vector is null");
    dst->x = v1.x-v2.x;
    dst->y = v1.y-v2.y;
    return dst;
}

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
Vec2* Vec2::scale(const Vec2& v, float s, Vec2* dst) {
    CUAssertLog(dst, "Assignment vector is null");
    dst->x = v.x*s;
    dst->y = v.y*s;
    return dst;
}

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
Vec2* Vec2::scale(const Vec2& v1, const Vec2& v2, Vec2* dst) {
    CUAssertLog(dst, "Assignment vector is null");
    dst->x = v1.x*v2.x;
    dst->y = v1.y*v2.y;
    return dst;
}

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
Vec2* Vec2::divide(const Vec2& v, float s, Vec2* dst) {
    CUAssertLog(dst, "Assignment vector is null");
    dst->x = v.x/s;
    dst->y = v.y/s;
    return dst;
}

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
Vec2* Vec2::divide(const Vec2& v1, const Vec2& v2, Vec2* dst) {
    CUAssertLog(dst, "Assignment vector is null");
    dst->x = v1.x/v2.x;
    dst->y = v1.y/v2.y;
    return dst;
}

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
Vec2* Vec2::reciprocate(const Vec2& v, Vec2* dst) {
    CUAssertLog(dst, "Assignment vector is null");
    CUAssertLog(v.x != 0 && v.y != 0, "Reciprocating zero value");
    dst->x = 1.0f/v.x; dst->y = 1.0f/v.y;
    return dst;
}

/**
 * Negates the specified vector and stores the result in dst.
 *
 * @param v     The vector to negate.
 * @param dst   The destination vector.
 *
 * @return A reference to dst for chaining
 */
Vec2* Vec2::negate(const Vec2& v, Vec2* dst) {
    CUAssertLog(dst, "Assignment vector is null");
    dst->x = -v.x; dst->y = -v.y;
    return dst;
}


#pragma mark -
#pragma mark Arithmetic

/**
 * Divides this vector in place by the given factor.
 *
 * @param s The scalar to divide by
 *
 * @return A reference to this (modified) Vec2 for chaining.
 */
Vec2& Vec2::divide(float s) {
    CUAssertLog(s != 0,"Division by zero");
    x /= s; y /= s;
    return *this;
}

/**
 * Divides this vector nonuniformly by the given factors.
 *
 * @param sx The scalar to divide the x-axis
 * @param sy The scalar to divide the y-axis
 *
 * @return A reference to this (modified) Vec2 for chaining.
 */
Vec2& Vec2::divide(float sx, float sy) {
    CUAssertLog(sx != 0 && sy != 0,"Division by zero");
    x /= sx; y /= sy;
    return *this;
}

/**
 * Divides this vector in place by the given vector.
 *
 * This method is provided to support non-uniform scaling.
 *
 * @param s The vector to divide by
 *
 * @return A reference to this (modified) Vec2 for chaining.
 */
Vec2& Vec2::divide(const Vec2& v) {
    CUAssertLog(v.x != 0 && v.y != 0,"Division by zero");
    x /= v.x; y /= v.y;
    return *this;
}

#pragma mark -
#pragma mark Linear Algebra
/**
 * Returns the angle between this vector and other.
 *
 * The angle is directionaly and measured starting at this one.  If either 
 * vector is zero, the result is undefined.
 *
 * @param other The vector to sweep towards.
 *
 * @return the angle between this vector and other.
 */
float Vec2::getAngle(const Vec2& other) const {
    float dz = x * other.y - y * other.x;
    return (dz < CU_MATH_EPSILON && dz > -CU_MATH_EPSILON ? 0.0f : atan2f(dz, dot(other)));
}

/**
 * Normalizes this vector.
 *
 * This method normalizes Vec2 so that it is of unit length (i.e. the
 * length of the vector after calling this method will be 1.0f). If the
 * vector already has unit length or if the length of the vector is zero,
 * this method does nothing.
 *
 * @return This vector, after the normalization occurs.
 */
Vec2& Vec2::normalize() {
    float n = x * x + y * y;
    // Already normalized.
    if (n == 1.0f) {
        return *this;
    }
    
    n = sqrt(n);

    // Too close to zero.
    if (n < CU_MATH_FLOAT_SMALL) {
        return *this;
    }
    
    n = 1.0f / n;
    x *= n;
    y *= n;

    return *this;
}


/**
 * Rotates this vector by the angle (in radians) around the origin.
 *
 * @param point The point to rotate around.
 * @param angle The angle to rotate by (in radians).
 *
 * @return This vector, after the rotation occurs.
 */
Vec2& Vec2::rotate(float angle) {
    return rotate(Vec2::forAngle(angle));
}

/**
 * Rotates this vector by the angle (in radians) around the given point.
 *
 * @param angle The angle to rotate by (in radians).
 * @param point The point to rotate around.
 *
 * @return This vector, after the rotation occurs.
 */
Vec2& Vec2::rotate(float angle, const Vec2& point) {
    *this -= point;
    rotate(Vec2::forAngle(angle));
    return *this += point;
}

/**
 * Returns a copy of this vector rotated by the angle around the origin.
 *
 * The angle is measured in radians.
 * Note: this does not modify this vector.
 *
 * @param point The point to rotate around.
 * @param angle The angle to rotate by (in radians).
 *
 * @return a copy of this vector rotated by the angle around the origin.
 */
Vec2 Vec2::getRotation(float angle) {
    return getRotation(Vec2::forAngle(angle));
}

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
Vec2 Vec2::getRotation(float angle, const Vec2& point) {
    Vec2 result = (*this-point);
    result.rotate(Vec2::forAngle(angle));
    return result + point;
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
float Vec2::dot(const Vec2& v1, const Vec2& v2) {
    return (v1.x * v2.x + v1.y * v2.y);
}

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
float Vec2::cross(const Vec2& v1, const Vec2& v2) {
    return v1.x*v2.y - v1.y*v2.x;
}

/**
 * Normalizes the specified vector and stores the result in dst.
 *
 * If the vector already has unit length or if the length of the
 *  vector is zero, this method copies v into dst.
 *
 * @param v     The vector to normalize.
 * @param dst   The destination vector.
 *
 * @return A reference to dst for chaining
 */
Vec2* Vec2::normalize(const Vec2& v, Vec2* dst) {
    float n = v.x * v.x + v.y * v.y;
    // Already normalized.
    if (n == 1.0f) {
        *dst = v;
        return dst;
    }
    
    n = sqrt(n);
    
    // Do nothing if too close to zero.
    if (n >= CU_MATH_FLOAT_SMALL) {
        n = 1.0f / n;
        dst->x = v.x*n;
        dst->y = v.y*n;
    }
    
    return dst;
}

/**
 * Computes the midpoint between two points and stores it in dst.
 *
 * @param v1    The first point.
 * @param v2    The second point.
 * @param dst   The destination vector.
 *
 * @return A reference to dst for chaining
 */
Vec2* Vec2::midpoint(const Vec2& v1, const Vec2& v2, Vec2* dst) {
    CUAssertLog(dst, "Assignment vector is null");
    dst->x = (v1.x + v2.x) / 2.0f;
    dst->y = (v1.y + v2.y) / 2.0f;
    return dst;
}

/**
 * Computes the projection of one vector on to another and stores it in dst.
 *
 * @param v1    The original vector.
 * @param v2    The vector to project on.
 * @param dst   The destination vector.
 *
 * @return A reference to dst for chaining
 */
Vec2* Vec2::project(const Vec2& v1, const Vec2& v2, Vec2* dst) {
    CUAssertLog(dst, "Assignment vector is null");
    *dst = v2 * (v1.dot(v2)/v2.dot(v2));
    return dst;
}

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
Vec2* Vec2::lerp(const Vec2& v1, const Vec2& v2, float alpha, Vec2* dst) {
    CUAssertLog(dst, "Assignment vector is null");
    *dst = v1 * (1.f - alpha) + v2 * alpha;
    return dst;
}

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
bool Vec2::doesLineIntersect(const Vec2& A, const Vec2& B,
                             const Vec2& C, const Vec2& D,
                             float *S, float *T) {
    // FAIL: Line undefined
    if ( (A.x==B.x && A.y==B.y) || (C.x==D.x && C.y==D.y) ) {
        return false;
    }
    
    const float denom = (B-A).cross(D-C);
    if (denom == 0) {
        // Lines parallel or overlap
        return false;
    }
    
    if (S != nullptr) *S = (D-C).cross(A-C) / denom;
    if (T != nullptr) *T = (B-A).cross(A-C) / denom;
    
    return true;
}

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
bool Vec2::doesLineOverlap(const Vec2& A, const Vec2& B,
                           const Vec2& C, const Vec2& D) {
    
    // FAIL: Line undefined
    if ( (A.x==B.x && A.y==B.y) || (C.x==D.x && C.y==D.y) ) {
        return false;
    }
    
    return ((B-A).cross(D-C) == 0 && ((D-C).cross(A-C) == 0 || (B-A).cross(A-C) == 0));
}
    

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
bool Vec2::isLineParallel(const Vec2& A, const Vec2& B,
                          const Vec2& C, const Vec2& D) {
    // FAIL: Line undefined
    if ( (A.x==B.x && A.y==B.y) || (C.x==D.x && C.y==D.y) ) {
        return false;
    }
    
    if ((B-A).cross(D-C) == 0) {
        // Check for line overlap
        return !((D-C).cross(A-C) == 0 || (B-A).cross(A-C) == 0);
    }
    
    return false;
}

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
bool Vec2::doesSegmentIntersect(const Vec2& A, const Vec2& B, const Vec2& C, const Vec2& D) {
    float S, T;
    return (doesLineIntersect(A, B, C, D, &S, &T ) &&
            (S >= 0.0f && S <= 1.0f && T >= 0.0f && T <= 1.0f));
}

/** 
 * Returns true if the lines AB and CD overlap.
 *
 * The lines are all one-dimensional on the number line.  SE is the overlap
 * part with S the initial parameter and E the terminal parameter.  If the
 * lines do not overlap, S and E are left unchanged.
 */
bool isOneDimensionSegmentOverlap(float A, float B, float C, float D, float *S, float * E) {
    float ABmin = std::min(A, B);
    float ABmax = std::max(A, B);
    float CDmin = std::min(C, D);
    float CDmax = std::max(C, D);
    
    if (ABmax < CDmin || CDmax < ABmin) {
        // ABmin->ABmax->CDmin->CDmax or CDmin->CDmax->ABmin->ABmax
        return false;
    } else {
        if (ABmin >= CDmin && ABmin <= CDmax) {
            // CDmin->ABmin->CDmax->ABmax or CDmin->ABmin->ABmax->CDmax
            if (S != nullptr) *S = ABmin;
            if (E != nullptr) *E = CDmax < ABmax ? CDmax : ABmax;
        } else if (ABmax >= CDmin && ABmax <= CDmax) {
            // ABmin->CDmin->ABmax->CDmax
            if (S != nullptr) *S = CDmin;
            if (E != nullptr) *E = ABmax;
        } else {
            // ABmin->CDmin->CDmax->ABmax
            if (S != nullptr) *S = CDmin;
            if (E != nullptr) *E = CDmax;
        }
        return true;
    }
}

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
bool Vec2::doesSegmentOverlap(const Vec2& A, const Vec2& B,
                              const Vec2& C, const Vec2& D,
                              Vec2* S, Vec2* E) {
    if (doesLineOverlap(A, B, C, D)) {
        return isOneDimensionSegmentOverlap(A.x, B.x, C.x, D.x, &S->x, &E->x) &&
               isOneDimensionSegmentOverlap(A.y, B.y, C.y, D.y, &S->y, &E->y);
    }
    
    return false;
}

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
Vec2 Vec2::getIntersection(const Vec2& A, const Vec2& B, const Vec2& C, const Vec2& D) {
    float S, T;
    
    if (doesLineIntersect(A, B, C, D, &S, &T)) {
        // Vec2 of intersection
        Vec2 P;
        P.x = A.x + S * (B.x - A.x);
        P.y = A.y + S * (B.y - A.y);
        return P;
    }
    
    return Vec2::ZERO;
}


#pragma mark -
#pragma mark Conversion Methods
/**
 * Returns a string representation of this vector for debuggging purposes.
 *
 * If verbose is true, the string will include class information.  This
 * allows us to unambiguously identify the class.
 *
 * @param verbose Whether to include class information
 *
 * @return a string representation of this vector for debuggging purposes.
 */
std::string Vec2::toString(bool verbose) const {
    std::stringstream ss;
    ss << (verbose ? "cugl::Vec2(" : "(");
    ss <<  to_string(x);
    ss << ",";
    ss << to_string(y);
    ss << ")";
    return ss.str();
}


/** Cast from Vec2 to Size. */
Vec2::operator Size() const {
    return Size(x,y);
}

/**
 * Creates a vector from the given size.
 *
 * The width is converted to the x coordinate and height to y.
 *
 * @param size The size to convert
 */
Vec2::Vec2(const Size& size) {
    x = size.width; y = size.height;
}

/**
 * Sets the coordinates of this vector to those of the given size.
 *
 * The width is converted to the x coordinate and height to y.
 *
 * @param size The size to convert.
 *
 * @return A reference to this (modified) Vec2 for chaining.
 */
Vec2& Vec2::operator= (const Size& size) {
    x = size.width; y = size.height;
    return *this;
}

/**
 * Adds the given size to this vector in place.
 *
 * @param right The Size to add
 *
 * @return A reference to this (modified) Vec2 for chaining.
 */
Vec2& Vec2::operator+=(const Size& right) {
    x += right.width; y += right.height;
    return *this;
}

/**
 * Subtracts the given size from this vector in place.
 *
 * @param right The Size to subtract
 *
 * @return A reference to this (modified) Vec2 for chaining.
 */
Vec2& Vec2::operator-=(const Size& right) {
    x -= right.width; y -= right.height;
    return *this;
}

/**
 * Casts from Vec2 to Vec3.
 *
 * The z-value is set to 0.
 */
Vec2::operator Vec3() const {
    return Vec3(x,y,0);
}

/**
 * Creates a 2d vector from the given 3d one.
 *
 * The z-value is dropped.
 *
 * @param v The vector to convert
 */
Vec2::Vec2(const Vec3& v) {
    x = v.x; y = v.y;
}

/**
 * Sets the coordinates of this vector to those of the given 3d vector.
 *
 * The z-value is dropped.
 *
 * @param v The vector to convert
 *
 * @return A reference to this (modified) Vec2 for chaining.
 */
Vec2& Vec2::operator= (const Vec3& v) {
    x = v.x; y = v.y;
    return *this;
}

/**
 * Casts from Vec2 to Vec4.
 *
 * The z-value is set to 0, but the w-value is set to 1.  That is
 * because the standard usage of Vec4 objects is homogenous coords.
 */
Vec2::operator Vec4() const {
    return Vec4(x,y,0,1);
}

/**
 * Creates a 2d vector from the given 4d one.
 *
 * All coordinates are divided by the w-coordinate (assuming it is not
 * zero) before this conversion.
 *
 * @param v The vector to convert
 */
Vec2::Vec2(const Vec4& v) {
    float d = v.w == 0 ? 1 : 1/v.w;
    x = v.x*d; y = v.y*d;
}

/**
 * Sets the coordinates of this vector to those of the given 4d vector.
 *
 * All coordinates are divided by the w-coordinate (assuming it is not
 * zero) before this conversion. Afterwards, both z and w are dropped.
 *
 * @param v The vector to convert
 *
 * @return A reference to this (modified) Vec2 for chaining.
 */
Vec2& Vec2::operator= (const Vec4& v) {
    float d = v.w == 0 ? 1 : 1/v.w;
    x = v.x*d; y = v.y*d;
    return *this;
}


#pragma mark -
#pragma mark Constants

/** The zero vector Vec2(0,0) */
const Vec2 Vec2::ZERO(0.0f, 0.0f);
/** The unit vector Vec2(1,1) */
const Vec2 Vec2::ONE(1.0f, 1.0f);
/** The x-axis Vec2(1,0) */
const Vec2 Vec2::UNIT_X(1.0f, 0.0f);
/** The y-axis Vec2(0,1) */
const Vec2 Vec2::UNIT_Y(0.0f, 1.0f);

/** The relative anchor Vec2(0.5, 0.5) in the unit square */
const Vec2 Vec2::ANCHOR_CENTER(0.5f, 0.5f);
/** The relative anchor Vec2(0, 0) in the unit square */
const Vec2 Vec2::ANCHOR_BOTTOM_LEFT(0.0f, 0.0f);
/** The relative anchor Vec2(0, 1) in the unit square */
const Vec2 Vec2::ANCHOR_TOP_LEFT(0.0f, 1.0f);
/** The relative anchor Vec2(1, 0) in the unit square */
const Vec2 Vec2::ANCHOR_BOTTOM_RIGHT(1.0f, 0.0f);
/** The relative anchor Vec2(1, 1) in the unit square */
const Vec2 Vec2::ANCHOR_TOP_RIGHT(1.0f, 1.0f);
/** The relative anchor Vec2(1, 0.5) in the unit square */
const Vec2 Vec2::ANCHOR_MIDDLE_RIGHT(1.0f, 0.5f);
/** The relative anchor Vec2(0, 0.5) in the unit square */
const Vec2 Vec2::ANCHOR_MIDDLE_LEFT(0.0f, 0.5f);
/** The relative anchor Vec2(0.5, 1) in the unit square */
const Vec2 Vec2::ANCHOR_TOP_CENTER(0.5f, 1.0f);
/** The relative anchor Vec2(0.5, 0) in the unit square */
const Vec2 Vec2::ANCHOR_BOTTOM_CENTER(0.5f, 0.0f);

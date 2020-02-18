//
//  CUAffine2.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a 2d affine transform.  It has some of the
//  functionality of Mat4, with a lot less memory footprint.  Profiling suggests
//  that this class is 20% faster than Mat4 when only 2d functionality is needed.
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
//  Version: 6/12/16

#include <cugl/math/CUAffine2.h>

#include <sstream>
#include <algorithm>
#include <SDL/SDL.h>
#include <cugl/util/CUDebug.h>
#include <cugl/math/CUAffine2.h>
#include <cugl/util/CUStrings.h>
#include <cugl/math/CUMat4.h>

using namespace cugl;

#define MATRIX_SIZE ( sizeof(float) * 4 )

#pragma mark -
#pragma mark Constructors
/**
 * Creates the identity transform.
 *
 *     1  0
 *     0  1 + (0,0)
 */
Affine2::Affine2() {
    m[0] = m[3] = 1;
    m[1] = m[2] = 0;
}

/**
 * Constructs a matrix initialized to the specified values.
 *
 * @param m11   The first element of the first row.
 * @param m12   The second element of the first row.
 * @param m21   The first element of the second row.
 * @param m22   The second element of the second row.
 * @param tx    The translation offset for the x-coordinate.
 * @param ty    The translation offset for the y-coordinate.
 */
Affine2::Affine2(float m11, float m12, float m21, float m22, float tx, float ty) {
    set(m11,m12,m21,m22,tx,ty);
}


/**
 * Creates a matrix initialized to the specified column-major array.
 *
 * The passed-in array is in column-major order, with the last two elements
 * being the translation offset.  Hence the memory layout of the array is
 * as follows:
 *
 *     0   2
 *     1   3 + (4,5)
 *
 * @param mat An array containing 6 elements in column-major order.
 */
Affine2::Affine2(const float* mat) {
    CUAssertLog(mat, "Source array is null");
    memcpy(this->m, mat, MATRIX_SIZE);
    offset.set(&mat[4]);
}

/**
 * Constructs a new transform that is the copy of the specified one.
 *
 * @param copy The transform to copy.
 */
Affine2::Affine2(const Affine2& copy) {
    memcpy(this->m, copy.m, MATRIX_SIZE);
    offset = copy.offset;
}

/**
 * Constructs a new transform that contains the resources of the specified one.
 *
 * @param copy The transform contributing resources.
 */
Affine2::Affine2(Affine2&& copy) {
    memcpy(this->m, copy.m, MATRIX_SIZE);
    offset = copy.offset;
}

#pragma mark -
#pragma mark Static Constructors
/**
 * Creates a uniform scale transform.
 *
 * @param scale The amount to scale.
 * @param dst   A transform to store the result in.
 *
 * @return A reference to dst for chaining
 */
Affine2* Affine2::createScale(float scale, Affine2* dst) {
    CUAssertLog(dst,"Assignment transform is null");
    
    dst->m[0] = dst->m[3] = scale;
    dst->m[1] = dst->m[2] = 0.0f;
    dst->offset.setZero();
    return dst;
}

/**
 * Creates a nonuniform scale transform.
 *
 * @param sx    The amount to scale along the x-axis.
 * @param sy    The amount to scale along the y-axis.
 * @param dst   A transform to store the result in.
 *
 * @return A reference to dst for chaining
 */
Affine2* Affine2::createScale(float sx, float sy, Affine2* dst) {
    CUAssertLog(dst,"Assignment transform is null");
    
    dst->m[0] = sx;
    dst->m[3] = sy;
    dst->m[1] = dst->m[2] = 0.0f;
    dst->offset.setZero();
    return dst;
}

/**
 * Creates a nonuniform scale transform from the given vector.
 *
 * @param scale The nonuniform scale value.
 * @param dst   A transform to store the result in.
 *
 * @return A reference to dst for chaining
 */
Affine2* Affine2::createScale(const Vec2& scale, Affine2* dst) {
    CUAssertLog(dst,"Assignment transform is null");
    
    dst->m[0] = scale.x;
    dst->m[3] = scale.y;
    dst->m[1] = dst->m[2] = 0.0f;
    dst->offset.setZero();
    return dst;
}

/**
 * Creates a rotation transform for the given angle
 *
 * The angle measurement is in radians.  The rotation is counter
 * clockwise about the z-axis.
 *
 * @param angle The angle (in radians).
 * @param dst   A transform to store the result in.
 *
 * @return A reference to dst for chaining
 */
Affine2* Affine2::createRotation(float angle, Affine2* dst) {
    CUAssertLog(dst,"Assignment transform is null");
    
    float c = cos(angle);
    float s = sin(angle);

    dst->m[0] = c;
    dst->m[1] = s;
    dst->m[2] = -s;
    dst->m[3] = c;
    dst->offset.setZero();
    return dst;
}

/**
 * Creates a translation transform from the given offset.
 *
 * @param trans The translation offset.
 * @param dst   A transform to store the result in.
 *
 * @return A reference to dst for chaining
 */
Affine2* Affine2::createTranslation(const Vec2& trans, Affine2* dst) {
    CUAssertLog(dst,"Assignment transform is null");
    
    dst->m[0] = dst->m[3] = 1.0f;
    dst->m[1] = dst->m[2] = 0.0f;
    dst->offset.set(trans);
    return dst;
}

/**
 * Creates a translation transform from the given parameters.
 *
 * @param tx    The translation on the x-axis.
 * @param ty    The translation on the y-axis.
 * @param dst   A transform to store the result in.
 *
 * @return A reference to dst for chaining
 */
Affine2* Affine2::createTranslation(float tx, float ty, Affine2* dst) {
    CUAssertLog(dst,"Assignment transform is null");
    
    dst->m[0] = dst->m[3] = 1.0f;
    dst->m[1] = dst->m[2] = 0.0f;
    dst->offset.set(tx,ty);
    return dst;
}

#pragma mark -
#pragma mark Setters
/**
 * Sets the individal values of this transform.
 *
 * @param m11   The first element of the first row.
 * @param m12   The second element of the first row.
 * @param m21   The first element of the second row.
 * @param m22   The second element of the second row.
 * @param tx    The translation offset for the x-coordinate.
 * @param ty    The translation offset for the y-coordinate.
 *
 * @return A reference to this (modified) Affine2 for chaining.
 */
Affine2& Affine2::set(float m11, float m12, float m21, float m22, float tx, float ty) {
    m[0] = m11;
    m[2] = m12;
    m[1] = m21;
    m[3] = m22;
    offset.set(tx,ty);
    return *this;
}

/**
 * Sets the values of this transform to those in the specified column-major array.
 *
 * The passed-in array is in column-major order, with the last two elements
 * being the translation offset.  Hence the memory layout of the array is
 * as follows:
 *
 *     0   2
 *     1   3 + (4,5)
 *
 * @param mat An array containing 6 elements in column-major order.
 *
 * @return A reference to this (modified) Affine2 for chaining.
 */
Affine2& Affine2::set(const float* mat) {
    CUAssertLog(mat, "Source array is null");
    memcpy(this->m, mat, MATRIX_SIZE);
    offset.set(&mat[4]);
    return *this;
}

/**
 * Sets the elements of this transform to those in the specified transform.
 *
 * @param mat The transform to copy.
 *
 * @return A reference to this (modified) Affine2 for chaining.
 */
Affine2& Affine2::set(const Affine2& mat) {
    memcpy(this->m, mat.m, MATRIX_SIZE);
    offset = mat.offset;
    return *this;
}

/**
 * Sets this transform to the identity transform.
 *
 * @return A reference to this (modified) Affine2 for chaining.
 */
Affine2& Affine2::setIdentity() {
    m[0] = m[3] = 1.0f;
    m[1] = m[2] = 0.0f;
    offset.setZero();
    return *this;
}

/**
 * Sets all elements of the current transform to zero.
 *
 * @return A reference to this (modified) Affine2 for chaining.
 */
Affine2& Affine2::setZero() {
    memset(m, 0, MATRIX_SIZE);
    offset.setZero();
    return *this;
}

#pragma mark -
#pragma mark Static Arithmetic
/**
 * Adds the specified offset to the given and stores the result in dst.
 *
 * Addition is applied to the offset only; the core matrix is unchanged.
 *
 * @param m     The initial transform.
 * @param v     The offset to add.
 * @param dst   A transform to store the result in.
 *
 * @return A reference to dst for chaining
 */
Affine2* Affine2::add(const Affine2& m, const Vec2& v, Affine2* dst) {
    dst->set(m);
    dst->offset.add(v);
    return dst;
}

/**
 * Subtracts the offset v from m and stores the result in dst.
 *
 * Subtraction is applied to the offset only; the core matrix is unchanged.
 *
 * @param m     The initial transform.
 * @param v     The offset to subtract.
 * @param dst   A transform to store the result in.
 *
 * @return A reference to dst for chaining
 */
Affine2* Affine2::subtract(const Affine2& m, const Vec2& v, Affine2* dst) {
    dst->set(m);
    dst->offset.subtract(v);
    return dst;
}

/**
 * Multiplies the specified transform by a scalar and stores the result in dst.
 *
 * The scalar is applied to BOTH the core matrix and the offset.
 *
 * @param mat       The transform.
 * @param scalar    The scalar value.
 * @param dst       A transform to store the result in.
 *
 * @return A reference to dst for chaining
 */
Affine2* Affine2::multiply(const Affine2& mat, float scalar, Affine2* dst) {
    dst->m[0] = mat.m[0]*scalar; dst->m[1] = mat.m[1]*scalar;
    dst->m[2] = mat.m[2]*scalar; dst->m[3] = mat.m[3]*scalar;
    dst->offset = mat.offset*scalar;
    return dst;
}

/**
 * Multiplies m1 by the transform m2 and stores the result in dst.
 *
 * Transform multiplication is defined as standard function composition.
 * The transform m2 is on the right.  This means that it corresponds to
 * an subsequent transform; transforms are applied left-to-right.
 *
 * @param m1    The first transform to multiply.
 * @param m2    The second transform to multiply.
 * @param dst   A matrix to store the result in.
 *
 * @return A reference to dst for chaining
 */
Affine2* Affine2::multiply(const Affine2& m1, const Affine2& m2, Affine2* dst) {
    // Need to be prepared for them to be the same
    float a = m2.m[0] * m1.m[0] + m2.m[2] * m1.m[1];
    float b = m2.m[0] * m1.m[2] + m2.m[2] * m1.m[3];
    float c = m2.m[1] * m1.m[0] + m2.m[3] * m1.m[1];
    float d = m2.m[1] * m1.m[2] + m2.m[3] * m1.m[3];
    float tx = m1.offset.x * m2.m[0] + m1.offset.y * m2.m[1] + m2.offset.x;
    float ty = m1.offset.x * m2.m[2] + m1.offset.y * m2.m[3] + m2.offset.y;
    dst->m[0] = a;
    dst->m[1] = c;
    dst->m[2] = b;
    dst->m[3] = d;
    dst->offset.set(tx,ty);
    return dst;
}

/**
 * Inverts m1 and stores the result in dst.
 *
 * If the transform cannot be inverted, this method stores the zero transform
 * in dst.
 *
 * @param m1    The transform to negate.
 * @param dst   A transform to store the result in.
 *
 * @return A reference to dst for chaining
 */
Affine2* Affine2::invert(const Affine2& m1, Affine2* dst) {
    float det = m1.getDeterminant();
    if (det == 0.0f) {
        dst->setZero();
        return dst;
    }
    // Need to be prepared for transforms to be the same.
    det = 1.0f/det;
    float m11 = m1.m[3]*det;
    float m12 = -m1.m[2]*det;
    float m21 = -m1.m[1]*det;
    float m22 = m1.m[0]*det;
    dst->m[0] = m11;
    dst->m[1] = m21;
    dst->m[2] = m12;
    dst->m[3] = m22;
    dst->offset = m1.offset.getNegation();
    return dst;
}


#pragma mark -
#pragma mark Comparisons
/**
 * Returns true if the transforms are exactly equal to each other.
 *
 * This method may be unreliable given that the elements are floats.
 * It should only be used to compared transform that have not undergone
 * a lot of manipulation.
 *
 * @param aff   The transform to compare against.
 *
 * @return true if the transforms are exactly equal to each other.
 */
bool Affine2::isExactly(const Affine2& aff) const {
    return memcmp(&m[0],&(aff.m[0]),MATRIX_SIZE) == 0 && offset == aff.offset;
}

/**
 * Returns true if the transforms are within tolerance of each other.
 *
 * The tolerance is applied to each element of the transform individually.
 *
 * @param aff       The transform to compare against.
 * @param variance  The comparison tolerance.
 *
 * @return true if the transforms are within tolerance of each other.
 */
bool Affine2::equals(const Affine2& aff, float variance) const {
    bool similar = true;
    for(int ii = 0; similar && ii < 4; ii++) {
        similar = (fabsf(m[ii]-aff.m[ii]) <= variance);
    }
    bool tmp = similar && offset.equals(aff.offset,variance);
    return tmp;
}


#pragma mark -
#pragma mark Affine Attributes
/**
 * Returns true if this transform is equal to the identity transform.
 *
 * The optional comparison tolerance takes into accout that elements
 * are floats and this may not be exact.  The tolerance is applied to
 * each element individually.  By default, the match must be exact.
 *
 * @param variance The comparison tolerance
 *
 * @return true if this transform is equal to the identity transform.
 */
bool Affine2::isIdentity(float variance) const {
    return equals(IDENTITY,variance);
}

/**
 * Decomposes the scale, rotation and translation components of the given matrix.
 *
 * To work properly, the matrix must have been constructed in the following
 * order: scale, then rotate, then translation.  While the rotation matrix
 * will always be correct, the scale and translation are not guaranteed
 * to be correct.
 *
 * If any pointer is null, the method simply does not assign that result.
 * However, it will still continue to compute the component with non-null
 * vectors to store the result.
 *
 * If the scale component is too small, then it may be impossible to
 * extract the rotation. In that case, if the rotation pointer is not
 * null, this method will return false.
 *
 * @param mat   The transform to decompose.
 * @param scale The scale component.
 * @param rot   The rotation component.
 * @param trans The translation component.
 *
 * @return true if all requested components were properly extracted
 */
bool Affine2::decompose(const Affine2& mat, Vec2* scale, float* rot, Vec2* trans) {
    if (trans != nullptr) {
        // Extract the translation.
        trans->set(mat.offset);
    }
    
    // Nothing left to do.
    if (scale == nullptr && rot == nullptr) {
        return true;
    }
    
    // Extract the scale.
    // This is simply the length of each axis (row/column) in the matrix.
    Vec2 xaxis(mat.m[0], mat.m[1]);
    float scaleX = xaxis.length();
    
    Vec2 yaxis(mat.m[2], mat.m[3]);
    float scaleY = yaxis.length();
    
    // Determine if we have a negative scale (true if determinant is less than zero).
    // In this case, we simply negate a single axis of the scale.
    float det = mat.getDeterminant();
    if (det < 0) {
        scaleY = -scaleY;
    }
    
    if (scale != nullptr) {
        scale->set(scaleX,scaleY);
    }
    
    // Nothing left to do.
    if (rot == nullptr) {
        return true;
    }
    
    // Scale too close to zero, can't decompose rotation.
    if (scaleX < CU_MATH_EPSILON || fabsf(scaleY) < CU_MATH_EPSILON) {
        return false;
    }
    
    float rn;
    
    // Factor the scale out of the matrix axes.
    rn = 1.0f / scaleX;
    xaxis.x *= rn;
    xaxis.y *= rn;

    rn = 1.0f / scaleY;
    yaxis.x *= rn;
    yaxis.y *= rn;

    *rot = atan2f(xaxis.y,yaxis.y);
    return true;
}

#pragma mark -
#pragma mark Vector Operations

/**
 * Transforms the point and stores the result in dst.
 *
 * @param aff   The affine transform.
 * @param point The point to transform.
 * @param dst   A vector to store the transformed point in.
 *
 * @return A reference to dst for chaining
 */
Vec2* Affine2::transform(const Affine2& aff, const Vec2& point, Vec2* dst) {
    float x = aff.m[0]*point.x+aff.m[1]*point.y+aff.offset.x;
    float y = aff.m[2]*point.x+aff.m[3]*point.y+aff.offset.y;
    dst->set(x,y);
    return dst;
}

/**
 * Transforms the rectangle and stores the result in dst.
 *
 * This method transforms the four defining points of the rectangle.  It
 * then computes the minimal bounding box storing these four points
 *
 * @param aff   The affine transform.
 * @param rect  The rect to transform.
 * @param dst   A rect to store the transformed rectangle in.
 *
 * @return A reference to dst for chaining
 */
cugl::Rect* Affine2::transform(const Affine2& aff, const Rect& rect, Rect* dst) {
    Vec2 point1(rect.getMinX(),rect.getMinY());
    Vec2 point2(rect.getMinX(),rect.getMaxY());
    Vec2 point3(rect.getMaxX(),rect.getMinY());
    Vec2 point4(rect.getMaxX(),rect.getMaxY());
    Affine2::transform(aff,point1,&point1);
    Affine2::transform(aff,point2,&point2);
    Affine2::transform(aff,point3,&point3);
    Affine2::transform(aff,point4,&point4);
    float minx = std::min(point1.x,std::min(point2.x,std::min(point3.x,point4.x)));
    float maxx = std::max(point1.x,std::max(point2.x,std::max(point3.x,point4.x)));
    float miny = std::min(point1.y,std::min(point2.y,std::min(point3.y,point4.y)));
    float maxy = std::max(point1.y,std::max(point2.y,std::max(point3.y,point4.y)));
    dst->origin.set(minx,miny);
    dst->size.set(maxx-minx,maxy-miny);
    return dst;
}

/**
 * Returns a copy of the given rectangle transformed.
 *
 * This method transforms the four defining points of the rectangle.  It
 * then computes the minimal bounding box storing these four points
 *
 * Note: This does not modify the original rectangle. To transform a
 * point in place, use the static method.
 *
 * @param rect  The rect to transform.
 *
 * @return A reference to dst for chaining
 */
cugl::Rect Affine2::transform(const Rect& rect) const {
    Rect result;
    return *(transform(*this,rect,&result));
}

#pragma mark -
#pragma mark Conversion Methods

/**
 * Returns a string representation of this transform for debugging purposes.
 *
 * If verbose is true, the string will include class information.  This
 * allows us to unambiguously identify the class.
 *
 * @param verbose Whether to include class information
 *
 * @return a string representation of this transform for debugging purposes.
 */
std::string Affine2::toString(bool verbose) const {
    std::stringstream ss;
    if (verbose) {
        ss << "cugl::Affine2";
    }
    const int PRECISION = 8;
    ss << "\n";
    ss << "|  ";
    ss <<  cugl::to_string(m[0]).substr(0,PRECISION);
    ss << ", ";
    ss <<  cugl::to_string(m[2]).substr(0,PRECISION);
    ss << "  | ";
    ss << "\n";
    ss << "|  ";
    ss <<  cugl::to_string(m[1]).substr(0,PRECISION);
    ss << ", ";
    ss <<  cugl::to_string(m[3]).substr(0,PRECISION);
    ss << "  | + ";
    ss << offset.toString(false);
    return ss.str();
}

/** Cast from Affine2 to a Mat4. */
Affine2::operator Mat4() const {
    Mat4 result(m[0],m[2],0,offset.x, m[1],m[3],0,offset.y, 0,0,1,0, 0,0,0,1);
    return result;
}

/**
 * Creates an affine transform from the given matrix.
 *
 * The z values are all uniformly ignored.  However, it the final element
 * of the matrix is not 1 (e.g. the translation has a w value of 1), then
 * it divides the entire matrix before creating the affine transform
 *
 * @param mat The matrix to convert
 */
Affine2::Affine2(const Mat4& mat) {
    float v = 1.0f;
    if (mat.m[15] != 1.0f && fabsf(mat.m[15]) > CU_MATH_EPSILON) {
        v = 1.0f/mat.m[15];
    }
    m[0] = mat.m[0]*v;
    m[1] = mat.m[1]*v;
    m[2] = mat.m[4]*v;
    m[3] = mat.m[5]*v;
    offset.x = mat.m[12]*v;
    offset.y = mat.m[13]*v;
}

/**
 * Sets the elements of this transform to those of the given matrix.
 *
 * The z values are all uniformly ignored.  However, it the final element
 * of the matrix is not 1 (e.g. the translation has a w value of 1), then
 * it divides the entire matrix before creating the affine transform
 *
 * @param mat The matrix to convert
 *
 * @return A reference to this (modified) Affine2 for chaining.
 */
Affine2& Affine2::operator= (const Mat4& mat) {
    return set(mat);
}

/**
 * Sets the elements of this transform to those of the given matrix.
 *
 * The z values are all uniformly ignored.  However, it the final element
 * of the matrix is not 1 (e.g. the translation has a w value of 1), then
 * it divides the entire matrix before creating the affine transform
 *
 * @param mat The matrix to convert
 *
 * @return A reference to this (modified) Affine2 for chaining.
 */
Affine2& Affine2::set(const Mat4& mat) {
    float v = 1.0f;
    if (mat.m[15] != 1.0f && fabsf(mat.m[15]) > CU_MATH_EPSILON) {
        v = 1.0f/mat.m[15];
    }
    m[0] = mat.m[0]*v;
    m[1] = mat.m[1]*v;
    m[2] = mat.m[4]*v;
    m[3] = mat.m[5]*v;
    offset.x = mat.m[12]*v;
    offset.y = mat.m[13]*v;
    return *this;
}

#pragma mark -
#pragma mark Constants

/** The identity transform (ones on the diagonal) */
const Affine2 Affine2::IDENTITY(1.0f,0.0f,0.0f,1.0f,0.0f,0.0f);

/** The transform with all zeroes */
const Affine2 Affine2::ZERO(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

/** The transform with all ones */
const Affine2 Affine2::ONE(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

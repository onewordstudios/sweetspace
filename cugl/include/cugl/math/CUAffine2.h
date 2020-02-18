//
//  CUAffine2.h
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
#ifndef __CU_AFFINE2_H__
#define __CU_AFFINE2_H__

#include <math.h>
#include <assert.h>
#include "CUMathBase.h"
#include "CUVec2.h"
#include "CURect.h"

namespace cugl {

// Forward reference
class Mat4;

/**
 * This class defines an affine transform on 2D space.
 *
 * In the case where you are only manipulating 2D points, this class may be
 * faster than Mat4, even with the vectorization support.  For an affine 
 * transform in 3d space, use Mat4.
 *
 * The affine transform is stored as a 4-element array (containing the core
 * matrix, plus a Vec2 offset.  Keeping with the convention of Mat4, the core
 * matrix is stored in column-order format.  In addition, we assume that all
 * operations are multiplied on the right.
 *
 */
class Affine2 {
#pragma mark Values
public:
    /** The core matrix */
    float m[4];
    /** The translation offset */
    Vec2 offset;
    
    /** The transform with all zeroes */
    static const Affine2 ZERO;
    /** The transform with all ones */
    static const Affine2 ONE;
    /** The identity transform (ones on the diagonal) */
    static const Affine2 IDENTITY;
    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates the identity transform.
     *
     *     1  0
     *     0  1 + (0,0)
     */
    Affine2();
    
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
    Affine2(float m11, float m12, float m21, float m22, float tx, float ty);
    
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
    Affine2(const float* mat);
    
    /**
     * Constructs a new transform that is the copy of the specified one.
     *
     * @param copy The transform to copy.
     */
    Affine2(const Affine2& copy);
    
    /**
     * Constructs a new transform that contains the resources of the specified one.
     *
     * @param copy The transform contributing resources.
     */
    Affine2(Affine2&& copy);
    
    /**
     * Destroys this transform, releasing all resources.
     */
    ~Affine2() {}
    
    
#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a uniform scale transform.
     *
     * @param scale The amount to scale.
     *
     * @return a uniform scale transform.
     */
    static Affine2 createScale(float scale) {
        Affine2 result;
        return *(createScale(scale,&result));
    }

    /**
     * Creates a uniform scale transform, putting it in dst.
     *
     * @param scale The amount to scale.
     * @param dst   A transform to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Affine2* createScale(float scale, Affine2* dst);
    
    /**
     * Returns a nonuniform scale transform.
     *
     * @param sx    The amount to scale along the x-axis.
     * @param sy    The amount to scale along the y-axis.
     *
     * @return a nonuniform scale transform.
     */
    static Affine2 createScale(float sx, float sy) {
        Affine2 result;
        return *(createScale(sx,sy,&result));
    }

    /**
     * Creates a nonuniform scale transform, putting it in dst.
     *
     * @param sx    The amount to scale along the x-axis.
     * @param sy    The amount to scale along the y-axis.
     * @param dst   A transform to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Affine2* createScale(float sx, float sy, Affine2* dst);
    
    /**
     * Returns a nonuniform scale transform from the given vector.
     *
     * @param scale The nonuniform scale value.
     *
     * @return a nonuniform scale transform from the given vector.
     */
    static Affine2 createScale(const Vec2& scale) {
        Affine2 result;
        return *(createScale(scale,&result));
    }

    /**
     * Creates a nonuniform scale transform from the given vector, putting it in dst.
     *
     * @param scale The nonuniform scale value.
     * @param dst   A transform to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Affine2* createScale(const Vec2& scale, Affine2* dst);
    
    /**
     * Returns a rotation transform for the given angle.
     *
     * The angle measurement is in radians.  The rotation is counter
     * clockwise about the z-axis.
     *
     * @param angle The angle (in radians).
     *
     * @return a rotation transform for the given angle.
     */
    static Affine2 createRotation(float angle) {
        Affine2 result;
        return *(createRotation(angle,&result));
    }

    /**
     * Creates a rotation transform for the given angle, putting it in dst.
     *
     * The angle measurement is in radians.  The rotation is counter
     * clockwise about the z-axis.
     *
     * @param angle The angle (in radians).
     * @param dst   A transform to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Affine2* createRotation(float angle, Affine2* dst);
    
    /**
     * Returns a translation transform from the given offset
     *
     * @param trans The translation offset.
     *
     * @return a translation transform from the given offset
     */
    static Affine2 createTranslation(const Vec2& trans) {
        Affine2 result;
        return *(createTranslation(trans,&result));
    }
    
    /**
     * Creates a translation transform from the given offset, putting it in dst.
     *
     * @param trans The translation offset.
     * @param dst   A transform to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Affine2* createTranslation(const Vec2& trans, Affine2* dst);
    
    /**
     * Returns a translation transform from the given parameters.
     *
     * @param tx    The translation on the x-axis.
     * @param ty    The translation on the y-axis.
     *
     * @return a translation transform from the given parameters.
     */
    static Affine2 createTranslation(float tx, float ty) {
        Affine2 result;
        return *(createTranslation(tx,ty,&result));
    }

    /**
     * Creates a translation transform from the given parameters, putting it in dst.
     *
     * @param tx    The translation on the x-axis.
     * @param ty    The translation on the y-axis.
     * @param dst   A transform to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Affine2* createTranslation(float tx, float ty, Affine2* dst);
    
    
#pragma mark -
#pragma mark Setters
    /**
     * Sets the elements of this transform to those in the specified transform.
     *
     * @param mat The transform to copy.
     *
     * @return A reference to this (modified) Affine2 for chaining.
     */
    Affine2& operator=(const Affine2& mat) {
        return set(mat);
    }
    
    /**
     * Sets the elements of this matrix to those in the specified one.
     *
     * @param mat The matrix to take resources from.
     *
     * @return A reference to this (modified) Mat4 for chaining.
     */
    Affine2& operator=(Affine2&& mat) {
        memcpy(this->m, mat.m, sizeof(float)*4);
        this->offset = mat.offset;
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
     * @param array An array containing 6 elements in column-major order.
     *
     * @return A reference to this (modified) Affine2 for chaining.
     */
    Affine2& operator=(const float* array) {
        return set(array);
    }
    
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
    Affine2& set(float m11, float m12, float m21, float m22, float tx, float ty);
    
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
    Affine2& set(const float* mat);
    
    /**
     * Sets the elements of this transform to those in the specified transform.
     *
     * @param mat The transform to copy.
     *
     * @return A reference to this (modified) Affine2 for chaining.
     */
    Affine2& set(const Affine2& mat);
    
    /**
     * Sets this transform to the identity transform.
     *
     * @return A reference to this (modified) Affine2 for chaining.
     */
    Affine2& setIdentity();
    
    /**
     * Sets all elements of the current transform to zero.
     *
     * @return A reference to this (modified) Affine2 for chaining.
     */
    Affine2& setZero();
    
    
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
    static Affine2* add(const Affine2& m, const Vec2& v, Affine2* dst);
    
    /**
     * Subtracts the offset v from m and stores the result in dst.
     *
     * Subtraction is applied to the offset only; the core matrix is unchanged.
     *
     * @param m1    The initial transform.
     * @param v     The offset to subtract.
     * @param dst   A transform to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Affine2* subtract(const Affine2& m1, const Vec2& v, Affine2* dst);
    
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
    static Affine2* multiply(const Affine2& mat, float scalar, Affine2* dst);
    
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
    static Affine2* multiply(const Affine2& m1, const Affine2& m2, Affine2* dst);
    
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
    static Affine2* invert(const Affine2& m1, Affine2* dst);
    
#pragma mark -
#pragma mark Arithmetic
    /**
     * Adds the specified offset to this transform.
     *
     * Addition is applied to the offset only; the core matrix is unchanged.
     *
     * @param v The offset to add.
     *
     * @return A reference to the Affine2 after addition.
     */
    Affine2& add(const Vec2& v) {
        return *(add(*this,v,this));
    }
    
    /**
     * Subtracts the specified offset from the current transform.
     *
     * Subtraction is applied to the offset only; the core matrix is unchanged.
     *
     * @param v The offset to subtract.
     *
     * @return A reference to the Affine2 after subtraction.
     */
    Affine2& subtract(const Vec2& v) {
        return *(subtract(*this,v,this));
    }
    
    /**
     * Multiplies the components of this transform by the specified scalar.
     *
     * The scalar is applied to BOTH the core matrix and the offset.
     *
     * @param scalar The scalar value.
     *
     * @return A reference to the Affine2 after multiplication.
     */
    Affine2& multiply(float scalar) {
        return *(multiply(*this,scalar,this));
    }
    
    /**
     * Multiplies this matrix by the specified one.
     *
     * Transform multiplication is defined as standard function composition.
     * The transform m2 is on the right.  This means that it corresponds to
     * an subsequent transform; transforms are applied left-to-right.
     *
     * @param aff The transform to multiply.
     *
     * @return A reference to the Affine2 after multiplication.
     */
    Affine2& multiply(const Affine2& aff) {
        return *(multiply(*this,aff,this));
    }
    
    /**
     * Inverts this transform in place.
     *
     * If the transform cannot be inverted, this method sets it to the zero
     * transform.
     *
     * @return A reference to the Affine2 after the inversion.
     */
    Affine2& invert() {
        return *(invert(*this,this));
    }
    
    /**
     * Returns a copy of the inverse of this transform.
     *
     * If the transform cannot be inverted, this method returns the zero transform.
     *
     * Note: This does not modify the transform.
     *
     * @return a copy of the inverse of this transform.
     */
    Affine2 getInverse() const {
        Affine2 result;
        invert(*this,&result);
        return result;
    }
    
#pragma mark -
#pragma mark Operators
    /**
     * Adds the given offset to this transform in place.
     *
     * Addition is applied to the offset only; the core matrix is unchanged.
     *
     * @param v The offset to add
     *
     * @return A reference to this (modified) Affine2 for chaining.
     */
    Affine2& operator+=(const Vec2& v) {
        return *(add(*this,v,this));
    }
    
    /**
     * Subtracts the given offset from this transform in place.
     *
     * Subtraction is applied to the offset only; the core matrix is unchanged.
     *
     * @param v The offset to subtract
     *
     * @return A reference to this (modified) Affine2 for chaining.
     */
    Affine2& operator-=(const Vec2& v) {
        return *(subtract(*this,v,this));
    }
    
    /**
     * Multiplies the components of this transform by the specified scalar.
     *
     * The scalar is applied to BOTH the core matrix and the offset.
     *
     * @param scalar The scalar value.
     *
     * @return A reference to this (modified) Affine2 for chaining.
     */
    Affine2& operator*=(float scalar) {
        return *(multiply(*this,scalar,this));
    }

    /**
     * Right-multiplies this transform by the given transform.
     *
     * Transform multiplication is defined as standard function composition.
     * The transform aff is on the right.  This means that it corresponds to
     * an subsequent transform; transforms are applied left-to-right.
     *
     * @param aff The transform to multiply by.
     *
     * @return A reference to this (modified) Affine2 for chaining.
     */
    Affine2& operator*=(const Affine2& aff) {
        return *(multiply(*this,aff,this));
    }
    
    /**
     * Returns the sum of this transform with the given offset.
     *
     * Addition is applied to the offset only; the core matrix is unchanged.
     *
     * Note: This does not modify the transform.
     *
     * @param v The offset to add.
     *
     * @return the sum of this transform with the given offset.
     */
    const Affine2 operator+(const Vec2& v) const {
        Affine2 result;
        return *(add(*this,v,&result));
    }
    
    /**
     * Returns the difference of this matrix with the given offset.
     *
     * Subtraction is applied to the offset only; the core matrix is unchanged.
     *
     * Note: This does not modify the transform.
     *
     * @param v The offset to subtract.
     *
     * @return the difference of this transform and the given offset.
     */
    const Affine2 operator-(const Vec2& v) const {
        Affine2 result;
        return *(subtract(*this,v,&result));
    }
    
    /**
     * Returns a copy of this matrix with all elements multiplied by the scalar.
     *
     * The scalar is applied to BOTH the core matrix and the offset.
     *
     * Note: This does not modify the matrix.
     *
     * @param scalar The scalar value.
     *
     * @return the product of this transform with the given scalar.
     */
    const Affine2 operator*(float scalar) const {
        Affine2 result;
        return *(multiply(*this,scalar,&result));
    }

    
    /**
     * Returns the matrix product of this matrix with the given matrix.
     *
     * Transform multiplication is defined as standard function composition.
     * The transform aff is on the right.  This means that it corresponds to
     * an subsequent transform; transforms are applied left-to-right.
     *
     * Note: This does not modify the matrix.
     *
     * @param aff   The transform to multiply by.
     *
     * @return the matrix product of this matrix with the given matrix.
     */
    const Affine2 operator*(const Affine2& aff) const {
        Affine2 result;
        return *(multiply(*this,aff,&result));
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
    bool isExactly(const Affine2& aff) const;
    
    /**
     * Returns true if the transforms are within tolerance of each other.
     *
     * The tolerance is applied to each element of the transform individually.
     *
     * @param mat       The transform to compare against.
     * @param variance  The comparison tolerance.
     *
     * @return true if the transforms are within tolerance of each other.
     */
    bool equals(const Affine2& mat, float variance=CU_MATH_EPSILON) const;
    
    /**
     * Returns true if this transform is equal to the given transform.
     *
     * Comparison is exact, which may be unreliable given that the elements
     * are floats. It should only be used to compared transform that have not
     * undergone a lot of manipulation.
     *
     * @param aff   The transform to compare against.
     *
     * @return true if this transform is equal to the given transform.
     */
    bool operator==(const Affine2& aff) const {
        return isExactly(aff);
    }
    
    /**
     * Returns true if this transform is not equal to the given transform.
     *
     * Comparison is exact, which may be unreliable given that the elements
     * are floats.
     *
     * @param aff   The transform to compare against.
     *
     * @return true if this transform is not equal to the given transform.
     */
    bool operator!=(const Affine2& aff) const {
        return !isExactly(aff);
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
    bool isIdentity(float variance=0.0f) const;
    
    /**
     * Returns true if this transform is invertible.
     *
     * The optional comparison tolerance takes into accout that elements
     * are floats and this may not be exact.  The tolerance is applied to
     * the determinant of the core matrix.
     *
     * @return true if this transform is invertible.
     */
    bool isInvertible(float variance=CU_MATH_EPSILON) const {
        return fabsf(getDeterminant()) > variance;
    }
    
    /**
     * Returns the determinant of this transform.
     *
     * The determinant is a feature of the core matrix.  The offset is
     * ignored.
     *
     * @return the determinant of this transform.
     */
    float getDeterminant() const {
        return m[0]*m[3]-m[2]*m[1];
    }
    
    /**
     * Returns the scale component of this transform.
     *
     * If the scale component of this matrix has negative parts,
     * it is not possible to always extract the exact scalar component.
     * In that case, a scale vector that is mathematically equivalent to
     * the original scale vector is extracted and returned.
     *
     * To work properly, the transform must have been constructed in the following
     * order: scale, then rotate, then translation. In any other order, the
     * scale is not guaranteed to be correct.
     *
     * @return the scale component of this transform.
     */
    Vec2 getScale() const {
        Vec2 result;
        decompose(*this,&result,nullptr,nullptr);
        return result;
    }
    
    /**
     * Returns the rotational angle of this transform.
     *
     * If the scale component is too close to zero, we cannot extract the
     * rotation.  In that case, we return the zero angle.
     (
     * @return the rotational angle of this transform.
     */
    float getRotation() const {
        float result;
        decompose(*this,nullptr,&result,nullptr);
        return result;
    }
    
    /**
     * Returns the translational component of this transform.
     *
     * To work properly, the transform must have been constructed in the following
     * order: scale, then rotate, then translation. In any other order, the
     * translation is not guaranteed to be correct.
     *
     * @return the translational component of this transform.
     */
    Vec2 getTranslation() const {
        Vec2 result;
        decompose(*this,nullptr,nullptr,&result);
        return result;
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
    static Vec2* transform(const Affine2& aff, const Vec2& point, Vec2* dst);
    
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
    static Rect* transform(const Affine2& aff, const Rect& rect, Rect* dst);
    
    /**
     * Returns a copy of the given point transformed.
     *
     * Note: This does not modify the original point. To transform a
     * point in place, use the static method (or the appropriate operator).
     *
     * @param point The point to transform.
     *
     * @return a copy of this point transformed.
     */
    Vec2 transform(const Vec2& point) const {
        Vec2 result;
        return *(transform(*this,point,&result));
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
    Rect transform(const Rect& rect) const;
    
    
#pragma mark -
#pragma mark Static Transform Manipulation
    /**
     * Applies a rotation to the given transform and stores the result in dst.
     *
     * The rotation is in radians, counter-clockwise about the z-axis.
     *
     * The rotation is applied on the right.  Given our convention, that means
     * that it takes place AFTER any previously applied transforms.
     *
     * @param aff   The transform to rotate.
     * @param angle The angle (in radians).
     * @param dst   A transform to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Affine2* rotate(const Affine2& aff, float angle, Affine2* dst) {
        Affine2 result;
        createRotation(angle, &result);
        multiply(aff, result, dst);
        return dst;
    }
    
    /**
     * Applies a uniform scale to the given transform and stores the result in dst.
     *
     * The scaling operation is applied on the right.  Given our convention,
     * that means that it takes place AFTER any previously applied transforms.
     *
     * @param aff   The transform to scale.
     * @param value The scalar to multiply by.
     * @param dst   A transform to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Affine2* scale(const Affine2& aff, float value, Affine2* dst) {
        Affine2 result;
        createScale(value, &result);
        multiply(aff, result, dst);
        return dst;
    }
    
    /**
     * Applies a non-uniform scale to the given transform and stores the result in dst.
     *
     * The scaling operation is applied on the right.  Given our convention,
     * that means that it takes place AFTER any previously applied transforms.
     *
     * @param aff   The transform to scale.
     * @param s     The vector storing the individual scaling factors
     * @param dst   A transform to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Affine2* scale(const Affine2& aff, const Vec2& s, Affine2* dst) {
        Affine2 result;
        createScale(s, &result);
        multiply(aff, result, dst);
        return dst;
    }
    
    /**
     * Applies a non-uniform scale to the given transform and stores the result in dst.
     *
     * The scaling operation is applied on the right.  Given our convention,
     * that means that it takes place AFTER any previously applied transforms.
     *
     * @param aff   The transform to scale.
     * @param sx    The amount to scale along the x-axis.
     * @param sy    The amount to scale along the y-axis.
     * @param dst   A transform to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Affine2* scale(const Affine2& aff, float sx, float sy, Affine2* dst) {
        Affine2 result;
        createScale(sx,sy, &result);
        multiply(aff, result, dst);
        return dst;
    }
    
    /**
     * Applies a translation to the given transform and stores the result in dst.
     *
     * The translation operation is applied on the right.  Given our convention,
     * that means that it takes place AFTER any previously applied transforms.
     *
     * @param aff   The transform to translate.
     * @param t     The vector storing the individual translation offsets
     * @param dst   A transform to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Affine2* translate(const Affine2& aff, const Vec2& t, Affine2* dst) {
        Affine2 result;
        createTranslation(t, &result);
        multiply(aff, result, dst);
        return dst;
    }
    
    /**
     * Applies a translation to the given transform and stores the result in dst.
     *
     * The translation operation is applied on the right.  Given our convention,
     * that means that it takes place AFTER any previously applied transforms.
     *
     * @param aff   The transform to translate.
     * @param tx    The translation offset for the x-axis.
     * @param ty    The translation offset for the y-axis.
     * @param dst   A transform to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Affine2* translate(const Affine2& aff, float tx, float ty, Affine2* dst) {
        Affine2 result;
        createTranslation(tx,ty, &result);
        multiply(aff, result, dst);
        return dst;
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
    static bool decompose(const Affine2& mat, Vec2* scale, float* rot, Vec2* trans);
    
    
#pragma mark -
#pragma mark Matrix Transforms
    /**
     * Applies a rotation to this transform.
     *
     * The rotation is in radians, counter-clockwise about the given axis.
     *
     * The rotation is applied on the right.  Given our convention, that means
     * that it takes place AFTER any previously applied transforms.
     *
     * @param angle The angle (in radians).
     *
     * @return This transform, after rotation.
     */
    Affine2& rotate(float angle) {
        return *(rotate(*this,angle,this));
    }
    
    /**
     * Applies a uniform scale to this transform.
     *
     * The scaling operation is applied on the right.  Given our convention,
     * that means that it takes place AFTER any previously applied transforms.
     *
     * @param value The scalar to multiply by.
     *
     * @return This transform, after scaling.
     */
    Affine2& scale(float value) {
        return *(scale(*this,value,this));
    }
    
    /**
     * Applies a non-uniform scale to this transform.
     *
     * The scaling operation is applied on the right.  Given our convention,
     * that means that it takes place AFTER any previously applied transforms.
     *
     * @param s     The vector storing the individual scaling factors
     *
     * @return This transform, after scaling.
     */
    Affine2& scale(const Vec2& s) {
        return *(scale(*this,s,this));
    }
    
    /**
     * Applies a non-uniform scale to this transform.
     *
     * The scaling operation is applied on the right.  Given our convention,
     * that means that it takes place AFTER any previously applied transforms.
     *
     * @param sx    The amount to scale along the x-axis.
     * @param sy    The amount to scale along the y-axis.
     *
     * @return This transform, after scaling.
     */
    Affine2& scale(float sx, float sy) {
        return *(scale(*this,sx,sy,this));
    }
    
    /**
     * Applies a translation to this transform.
     *
     * The translation operation is applied on the right.  Given our convention,
     * that means that it takes place AFTER any previously applied transforms.
     *
     * @param t     The vector storing the individual translation offsets
     *
     * @return This transform, after translation.
     */
    Affine2& translate(const Vec2& t) {
        return *(translate(*this,t,this));
    }
    
    /**
     * Applies a translation to this transform.
     *
     * The translation operation is applied on the right.  Given our convention,
     * that means that it takes place AFTER any previously applied transforms.
     *
     * @param tx    The translation offset for the x-axis.
     * @param ty    The translation offset for the y-axis.
     *
     * @return This transform, after translation.
     */
    Affine2& translate(float tx, float ty) {
        return *(translate(*this,tx,ty,this));
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
    std::string toString(bool verbose = false) const;
    
    /** Cast from Vec4 to a string. */
    operator std::string() const { return toString(); }
    
    /** Cast from Affine2 to a Mat4. */
    operator Mat4() const;
    
    /**
     * Creates an affine transform from the given matrix.
     *
     * The z values are all uniformly ignored.  However, it the final element
     * of the matrix is not 1 (e.g. the translation has a w value of 1), then
     * it divides the entire matrix before creating the affine transform
     *
     * @param mat The matrix to convert
     */
    explicit Affine2(const Mat4& mat);
    
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
    Affine2& operator= (const Mat4& mat);

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
    Affine2& set(const Mat4& mat);
};
    
#pragma mark -
#pragma mark Vector Operations
// To avoid confusion, we NEVER support vector on the right ops.
    
/**
 * Transforms the given point in place.
 *
 *
 * @param v The point to transform.
 * @param m The transform to apply.
 *
 * @return this point, after the transformation occurs.
 */
 inline Vec2& operator*=(Vec2& v, const Affine2& m) {
     return *(Affine2::transform(m,v,&v));
 }
    
/**
 * Returns a copy of the vector after it is transformed.
 *
 * @param v The point to transform.
 * @param m The transform to apply.
 *
 * @return a copy of the vector after it is transformed.
 */
inline const Vec2 operator*(const Vec2& v, const Affine2& m) {
    Vec2 result;
    Affine2::transform(m,v,&result);
    return result;
}
    
/**
 * Multiplies the components of the given matrix by the specified scalar.
 *
 * The scalar is applied to BOTH the core matrix and the offset.
 *
 * @param scalar The scalar value.
 * @param m The transform to scale.
 *
 * @return a copy of the scaled transform
 */
inline const Affine2 operator*(float scalar, const Affine2& m) {
    Affine2 result(m);
    return result.multiply(scalar);
}
    
}
#endif /* __CU_AFFINE2_H__ */

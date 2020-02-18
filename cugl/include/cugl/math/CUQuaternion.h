//
//  CUQuaternion.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a quaternion, which is a way of representing
//  rotations in 3d sapce..  It has support for basic arithmetic, as well as
//  as the standard quaternion interpolations.
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
//  Version: 6/5/16
#ifndef __CU_QUATERNION_H__
#define __CU_QUATERNION_H__

#include <math.h>
#include "CUVec3.h"
#include "CUMathBase.h"

namespace cugl {
    
// Forward declarations
class Mat4;

/**
 * This class provides a quaternion that represents an object orientation.
 *
 * Quaternions are typically used as a replacement for euler angles and 
 * rotation matrices as a way to achieve smooth interpolation and avoid gimbal 
 * lock.
 *
 * Note that this quaternion class does not automatically keep the quaternion 
 * normalized. Therefore, care must be taken to normalize the quaternion when 
 * necessary, by calling the normalize method.
 *
 * This class provides three methods for doing quaternion interpolation: 
 * lerp, slerp, and nlerp.  The advatanges of each of these interpolation
 * types are discussed at
 *
 *  https://keithmaggio.wordpress.com/2011/02/15/math-magician-lerp-slerp-and-nlerp/
 *
 * lerp (linear interpolation): 
 * The interpolation curve gives a straight line in quaternion space. It is 
 * simple and fast to compute. The only problem is that it does not provide 
 * constant angular velocity. Note that a constant velocity is not necessarily 
 * a requirement for a curve.
 *
 * The lerp method provided here interpolates strictly in quaternion space. Note
 * that the resulting path may pass through the origin if interpolating between
 * a quaternion and its exact negative.
 *
 * slerp (spherical linear interpolation): 
 * The interpolation curve forms a great arc on the quaternion unit sphere.
 * Slerp provides constant angular velocity and is torque minimal.  However,
 * it is not commutative and is very computationally expensive.
 *
 * The slerp method provided here is intended for interpolation of principal
 * rotations. It treats +q and -q as the same principal rotation and is at 
 * liberty to use the negative of either input. The resulting path is always 
 * the shorter arc.
 *
 * nlerp (normalized linear interpolation):
 * The interpolation curve gives a straight line in quaternion space, but 
 * normalizes the result.  When the input quaternions are themselves unit
 * quaternions, this provides a fast alternative to slerp. Again, it does
 * not provide constant angular velocity, but it is communitative and is
 * torque minimal.
 *
 * The nlerp method provided here interpolates uses lerp as its base.
 *
 * This class is in standard layout with fields of uniform type.  This means
 * that it is safe to reinterpret_cast objects to float arrays.
 */
class Quaternion {

#pragma mark Values
public:
#if defined CU_MATH_VECTOR_SSE
    union {
        struct {
            /** The x-coordinate. */
            float x;
            /** The y-coordinate. */
            float y;
            /** The z-coordinate. */
            float z;
            /** The w-coordinate. */
            float w;
        };
        __m128 v;
    };
#elif defined CU_MATH_VECTOR_NEON64
    union {
        struct {
            /** The x-coordinate. */
            float x;
            /** The y-coordinate. */
            float y;
            /** The z-coordinate. */
            float z;
            /** The w-coordinate. */
            float w;
        };
        float32x4_t v;
    };
#else
    /** The x-value of the quaternion's vector component. */
    float x;
    /** The y-value of the quaternion's vector component. */
    float y;
    /** The z-value of the quaternion's vector component. */
    float z;
    /** The scalar component of the quaternion. */
    float w;
#endif
    
    /** The zero to Quaternion(0,0,0,0) */
    static const Quaternion ZERO;
    /** The identity Quaternion(0,0,0,1) */
    static const Quaternion IDENTITY;
    
    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Constructs a quaternion initialized to (0, 0, 0, 1).
     */
    Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    
    /**
     * Constructs a quaternion initialized to the specified values.
     *
     * @param x The x component of the quaternion.
     * @param y The y component of the quaternion.
     * @param z The z component of the quaternion.
     * @param w The w component of the quaternion.
     */
    Quaternion(float x, float y, float z, float w) {
    #if defined CU_MATH_VECTOR_SSE
        v = _mm_set_ps(w, z, y, x);
    #elif defined CU_MATH_VECTOR_NEON64
        v = {x, y, z, w};
    #else
        this->x = x; this->y = y; this->z = z; this->w = w;
    #endif
    }
    
    /**
     * Constructs a new quaternion from the values in the specified array.
     *
     * The elements of the array are in the order x, y, z, and w.
     *
     * @param array An array containing the elements of the quaternion.
     */
    Quaternion(float* array);
    
    /**
     * Constructs a quaternion equal to the rotation from the specified axis and angle.
     *
     * @param axis  A vector describing the axis of rotation.
     * @param angle The angle of rotation (in radians).
     */
    Quaternion(const Vec3& axis, float angle);

    
#pragma mark -
#pragma mark Static Constructors
    /**
     * Creates a quaternion equal to the rotational part of the matrix.
     *
     * The result is stored in dst.
     *
     * @param m     The matrix.
     * @param dst   A quaternion to store the conjugate in.
     *
     * @return A reference to dst for chaining
     */
    static Quaternion* createFromRotationMatrix(const Mat4& m, Quaternion* dst);
    
    /**
     * Creates this quaternion equal to the rotation from the specified axis and angle.
     *
     * The result is stored in dst.
     *
     * @param axis  A vector describing the axis of rotation.
     * @param angle The angle of rotation (in radians).
     * @param dst   A quaternion to store the conjugate in.
     *
     * @return A reference to dst for chaining
     */
    static Quaternion* createFromAxisAngle(const Vec3& axis, float angle, Quaternion* dst);
    

#pragma mark -
#pragma mark Setters
    /**
     * Sets the elements of this quaternion from the values in the specified array.
     *
     * The elements of the array are in the order x, y, z, and w.
     *
     * @param array An array containing the elements of the quaternion.
     *
     * @return A reference to this (modified) Quaternion for chaining.
     */
    Quaternion& operator=(const float* array) {
        return set(array);
    }
    
    /**
     * Sets the elements of the quaternion to the specified values.
     *
     * @param x The x component of the quaternion.
     * @param y The y component of the quaternion.
     * @param z The z component of the quaternion.
     * @param w The w component of the quaternion.
     *
     * @return A reference to this (modified) Quaternion for chaining.
     */
    Quaternion& set(float x, float y, float z, float w) {
    #if defined CU_MATH_VECTOR_SSE
        v = _mm_set_ps(w, z, y, x);
    #elif defined CU_MATH_VECTOR_NEON64
        v = {x, y, z, w};
    #else
        this->x = x; this->y = y; this->z = z; this->w = w;
    #endif
        return *this;
    }
    
    /**
     * Sets the elements of this quaternion from the values in the specified array.
     *
     * The elements of the array are in the order x, y, z, and w.
     *
     * @param array An array containing the elements of the quaternion.
     *
     * @return A reference to this (modified) Quaternion for chaining.
     */
    Quaternion& set(const float* array);
    
    /**
     * Sets the quaternion equal to the rotation from the specified axis and angle.
     *
     * @param axis  The axis of rotation.
     * @param angle The angle of rotation (in radians).
     *
     * @return A reference to this (modified) Quaternion for chaining.
     */
    Quaternion& set(const Vec3& axis, float angle);
    
    /**
     * Sets the elements of this quaternion to those in the specified quaternion.
     *
     * @param q  The quaternion to copy.
     *
     * @return A reference to this (modified) Quaternion for chaining.
     */
    Quaternion& set(const Quaternion& q) {
    #if defined CU_MATH_VECTOR_SSE || defined CU_MATH_VECTOR_NEON64
        v = q.v;
    #else
        x = q.x; y = q.y; z = q.z; w = q.w;
    #endif
        return *this;
    }
    
    /**
     * Sets this quaternion to be equal to the identity quaternion.
     *
     * @return A reference to this (modified) Quaternion for chaining.
     */
    Quaternion& setIdentity() {
    #if defined CU_MATH_VECTOR_SSE
        v = _mm_setzero_ps();
        v[3] = 1.0f;
    #elif defined CU_MATH_VECTOR_NEON64
        v = vmovq_n_f32(0.0f);
        v[3] = 1.0f;
    #else
        x = y = z = 0; w = 1;
    #endif
        return *this;
    }

    /**
     * Sets this quaternion to be equal to the zero quaternion.
     *
     * @return A reference to this (modified) Quaternion for chaining.
     */
    Quaternion& setZero() {
    #if defined CU_MATH_VECTOR_SSE
        v = _mm_setzero_ps();
    #elif defined CU_MATH_VECTOR_NEON64
        v = vmovq_n_f32(0.0f);
    #else
        x = y = z = w = 0;
    #endif
        return *this;
    }
    
    
#pragma mark -
#pragma mark Static Arithmetic
    /**
     * Adds the specified quaternions and stores the result in dst.
     *
     * @param q1    The first quaternion.
     * @param q2    The second quaternion.
     * @param dst   A quaternion to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Quaternion* add(const Quaternion& q1, const Quaternion& q2, Quaternion* dst);

    /**
     * Subtacts the quaternions q2 from q1 and stores the result in dst.
     *
     * @param q1    The first quaternion.
     * @param q2    The second quaternion.
     * @param dst   A quaternion to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Quaternion* subtract(const Quaternion& q1, const Quaternion& q2, Quaternion* dst);

    /**
     * Multiplies the specified quaternions and stores the result in dst.
     *
     * This method performs standard quaternion multiplication
     *
     * @param q1    The first quaternion.
     * @param q2    The second quaternion.
     * @param dst   A quaternion to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Quaternion* multiply(const Quaternion& q1, const Quaternion& q2, Quaternion* dst);

    /**
     * Divides a quaternion by another and stores the result in dst.
     *
     * This method performs standard quaternion division.  That is, it multiplies
     * the first quaternion by the inverse of the second.
     *
     * @param q1    The initial quaternion.
     * @param q2    The quaternion to divide by.
     * @param dst   A quaternion to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Quaternion* divide(const Quaternion& q1, const Quaternion& q2, Quaternion* dst);

    /**
     * Scales the specified quaternion by s and stores the result in dst.
     *
     * @param q1    The first quaternion.
     * @param s     The scalar value.
     * @param dst   A quaternion to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Quaternion* scale(const Quaternion& q1, float s, Quaternion* dst);

    /**
     * Conjugates the specified quaternion and stores the result in dst.
     *
     * @param quat  The quaternion to conjugate.
     * @param dst   A quaternion to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Quaternion* conjugate(const Quaternion& quat, Quaternion* dst);

    /**
     * Inverts the specified quaternion and stores the result in dst.
     *
     * Note that the inverse of a quaternion is equal to its conjugate
     * when the quaternion is unit-length. For this reason, it is more
     * efficient to use the conjugate method directly when you know your
     * quaternion is already unit-length.
     *
     * If the inverse cannot be computed, this method stores a quaternion
     * with NaN values in dst.
     *
     * @param quat  The quaternion to invert.
     * @param dst   A quaternion to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Quaternion* invert(const Quaternion& quat, Quaternion* dst);

    /**
     * Normalizes the specified quaternion and stores the result in dst.
     *
     * If the quaternion already has unit length or if the length of the
     *  quaternion is zero, this method copies quat into dst.
     *
     * @param quat  The quaternion to normalize.
     * @param dst   A quaternion to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Quaternion* normalize(const Quaternion& quat, Quaternion* dst);

    /**
     * Negates the specified quaternion and stores the result in dst.
     *
     * @param quat  The quaternion to negate.
     * @param dst   A quaternion to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Quaternion* negate(const Quaternion& quat, Quaternion* dst);

    /**
     * Returns the dot product of the two quaternions
     *
     * @param q1    The first quaternion.
     * @param q2    The second quaternion.
     *
     * @return The dot product.
     */
    static float dot(const Quaternion& q1, const Quaternion& q2);
    
    
#pragma mark -
#pragma mark Arithmetic
    /**
     * Adds the specified quaternion to this one in place.
     *
     * @param q The quaternion to add.
     *
     * @return this quaternion, after addition
     */
    Quaternion& add(const Quaternion& q) {
    #if defined CU_MATH_VECTOR_SSE
        v = _mm_add_ps(v,q.v);
    #elif defined CU_MATH_VECTOR_NEON64
        v = vaddq_f32(v,q.v);
    #else
        x += q.x; y += q.y; z += q.z;  w += q.w;
    #endif
        return *this;
    }

    /**
     * Subtracts the specified quaternion from this one in place.
     *
     * @param q The quaternion to subtract.
     *
     * @return this quaternion, after subtraction
     */
    Quaternion& subtract(const Quaternion& q) {
    #if defined CU_MATH_VECTOR_SSE
        v = _mm_sub_ps(v,q.v);
    #elif defined CU_MATH_VECTOR_NEON64
        v = vsubq_f32(v,q.v);
    #else
        x -= q.x; y -= q.y; z -= q.z; w -= q.w;
    #endif
        return *this;
    }
    
    /**
     * Multiplies this quaternion by the specified one in place.
     *
     * @param q The quaternion to multiply.
     *
     * @return this quaternion, after multiplication
     */
    Quaternion& multiply(const Quaternion& q) {
        return *(multiply(*this,q,this));
    }

    
    /**
     * Divides this quaternion by the specified one in place.
     *
     * Division is the same as multiplication by the inverse of q.
     *
     * @param q The quaternion to divide by.
     *
     * @return this quaternion, after division
     */
    Quaternion& divide(const Quaternion& q) {
        return *(divide(*this,q,this));
    }

    
    /**
     * Scales this quaternion by the specified value in place.
     *
     * @param s The value to scale the quaternion.
     *
     * @return this quaternion, after multiplication
     */
    Quaternion& scale(float s) {
    #if defined CU_MATH_VECTOR_SSE
        v = _mm_mul_ps(v,_mm_set1_ps(s));
    #elif defined CU_MATH_VECTOR_NEON64
        v = vmulq_n_f32(v,(float32_t)s);
    #else
        x *= s; y *= s; z *= s; w *= s;
    #endif
        return *this;
    }

    /**
     * Sets this quaternion to the conjugate of itself.
     *
     * @return this quaternion, after conjugation
     */
    Quaternion& conjugate() {
    #if defined CU_MATH_VECTOR_SSE
        float tmp = w;
        v = _mm_sub_ps(_mm_setzero_ps(), v);
        w = tmp;
    #elif defined CU_MATH_VECTOR_NEON64
        float tmp = w;
        v = vnegq_f32(v);
        w = tmp;
    #else
        x = -x; y = -y; z = -z;
    #endif
        return *this;
  }
    
    /**
     * Returns the conjugate of this quaternion.
     *
     * @return the conjugate of this quaternion.
     */
    Quaternion getConjugate() const {
        Quaternion result;
        return *(conjugate(*this,&result));
    }
    
    /**
     * Sets this quaternion to the inverse of itself.
     *
     * Note that the inverse of a quaternion is equal to its conjugate
     * when the quaternion is unit-length. For this reason, it is more
     * efficient to use the conjugate method directly when you know your
     * quaternion is already unit-length.
     *
     * If the inverse cannot be computed, this method sets the quaternion
     * to have NaN values.
     *
     * @return this quaternion, after inverting
     */
    Quaternion& invert() {
        return *(invert(*this,this));
    }
    
    /**
     * Gets the inverse of this quaternion.
     *
     * Note that the inverse of a quaternion is equal to its conjugate
     * when the quaternion is unit-length. For this reason, it is more
     * efficient to use the conjugate method directly when you know your
     * quaternion is already unit-length.
     *
     * If the inverse cannot be computed, this method returns a quaternion
     * with all NaN values instead.
     *
     * @return the inverse of this quaternion.
     */
    Quaternion getInverse() const {
        Quaternion result;
        return *(invert(*this,&result));
    }

    /**
     * Normalizes this quaternion to have unit length.
     *
     * If the quaternion already has unit length or if the length of the
     * quaternion is zero, this method does nothing.
     *
     * @return this quaternion, after normalization
     */
    Quaternion& normalize() {
        return *(normalize(*this,this));
    }
    
    /**
     * Returns a normalized copy of this quaternion.
     *
     * If the quaternion already has unit length or if the length of the
     * quaternion is zero, this method simply copies this quaternion.
     *
     * Note: This method does not modify the quaternion
     *
     * @return a normalized copy of this quaternion.
     */
    Quaternion getNormalization() const {
        Quaternion result;
        return *(normalize(*this,&result));
    }
    
    /**
     * Negates this quaternion in place.
     *
     * @return this quaternion, after negation
     */
    Quaternion& negate() {
    #if defined CU_MATH_VECTOR_SSE
        v = _mm_sub_ps(_mm_setzero_ps(), v);
    #elif defined CU_MATH_VECTOR_NEON64
        v = vsubq_f32(vmovq_n_f32(0.0f), v);
    #else
        x = -x; y = -y; z = -z; w = -w;
    #endif
        return *this;
    }
    
    /**
     * Returns a negated copy of this quaternion.
     *
     * Note: This method does not modify the quaternion
     *
     * @return a negated copy of this quaternion.
     */
    Quaternion getNegation() const {
        Quaternion result;
        return *(negate(*this,&result));
    }
    
    /**
     * Returns the dot product of this quaternion with the specified one.
     *
     * @param q The quaternion to compute the dot product with.
     *
     * @return The dot product.
     */
    float dot(const Quaternion& q) const {
        return dot(*this,q);
    }

    
    
#pragma mark -
#pragma mark Operators
    /**
     * Adds the given quaternion to this one in place.
     *
     * @param q The quaternion to add
     *
     * @return A reference to this (modified) Quaternion for chaining.
     */
    Quaternion& operator+=(const Quaternion& q) {
        return add(q);
    }
    
    /**
     * Subtracts the given quaternion from this one in place.
     *
     * @param q The quaternion to subtract
     *
     * @return A reference to this (modified) Quaternion for chaining.
     */
    Quaternion& operator-=(const Quaternion& q) {
        return subtract(q);
    }
    
    /**
     * Scales this quaternion in place by the given factor.
     *
     * @param s The value to scale by
     *
     * @return A reference to this (modified) Quaternion for chaining.
     */
    Quaternion& operator*=(float s) {
        return scale(s);
    }
    
    /**
     * Multiplies this quaternion in place by the given quaternion.
     *
     * This method performs standard quaternion multiplication
     *
     * @param q The quaternion to multiply by
     *
     * @return A reference to this (modified) Quaternion for chaining.
     */
    Quaternion& operator*=(const Quaternion& q) {
        return *(multiply(*this,q,this));
    }
    
    /**
     * Divides this quaternion in place by the given factor.
     *
     * @param s The scalar to divide by
     *
     * @return A reference to this (modified) Quaternion for chaining.
     */
    Quaternion& operator/=(float s) {
        return *(scale(*this,1.0f/s,this));
    }
    
    /**
     * Divides this quaternion in place by the given quaternion.
     *
     * This method is the same as multiplying by the inverse of the given
     * quaternion.  If the quaternion is not invertible this method will
     * store NaN in the current quaternion.
     *
     * @param q The quaternion to divide by
     *
     * @return A reference to this (modified) Quaternion for chaining.
     */
    Quaternion& operator/=(const Quaternion& q) {
        return *(divide(*this,q,this));
    }
    
    /**
     * Returns the sum of this quaternion with the given quaternion.
     *
     * Note: this does not modify this quaternion.
     *
     * @param q The quaternion to add.
     *
     * @return The sum of this quaternion with the given quaternion.
     */
    const Quaternion operator+(const Quaternion& q) const {
        Quaternion result;
        return *(add(*this,q,&result));
    }
    
    /**
     * Returns the difference of this quaternion with the given quaternion.
     *
     * Note: this does not modify this quaternion.
     *
     * @param q The quaternion to subtract.
     *
     * @return The difference of this quaternion with the given quaternion.
     */
    const Quaternion operator-(const Quaternion& q) const  {
        Quaternion result;
        return *(subtract(*this,q,&result));
    }
    
    /**
     * Returns the negation of this quaternion.
     *
     * Note: this does not modify this quaternion.
     *
     * @return The negation of this quaternion.
     */
    const Quaternion operator-() const {
        Quaternion result;
        return *(negate(*this,&result));
    }
    
    /**
     * Returns the scalar product of this quaternion with the given value.
     *
     * Note: this does not modify this quaternion.
     *
     * @param s The value to scale by.
     *
     * @return The scalar product of this quaternion with the given value.
     */
    const Quaternion operator*(float s) const {
        Quaternion result;
        return *(scale(*this,s,&result));
    }
    
    /**
     * Returns the product of this quaternion with the given quaternion.
     *
     * This method performs standard quaternion multiplication
     *
     * Note: this does not modify this quaternion.
     *
     * @param q The quaternion to multiply by.
     *
     * @return the product of this quaternion with the given quaternion.
     */
    const Quaternion operator*(const Quaternion& q) const {
        Quaternion result;
        return *(multiply(*this,q,&result));
    }
    
    /**
     * Returns a copy of this quaternion divided by the given constant
     *
     * Note: this does not modify this quaternion.
     *
     * @param s the constant to divide this quaternion with
     *
     * @return A copy of this vector quaternion by the given constant
     */
    const Quaternion operator/(float s) const {
        Quaternion result;
        return *(scale(*this,1.0f/s,&result));
    }
    
    /**
     * Returns a copy of this quaternion divided by the given quaternion.
     *
     * This method is the same as multiplying by the inverse of the given
     * quaternion.  If the quaternion is not invertible this method will
     * store NaN in the current quaternion.
     *
     * Note: this does not modify this quaternion.
     *
     * @param q The quaternion to divide by
     *
     * @return a copy of this quaternion divided by the given quaternion.
     */
    const Quaternion operator/(const Quaternion& q) const {
        Quaternion result;
        return *(divide(*this,q,&result));
    }

    
#pragma mark -
#pragma mark Comparisons
    /**
     * Returns true if this quaternion is equal to the given quaternion.
     *
     * Comparison is exact, which may be unreliable given that the attributes
     * are floats.
     *
     * @param q The quaternion to compare against.
     *
     * @return True if this quaternion is equal to the given quaternion.
     */
    bool operator==(const Quaternion& q) const;
    
    /**
     * Returns true if this quaternion is not equal to the given quaternion.
     *
     * Comparison is exact, which may be unreliable given that the attributes
     * are floats.
     *
     * @param q The quaternion to compare quaternion.
     *
     * @return True if this quaternion is not equal to the given quaternion.
     */
    bool operator!=(const Quaternion& q) const;
    
    /**
     * Returns true if the quaternions are within tolerance of each other.
     *
     * The tolerance bounds the norm of the difference between the two
     * quaternions
     *
     * @param q         The vector to compare against.
     * @param variance  The comparison tolerance.
     *
     * @return true if the quaternions are within tolerance of each other.
     */
    bool equals(const Quaternion& q, float variance=CU_MATH_EPSILON) const;

#pragma mark -
#pragma mark Linear Attributes
    /**
     * Returns the norm of this quaternion.
     *
     * @return The   of the quaternion.
     *
     * {@see normSquared}
     */
    float norm() const {
        return sqrt(normSquared());
    }
    
    /**
     * Returns the squared norm of this quaternion.
     *
     * This method is faster than norm because it does not need to compute
     * a square root.  Hence it is best to us this method when it is not
     * necessary to get the exact length of a quaternions (e.g. when simply
     * comparing the norm to a threshold value).
     *
     * @return The squared norm of the quaternion.
     *
     * {@see length}
     */
    float normSquared() const {
        return dot(*this);
    }
    
    /**
     * Returns true this quaternion contains all zeros.
     *
     * Comparison is exact, which may be unreliable given that the attributes
     * are floats.
     *
     * @return true if this quaternion contains all zeros, false otherwise.
     */
    bool isZero() const;
    
    /**
     * Returns true if this quaternion is with tolerance of the zero quaternion.
     *
     * The tolerance bounds the quaternion norm.
     *
     * @param variance The comparison tolerance
     *
     * @return true if this quaternion is with tolerance of the zero quaternion.
     */
    bool isNearZero(float variance=CU_MATH_EPSILON) const {
        return equals(ZERO,variance);
    }
    
    /**
     * Returns true if this quaternion is the identity.
     *
     * @return true if this quaternion is the identity.
     */
    bool isIdentity() const;

    /**
     * Returns true if this quaternion is near the identity.
     *
     * The tolerance bounds the quaternion norm of the differences.
     *
     * @param variance The comparison tolerance
     *
     * @return true if this quaternion is near the identity.
     */
    bool isNearIdentity(float variance=CU_MATH_EPSILON) const {
        return equals(IDENTITY,variance);
    }

    /**
     * Returns true if this vector is a unit vector.
     *
     * @param variance The comparison tolerance
     *
     * @return true if this vector is a unit vector.
     */
    bool isUnit(float variance=CU_MATH_EPSILON) const {
        float dot = norm()-1.0f;
        return fabsf(dot) <= variance;
    }
    
    /**
     * Converts this Quaternion4f to axis-angle notation.
     *
     * The angle is in radians.  The axis is normalized.
     *
     * @param e The Vec3f which stores the axis.
     *
     * @return The angle (in radians).
     */
    float toAxisAngle(Vec3* e) const;
    
    
#pragma mark -
#pragma mark Static Interpolation
    /**
     * Interpolates between two quaternions using linear interpolation.
     *
     * The interpolation curve for linear interpolation between quaternions 
     * gives a straight line in quaternion space.
     *
     * The interpolation coefficient MUST be between 0 and 1 (inclusive). Any
     * other values will cause an error.
     *
     * @param q1    The first quaternion.
     * @param q2    The second quaternion.
     * @param t     The interpolation coefficient in [0,1].
     * @param dst   A quaternion to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Quaternion* lerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion* dst);
    
    /**
     * Interpolates between two quaternions using spherical linear interpolation.
     *
     * Spherical linear interpolation provides smooth transitions between 
     * different orientations and is often useful for animating models or 
     * cameras in 3D.
     *
     * The interpolation coefficient MUST be between 0 and 1 (inclusive). Any
     * other values will cause an error.
     * 
     * In addition, the input quaternions must be at (or close to) unit length.
     * When assertions are enabled, it will test this via the isUnit() method.
     *
     * @param q1    The first quaternion.
     * @param q2    The second quaternion.
     * @param t     The interpolation coefficient in [0,1].
     * @param dst   A quaternion to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Quaternion* slerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion* dst);

    /**
     * Interpolates between two quaternions using normalized linear interpolation.
     *
     * Normalized linear interpolation provides smooth transitions between
     * different orientations and is often useful for animating models or
     * cameras in 3D.
     *
     * The interpolation coefficient MUST be between 0 and 1 (inclusive). Any
     * other values will cause an error.
     *
     * In addition, the input quaternions must be at (or close to) unit length.
     * When assertions are enabled, it will test this via the isUnit() method.
     *
     * @param q1    The first quaternion.
     * @param q2    The second quaternion.
     * @param t     The interpolation coefficient in [0,1].
     * @param dst   A quaternion to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Quaternion* nlerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion* dst);
    
    /**
     * Rotates the vector by this quaternion and stores the result in dst.
     *
     * The rotation is defined by the matrix associated with the vector. 
     *
     * @param v     The vector to rotate.
     * @param quat  The rotation quaternion.
     * @param dst   A vector to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Vec3* rotate(const Vec3& v, const Quaternion& quat, Vec3* dst);
    
    
#pragma mark -
#pragma mark Interpolation
    /**
     * Interpolates between this quaternion and another.
     *
     * The interpolation curve for linear interpolation between quaternions
     * gives a straight line in quaternion space.
     *
     * The interpolation coefficient MUST be between 0 and 1 (inclusive). Any
     * other values will cause an error.
     *
     * @param q     The quaternion to interpolate.
     * @param t     The interpolation coefficient in [0,1].
     *
     * @return this quaternion, after interpolation.
     */
    Quaternion& lerp(const Quaternion& q, float t) {
        return *(lerp(*this,q,t,this));
    }
    
    /**
     * Interpolates between this quaternion and another with spherical linear interpolation.
     *
     * Spherical linear interpolation provides smooth transitions between
     * different orientations and is often useful for animating models or
     * cameras in 3D.
     *
     * The interpolation coefficient MUST be between 0 and 1 (inclusive). Any
     * other values will cause an error.
     *
     * In addition, the input quaternions must be at (or close to) unit length.
     * When assertions are enabled, it will test this via the isUnit() method.
     *
     * @param q     The quaternion to interpolate.
     * @param t     The interpolation coefficient in [0,1].
     *
     * @return this quaternion, after interpolation.
     */
    Quaternion& slerp(const Quaternion& q, float t) {
        return *(slerp(*this,q,t,this));
    }
    
    /**
     * Interpolates between this quaternion and another with normalized linear interpolation.
     *
     * Normalized linear interpolation provides smooth transitions between
     * different orientations and is often useful for animating models or
     * cameras in 3D.
     *
     * The interpolation coefficient MUST be between 0 and 1 (inclusive). Any
     * other values will cause an error.
     *
     * In addition, the input quaternions must be at (or close to) unit length.
     * When assertions are enabled, it will test this via the isUnit() method.
     *
     * @param q     The quaternion to interpolate.
     * @param t     The interpolation coefficient in [0,1].
     *
     * @return this quaternion, after interpolation.
     */
    Quaternion& nlerp(const Quaternion& q, float t) {
        return *(slerp(*this,q,t,this));
    }
    
    /**
     * Returns an interpolation between this quaternion and another.
     *
     * The interpolation curve for linear interpolation between quaternions
     * gives a straight line in quaternion space.
     *
     * The interpolation coefficient MUST be between 0 and 1 (inclusive). Any
     * other values will cause an error.
     *
     * @param q     The quaternion to interpolate.
     * @param t     The interpolation coefficient in [0,1].
     *
     * Note: this does not modify this quaternion.
     *
     * @return an interpolation between this quaternion and another.
     */
    Quaternion getLerp(const Quaternion& q, float t) {
        Quaternion result;
        return *(lerp(*this,q,t,&result));
    }
    
    /**
     * Returns a spherical linear interpolation between this quaternion and another.
     *
     * Spherical linear interpolation provides smooth transitions between
     * different orientations and is often useful for animating models or
     * cameras in 3D.
     *
     * The interpolation coefficient MUST be between 0 and 1 (inclusive). Any
     * other values will cause an error.
     *
     * In addition, the input quaternions must be at (or close to) unit length.
     * When assertions are enabled, it will test this via the isUnit() method.
     *
     * Note: this does not modify this quaternion.
     *
     * @param q     The quaternion to interpolate.
     * @param t     The interpolation coefficient in [0,1].
     *
     * @return a spherical linear interpolation of this quaternion.
     */
    Quaternion getSlerp(const Quaternion& q, float t) {
        Quaternion result;
        return *(slerp(*this,q,t,&result));
    }
    
    /**
     * Returns a normalized linear interpolation between this quaternion and another.
     *
     * Normalized linear interpolation provides smooth transitions between
     * different orientations and is often useful for animating models or
     * cameras in 3D.
     *
     * The interpolation coefficient MUST be between 0 and 1 (inclusive). Any
     * other values will cause an error.
     *
     * In addition, the input quaternions must be at (or close to) unit length.
     * When assertions are enabled, it will test this via the isUnit() method.
     *
     * Note: this does not modify this quaternion.
     *
     * @param q     The quaternion to interpolate.
     * @param t     The interpolation coefficient in [0,1].
     *
     * @return a spherical linear interpolation of this quaternion.
     */
    Quaternion getNlerp(const Quaternion& q, float t) {
        Quaternion result;
        return *(nlerp(*this,q,t,&result));
    }
    
    /**
     * Returns a copy of the vector rotated by this quaternion.
     *
     * The rotation is defined by the matrix associated with the vector.      *
     * @param v     The vector to rotate.
     *
     * @return a copy of the vector rotated by this quaternion.
     */
    Vec3 getRotation(const Vec3& v) {
        Vec3 result;
        return *(rotate(v,*this,&result));
    }
    
#pragma mark -
#pragma mark Conversion Methods
public:
    /**
     * Returns a string representation of this quaternion for debuggging purposes.
     *
     * If verbose is true, the string will include class information.  This
     * allows us to unambiguously identify the class.
     *
     * @param verbose Whether to include class information
     *
     * @return a string representation of this vector for debuggging purposes.
     */
    std::string toString(bool verbose = false) const;
    
    /** Cast from Quaternion to a string. */
    operator std::string() const { return toString(); }
    
    /** Cast from Quaternion to a Vec4. */
    operator Vec4() const;
    
    /**
     * Creates a quaternion from the given vector.
     *
     * @param vector The vector to convert
     */
    explicit Quaternion(const Vec4& vector);
    
    /**
     * Sets the coordinates of this quaternion to those of the given vector.
     *
     * @param vector The vector to convert
     *
     * @return A reference to this (modified) Qauternion for chaining.
     */
    Quaternion& operator= (const Vec4& vector) {
        return set(vector);
    }

    /**
     * Sets the coordinates of this quaternion to those of the given vector.
     *
     * @param vector The vector to convert
     *
     * @return A reference to this (modified) Qauternion for chaining.
     */
    Quaternion& set(const Vec4& vector);
    
    /**
     * Cast from Quaternion to a Matrix. 
     *
     * The matrix is a rotation matrix equivalent to the rotation represented
     * by this quaternion.
     */
    operator Mat4() const;

    /**
     * Constructs a quaternion equal to the rotational part of the specified matrix.
     *
     * This constructor may fail, particularly if the scale component of the
     * matrix is too small.  In that case, this method intializes this
     * quaternion to the zero quaternion.
     *
     * @param m The matrix to extract the rotation.
     */
    explicit Quaternion(const Mat4& m);

    /**
     * Sets quaternion equal to the rotational part of the specified matrix.
     *
     * This constructor may fail, particularly if the scale component of the
     * matrix is too small.  In that case, this method intializes this
     * quaternion to the zero quaternion.
     *
     * @param m The matrix to extract the rotation.
     *
     * @return A reference to this (modified) Qauternion for chaining.
     */
    Quaternion& operator= (const Mat4& m) {
        return set(m);
    }

    /**
     * Sets quaternion equal to the rotational part of the specified matrix.
     *
     * This constructor may fail, particularly if the scale component of the
     * matrix is too small.  In that case, this method intializes this
     * quaternion to the zero quaternion.
     *
     * @param m The matrix to extract the rotation.
     *
     * @return A reference to this (modified) Qauternion for chaining.
     */
    Quaternion& set(const Mat4& m);
    
};

/**
 * Rotates the vector in place by the quaternion.
 *
 * The rotation is defined by the matrix associated with the vector.  As per
 * the convention for {@link Mat4}, we only allow vectors to be multiplied
 * on the right by quaternions.
 *
 * @param v     The vector to rotate.
 * @param quat  The rotation quaternion.
 *
 * @return a reference to the vector, after rotation.
 */
inline Vec3& operator*=(Vec3& v, const Quaternion& quat) {
    return *(Quaternion::rotate(v,quat,&v));
}
    
/**
 * Returns a copy of the vector rotated by the quaternion.
 *
 * The rotation is defined by the matrix associated with the vector.  As per
 * the convention for {@link Mat4}, we only allow vectors to be multiplied 
 * on the right by quaternions.
 *
 * @param v     The vector to rotate.
 * @param quat  The rotation quaternion.
 *
 * @return a copy of the vector rotated by the quaternion.
 */
inline const Vec3 operator*(const Vec3& v, const Quaternion& quat) {
    Vec3 result;
    return *(Quaternion::rotate(v,quat,&result));
}
    
/**
 * Returns the scalar product of the quaternion with the given value.
 *
 *
 * @param s     The value to scale by.
 * @param quat  The quaternion to scale.
 *
 * @return The scalar product of this quaternion with the given value.
*/
inline const Quaternion operator*(float s, const Quaternion& quat) {
    Quaternion result;
    return *(Quaternion::scale(quat,s,&result));
}


}

#endif /* __CU_QUATERNION_H__ */

//
//  CUQuaternion.cpp
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
#include <SDL/SDL.h>
#include <sstream>
#include <cugl/util/CUDebug.h>
#include <cugl/util/CUStrings.h>
#include <cugl/math/CUQuaternion.h>
#include <cugl/math/CUVec3.h>
#include <cugl/math/CUVec4.h>
#include <cugl/math/CUMat4.h>
#include "cuACC128.inl"

using namespace cugl;


#pragma mark -
#pragma mark Constructors
/**
 * Constructs a new quaternion from the values in the specified array.
 *
 * The elements of the array are in the order x, y, z, and w.
 *
 * @param array An array containing the elements of the quaternion.
 */
Quaternion::Quaternion(float* array) {
    CUAssertLog(array, "Source array is null");
    x = array[0]; y = array[1]; z = array[2]; w = array[3];
}

/**
 * Constructs a quaternion equal to the rotation from the specified axis and angle.
 *
 * @param axis  A vector describing the axis of rotation.
 * @param angle The angle of rotation (in radians).
 */
Quaternion::Quaternion(const Vec3& axis, float angle) {
    createFromAxisAngle(axis, angle, this);
}


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
Quaternion* Quaternion::createFromRotationMatrix(const Mat4& m, Quaternion* dst) {
    CUAssertLog(dst, "Assignment pointer is null");
    Mat4::decompose(m, nullptr, dst, nullptr);
    return dst;
}

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
Quaternion* Quaternion::createFromAxisAngle(const Vec3& axis, float angle, Quaternion* dst) {
    CUAssertLog(dst, "Assignment pointer is null");
    float halfAngle = angle * 0.5f;
    float sinHalfAngle = sinf(halfAngle);
    
    Vec4 normal(axis,0.0f);
    normal.normalize();
    *dst = normal*sinHalfAngle;
    dst->w = cosf(halfAngle);
    return dst;
}


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
Quaternion& Quaternion::set(const float* array) {
    CUAssertLog(array, "Source array is null");
    x = array[0]; y = array[1]; z = array[2]; w = array[3];
    return *this;
}

/**
 * Sets the quaternion equal to the rotation from the specified axis and angle.
 *
 * @param axis  The axis of rotation.
 * @param angle The angle of rotation (in radians).
 *
 * @return A reference to this (modified) Quaternion for chaining.
 */
Quaternion& Quaternion::set(const Vec3& axis, float angle) {
    return *(createFromAxisAngle(axis, angle, this));
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
Quaternion* Quaternion::add(const Quaternion& q1, const Quaternion& q2, Quaternion* dst) {
    CUAssertLog(dst, "Assignment pointer is null");
#if defined CU_MATH_VECTOR_SSE
    dst->v = _mm_add_ps(q1.v,q2.v);
#elif defined CU_MATH_VECTOR_NEON64
    dst->v = vaddq_f32(q1.v,q2.v);
#else
    dst->x = q1.x+q2.x; dst->y = q1.y+q2.y; dst->z = q1.z+q2.z; dst->w = q1.w+q2.w;
#endif
    return dst;
}

/**
 * Subtacts the quaternions q2 from q1 and stores the result in dst.
 *
 * @param q1    The first quaternion.
 * @param q2    The second quaternion.
 * @param dst   A quaternion to store the result in.
 *
 * @return A reference to dst for chaining
 */
Quaternion* Quaternion::subtract(const Quaternion& q1, const Quaternion& q2, Quaternion* dst) {
    CUAssertLog(dst, "Assignment pointer is null");
#if defined CU_MATH_VECTOR_SSE
    dst->v = _mm_sub_ps(q1.v,q2.v);
#elif defined CU_MATH_VECTOR_NEON64
    dst->v = vsubq_f32(q1.v,q2.v);
#else
    dst->x = q1.x-q2.x; dst->y = q1.y-q2.y; dst->z = q1.z-q2.z; dst->w = q1.w-q2.w;
#endif
    return dst;
}

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
Quaternion* Quaternion::multiply(const Quaternion& q1, const Quaternion& q2, Quaternion* dst) {
    CUAssertLog(dst, "Assignment pointer is null");
#if defined CU_MATH_VECTOR_SSE
    __m128 tmp = _mm_mul_ps(_mm_set1_ps(q1.w),q2.v);
    tmp = _mm_fmadd_ps(_mm_setr_ps(q1.x,-q1.x,q1.x,-q1.x),_mm_shuffle_ps(q2.v,q2.v,_MM_SHUFFLE(0,1,2,3)),tmp);
    tmp = _mm_fmadd_ps(_mm_setr_ps(q1.y,q1.y,-q1.y,-q1.y),_mm_shuffle_ps(q2.v,q2.v,_MM_SHUFFLE(1,0,3,2)),tmp);
    tmp = _mm_fmadd_ps(_mm_setr_ps(-q1.z,q1.z,q1.z,-q1.z),_mm_shuffle_ps(q2.v,q2.v,_MM_SHUFFLE(2,3,0,1)),tmp);
    dst->v = tmp;
#else
    float x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
    float y = q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x;
    float z = q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w;
    float w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
    
    dst->x = x;
    dst->y = y;
    dst->z = z;
    dst->w = w;
#endif
    return dst;
}

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
Quaternion* Quaternion::divide(const Quaternion& q1, const Quaternion& q2, Quaternion* dst) {
    Quaternion inverse;
    invert(q2,&inverse);
    return multiply(q1,inverse,dst);
}

/**
 * Scales the specified quaternion by s and stores the result in dst.
 *
 * @param q1    The first quaternion.
 * @param s     The scalar value.
 * @param dst   A quaternion to store the result in.
 *
 * @return A reference to dst for chaining
 */
Quaternion* Quaternion::scale(const Quaternion& q1, float s, Quaternion* dst) {
    CUAssertLog(dst, "Assignment pointer is null");
#if defined CU_MATH_VECTOR_SSE
    dst->v = _mm_mul_ps(q1.v,_mm_set1_ps(s));
#elif defined CU_MATH_VECTOR_NEON64
    dst->v = vmulq_n_f32(q1.v,(float32_t)s);
#else
    dst->x = q1.x*s; dst->y = q1.y*s; dst->z = q1.z*s; dst->w = q1.w*s;
#endif
    return dst;
}

/**
 * Conjugates the specified quaternion and stores the result in dst.
 *
 * @param q1    The quaternion to conjugate.
 * @param dst   A quaternion to store the result in.
 *
 * @return A reference to dst for chaining
 */
Quaternion* Quaternion::conjugate(const Quaternion& quat, Quaternion* dst) {
    CUAssertLog(dst, "Assignment pointer is null");
#if defined CU_MATH_VECTOR_SSE
    dst->v = _mm_sub_ps(_mm_setzero_ps(),quat.v);
    dst->w = quat.w;
#elif defined CU_MATH_VECTOR_NEON64
    dst->v = vnegq_f32(quat.v);
    dst->w = quat.w;
#else
    dst->x = -quat.x; dst->y = -quat.y; dst->z = -quat.z; dst->w = quat.w;
#endif
    return dst;
}

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
 * @param q1    The quaternion to invert.
 * @param dst   A quaternion to store the result in.
 *
 * @return A reference to dst for chaining
 */
Quaternion* Quaternion::invert(const Quaternion& quat, Quaternion* dst) {
    CUAssertLog(dst, "Assignment pointer is null");
#if defined CU_MATH_VECTOR_SSE
    float n = _mm_dp_ps(quat.v,quat.v,0xF1)[0];
    n = (n < CU_MATH_FLOAT_SMALL ? 1.0f : 1.0f/n);
    dst->v = _mm_mul_ps(quat.v,_mm_setr_ps(-n,-n,-n,n));
#elif defined CU_MATH_VECTOR_NEON64
    float n = vdotq_f32(quat.v,quat.v);
    n = -(n < CU_MATH_FLOAT_SMALL ? 1.0f : 1.0f/n);
    dst->v = vmulq_f32(quat.v,vld1q_dup_f32(&n));
    dst->w = -dst->w;
#else
    float n = quat.normSquared();
    n = (n < CU_MATH_FLOAT_SMALL ? NAN : 1.0f/n);
    dst->x = -quat.x*n; dst->y = -quat.y*n; dst->z = -quat.z*n; dst->w = quat.w*n;
#endif
    return dst;
}

/**
 * Normalizes the specified quaternion and stores the result in dst.
 *
 * If the quaternion already has unit length or if the length of the
 *  quaternion is zero, this method copies quat into dst.
 *
 * @param q1    The quaternion to normalize.
 * @param dst   A quaternion to store the result in.
 *
 * @return A reference to dst for chaining
 */
Quaternion* Quaternion::normalize(const Quaternion& quat, Quaternion* dst) {
    CUAssertLog(dst, "Assignment pointer is null");
#if defined CU_MATH_VECTOR_SSE
    float n = sqrt(_mm_dp_ps(quat.v,quat.v,0xF1)[0]);
    n = (n < CU_MATH_FLOAT_SMALL ? 1.0f : 1.0f/n);
    dst->v = _mm_mul_ps(quat.v,_mm_set1_ps(n));
#elif defined CU_MATH_VECTOR_NEON64
    float n = sqrt(vdotq_f32(quat.v,quat.v));
    n = (n < CU_MATH_FLOAT_SMALL ? 1.0f : 1.0f/n);
    dst->v = vmulq_f32(quat.v,vld1q_dup_f32(&n));
#else
    float n = quat.norm();
    n = (n < CU_MATH_EPSILON ? NAN : (n == 1.0f ? 1.0f : 1.0f/n));
    dst->x = quat.x*n; dst->y = quat.y*n; dst->z = quat.z*n; dst->w = quat.w*n;
#endif
    return dst;
}

/**
 * Negates the specified quaternion and stores the result in dst.
 *
 * @param q1    The quaternion to negate.
 * @param dst   A quaternion to store the result in.
 *
 * @return A reference to dst for chaining
 */
Quaternion* Quaternion::negate(const Quaternion& quat, Quaternion* dst) {
    CUAssertLog(dst, "Assignment pointer is null");
#if defined CU_MATH_VECTOR_SSE
    dst->v = _mm_sub_ps(_mm_setzero_ps(),quat.v);
#elif defined CU_MATH_VECTOR_NEON64
    dst->v = vnegq_f32(quat.v);
#else
    dst->x = -quat.x; dst->y = -quat.y; dst->z = -quat.z; dst->w = -quat.w;
#endif
    return dst;
}

/**
 * Returns the dot product of the two quaternions
 *
 * @param q1    The first quaternion.
 * @param q2    The second quaternion.
 *
 * @return The dot product.
 */
float Quaternion::dot(const Quaternion& q1, const Quaternion& q2) {
#if defined CU_MATH_VECTOR_SSE
    return _mm_dp_ps(q1.v,q2.v,0xF1)[0];
#elif defined CU_MATH_VECTOR_NEON64
    return vdotq_f32(q1.v,q2.v);
#else
    return (q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w);
#endif
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
bool Quaternion::operator==(const Quaternion& q) const {
#if defined CU_MATH_VECTOR_SSE
    return _mm_movemask_ps(_mm_cmpeq_ps(v,q.v)) == 0x0F;
#elif defined CU_MATH_VECTOR_NEON64
    return vmaskq_f32(vceqq_f32(v,q.v)) == 0x0F;
#else
    return x == q.x && y == q.y && z == q.z && w == q.w;
#endif
}

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
bool Quaternion::operator!=(const Quaternion& q) const {
#if defined CU_MATH_VECTOR_SSE
    return _mm_movemask_ps(_mm_cmpeq_ps(v,q.v)) != 0x0F;
#elif defined CU_MATH_VECTOR_NEON64
    return vmaskq_f32(vceqq_f32(v,q.v)) != 0x0F;
#else
    return x != q.x || y != q.y || z != q.z || w != q.w;
#endif
}

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
bool Quaternion::equals(const Quaternion& q, float variance) const {
#if defined CU_MATH_VECTOR_SSE
    __m128 diff = _mm_max_ps(_mm_sub_ps(v,q.v),_mm_sub_ps(q.v,v));
    return _mm_movemask_ps(_mm_cmplt_ps(diff,_mm_set1_ps(variance))) == 0x0F;
#elif defined CU_MATH_VECTOR_NEON64
    return vmaskq_f32(vcltq_f32(vabdq_f32(v, q.v),vld1q_dup_f32(&variance))) == 0x0F;
#else
    bool a = fabsf(x-q.x) < variance;
    bool b = fabsf(y-q.y) < variance;
    bool c = fabsf(z-q.z) < variance;
    bool d = fabsf(w-q.w) < variance;
    return a && b && c && d;
#endif
}


#pragma mark -
#pragma mark Linear Attributes

/**
 * Converts this Quaternion4f to axis-angle notation.
 *
 * The angle is in radians.  The axis is normalized.
 *
 * @param e The Vec3f which stores the axis.
 *
 * @return The angle (in radians).
 */
float Quaternion::toAxisAngle(Vec3* e) const {
    CUAssertLog(e, "Assignment pointer is null");
    
    Quaternion q(x, y, z, w);
    q.normalize();
    e->x = q.x;
    e->y = q.y;
    e->z = q.z;
    e->normalize();
    
    return (2.0f * acosf(q.w));
}

/**
 * Returns true this quaternion contains all zeros.
 *
 * Comparison is exact, which may be unreliable given that the attributes
 * are floats.
 *
 * @return true if this quaternion contains all zeros, false otherwise.
 */
bool Quaternion::isZero() const {
#if defined CU_MATH_VECTOR_SSE
    return _mm_movemask_ps(_mm_cmpeq_ps(v,_mm_setzero_ps())) == 0x0F;
#elif defined CU_MATH_VECTOR_NEON64
    return vmaskq_f32(vceqq_f32(v,vmovq_n_f32(0.0f))) == 0x0F;
#else
    return x == 0.0f && y == 0.0f && z == 0.0f && w == 0.0f;
#endif
}

/**
 * Returns true if this quaternion is the identity.
 *
 * @return true if this quaternion is the identity.
 */
bool Quaternion::isIdentity() const {
#if defined CU_MATH_VECTOR_SSE
    __m128 tmp = _mm_setr_ps(0.0f,0.0f,0.0f,1.0f);
    return _mm_movemask_ps(_mm_cmpeq_ps(v,tmp)) == 0x0F;
#elif defined CU_MATH_VECTOR_NEON64
    float32x4_t tmp = {0.0f,0.0f,0.0f,1.0f};
    return vmaskq_f32(vceqq_f32(v,tmp)) == 0x0F;
#else
    return x == 0.0f && y == 0.0f && z == 0.0f && w == 1.0f;
#endif
}

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
Quaternion* Quaternion::lerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion* dst) {
    CUAssertLog(dst, "Assignment pointer is null");
    CUAssertLog(!(t < 0.0f || t > 1.0f), "Interpolation coefficient out of range: %.3f", t);
    *dst = q1+t*(q2-q1);
    return dst;
}

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
 * Note: For accurate interpolation, the input quaternions must be at
 * (or close to) unit length. This method does not automatically normalize
 * the input quaternions, so it is up to the caller to ensure they call
 * normalize beforehand, if necessary.
 *
 * Code adapted from
 * http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/slerp/
 *
 * @param q1    The first quaternion.
 * @param q2    The second quaternion.
 * @param t     The interpolation coefficient in [0,1].
 * @param dst   A quaternion to store the result in.
 *
 * @return A reference to dst for chaining
 */
Quaternion* Quaternion::slerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion* dst) {
    CUAssertLog(dst, "Assignment pointer is null");
    CUAssertLog(!(t < 0.0f || t > 1.0f), "Interpolation coefficient out of range: %.3f", t);
    CUAssertLog(q1.isUnit(), "First quaternion is not a unit quaternion");
    CUAssertLog(q2.isUnit(), "Second quaternion is not a unit quaternion");

    // Calculate angle between them.
    float cosHalfTheta = dot(q1,q2);
    
    // if qa=qb or qa=-qb then theta = 0 and we can return qa
    if (fabsf(cosHalfTheta) >= 1.0){
        *dst = q1;
        return dst;
    }
    // Calculate temporary values.
    float halfTheta = acosf(cosHalfTheta);
    float sinHalfTheta = sqrtf(1.0f - cosHalfTheta*cosHalfTheta);
    
    // if theta = 180 degrees then result is not fully defined
    // we could rotate around any axis normal to qa or qb
    const float ANGLE_THRESH = 0.001f;
    if (fabsf(sinHalfTheta) < ANGLE_THRESH) {
        *dst = (q1+q2)*0.5f;
        return dst;
    }
    float ratioA = sinf((1 - t) * halfTheta) / sinHalfTheta;
    float ratioB = sinf(t * halfTheta) / sinHalfTheta;

    //calculate Quaternion.
    *dst = q1*ratioA+q2*ratioB;
    return dst;
}

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
Quaternion* Quaternion::nlerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion* dst) {
    CUAssertLog(q1.isUnit(), "First quaternion is not a unit quaternion");
    CUAssertLog(q2.isUnit(), "Second quaternion is not a unit quaternion");
    lerp(q1,q2,t,dst);
    dst->normalize();
    return dst;
}

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
Vec3* Quaternion::rotate(const Vec3& v, const Quaternion& quat, Vec3* dst) {
    Vec4 uv, uuv;
    Vec4 qvec = quat;
    Vec4 vvec(v,0.0f);
    qvec.w = 0.0f;
    
    Vec4::cross(qvec, vvec, &uv);
    Vec4::cross(qvec, uv,  &uuv);
    //Vec3 uv, uuv;
    //Vec3 qvec(quat.x,quat.y, quat.z);
    //Vec3::cross(qvec, v, &uv);
    //Vec3::cross(qvec, uv, &uuv);

    uv  *= (2.0f * quat.w);
    uuv *= 2.0f;
    vvec = vvec + uv + uuv;
    dst->x = vvec.x; dst->y = vvec.y; dst->z = vvec.z;
    return dst;
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
std::string Quaternion::toString(bool verbose) const {
    std::stringstream ss;
    ss << (verbose ? "cugl::Quaternion[" : "");
    ss <<  to_string(w);
    ss << "+";
    ss << to_string(x);
    ss << "i+";
    ss << to_string(y);
    ss << "j+";
    ss << to_string(z);
    ss << (verbose ? "k]" : "k");
    return ss.str();
}

/** Cast from Quaternion to a Vec4. */
Quaternion::operator Vec4() const {
    return Vec4(x,y,z,w);
}

/**
 * Sets the coordinates of this quaternion to those of the given vector.
 *
 * @param vector The vector to convert
 *
 * @return A reference to this (modified) Qauternion for chaining.
 */
Quaternion& Quaternion::set(const Vec4& vector) {
    x = vector.x; y = vector.y; z = vector.z; w = vector.w;
    return *this;
}

/**
 * Cast from Quaternion to a Matrix.
 *
 * The matrix is a rotation matrix equivalent to the rotation represented
 * by this quaternion.
 */
Quaternion::operator Mat4() const {
    Mat4 result(*this);
    return result;
}

/**
 * Constructs a quaternion equal to the rotational part of the specified matrix.
 *
 * This constructor may fail, particularly if the scale component of the
 * matrix is too small.  In that case, this method intializes this
 * quaternion to the zero quaternion.
 *
 * @param m The matrix to extract the rotation.
 */
Quaternion::Quaternion(const Mat4& m) {
    createFromRotationMatrix(m, this);
}

/**
 * Sets the quaternion equal to the rotational part of the specified matrix.
 *
 * This constructor may fail, particularly if the scale component of the
 * matrix is too small.  In that case, this method intializes this
 * quaternion to the zero quaternion.
 *
 * @param m The matrix.
 *
 * @return A reference to this (modified) Quaternion for chaining.
 */
Quaternion& Quaternion::set(const Mat4& m) {
    return *(createFromRotationMatrix(m, this));
}

#pragma mark -
#pragma mark Constants

/** The zero quaternion Quaternion(0,0,0,0) */
const Quaternion Quaternion::ZERO(0,0,0,0);
/** The identity quaternion Quaternion(0,0,0,1) */
const Quaternion Quaternion::IDENTITY(0,0,0,1);


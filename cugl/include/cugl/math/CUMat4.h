//
//  CUMat4.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a 4d matrix, which is the standard transform
//  matrix in OpenGL.  The class has support for basic camera creation, as well
//  as the traditional transforms.  It can transform any of Vec2, Vec3, and Vec4.
//
//  This version (2018) no longer supports manual vectorization for AVX, Neon.
//  Because these matrices are small, the compiler seems to be able to optimize
//  the naive code better.  Naive code with -O3 outperforms the manual vectorization
//  by almost a full order of magnitude.
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
//  Version: 4/3/18

#ifndef __CU_MAT4_H__
#define __CU_MAT4_H__

#include <math.h>
#include <assert.h>
#include "CUMathBase.h"
#include "CUVec2.h"
#include "CUVec3.h"
#include "CUVec4.h"
#include "CURect.h"

namespace cugl {

// Forward references
class Quaternion;
class Affine2;
    
/**
 * This class defines a 4 x 4 floating point matrix representing a 3D transformation.
 *
 * Vectors are treated as columns, resulting in a matrix that is represented as 
 * follows, where x, y and z are the translation components of the matrix:
 *
 *     1  0  0  x
 *     0  1  0  y
 *     0  0  1  z
 *     0  0  0  1
 *
 * This matrix class is directly compatible with OpenGL since its elements are
 * laid out in memory exactly as they are expected by OpenGL.
 *
 * The matrix uses column-major format such that array indices increase down 
 * column first. However, this is only a data representation format, and it 
 * should not have any affect on issues such as multiplication order.
 *
 * With that said, the convention in OpenGL (and respected by this class) 
 * is that transforms are applied by multiplying a vector on the right. For 
 * example, suppose we have a translation matrix T and a rotation matrix R.
 * To first rotate an object around the origin and then translate it, you would 
 * multiply the two matrices as RT, with T on the right.
 */
class Mat4 {
#pragma mark Values
public:
#if defined CU_MATH_VECTOR_SSE
	__attribute__((__aligned__(16))) union {
        __m128 col[4];
        float m[16];
    };
#elif defined CU_MATH_VECTOR_NEON64
    __attribute__((__aligned__(16))) union {
        float32x4_t col[4];
        float m[16];
    };
#else
	/** The underlying matrix elements */
    float m[16];
#endif
    
    /** The matrix with all zeroes */
    static const Mat4 ZERO;
    /** The matrix with all ones */
    static const Mat4 ONE;
    /** The identity matrix (ones on the diagonal) */
    static const Mat4 IDENTITY;

    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates the identity matrix.
     *
     *     1  0  0  0
     *     0  1  0  0
     *     0  0  1  0
     *     0  0  0  1
     */
    Mat4();
    
    /**
     * Constructs a matrix initialized to the specified values.
     *
     * @param m11 The first element of the first row.
     * @param m12 The second element of the first row.
     * @param m13 The third element of the first row.
     * @param m14 The fourth element of the first row.
     * @param m21 The first element of the second row.
     * @param m22 The second element of the second row.
     * @param m23 The third element of the second row.
     * @param m24 The fourth element of the second row.
     * @param m31 The first element of the third row.
     * @param m32 The second element of the third row.
     * @param m33 The third element of the third row.
     * @param m34 The fourth element of the third row.
     * @param m41 The first element of the fourth row.
     * @param m42 The second element of the fourth row.
     * @param m43 The third element of the fourth row.
     * @param m44 The fourth element of the fourth row.
     */
    Mat4(float m11, float m12, float m13, float m14,
         float m21, float m22, float m23, float m24,
         float m31, float m32, float m33, float m34,
         float m41, float m42, float m43, float m44);
    
    /**
     * Creates a matrix initialized to the specified column-major array.
     *
     * The passed-in array is in column-major order, so the memory layout of 
     * the array is as follows:
     *
     *     0   4   8   12
     *     1   5   9   13
     *     2   6   10  14
     *     3   7   11  15
     *
     * @param mat An array containing 16 elements in column-major order.
     */
    Mat4(const float* mat);
    
    /**
     * Constructs a new matrix that is the copy of the specified one.
     *
     * @param copy The matrix to copy.
     */
    Mat4(const Mat4& copy);

    /**
     * Constructs a new matrix that contains the resources of the specified one.
     *
     * @param copy The matrix contributing resources.
     */
    Mat4(Mat4&& copy);
    
    /**
     * Constructs a new matrix that is specified by the given quaternion.
     *
     * @param rotation The quaternion specifying a rotation.
     */
    Mat4(const Quaternion& rotation) {
        createRotation(rotation, this);
    }
    
    /**
     * Destroys this matrix, releasing all resources.
     */
    ~Mat4() {}
 
    
#pragma mark -
#pragma mark Static Constructors
    /**
     * Creates a view matrix based on the specified input vectors.
     *
     * @param eye       The eye position.
     * @param target    The target's center position.
     * @param up        The up vector.
     *
     * @return a view matrix based on the specified input vectors.
     */
    static Mat4 createLookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
        Mat4 result;
        return *(createLookAt(eye,target,up,&result));
    }

    
    /**
     * Creates a view matrix based on the specified input vectors, putting it in dst.
     *
     * @param eye       The eye position.
     * @param target    The target's center position.
     * @param up        The up vector.
     * @param dst       A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* createLookAt(const Vec3& eye, const Vec3& target, const Vec3& up, Mat4* dst);
    
    /**
     * Returns a view matrix based on the specified input parameters.
     *
     * @param eyeX      The eye x-coordinate position.
     * @param eyeY      The eye y-coordinate position.
     * @param eyeZ      The eye z-coordinate position.
     * @param targetX   The target's center x-coordinate position.
     * @param targetY   The target's center y-coordinate position.
     * @param targetZ   The target's center z-coordinate position.
     * @param upX       The up vector x-coordinate value.
     * @param upY       The up vector y-coordinate value.
     * @param upZ       The up vector z-coordinate value.
     *
     * @return a view matrix based on the specified input parameters.
     */
    static Mat4 createLookAt(float eyeX,    float eyeY,     float eyeZ,
                             float targetX, float targetY,  float targetZ,
                             float upX,     float upY,      float upZ) {
        Mat4 result;
        return *(createLookAt(eyeX, eyeY, eyeZ,
                              targetX, targetY, targetZ,
                              upX, upY, upZ, &result));
    }
    
    /**
     * Creates a view matrix based on the specified input parameters, putting it in dst.
     *
     * @param eyeX      The eye x-coordinate position.
     * @param eyeY      The eye y-coordinate position.
     * @param eyeZ      The eye z-coordinate position.
     * @param targetX   The target's center x-coordinate position.
     * @param targetY   The target's center y-coordinate position.
     * @param targetZ   The target's center z-coordinate position.
     * @param upX       The up vector x-coordinate value.
     * @param upY       The up vector y-coordinate value.
     * @param upZ       The up vector z-coordinate value.
     * @param dst       A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* createLookAt(float eyeX,    float eyeY,     float eyeZ,
                              float targetX, float targetY,  float targetZ,
                              float upX,     float upY,      float upZ,      Mat4* dst);
    

    /**
     * Returns a perspective projection matrix based on a field of view.
     *
     * Projection space refers to the space after applying projection
     * transformation from view space. After the projection transformation,
     * visible content has x- and y-coordinates ranging from -1 to 1, and a
     * z-coordinate ranging from 0 to 1. To obtain the viewable area
     * (in world space) of a scene, create a bounding frustum and pass the
     * combined view and projection matrix to the constructor.
     *
     * @param fieldOfView   The field of view in the y direction (in degrees).
     * @param aspectRatio   The aspect ratio, defined as view space width divided by height.
     * @param zNearPlane    The distance to the near view plane.
     * @param zFarPlane     The distance to the far view plane.
     *
     * @return a perspective projection matrix based on a field of view.
     */
    static Mat4 createPerspective(float fieldOfView, float aspectRatio,
                                  float zNearPlane, float zFarPlane) {
        Mat4 result;
        return *(createPerspective(fieldOfView, aspectRatio, zNearPlane, zFarPlane, &result));
    }

    /**
     * Creates a perspective projection matrix based on a field of view, putting it in dst.
     *
     * Projection space refers to the space after applying projection 
     * transformation from view space. After the projection transformation, 
     * visible content has x- and y-coordinates ranging from -1 to 1, and a 
     * z-coordinate ranging from 0 to 1. To obtain the viewable area 
     * (in world space) of a scene, create a bounding frustum and pass the 
     * combined view and projection matrix to the constructor.
     *
     * @param fieldOfView   The field of view in the y direction (in degrees).
     * @param aspectRatio   The aspect ratio, defined as view space width divided by height.
     * @param zNearPlane    The distance to the near view plane.
     * @param zFarPlane     The distance to the far view plane.
     * @param dst           A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* createPerspective(float fieldOfView, float aspectRatio,
                                   float zNearPlane, float zFarPlane, Mat4* dst);
    
    /**
     * Returns an orthographic projection matrix anchored at the origin.
     *
     * Projection space refers to the space after applying projection
     * transformation from view space. After the projection transformation,
     * visible content has x and y coordinates ranging from -1 to 1, and z
     * coordinates ranging from 0 to 1. Unlike perspective projection, there is
     * no perspective foreshortening in orthographic projection.
     *
     * The viewable area of this orthographic projection places the origin at
     * the center, with the given width and height..  The z-axis is bound between
     * zNearPlane and zFarPlane. These values are relative to the position and
     * x, y, and z-axes of the view.
     *
     * To obtain the viewable area (in world space) of a scene, create a
     * bounding frustum and pass the combined view and projection matrix to
     * the constructor.
     *
     * @param width         The width of the view.
     * @param height        The height of the view.
     * @param zNearPlane    The minimum z-value of the view volume.
     * @param zFarPlane     The maximum z-value of the view volume.
     *
     * @return an orthographic projection matrix anchored at the origin.
     */
    static Mat4 createOrthographic(float width, float height,
                                   float zNearPlane, float zFarPlane) {
        Mat4 result;
        return *(createOrthographic(width, height, zNearPlane, zFarPlane, &result));
    }

    
    /**
     * Creates an orthographic projection matrix anchored at the origin, putting it in dst.
     *
     * Projection space refers to the space after applying projection 
     * transformation from view space. After the projection transformation, 
     * visible content has x and y coordinates ranging from -1 to 1, and z 
     * coordinates ranging from 0 to 1. Unlike perspective projection, there is
     * no perspective foreshortening in orthographic projection.
     *
     * The viewable area of this orthographic projection places the origin at
     * the center, with the given width and height..  The z-axis is bound between
     * zNearPlane and zFarPlane. These values are relative to the position and
     * x, y, and z-axes of the view.
     *
     * To obtain the viewable area (in world space) of a scene, create a 
     * bounding frustum and pass the combined view and projection matrix to
     * the constructor.
     *
     * @param width         The width of the view.
     * @param height        The height of the view.
     * @param zNearPlane    The minimum z-value of the view volume.
     * @param zFarPlane     The maximum z-value of the view volume.
     * @param dst           A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* createOrthographic(float width, float height,
                                    float zNearPlane, float zFarPlane, Mat4* dst) {
        float halfWidth = width / 2.0f;
        float halfHeight = height / 2.0f;
        return createOrthographicOffCenter(-halfWidth, halfWidth, -halfHeight, halfHeight,
                                           zNearPlane, zFarPlane, dst);
    }
    
    /**
     * Returns an orthographic projection matrix.
     *
     * Projection space refers to the space after applying projection
     * transformation from view space. After the projection transformation,
     * visible content has x and y coordinates ranging from -1 to 1, and z
     * coordinates ranging from 0 to 1. Unlike perspective projection, there is
     * no perspective foreshortening in orthographic projection.
     *
     * The viewable area of this orthographic projection extends from left to
     * right on the x-axis and bottom to top on the y-axis. The z-axis is bound
     * between zNearPlane and zFarPlane. These values are relative to the
     * position and x, y, and z-axes of the view.
     *
     * To obtain the viewable area (in world space) of a scene, create a
     * bounding frustum and pass the combined view and projection matrix to
     * the constructor.
     *
     * @param left The minimum x-value of the view volume.
     * @param right The maximum x-value of the view volume.
     * @param bottom The minimum y-value of the view volume.
     * @param top The maximum y-value of the view volume.
     * @param zNearPlane The minimum z-value of the view volume.
     * @param zFarPlane The maximum z-value of the view volume.
     *
     * @return an orthographic projection matrix.
     */
    static Mat4 createOrthographicOffCenter(float left, float right, float bottom, float top,
                                            float zNearPlane, float zFarPlane) {
        Mat4 result;
        return *(createOrthographicOffCenter(left, right, bottom, top, zNearPlane, zFarPlane, &result));
    }

    
    /**
     * Creates an orthographic projection matrix, putting it in dst.
     *
     * Projection space refers to the space after applying projection
     * transformation from view space. After the projection transformation,
     * visible content has x and y coordinates ranging from -1 to 1, and z
     * coordinates ranging from 0 to 1. Unlike perspective projection, there is
     * no perspective foreshortening in orthographic projection.
     *
     * The viewable area of this orthographic projection extends from left to 
     * right on the x-axis and bottom to top on the y-axis. The z-axis is bound 
     * between zNearPlane and zFarPlane. These values are relative to the 
     * position and x, y, and z-axes of the view.
     *
     * To obtain the viewable area (in world space) of a scene, create a
     * bounding frustum and pass the combined view and projection matrix to
     * the constructor.
     *
     * @param left The minimum x-value of the view volume.
     * @param right The maximum x-value of the view volume.
     * @param bottom The minimum y-value of the view volume.
     * @param top The maximum y-value of the view volume.
     * @param zNearPlane The minimum z-value of the view volume.
     * @param zFarPlane The maximum z-value of the view volume.
     * @param dst A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* createOrthographicOffCenter(float left, float right, float bottom, float top,
                                             float zNearPlane, float zFarPlane, Mat4* dst);

    /**
     * Returns a uniform scale matrix.
     *
     * @param scale The amount to scale.
     *
     * @return a uniform scale matrix.
     */
    static Mat4 createScale(float scale) {
        Mat4 result;
        return *(createScale(scale,&result));
    }
    
    /**
     * Creates a uniform scale matrix, putting it in dst.
     *
     * @param scale The amount to scale.
     * @param dst   A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* createScale(float scale, Mat4* dst);

    /**
     * Returns a nonuniform scale matrix.
     *
     * @param sx    The amount to scale along the x-axis.
     * @param sy    The amount to scale along the y-axis.
     * @param sz    The amount to scale along the z-axis.
     *
     * @return a nonuniform scale matrix.
     */
    static Mat4 createScale(float sx, float sy, float sz) {
        Mat4 result;
        return *(createScale(sx,sy,sz,&result));
    }

    /**
     * Creates a nonuniform scale matrix, putting it in dst.
     *
     * @param sx    The amount to scale along the x-axis.
     * @param sy    The amount to scale along the y-axis.
     * @param sz    The amount to scale along the z-axis.
     * @param dst   A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* createScale(float sx, float sy, float sz, Mat4* dst);

    /**
     * Returns a nonuniform scale matrix from the given vector.
     *
     * @param scale The nonuniform scale value.
     *
     * @return a nonuniform scale matrix from the given vector.
     */
    static Mat4 createScale(const Vec3& scale) {
        Mat4 result;
        return *(createScale(scale,&result));
    }

    /**
     * Creates a nonuniform scale matrix from the given vector, putting it in dst.
     *
     * @param scale The nonuniform scale value.
     * @param dst   A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* createScale(const Vec3& scale, Mat4* dst);
  
    /**
     * Returns a rotation matrix from the specified quaternion.
     *
     * @param quat  A quaternion describing a 3D orientation.
     *
     * @return a rotation matrix from the specified quaternion.
     */
    static Mat4 createRotation(const Quaternion& quat) {
        Mat4 result;
        return *(createRotation(quat,&result));
    }
    
    /**
     * Creates a rotation matrix from the specified quaternion, putting it in dst.
     *
     * @param quat  A quaternion describing a 3D orientation.
     * @param dst   A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* createRotation(const Quaternion& quat, Mat4* dst);
    
    /**
     * Returns a rotation matrix from the specified axis and angle.
     *
     * The angle measurement is in radians.  The rotation is counter
     * clockwise about the axis.
     *
     * @param axis  A vector describing the axis to rotate about.
     * @param angle The angle (in radians).
     *
     * @return a rotation matrix from the specified axis and angle.
     */
    static Mat4 createRotation(const Vec3& axis, float angle) {
        Mat4 result;
        return *(createRotation(axis,angle,&result));
    }
    
    /**
     * Creates a rotation matrix from the specified axis and angle, putting it in dst.
     *
     * The angle measurement is in radians.  The rotation is counter
     * clockwise about the axis.
     *
     * @param axis  A vector describing the axis to rotate about.
     * @param angle The angle (in radians).
     * @param dst   A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* createRotation(const Vec3& axis, float angle, Mat4* dst);
    
    /**
     * Returns a matrix specifying a rotation around the x-axis.
     *
     * The angle measurement is in radians.  The rotation is counter
     * clockwise about the axis.
     *
     * @param angle The angle of rotation (in radians).
     *
     * @return a matrix specifying a rotation around the x-axis.
     */
    static Mat4 createRotationX(float angle) {
        Mat4 result;
        return *(createRotationX(angle,&result));
    }

    /**
     * Creates a matrix specifying a rotation around the x-axis, putting it in dst.
     *
     * The angle measurement is in radians.  The rotation is counter
     * clockwise about the axis.
     *
     * @param angle The angle of rotation (in radians).
     * @param dst   A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* createRotationX(float angle, Mat4* dst);
    
    /**
     * Returns a matrix specifying a rotation around the y-axis.
     *
     * The angle measurement is in radians.  The rotation is counter
     * clockwise about the axis.
     *
     * @param angle The angle of rotation (in radians).
     *
     * @return a matrix specifying a rotation around the y-axis.
     */
    static Mat4 createRotationY(float angle) {
        Mat4 result;
        return *(createRotationY(angle,&result));
    }
    
    /**
     * Creates a matrix specifying a rotation around the y-axis, putting it in dst.
     *
     * The angle measurement is in radians.  The rotation is counter
     * clockwise about the axis.
     *
     * @param angle The angle of rotation (in radians).
     * @param dst   A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* createRotationY(float angle, Mat4* dst);
    
    /**
     * Returns a matrix specifying a rotation around the z-axis.
     *
     * The angle measurement is in radians.  The rotation is counter
     * clockwise about the axis.
     *
     * @param angle The angle of rotation (in radians).
     *
     * @return a matrix specifying a rotation around the z-axis.
     */
    static Mat4 createRotationZ(float angle) {
        Mat4 result;
        return *(createRotationZ(angle,&result));
    }
    
    /**
     * Creates a matrix specifying a rotation around the z-axis, putting it in dst.
     *
     * The angle measurement is in radians.  The rotation is counter
     * clockwise about the axis.
     *
     * @param angle The angle of rotation (in radians).
     * @param dst   A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* createRotationZ(float angle, Mat4* dst);

    /**
     * Returns a translation matrix from the given offset.
     *
     * @param trans The translation offset.
     *
     * @return a translation matrix from the given offset.
     */
    static Mat4 createTranslation(const Vec3& trans) {
        Mat4 result;
        return *(createTranslation(trans,&result));
    }

    /**
     * Creates a translation matrix from the given offset, putting it in dst.
     *
     * @param trans The translation offset.
     * @param dst   A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* createTranslation(const Vec3& trans, Mat4* dst);
    
    /**
     * Returns a translation matrix from the given parameters.
     *
     * @param tx    The translation on the x-axis.
     * @param ty    The translation on the y-axis.
     * @param tz    The translation on the z-axis.
     *
     * @return A reference to dst for chaining
     */
    static Mat4 createTranslation(float tx, float ty, float tz) {
        Mat4 result;
        return *(createTranslation(tx,ty,tz,&result));
    }

    /**
     * Creates a translation matrix from the given parameters, putting it in dst.
     *
     * @param tx    The translation on the x-axis.
     * @param ty    The translation on the y-axis.
     * @param tz    The translation on the z-axis.
     * @param dst   A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* createTranslation(float tx, float ty, float tz, Mat4* dst);
    
    
#pragma mark -
#pragma mark Setters
    /**
     * Sets the elements of this matrix to those in the specified matrix.
     *
     * @param mat The matrix to copy.
     *
     * @return A reference to this (modified) Mat4 for chaining.
     */
    Mat4& operator=(const Mat4& mat) {
        return set(mat);
    }

    /**
     * Sets the elements of this matrix to those in the specified one.
     *
     * @param mat The matrix to take resources from.
     *
     * @return A reference to this (modified) Mat4 for chaining.
     */
    Mat4& operator=(Mat4&& mat) {
        memcpy(this->m, mat.m, sizeof(float)*16);
        return *this;
    }
    
    /**
     * Sets the values of this matrix to those in the specified column-major array.
     *
     * The passed-in array is in column-major order, so the memory layout of
     * the array is as follows:
     *
     *     0   4   8   12
     *     1   5   9   13
     *     2   6   10  14
     *     3   7   11  15
     *
     * @param array An array containing 16 elements in column-major order.
     *
     * @return A reference to this (modified) Mat4 for chaining.
     */
    Mat4& operator=(const float* array) {
        return set(array);
    }
    
    /**
     * Sets this matrix as a rotation matrix from the specified quaternion.
     *
     * @param quat  A quaternion describing a 3D orientation.
     *
     * @return A reference to this (modified) Mat4 for chaining.
     */
    Mat4& operator=(const Quaternion& quat) {
        return set(quat);
    }
    
    /**
     * Sets the individal values of this matrix.
     *
     * @param m11 The first element of the first row.
     * @param m12 The second element of the first row.
     * @param m13 The third element of the first row.
     * @param m14 The fourth element of the first row.
     * @param m21 The first element of the second row.
     * @param m22 The second element of the second row.
     * @param m23 The third element of the second row.
     * @param m24 The fourth element of the second row.
     * @param m31 The first element of the third row.
     * @param m32 The second element of the third row.
     * @param m33 The third element of the third row.
     * @param m34 The fourth element of the third row.
     * @param m41 The first element of the fourth row.
     * @param m42 The second element of the fourth row.
     * @param m43 The third element of the fourth row.
     * @param m44 The fourth element of the fourth row.
     *
     * @return A reference to this (modified) Mat4 for chaining.
     */
    Mat4& set(float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24,
              float m31, float m32, float m33, float m34, float m41, float m42, float m43, float m44);
    
    /**
     * Sets the values of this matrix to those in the specified column-major array.
     *
     * The passed-in array is in column-major order, so the memory layout of
     * the array is as follows:
     *
     *     0   4   8   12
     *     1   5   9   13
     *     2   6   10  14
     *     3   7   11  15
     *
     * @param mat An array containing 16 elements in column-major order.
     *
     * @return A reference to this (modified) Mat4 for chaining.
     */
    Mat4& set(const float* mat);
    
    /**
     * Sets this matrix as a rotation matrix from the specified quaternion.
     *
     * @param quat  A quaternion describing a 3D orientation.
     *
     * @return A reference to this (modified) Mat4 for chaining.
     */
    Mat4& set(const Quaternion& quat);
    
    /**
     * Sets the elements of this matrix to those in the specified matrix.
     *
     * @param mat The matrix to copy.
     *
     * @return A reference to this (modified) Mat4 for chaining.
     */
    Mat4& set(const Mat4& mat);
    
    /**
     * Sets this matrix to the identity matrix.
     *
     * @return A reference to this (modified) Mat4 for chaining.
     */
    Mat4& setIdentity();
    
    /**
     * Sets all elements of the current matrix to zero.
     *
     * @return A reference to this (modified) Mat4 for chaining.
     */
    Mat4& setZero();
   
    
#pragma mark -
#pragma mark Static Arithmetic
    
    /**
     * Adds a scalar to each component of mat and stores the result in dst.
     *
     * @param mat       The matrix to add to.
     * @param scalar    The scalar value to add.
     * @param dst       A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* add(const Mat4& mat, float scalar, Mat4* dst);
    
    /**
     * Adds the specified matrices and stores the result in dst.
     *
     * @param m1    The first matrix.
     * @param m2    The second matrix.
     * @param dst   The destination matrix to add to.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* add(const Mat4& m1, const Mat4& m2, Mat4* dst);

    /**
     * Subtracts a scalar from each component of mat and stores the result in dst.
     *
     * @param mat       The matrix to subtract from.
     * @param scalar    The scalar value to subtract.
     * @param dst       A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* subtract(const Mat4& mat, float scalar, Mat4* dst);
    
    /**
     * Subtracts the matrix m2 from m1 and stores the result in dst.
     *
     * @param m1    The first matrix.
     * @param m2    The second matrix.
     * @param dst   A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* subtract(const Mat4& m1, const Mat4& m2, Mat4* dst);

    /**
     * Multiplies the specified matrix by a scalar and stores the result in dst.
     *
     * @param mat       The matrix.
     * @param scalar    The scalar value.
     * @param dst       A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* multiply(const Mat4& mat, float scalar, Mat4* dst);

    /**
     * Multiplies m1 by the matrix m2 and stores the result in dst.
     *
     * The matrix m2 is on the right.  This means that it corresponds to
     * an subsequent trasnform, when looking at a sequence of transforms.
     *
     * @param m1    The first matrix to multiply.
     * @param m2    The second matrix to multiply.
     * @param dst   A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* multiply(const Mat4& m1, const Mat4& m2, Mat4* dst);

    /**
     * Negates m1 and stores the result in dst.
     *
     * @param m1    The matrix to negate.
     * @param dst   A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* negate(const Mat4& m1, Mat4* dst);

    /**
     * Inverts m1 and stores the result in dst.
     *
     * If the matrix cannot be inverted, this method stores the zero matrix
     * in dst.
     *
     * @param m1    The matrix to negate.
     * @param dst   A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* invert(const Mat4& m1, Mat4* dst);

    /**
     * Transposes m1 and stores the result in dst.
     *
     * Transposing a matrix swaps columns and rows. This allows to transform
     * a vector by multiplying it on the left. If the matrix is orthonormal,
     * this is also the inverse.
     *
     * @param m1    The matrix to negate.
     * @param dst   A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* transpose(const Mat4& m1, Mat4* dst);
    
#pragma mark -
#pragma mark Arithmetic
    /**
     * Adds a scalar value to each component of this matrix.
     *
     * @param scalar The scalar to add.
     * 
     * @return A reference to the Mat4 after addition.
     */
    Mat4& add(float scalar) {
        return *(add(*this,scalar,this));
    }
    
    /**
     * Adds the specified matrix to this matrix.
     *
     * @param mat The matrix to add.
     *
     * @return A reference to the Mat4 after addition.
     */
    Mat4& add(const Mat4& mat) {
        return *(add(*this,mat,this));
    }
    
    /**
     * Subtracts a scalar value from each component of this matrix.
     *
     * @param scalar The scalar to subtract.
     *
     * @return A reference to the Mat4 after subtraction.
     */
    Mat4& subtract(float scalar) {
        return *(subtract(*this,scalar,this));
    }
    
    /**
     * Subtracts the specified matrix from the current matrix.
     *
     * @param mat The matrix to subtract.
     *
     * @return A reference to the Mat4 after subtraction.
     */
    Mat4& subtract(const Mat4& mat) {
        return *(subtract(*this,mat,this));
    }

    /**
     * Multiplies the components of this matrix by the specified scalar.
     *
     * @param scalar The scalar value.
     *
     * @return A reference to the Mat4 after multiplication.
     */
    Mat4& multiply(float scalar) {
        return *(multiply(*this,scalar,this));
    }
    
    /**
     * Multiplies this matrix by the specified one.
     *
     * The matrix mat is on the right.  This means that it corresponds to
     * a subsequent transform, when looking at the order of transforms.
     *
     * @param mat The matrix to multiply.
     *
     * @return A reference to the Mat4 after multiplication.
     */
    Mat4& multiply(const Mat4& mat) {
        return *(multiply(*this,mat,this));
    }
    
    /**
     * Negates this matrix in place.
     *
     * @return A reference to the Mat4 after negation.
     */
    Mat4& negate() {
        return *(negate(*this,this));
    }
    
    /**
     * Returns a copy of this matrix with all elements negated.
     *
     * Note: This does not modify the matrix.
     *
     * @return a copy of this matrix with all elements negated.
     */
    Mat4 getNegation() const {
        Mat4 result;
        negate(*this,&result);
        return result;
    }
    
    /**
     * Inverts this matrix in place.
     *
     * If the matrix cannot be inverted, this method sets it to the zero
     * matrix.
     *
     * @return A reference to the Mat4 after the inversion.
     */
    Mat4& invert() {
        return *(invert(*this,this));
    }
    
    /**
     * Returns a copy of the inverse of this matrix.
     *
     * If the matrix cannot be inverted, this method returns the zero matrix.
     *
     * Note: This does not modify the matrix.
     *
     * @return a copy of the inverse of this matrix.
     */
    Mat4 getInverse() const {
        Mat4 result;
        invert(*this,&result);
        return result;
    }
    
    /**
     * Transposes this matrix in place.
     *
     * Transposing a matrix swaps columns and rows. This allows to transform
     * a vector by multiplying it on the left. If the matrix is orthonormal,
     * this is also the inverse.
     *
     * @return A reference to the Mat4 after the transposition.
     */
    Mat4& transpose() {
        return *(transpose(*this,this));
    }
    
    /**
     * Returns a copy of the transpose of this matrix.
     *
     * Transposing a matrix swaps columns and rows. This allows to transform
     * a vector by multiplying it on the left. If the matrix is orthonormal,
     * this is also the inverse.
     *
     * Note: This does not modify the matrix.
     *
     * @return a copy of the transpose of this matrix.
     */
    Mat4 getTranspose() const {
        Mat4 result;
        transpose(*this,&result);
        return result;
    }
    
    
#pragma mark -
#pragma mark Operators
    /**
     * Adds the given matrix to this one in place.
     *
     * @param mat   The matrix to add
     *
     * @return A reference to this (modified) Mat4 for chaining.
     */
    Mat4& operator+=(const Mat4& mat) {
        return add(mat);
    }
    
    /**
     * Subtracts the given matrix from this one in place.
     *
     * @param mat   The matrix to subtract
     *
     * @return A reference to this (modified) Mat4 for chaining.
     */
    Mat4& operator-=(const Mat4& mat) {
        return subtract(mat);
    }
    
    /**
     * Right-multiplies this matrix by the given matrix.
     *
     * The matrix mat is on the right.  This means that it corresponds to
     * a subsequent transform, when looking at the order of transforms.
     *
     * @param mat   The matrix to multiply by.
     *
     * @return A reference to this (modified) Mat4 for chaining.
     */
    Mat4& operator*=(const Mat4& mat) {
        return multiply(mat);
    }
    
    /**
     * Multiplies the components of this matrix by the specified scalar.
     *
     * @param scalar The scalar value.
     *
     * @return A reference to this (modified) Mat4 for chaining.
     */
    Mat4& operator*=(float scalar) {
        return multiply(scalar);
    }
    
    /**
     * Returns the sum of this matrix with the given matrix.
     *
     * Note: This does not modify the matrix.
     *
     * @param mat   The matrix to add.
     *
     * @return the sum of this matrix with the given matrix.
     */
    const Mat4 operator+(const Mat4& mat) const {
        Mat4 result(*this);
        return result.add(mat);
    }
    
    /**
     * Returns the difference of this matrix with the given matrix.
     *
     * Note: This does not modify the matrix.
     *
     * @param mat   The matrix to subtract.
     *
     * @return the difference of this matrix with the given matrix.
     */
    const Mat4 operator-(const Mat4& mat) const {
        Mat4 result(*this);
        return result.subtract(mat);
    }

    /**
     * Returns the negation of this matrix.
     *
     * Note: This does not modify the matrix.
     *
     * @return the negation of this matrix.
     */
    const Mat4 operator-() const {
        return getNegation();
    }
    
    /**
     * Returns the matrix product of this matrix with the given matrix.
     *
     * The matrix mat is on the right.  This means that it corresponds to
     * an subsequent transform, when looking at a sequence of transforms.
     *
     * Note: This does not modify the matrix.
     *
     * @param mat   The matrix to multiply by.
     *
     * @return the matrix product of this matrix with the given matrix.
     */
    const Mat4 operator*(const Mat4& mat) const {
        Mat4 result(*this);
        return result.multiply(mat);
    }
    
    /**
     * Returns a copy of this matrix with all elements multiplied by the scalar.
     *
     * Note: This does not modify the matrix.
     *
     * @param scalar The scalar value.
     *
     * @return the matrix product of this matrix with the given matrix.
     */
    const Mat4 operator*(float scalar) const {
        Mat4 result(*this);
        return result.multiply(scalar);
    }

    
#pragma mark -
#pragma mark Comparisons
    /**
     * Returns true if the matrices are exactly equal to each other.
     *
     * This method may be unreliable given that the elements are floats.
     * It should only be used to compared matrices that have not undergone
     * a lot of transformations.
     *
     * @param mat       The matrix to compare against.
     *
     * @return true if the matrices are exactly equal to each other.
     */
    bool isExactly(const Mat4& mat) const;

    /**
     * Returns true if the matrices are within tolerance of each other.
     *
     * The tolerance is applied to each element of the matrix individually.
     *
     * @param mat       The matrix to compare against.
     * @param variance  The comparison tolerance.
     *
     * @return true if the matrices are within tolerance of each other.
     */
    bool equals(const Mat4& mat, float variance=CU_MATH_EPSILON) const;

    /**
     * Returns true if this matrix is equal to the given matrix.
     *
     * Comparison is exact, which may be unreliable given that the elements
     * are floats. It should only be used to compared matrices that have not
     * undergone a lot of transformations.
     *
     * @param mat   The matrix to compare against.
     *
     * @return true if this matrix is equal to the given matrix.
     */
    bool operator==(const Mat4& mat) const {
        return isExactly(mat);
    }
    
    /**
     * Returns true if this matrix is not equal to the given matrix.
     *
     * Comparison is exact, which may be unreliable given that the elements
     * are floats.
     *
     * @param mat   The matrix to compare against.
     *
     * @return true if this matrix is not equal to the given matrix.
     */
    bool operator!=(const Mat4& mat) const {
        return !isExactly(mat);
    }
    
    
#pragma mark -
#pragma mark Matrix Attributes
    /**
     * Returns true if this matrix is equal to the identity matrix.
     *
     * The optional comparison tolerance takes into accout that elements
     * are floats and this may not be exact.  The tolerance is applied to
     * each element individually.  By default, the match must be exact.
     *
     * @param variance The comparison tolerance
     *
     * @return true if this matrix is equal to the identity matrix.
     */
    bool isIdentity(float variance=CU_MATH_EPSILON) const;

    /**
     * Returns true if this matrix is invertible.
     *
     * The optional comparison tolerance takes into accout that elements
     * are floats and this may not be exact.  The tolerance is applied to
     * the matrix determinant.
     *
     * @return true if this matrix is invertible.
     */
    bool isInvertible(float variance=CU_MATH_EPSILON) const {
        return fabsf(getDeterminant()) > variance;
    }
    
    /**
     * Returns true if this matrix is orthogonal.
     *
     * The optional comparison tolerance takes into accout that elements
     * are floats and this may not be exact.  The tolerance is applied to
     * BOTH the normality test and the dot-product test for each pair.
     *
     * @return true if this matrix is orthogonal.
     */
    bool isOrthogonal(float variance=CU_MATH_EPSILON) const;
    
    /**
     * Returns the determinant of this matrix.
     *
     * @return the determinant of this matrix.
     */
    float getDeterminant() const;

    /**
     * Returns the scale component of this matrix.
     *
     * If the scale component of this matrix has negative parts,
     * it is not possible to always extract the exact scale component.
     * In that case, a scale vector that is mathematically equivalent to
     * the original scale vector is extracted and returned.
     *
     * To work properly, the matrix must have been constructed in the following
     * order: scale, then rotate, then translation. In any other order, the
     * scale is not guaranteed to be correct.
     *
     * @return the scale component of this matrix.
     */
    Vec3 getScale() const;
    
    /**
     * Returns the rotational component of this matrix.
     *
     * If the scale component is too close to zero, we cannot extract the
     * rotation.  In that case, we return the zero quaternion.
     (
     * @return the rotational component of this matrix.
     */
    Quaternion getRotation() const;
    
    /**
     * Returns the translational component of this matrix.
     *
     * To work properly, the matrix must have been constructed in the following
     * order: scale, then rotate, then translation. In any other order, the
     * translation is not guaranteed to be correct.
     *
     * @return the translational component of this matrix.
     */
    Vec3 getTranslation() const;
    
    /**
     * Returns the up vector of this matrix, when treated as a camera.
     *
     * @return the up vector of this matrix, when treated as a camera.
     */
    Vec3 getUpVector() const;
    
    /**
     * Returns the down vector of this matrix, when treated as a camera.
     *
     * @return the down vector of this matrix, when treated as a camera.
     */
    Vec3 getDownVector() const;
    /**
     * Returns the left vector of this matrix, when treated as a camera.
     *
     * @return the left vector of this matrix, when treated as a camera.
     */
    Vec3 getLeftVector() const;
    
    /**
     * Returns the right vector of this matrix, when treated as a camera.
     *
     * @return the right vector of this matrix, when treated as a camera.
     */
    Vec3 getRightVector() const;
    
    /**
     * Returns the forward vector of this matrix, when treated as a camera.
     *
     * @return the forward vector of this matrix, when treated as a camera.
     */
    Vec3 getForwardVector() const;
    
    /**
     * Returns the backward vector of this matrix, when treated as a camera.
     *
     * @return the backward vector of this matrix, when treated as a camera.
     */
    Vec3 getBackVector() const;
    

#pragma mark -
#pragma mark Static Vector Operations
    /**
     * Transforms the point by the given matrix, and stores the result in dst.
     *
     * The vector is treated as a point, which means that translation is
     * applied to the result.
     *
     * @param mat   The transform matrix.
     * @param point The point to transform.
     * @param dst   A vector to store the transformed point in.
     *
     * @return A reference to dst for chaining
     */
    static Vec2* transform(const Mat4& mat, const Vec2& point, Vec2* dst);
    
    /**
     * Transforms the rectangle by the given matrix, and stores the result in dst.
     *
     * This method transforms the four defining points of the rectangle.  It
     * then computes the minimal bounding box storing these four points.
     *
     * @param mat   The transform matrix.
     * @param rect  The rect to transform.
     * @param dst   A rect to store the transformed rectangle in.
     *
     * @return A reference to dst for chaining
     */
    static Rect* transform(const Mat4& mat, const Rect& rect, Rect* dst);
    
    /**
     * Transforms the vector by the given matrix, and stores the result in dst.
     *
     * The vector is treated as a direction, which means that translation is
     * not applied to the result.
     *
     * @param mat   The transform matrix.
     * @param vec   The vector to transform.
     * @param dst   A vector to store the transformed point in.
     *
     * @return A reference to dst for chaining
     */
    static Vec2* transformVector(const Mat4& mat, const Vec2& vec, Vec2* dst);
    
    /**
     * Transforms the point by the given matrix, and stores the result in dst.
     *
     * The vector is treated as a point, which means that translation is
     * applied to the result.
     *
     * @param mat   The transform matrix.
     * @param point The point to transform.
     * @param dst   A vector to store the transformed point in.
     *
     * @return A reference to dst for chaining
     */
    static Vec3* transform(const Mat4& mat, const Vec3& point, Vec3* dst);
    
    /**
     * Transforms the vector by the given matrix, and stores the result in dst.
     *
     * The vector is treated as a direction, which means that translation is
     * not applied to the result.
     *
     * @param mat   The transform matrix.
     * @param vec   The vector to transform.
     * @param dst   A vector to store the transformed point in.
     *
     * @return A reference to dst for chaining
     */
    static Vec3* transformVector(const Mat4& mat, const Vec3& vec, Vec3* dst);
    
    /**
     * Transforms the vector by the given matrix, and stores the result in dst.
     *
     * The vector is treated as is.  Hence whether or not translation is applied
     * depends on the value of w.
     *
     * @param mat   The transform matrix.
     * @param vec   The vector to transform.
     * @param dst   A vector to store the transformed point in.
     *
     * @return A reference to dst for chaining
     */
    static Vec4* transform(const Mat4& mat, const Vec4& vec, Vec4* dst);

    /**
     * Transforms the vector array by the given matrix, and stores the result in dst.
     *
     * The vector is array is treated as a list of 4 element vectors (@see Vec4).
     * The transform is applied in order and written to the output array.
     *
     * @param mat   	The transform matrix.
     * @param input   	The array of vectors to transform.
     * @param output	The array to store the transformed vectors.
     * @param size		The size of the two arrays.
     *
     * @return A reference to dst for chaining
     */
    static float* transform(const Mat4& mat, float const* input, float* output, size_t size);


#pragma mark -
#pragma mark Vector Operations
    /**
     * Returns a copy of this point transformed by the matrix.
     *
     * The vector is treated as a point, which means that translation is
     * applied to the result.
     *
     * Note: This does not modify the original point. To transform a
     * point in place, use the static method (or the appropriate operator).
     *
     * @param point The point to transform.
     *
     * @return a copy of this point transformed by the matrix.
     */
    Vec2 transform(const Vec2& point) const;
    
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

    /**
     * Returns a copy of this vector transformed by the matrix.
     *
     * The vector is treated as a direction, which means that translation is
     * not applied to the result.
     *
     * Note: This does not modify the original vector. To transform a
     * vector in place, use the static method (or the appropriate operator).
     *
     * @param vec The vector to transform.
     *
     * @return a copy of this point transformed by the matrix.
     */
    Vec2 transformVector(const Vec2& vec) const;
    
    /**
     * Returns a copy of this point transformed by the matrix.
     *
     * The vector is treated as a point, which means that translation is
     * applied to the result.
     *
     * Note: This does not modify the original point. To transform a
     * point in place, use the static method (or the appropriate operator).
     *
     * @param point The point to transform.
     *
     * @return a copy of this point transformed by the matrix.
     */
    Vec3 transform(const Vec3& point) const;
    
    /**
     * Returns a copy of this vector transformed by the matrix.
     *
     * The vector is treated as a direction, which means that translation is
     * not applied to the result.
     *
     * Note: This does not modify the original vector. To transform a
     * vector in place, use the static method (or the appropriate operator).
     *
     * @param vec The vector to transform.
     *
     * @return a copy of this point transformed by the matrix.
     */
    Vec3 transformVector(const Vec3& vec) const;
    
    /**
     * Returns a copy of this vector transformed by the matrix.
     *
     * The vector is treated as is.  Hence whether or not translation is applied
     * depends on the value of w.
     *
     * Note: This does not modify the original vector. To transform a
     * vector in place, use the static method (or the appropriate operator).
     *
     * @param vec   The vector to transform.
     *
     * @return a copy of this point transformed by the matrix.
     */
    Vec4 transform(const Vec4& vec) const;
    
#pragma mark -
#pragma mark Static Matrix Transforms
    /**
     * Applies a quaternion rotation to the given matrix and stores the result in dst.
     *
     * The rotation is applied on the right.  Given our convention, that means 
     * that it takes place AFTER any previously applied transforms.
     *
     * @param mat   The matrix to rotate.
     * @param quat  The quaternion to rotate by.
     * @param dst   A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* rotate(const Mat4& mat, const Quaternion& quat, Mat4* dst) {
        Mat4 result;
        createRotation(quat, &result);
        multiply(mat, result, dst);
        return dst;
    }

    /**
     * Applies an axis rotation to the given matrix and stores the result in dst.
     *
     * The rotation is in radians, counter-clockwise about the given axis.
     *
     * The rotation is applied on the right.  Given our convention, that means
     * that it takes place AFTER any previously applied transforms.
     *
     * @param mat   The matrix to rotate.
     * @param axis  The axis to rotate about.
     * @param angle The angle (in radians).
     * @param dst   A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* rotate(const Mat4& mat, const Vec3& axis, float angle, Mat4* dst) {
        Mat4 result;
        createRotation(axis, angle, &result);
        multiply(mat, result, dst);
        return dst;
    }

    /**
     * Applies an x-axis rotation to the given matrix and stores the result in dst.
     *
     * The rotation is in radians, counter-clockwise about the x-axis.
     *
     * The rotation is applied on the right.  Given our convention, that means
     * that it takes place AFTER any previously applied transforms.
     *
     * @param mat   The matrix to rotate.
     * @param angle The angle (in radians).
     * @param dst   A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* rotateX(const Mat4& mat, float angle, Mat4* dst) {
        Mat4 result;
        createRotationX(angle, &result);
        multiply(mat, result, dst);
        return dst;
    }
    
    /**
     * Applies an y-axis rotation to the given matrix and stores the result in dst.
     *
     * The rotation is in radians, counter-clockwise about the y-axis.
     *
     * The rotation is applied on the right.  Given our convention, that means
     * that it takes place AFTER any previously applied transforms.
     *
     * @param mat   The matrix to rotate.
     * @param angle The angle (in radians).
     * @param dst   A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* rotateY(const Mat4& mat, float angle, Mat4* dst) {
        Mat4 result;
        createRotationY(angle, &result);
        multiply(mat, result, dst);
        return dst;
    }
    
    /**
     * Applies an z-axis rotation to the given matrix and stores the result in dst.
     *
     * The rotation is in radians, counter-clockwise about the z-axis.
     *
     * The rotation is applied on the right.  Given our convention, that means
     * that it takes place AFTER any previously applied transforms.
     *
     * @param mat   The matrix to rotate.
     * @param angle The angle (in radians).
     * @param dst   A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* rotateZ(const Mat4& mat, float angle, Mat4* dst) {
        Mat4 result;
        createRotationZ(angle, &result);
        multiply(mat, result, dst);
        return dst;
    }

    /**
     * Applies a uniform scale to the given matrix and stores the result in dst.
     *
     * The scaling operation is applied on the right.  Given our convention, 
     * that means that it takes place AFTER any previously applied transforms.
     *
     * @param mat   The matrix to scale.
     * @param value The scalar to multiply by.
     * @param dst   A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* scale(const Mat4& mat, float value, Mat4* dst) {
        Mat4 result;
        createScale(value, &result);
        multiply(mat, result, dst);
        return dst;
   }
    
    /**
     * Applies a non-uniform scale to the given matrix and stores the result in dst.
     *
     * The scaling operation is applied on the right.  Given our convention,
     * that means that it takes place AFTER any previously applied transforms.
     *
     * @param mat   The matrix to scale.
     * @param s     The vector storing the individual scaling factors
     * @param dst   A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* scale(const Mat4& mat, const Vec3& s, Mat4* dst) {
        Mat4 result;
        createScale(s, &result);
        multiply(mat, result, dst);
        return dst;
    }
    
    /**
     * Applies a non-uniform scale to the given matrix and stores the result in dst.
     *
     * The scaling operation is applied on the right.  Given our convention,
     * that means that it takes place AFTER any previously applied transforms.
     *
     * @param mat   The matrix to scale.
     * @param sx    The amount to scale along the x-axis.
     * @param sy    The amount to scale along the y-axis.
     * @param sz    The amount to scale along the z-axis.
     * @param dst   A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* scale(const Mat4& mat, float sx, float sy, float sz, Mat4* dst) {
        Mat4 result;
        createScale(sx,sy,sz, &result);
        multiply(mat, result, dst);
        return dst;
    }

    /**
     * Applies a translation to the given matrix and stores the result in dst.
     *
     * The translation operation is applied on the right.  Given our convention,
     * that means that it takes place AFTER any previously applied transforms.
     *
     * @param mat   The matrix to translate.
     * @param t     The vector storing the individual translation offsets
     * @param dst   A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* translate(const Mat4& mat, const Vec3& t, Mat4* dst) {
        Mat4 result;
        createTranslation(t, &result);
        multiply(mat, result, dst);
        return dst;
    }

    /**
     * Applies a translation to the given matrix and stores the result in dst.
     *
     * The translation operation is applied on the right.  Given our convention,
     * that means that it takes place AFTER any previously applied transforms.
     *
     * @param mat   The matrix to translate.
     * @param tx    The translation offset for the x-axis.
     * @param ty    The translation offset for the y-axis.
     * @param tz    The translation offset for the z-axis.
     * @param dst   A matrix to store the result in.
     *
     * @return A reference to dst for chaining
     */
    static Mat4* translate(const Mat4& mat, float tx, float ty, float tz, Mat4* dst) {
        Mat4 result;
        createTranslation(tx,ty,tz, &result);
        multiply(mat, result, dst);
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
     * @param mat   The matrix to decompose.
     * @param scale The scale component.
     * @param rot   The rotation component.
     * @param trans The translation component.
     *
     * @return true if all requested components were properly extracted
     */
    static bool decompose(const Mat4& mat, Vec3* scale, Quaternion* rot, Vec3* trans);

    
#pragma mark -
#pragma mark Matrix Transforms
    /**
     * Applies a quaternion rotation to this matrix.
     *
     * The rotation is applied on the right.  Given our convention, that means
     * that it takes place AFTER any previously applied transforms.
     *
     * @param q  The quaternion to rotate by.
     *
     * @return This matrix, after rotation.
     */
    Mat4& rotate(const Quaternion& q) {
        return *(rotate(*this,q,this));
    }
    
    /**
     * Applies an axis rotation to the this matrix.
     *
     * The rotation is in radians, counter-clockwise about the given axis.
     *
     * The rotation is applied on the right.  Given our convention, that means
     * that it takes place AFTER any previously applied transforms.
     *
     * @param axis  The axis to rotate about.
     * @param angle The angle (in radians).
     *
     * @return This matrix, after rotation.
     */
    Mat4& rotate(const Vec3& axis, float angle) {
        return *(rotate(*this,axis,angle,this));
    }

    /**
     * Applies an x-axis rotation to this matrix.
     *
     * The rotation is in radians, counter-clockwise about the x-axis.
     *
     * The rotation is applied on the right.  Given our convention, that means
     * that it takes place AFTER any previously applied transforms.
     *
     * @param angle The angle (in radians).
     *
     * @return This matrix, after rotation.
     */
    Mat4& rotateX(float angle) {
        return *(rotateX(*this,angle,this));
    }
    
    /**
     * Applies a y-axis rotation to this matrix.
     *
     * The rotation is in radians, counter-clockwise about the y-axis.
     *
     * The rotation is applied on the right.  Given our convention, that means
     * that it takes place AFTER any previously applied transforms.
     *
     * @param angle The angle (in radians).
     *
     * @return This matrix, after rotation.
     */
    Mat4& rotateY(float angle) {
        return *(rotateY(*this,angle,this));
    }
    
    /**
     * Applies a z-axis rotation to this matrix.
     *
     * The rotation is in radians, counter-clockwise about the z-axis.
     *
     * The rotation is applied on the right.  Given our convention, that means
     * that it takes place AFTER any previously applied transforms.
     *
     * @param angle The angle (in radians).
     *
     * @return This matrix, after rotation.
     */
    Mat4& rotateZ(float angle) {
        return *(rotateZ(*this,angle,this));
    }
    
    /**
     * Applies a uniform scale to this matrix.
     *
     * The scaling operation is applied on the right.  Given our convention,
     * that means that it takes place AFTER any previously applied transforms.
     *
     * @param value The scalar to multiply by.
     *
     * @return This matrix, after scaling.
     */
    Mat4& scale(float value) {
        return *(scale(*this,value,this));
    }
    
    /**
     * Applies a non-uniform scale to this matrix.
     *
     * The scaling operation is applied on the right.  Given our convention,
     * that means that it takes place AFTER any previously applied transforms.
     *
     * @param s     The vector storing the individual scaling factors
     *
     * @return This matrix, after scaling.
     */
    Mat4& scale(const Vec3& s) {
        return *(scale(*this,s,this));
    }
    
    /**
     * Applies a non-uniform scale to this matrix.
     *
     * The scaling operation is applied on the right.  Given our convention,
     * that means that it takes place AFTER any previously applied transforms.
     *
     * @param sx    The amount to scale along the x-axis.
     * @param sy    The amount to scale along the y-axis.
     * @param sz    The amount to scale along the z-axis.
     *
     * @return This matrix, after scaling.
     */
    Mat4& scale(float sx, float sy, float sz) {
        return *(scale(*this,sx,sy,sz,this));
    }

    /**
     * Applies a translation to this matrix.
     *
     * The translation operation is applied on the right.  Given our convention,
     * that means that it takes place AFTER any previously applied transforms.
     *
     * @param t     The vector storing the individual translation offsets
     *
     * @return This matrix, after translation.
     */
    Mat4& translate(const Vec3& t) {
        return *(translate(*this,t,this));
    }
    
    /**
     * Applies a translation to this matrix.
     *
     * The translation operation is applied on the right.  Given our convention,
     * that means that it takes place AFTER any previously applied transforms.
     *
     * @param tx    The translation offset for the x-axis.
     * @param ty    The translation offset for the y-axis.
     * @param tz    The translation offset for the z-axis.
     *
     * @return This matrix, after translation.
     */
    Mat4& translate(float tx, float ty, float tz) {
        return *(translate(*this,tx,ty,tz,this));
    }
    
    
#pragma mark -
#pragma mark Conversion Methods
    /**
     * Returns a string representation of this matrix for debugging purposes.
     *
     * If verbose is true, the string will include class information.  This
     * allows us to unambiguously identify the class.
     *
     * @param verbose Whether to include class information
     *
     * @return a string representation of this matrix for debugging purposes.
     */
    std::string toString(bool verbose = false) const;
    
    /** Cast from Vec4 to a string. */
    operator std::string() const { return toString(); }

    /** 
     * Cast from Mat4 to a Affine2. 
     *
     * The z values are all uniformly ignored.  However, it the final element
     * of the matrix is not 1 (e.g. the translation has a w value of 1), then
     * it divides the entire matrix before creating the affine transform.
     *
     */
    operator Affine2() const;
    
    /**
     * Creates a matrix from the given affine transform.
     *
     * The z values are set to the identity.
     *
     * @param aff The transform to convert
     */
    explicit Mat4(const Affine2& aff);
    
    /**
     * Sets the elements of this matrix to those of the given transform.
     *
     * The z values are set to the identity.
     *
     * @param aff The transform to convert
     *
     * @return A reference to this (modified) Mat4 for chaining.
     */
    Mat4& operator= (const Affine2& aff);
    
    /**
     * Sets the elements of this matrix to those of the given transform.
     *
     * The z values are set to the identity.
     *
     * @param aff The transform to convert
     *
     * @return A reference to this (modified) Mat4 for chaining.
     */
    Mat4& set(const Affine2& aff);
};
    
#pragma mark -
#pragma mark Vector Operations
// To avoid confusion, we NEVER support vector on the right ops.
  
/**
 * Transforms the given point by the given matrix.
 *
 * The vector is treated as a point, which means that translation is
 * applied to the result.
 *
 * @param v The point to transform.
 * @param m The matrix to transform by.
 *
 * @return this point, after the transformation occurs.
 */
inline Vec2& operator*=(Vec2& v, const Mat4& m) {
    return *(Mat4::transform(m,v,&v));
}
    
/**
 * Returns a copy of the vector transformed by the given matrix.
 *
 * The vector is treated as a point, which means that translation is
 * applied to the result.
 *
 * @param v The point to transform.
 * @param m The matrix to transform by.
 *
 * @return a copy of the vector transformed by the given matrix.
 */
inline const Vec2 operator*(const Vec2& v, const Mat4& m) {
    return m.transform(v);
}
    
/**
 * Transforms the given point by the given matrix.
 *
 * The vector is treated as a point, which means that translation is
 * applied to the result.
 *
 * @param v The point to transform.
 * @param m The matrix to transform by.
 *
 * @return this point, after the transformation occurs.
 */
inline Vec3& operator*=(Vec3& v, const Mat4& m) {
    return *(Mat4::transform(m,v,&v));
}

/**
 * Returns a copy of the vector transformed by the given matrix.
 *
 * The vector is treated as a point, which means that translation is
 * applied to the result.
 *
 * @param v The point to transform.
 * @param m The matrix to transform by.
 *
 * @return a copy of the vector transformed by the given matrix.
 */
inline const Vec3 operator*(const Vec3& v, const Mat4& m) {
    return m.transform(v);
}

/**
 * Transforms the given point by the given matrix.
 *
 * The vector is treated as is.  Hence whether or not translation is applied
 * depends on the value of w.
 *
 * @param v The point to transform.
 * @param m The matrix to transform by.
 *
 * @return this point, after the transformation occurs.
 */
inline Vec4& operator*=(Vec4& v, const Mat4& m) {
    return *(Mat4::transform(m,v,&v));
}
    
/**
 * Returns a copy of the vector transformed by the given matrix.
 *
 * The vector is treated as is.  Hence whether or not translation is applied
 * depends on the value of w.
 *
 * @param v The point to transform.
 * @param m The matrix to transform by.
 *
 * @return a copy of the vector transformed by the given matrix.
 */
inline const Vec4 operator*(const Vec4& v, const Mat4& m) {
    return m.transform(v);
}
    
/**
 * Multiplies the components of the given matrix by the specified scalar.
 *
 * @param scalar The scalar value.
 * @param m The matrix to transform by.
 *
 * @return a copy of the scaled matrix
 */
inline const Mat4 operator*(float scalar, const Mat4& m) {
    Mat4 result(m);
    return result.multiply(scalar);
}

}

#endif /* __CU_MAT4_H__ */

//
//  CUCamera.h
//  Cornell University Game Library (CUGL)
//
//  This module provides the abstract base class for our camera classes.
//  Because it is abstract, it has only a basic constructor.  It has no
//  initializers or allocator.
//
//  This module is based on the original file Camera.java by Mario Zechner,
//  written for LibGDX ( https://libgdx.badlogicgames.com ). It has been
//  modified to support the CUGL framework.
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
//  Version: 6/24/16

#ifndef __CU_CAMERA_H__
#define __CU_CAMERA_H__

#include <cugl/math/CURect.h>
#include <cugl/math/CUVec2.h>
#include <cugl/math/CUVec3.h>
#include <cugl/math/CUMat4.h>
#include <cugl/math/CUQuaternion.h>
#include <cugl/math/CURay.h>
#include <cugl/util/CUDebug.h>

namespace cugl {
    
/** 
 * This is a base class for a camera. 
 *
 * The actual cameras are OrthographicCamera and  PerspectiveCamera.  This class
 * hash the shared functionality for transforms and queries in 3d space.  In
 * particular, it has several methods that help you to select a 3d object
 * with the mouse.
 */
class Camera {
#pragma mark Values
protected:
    /** The position of the camera **/
    Vec3 _position;
    /** The unit length direction vector of the camera **/
    Vec3 _direction;
    /** The unit length up vector of the camera **/
    Vec3 _upwards;
    
    /** The projection matrix **/
    Mat4 _projection;
    /** The view matrix **/
    Mat4 _modelview;
    /** The combined projection and view matrix **/
    Mat4 _combined;
    /** The inverse of the combined projection and view matrix **/
    Mat4 _inverse;
    
    /** The near clipping plane distance, has to be positive **/
    float _near;
    /** The far clipping plane distance, has to be positive **/
    float _far;
    
    /** The Window viewport **/
    Rect _viewport;
    
#pragma mark -
#pragma mark Constructor
public:
    /**
     * Creates a degenerate camera.
     *
     * All of the matrices are the identity and the viewport is empty.
     */
    Camera();
    
    /**
     * Deletes this camera, disposing all resources.
     */
    ~Camera() { dispose(); }
    
    /**
     * Returns this camera to the denegerate one.
     *
     * All of the matrices will be the identity and the viewport will be empty.
     */
    virtual void dispose();
    
    
#pragma mark -
#pragma mark Attributes
    /**
     * Returns the position of the camera
     *
     * @return the position of the camera
     */
    const Vec3& getPosition() const { return _position; }
    
    /**
     * Returns the unit length direction vector of the camera
     *
     * @return the unit length direction vector of the camera
     */
    const Vec3& getDirection() const { return _direction; }
    
    /**
     * Returns the unit length up vector of the camera
     *
     * @return the unit length up vector of the camera
     */
    const Vec3& getUp() const { return _upwards; }
    
    /**
     * Returns the near clipping plane distance (has to be positive)
     *
     * @return the near clipping plane distance (has to be positive)
     */
    float getNear() const { return _near; }
    
    /**
     * Sets the near clipping plane distance (has to be positive)
     *
     * Changing this value will have no effect on the underlying matrices
     * until you call the update() method.
     *
     * @param value The near clipping plane distance (has to be positive)
     */
    void setNear(float value) {
        CUAssertLog(value >= 0, "Value is not positive");
        _near = value;
    }
    
    /**
     * Returns the far clipping plane distance (has to be positive)
     *
     * @return the far clipping plane distance (has to be positive)
     */
    float getFar() const { return _far; }
    
    /**
     * Sets the far clipping plane distance (has to be positive)
     *
     * Changing this value will have no effect on the underlying matrices
     * until you call the update() method.
     *
     * @param value The far clipping plane distance (has to be positive)
     */
    void setFar(float value) {
        CUAssertLog(value >= 0, "Value is not positive");
        _far = value;
    }
    
    /**
     * Returns the viewport
     *
     * The viewport represents the "screen space".  However, it is not actually
     * screen space because the origin is still in the bottom left corner.
     * So we call ir Window space instead.
     *
     * @return the viewport size
     */
    const Rect& getViewport() const { return _viewport; }

    /**
     * Returns the projection matrix
     *
     * @return the projection matrix
     */
    const Mat4& getProjection() const { return _projection; }
    
    /**
     * Returns the view matrix
     *
     * @return the view matrix
     */
    const Mat4& getView() const { return _modelview; }
    
    /**
     * Returns the combined projection and view matrix
     *
     * @return the combined projection and view matrix
     */
    const Mat4& getCombined() const { return _combined; }

    /**
     * Returns the combined projection and view matrix
     *
     * @return the combined projection and view matrix
     */
    const Mat4& getInverseProjectView() const { return _inverse; }
    
#pragma mark -
#pragma mark Updates
    /**
     * Recalculates the projection and view matrix of this camera.
     *
     * Use this after you've manipulated any of the attributes of the camera. 
     */
    virtual void update() = 0;
    
    /** 
     * Sets the direction of the camera to look at the given point.
     *
     * This function assumes the up vector is normalized.
     *
     * @param target    The point to look at 
     */
    void lookAt(const Vec3& target);

    /**
     * Sets the direction of the camera to look at the given point.
     *
     * This function assumes the up vector is normalized.
     *
     * @param x The x-coordinate of the point to look at
     * @param y The y-coordinate of the point to look at
     * @param z The z-coordinate of the point to look at
     */
    void lookAt (float x, float y, float z) {
        lookAt(Vec3(x,y,z));
    }
    
    /** 
     * Normalizes the up vector to be orthogonal to direction.
     *
     * This method first calculats the right vector via a cross product 
     * between direction and up. Then it recalculates the up vector via a 
     * cross product between right and direction. 
     */
    void normalizeUp() {
        _upwards = _direction.cross(_upwards).normalize();
        _upwards.cross(_direction).normalize();
    }
    
#pragma mark -
#pragma mark View Transforms
    /**
     * Rotates the direction and up vector by the given Quaternion
     *
     * You must call update() for the view matrix to be updated. The direction
     * and up vector will not be orthogonalized until you call update().
     *
     * @param quat  The rotation quaternion.
     */
    void rotate(const Quaternion& quat) {
        Mat4 temp;
        Mat4::createRotation(quat,&temp);
        _direction *= temp;
        _upwards *= temp;
    }
    
    /**
     * Rotates the direction and up vector by the given angle around the given axis.
     *
     * You must call update() for the view matrix to be updated. The direction
     * and up vector will not be orthogonalized until you call update().
     *
     * @param axis  The axis to rotate about.
     * @param angle The angle (in radians).
     */
    void rotate(const Vec3& axis, float angle) {
        Mat4 temp;
        Mat4::createRotation(axis,angle,&temp);
        _direction *= temp;
        _upwards *= temp;
    }
    
    /** 
     * Rotates the direction and up vectod by the given angle around the x-axis.
     *
     * You must call update() for the view matrix to be updated. The direction
     * and up vector will not be orthogonalized until you call update().
     *
     * @param angle The angle (in radians).
     */
    void rotateX(float angle) {
        Mat4 temp;
        Mat4::createRotationX(angle,&temp);
        _direction *= temp;
        _upwards *= temp;
    }

    /**
     * Rotates the direction and up vectod by the given angle around the z-axis.
     *
     * You must call update() for the view matrix to be updated. The direction
     * and up vector will not be orthogonalized until you call update().
     *
     * @param angle The angle (in radians).
     */
    void rotateY(float angle) {
        Mat4 temp;
        Mat4::createRotationY(angle,&temp);
        _direction *= temp;
        _upwards *= temp;
    }

    /**
     * Rotates the direction and up vectod by the given angle around the z-axis.
     *
     * You must call update() for the view matrix to be updated. The direction
     * and up vector will not be orthogonalized until you call update().
     *
     * @param angle The angle (in radians).
     */
    void rotateZ(float angle) {
        Mat4 temp;
        Mat4::createRotationZ(angle,&temp);
        _direction *= temp;
        _upwards *= temp;
    }

    /**
     * Rotates the camera by the given angle around the direction vector.
     *
     * You must call update() for the view matrix to be updated. The direction
     * and up vector will not be orthogonalized until you call update().
     *
     * @param angle The angle (in radians).
     */
    void rotate(float angle) {
        rotate(_direction, angle);
    }

    /**
     * Rotates the direction and up vector of this camera by the given matrix. 
     *
     * You must call update() for the view matrix to be updated. The direction
     * and up vector will not be orthogonalized until you call update().
     *
     * The translational and scaling components of the matrix will be ignored.
     *
     * @param transform The rotation matrix 
     */
    void rotate(const Mat4& transform) {
        Mat4::transformVector(transform, _direction, &_direction);
        _direction.normalize();
        Mat4::transformVector(transform, _upwards, &_upwards);
        _upwards.normalize();
    }
    
    /**
     * Rotates the direction and up vector by the given angle around the given axis.
     *
     * This version of the method assumes the axis is attached to the given
     * point.
     *
     * You must call update() for the view matrix to be updated. The direction
     * and up vector will not be orthogonalized until you call update().
     *
     * @param point The point to attach the axis to
     * @param axis  The axis to rotate about.
     * @param angle The angle (in radians).
     */
    void rotateAround(const Vec3& point, const Vec3& axis, float angle);
    
    /** 
     * Moves the camera by the given vector.
     *
     * @param vec   The displacement vector
     */
    void translate(const Vec3& vec) {
        _position += vec;
    }
    
    /**
     * Moves the camera by the given vector.
     *
     * You must call update() for the view matrix to be updated. The direction
     * and up vector will not be orthogonalized until you call update().
     *
     * @param vec   The displacement vector
     */
    void translate (const Vec2& vec) {
        translate(vec.x,vec.y);
    }
    
    /**
     * Moves the camera by the given vector.
     *
     * You must call update() for the view matrix to be updated. The direction
     * and up vector will not be orthogonalized until you call update().
     *
     * @param tx    The displacement on the x-axis
     * @param ty    The displacement on the y-axis
     * @param tz    The displacement on the z-axis
     */
    void translate(float tx, float ty, float tz) {
        _position.x += tx;
        _position.y += ty;
        _position.z += tz;
    }
    
    /**
     * Moves the camera by the given vector.
     *
     * You must call update() for the view matrix to be updated. The direction
     * and up vector will not be orthogonalized until you call update().
     *
     * @param tx    The displacement on the x-axis
     * @param ty    The displacement on the y-axis
     */
    void translate (float tx, float ty) {
        _position.x += tx;
        _position.y += ty;
    }
    
    /**
     * Transforms the position, direction and up vector by the given matrix
     *
     * You must call update() for the view matrix to be updated. The direction
     * and up vector will not be orthogonalized until you call update().
     *
     * @param transform The transform matrix 
     */
    void transform(const Mat4& transform) {
        _position *= transform;
        rotate(transform);
    }
    
#pragma mark -
#pragma mark Coordinate Transforms
    /**
     * Returns the world space equivalent of a point in window coordinates.
     *
     * This is the same as GLU gluUnProject, but does not rely on OpenGL. 
     * Window coords are the location of the point in the viewport.  Ideally,
     * the viewport should be the same size as the screen, but these are not
     * screen coordinates.  The screen has the origin in the top left, while
     * Window coordinates still have the origin in the bottom left.
     *
     * While the viewport is a flat 2d plane, this method still requires a
     * 3d point.  The z-coordinate corresponds to the position of the point 
     * in the z-buffer.
     *
     * @param windowCoords  The point in window coordinates
     *
     * @return the world space equivalent of a point in window coordinates.
     */
    Vec3 unproject(const Vec3& windowCoords) const;
    
	/**
	 * Returns the world space equivalent of a point in window coordinates.
	 *
	 * This is the same as GLU gluUnProject, but does not rely on OpenGL.
	 * Window coords are the location of the point in the viewport.  Ideally,
	 * the viewport should be the same size as the screen, but these are not
	 * screen coordinates.  The screen has the origin in the top left, while
	 * Window coordinates still have the origin in the bottom left.
	 *
	 * While the viewport is a flat 2d plane, this method still requires a
	 * 3d point.  The z-coordinate corresponds to the position of the point
	 * in the z-buffer.
	 *
	 * @param windowCoords  The point in window coordinates
	 * @param viewport      The screen viewport
	 *
	 * @return the world space equivalent of a point in window coordinates.
	 */
	Vec3 unproject(const Vec3& windowCoords, const Rect& viewport) const;

    /**
     * Returns the world space equivalent of a point in screen coordinates.
     *
     * Ideally, window space and screen space would be the same space.  They
     * are both defined by the viewport and have the same offset and dimension.
     * However, screen coordinates have the origin in the top left while window
     * coordinates have the origin in the bottom left.
     *
     * In computing the world space coordinates, this method assumes that the
     * z-value of the original vector is the same as near, which is the
     * closest it can be the screen.
     *
     * This method is important for converting event coordinates (such as a
     * mouse click) to world coordinates.
     *
     * @param screenCoords  The point in screen coordinates
     *
     * @return the world space equivalent of a point in screen coordinates.
     */
    Vec3 screenToWorldCoords(const Vec2& screenCoords) const {
        return unproject(screenToWindowCoords(screenCoords));
    }
    
    /**
     * Returns the window space equivalent of a point in world coordinates.
     *
     * This is the same as GLU gluProject. Window coords are the location of 
     * the point in the viewport.  Ideally, the viewport should be the same 
     * size as the screen, but these are not screen coordinates. The screen has
     * the origin in the top left, while
     *
     * While the viewport is a flat 2d plane, this method still returns a
     * 3d point.  The z-coordinate corresponds to the position of the point
     * in the z-buffer.
     *
     * @param worldCoords   The point in wprld coordinates
     *
     * @return the window space equivalent of a point in world coordinates.
     */
    Vec3 project(const Vec3& worldCoords) const;

	/**
	 * Returns the window space equivalent of a point in world coordinates.
	 *
	 * This is the same as GLU gluProject. Window coords are the location of
	 * the point in the viewport.  Ideally, the viewport should be the same
	 * size as the screen, but these are not screen coordinates. The screen has
	 * the origin in the top left, while
	 *
	 * While the viewport is a flat 2d plane, this method still returns a
	 * 3d point.  The z-coordinate corresponds to the position of the point
	 * in the z-buffer.
	 *
	 * @param worldCoords   The point in world coordinates
	 * @param viewport      The screen viewport
	 *
	 * @return the window space equivalent of a point in world coordinates.
 	 */
	Vec3 project(const Vec3& worldCoords, const Rect& viewport) const;

    /**
     * Returns the screen space equivalent of a point in world coordinates.
     *
     * Ideally, window space and screen space would be the same space.  They
     * are both defined by the viewport and have the same offset and dimension.
     * However, screen coordinates have the origin in the top left while window
     * coordinates have the origin in the bottom left.
     *
     * This method is important for converting world coordinates to event
     * coordinates (such as a mouse click).
     *
     * @param worldCoords   The point in wprld coordinates
     *
     * @return the screen space equivalent of a point in world coordinates.
     */
    Vec2 worldToScreenCoords(const Vec3& worldCoords) const {
        return windowToScreenCoords(project(worldCoords));
    }
    
    /**
     * Returns a picking Ray from the coordinates given in window coordinates.
     *
     * A picking ray is used to select an object in 3d space.  It creates a
     * ray into the screen based on a selection in the viewport. You can then
     * use this ray to select an object.
     *
     * Window coords are the location of the point in the viewport.  Ideally,
     * the viewport should be the same size as the screen, but these are not
     * screen coordinates.  The screen has the origin in the top left, while
     * window coordinates still have the origin in the bottom left.
     *
     * While the viewport is a flat 2d plane, this method still takes a
     * 3d point.  The z-coordinate corresponds to the position of the point
     * in the z-buffer.
     *
     * @param windowCoords  The point in window coordinates
     *
     * Returns a picking Ray from the coordinates given in window coordinates.
     */
    Ray getPickRay(const Vec3& windowCoords) const;

	/**
	 * Returns a picking Ray from the coordinates given in window coordinates.
	 *
	 * A picking ray is used to select an object in 3d space.  It creates a
	 * ray into the screen based on a selection in the viewport. You can then
	 * use this ray to select an object.
	 *
	 * Window coords are the location of the point in the viewport.  Ideally,
	 * the viewport should be the same size as the screen, but these are not
	 * screen coordinates.  The screen has the origin in the top left, while
	 * window coordinates still have the origin in the bottom left.
	 *
	 * While the viewport is a flat 2d plane, this method still takes a
	 * 3d point.  The z-coordinate corresponds to the position of the point
	 * in the z-buffer.
	 *
	 * @param windowCoords  The point in window coordinates
	 * @param viewport      The screen viewport
	 *
	 * Returns a picking Ray from the coordinates given in window coordinates.
	 */
	Ray getPickRay(const Vec3& windowCoords, const Rect& viewport) const;

    /**
     * Returns a picking Ray from the coordinates given in screen coordinates.
     *
     * A picking ray is used to select an object in 3d space.  It creates a
     * ray into the screen based on a selection in the viewport. You can then
     * use this ray to select an object.
     *
     * Screen coords differ from window coordinates, as the the origin is
     * in the top left instad of the bottom left.  The SDL event system uses
     * screen space for input events, which is why this method is necessary.
     *
     * @param screenCoords  The point in screen coordinates
     *
     * Returns a picking Ray from the coordinates given in screen coordinates.
     */
    Ray getPickRayFromScreen(const Vec2& screenCoords) const {
        return getPickRay(screenToWindowCoords(screenCoords));
    }
        
    /**
     * Returns the window space equivalent of a point in screen coordinates.
     *
     * Ideally, window space and screen space would be the same space.  They
     * are both defined by the viewport and have the same offset and dimension.
     * However, screen coordinates have the origin in the top left while window
     * coordinates have the origin in the bottom left.
     *
     * We need this conversion because events (such as mouse clicks) register
     * in screen space, while drawing happens in window space.
     *
     * @param windowCoords  The point in window coordinates
     */
    Vec2 windowToScreenCoords(const Vec3& windowCoords) const;

	/**
	 * Returns the window space equivalent of a point in screen coordinates.
	 *
	 * Ideally, window space and screen space would be the same space.  They
	 * are both defined by the viewport and have the same offset and dimension.
	 * However, screen coordinates have the origin in the top left while window
	 * coordinates have the origin in the bottom left.
	 *
	 * We need this conversion because events (such as mouse clicks) register
	 * in screen space, while drawing happens in window space.
	 *
	 * @param windowCoords  The point in window coordinates
	 * @param viewport      The screen viewport
	 */
	Vec2 windowToScreenCoords(const Vec3& windowCoords, const Rect& viewport) const;

    /**
     * Returns the screen space equivalent of a point in window coordinates.
     *
     * Ideally, window space and screen space would be the same space.  They
     * are both defined by the viewport and have the same offset and dimension.
     * However, screen coordinates have the origin in the top left while window
     * coordinates have the origin in the bottom left.
     *
     * We need this conversion because events (such as mouse clicks) register
     * in screen space, while drawing happens in window space.
     *
     * While the viewport is a flat 2d plane, this method still takes a
     * 3d point.  The z-coordinate corresponds to the near position, closest
     * to the screen.
     *
     * @param screenCoords  The point in screen coordinates
     */
    Vec3 screenToWindowCoords(const Vec2& screenCoords) const;

	/**
	 * Returns the screen space equivalent of a point in window coordinates.
	 *
	 * Ideally, window space and screen space would be the same space.  They
	 * are both defined by the viewport and have the same offset and dimension.
	 * However, screen coordinates have the origin in the top left while window
	 * coordinates have the origin in the bottom left.
	 *
	 * We need this conversion because events (such as mouse clicks) register
	 * in screen space, while drawing happens in window space.
	 *
	 * While the viewport is a flat 2d plane, this method still takes a
	 * 3d point.  The z-coordinate corresponds to the near position, closest
	 * to the screen.
	 *
	 * @param screenCoords  The point in screen coordinates
	 * @param viewport      The screen viewport
	 */
	Vec3 screenToWindowCoords(const Vec2& screenCoords, const Rect& viewport) const;
};
    
}

#endif /* __CU_CAMERA_H__ */

//
//  CUCamera.cpp
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

#include <cugl/renderer/CUCamera.h>
#include <cugl/base/CUApplication.h>

using namespace cugl;

#pragma mark Camera Set-Up
/**
 * Creates a degenerate camera.
 *
 * All of the matrices are the identity and the viewport is empty.
 */
Camera::Camera() :
_position(Vec3::ZERO),
_direction(-Vec3::UNIT_Z),
_upwards(Vec3::UNIT_Y),
_near(1),
_far(100) {
	_projection = Mat4::IDENTITY;
	_modelview = Mat4::IDENTITY;
	_combined = Mat4::IDENTITY;
	_inverse = Mat4::IDENTITY;
}

/**
 * Returns this camera to the denegerate one.
 *
 * All of the matrices will be the identity and the viewport will be empty.
 */
void Camera::dispose() {
    _position  = Vec3::ZERO;
    _direction = -Vec3::UNIT_Z;
    _upwards    = Vec3::UNIT_Y;
    _near = 1;
    _far = 100;
    _projection = Mat4::IDENTITY;
    _modelview = Mat4::IDENTITY;
    _combined = Mat4::IDENTITY;
    _inverse = Mat4::IDENTITY;
    
    _viewport.set(Vec2::ZERO,Size::ZERO);
}

/**
 * Sets the direction of the camera to look at the given point.
 *
 * This function assumes the up vector is normalized.
 *
 * @param target    The point to look at
 */
void Camera::lookAt(const Vec3& target) {
    Vec3 temp = target-_position;
    temp.normalize();
    if (!temp.isZero()) {
        // Up and direction must ALWAYS be orthonormal vectors
        float dot = temp.dot(_upwards);
        if (fabsf(dot - 1) < CU_MATH_EPSILON) {
            // Collinear
            _upwards = -_direction;
        } else if (fabsf(dot + 1) < CU_MATH_EPSILON) {
            // Collinear opposite
            _upwards = _direction;
        }
        _direction = temp;
        
        _direction.set(temp);
        normalizeUp();
    }
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
void Camera::rotateAround(const Vec3& point, const Vec3& axis, float angle) {
    Mat4 temp;
    Mat4::createTranslation(-point.x,-point.y,-point.z,&temp);
    temp.rotate(axis,angle);
    temp.translate(point);
    _direction *= temp;
    _upwards *= temp;
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
Vec3 Camera::unproject(const Vec3& windowCoords) const {
    return unproject(windowCoords, Rect(Vec2::ZERO,Application::get()->getDisplaySize()));
}

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
Vec3 Camera::unproject(const Vec3& windowCoords, const Rect& viewport) const {
    Vec4 temp;
    temp.x = 2*(windowCoords.x-viewport.origin.x) / (viewport.size.width) - 1;
    temp.y = 2*(windowCoords.y-viewport.origin.y) / (viewport.size.height) - 1;
    temp.z = 2 * windowCoords.z - 1;
    temp.w = 1;
    temp *= _inverse;
    
    Vec3 result(temp.x,temp.y,temp.z);
    return result;
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
Vec3 Camera::project(const Vec3& worldCoords) const {
    return project(worldCoords, Rect(Vec2::ZERO,Application::get()->getDisplaySize()));
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
 * @param worldCoords   The point in world coordinates
 * @param viewport      The screen viewport
 *
 * @return the window space equivalent of a point in world coordinates.
 */
Vec3 Camera::project(const Vec3& worldCoords, const Rect& viewport) const {
    Vec4 temp(worldCoords,1);
    temp *= _combined;
    
    Vec3 result;
    result.x = viewport.size.width  * (temp.x + 1) / 2 + viewport.origin.x;
    result.y = viewport.size.height * (temp.y + 1) / 2 + viewport.origin.y;
    result.z = (temp.z + 1) / 2;
    return result;
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
 * @param viewport      The screen viewport
 *
 * Returns a picking Ray from the coordinates given in window coordinates.
 */
Ray Camera::getPickRay(const Vec3& windowCoords, const Rect& viewport) const {
    Ray result;
    Vec3 temp = windowCoords; temp.z = 0;
    result.origin    = unproject(temp,viewport);
    temp.z = 1;
    result.direction = unproject(temp,viewport);
    result.direction -= result.origin;
    result.direction.normalize();
    return result;
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
Vec2 Camera::windowToScreenCoords(const Vec3& windowCoords) const {
    return windowToScreenCoords(windowCoords, Rect(Vec2::ZERO,Application::get()->getDisplaySize()));
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
 * @param viewport      The screen viewport
 */
Vec2 Camera::windowToScreenCoords(const Vec3& windowCoords, const Rect& viewport) const {
    Vec2 result = windowCoords;
    result.y = viewport.size.height-result.y;
    return result;
}

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
Vec3 Camera::screenToWindowCoords(const Vec2& screenCoords) const {
    return screenToWindowCoords(screenCoords, Rect(Vec2::ZERO,Application::get()->getDisplaySize()));
}

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
Vec3 Camera::screenToWindowCoords(const Vec2& screenCoords, const Rect& viewport) const {
    Vec3 result(screenCoords.x,screenCoords.y,_near);
    result.y = viewport.size.height-result.y;
    return result;
}

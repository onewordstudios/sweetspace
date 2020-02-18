//
//  CUOrthographicCamera.h
//  Cornell University Game Library (CUGL)
//
//  This module provides an Orthographic camera class. As cugl's primary support
//  is for 2d (mobile) gameplay it is our primary camera class.
//
//  This module is based on the original file OrthographicCamera.java by
//  Mario Zechner, written for LibGDX ( https://libgdx.badlogicgames.com ). It
//  has been modified to support the CUGL framework.
//
//  This class uses our standard shared-pointer architecture.
//
//  1. The constructor does not perform any initialization; it just sets all
//     attributes to their defaults.
//
//  2. All initialization takes place via init methods, which can fail if an
//     object is initialized more than once.
//
//  3. All allocation takes place via static constructors which return a shared
//     pointer.
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

#include <cugl/renderer/CUOrthographicCamera.h>
#include <cugl/base/CUApplication.h>
#include <cugl/util/CUDebug.h>

using namespace cugl;

/**
 * Initializes a new OrthographicCamera for the given viewport.
 *
 * Offseting the viewport origin has no affect on the position attribute.
 * It only affects the coordinate conversion methods {@link Camera#project()}
 * and {@link Camera#unproject()}. It is supposed to represent the offset
 * of the viewport in a larger canvas.
 *
 * If yDown is true, the origin will be at the top left corner (similar
 * to screen coordinates).  Otherwise, it will place the origin at the
 * traditional OpenGL origin (bottom left corner).
 *
 * @param x         The viewport x offset
 * @param y         The viewport y offset
 * @param width     The viewport width
 * @param height    The viewport height
 * @param yDown     Whether to put the origin in the top left corner
 *
 * @return true if initialization was successful.
 */
bool OrthographicCamera::initOffset(float x, float y, float width, float height, bool yDown) {
    if (_initialized) {
        CUAssertLog(false, "Camera has already been initialized");
        return false;   // In case asserts are off
    }
    
    set(x,y,width,height,yDown);
    _initialized = true;
    return true;
}

/**
 * Returns this camera to the denegerate one, with all matrices the identity.
 */
void OrthographicCamera::dispose() {
    Camera::dispose();
    _zoom = 1; _near = 0; _initialized = false;
}

/**
 * Sets this OrthographicCamera to have the given viewport.
 *
 * Offseting the viewport origin has no affect on the position attribute.
 * It only affects the coordinate conversion methods {@link Camera#project()}
 * and {@link Camera#unproject()}. It is supposed to represent the offset
 * of the viewport in a larger canvas.
 *
 * If yDown is true, the origin will be at the top left corner (similar
 * to screen coordinates).  Otherwise, it will place the origin at the
 * traditional OpenGL origin (bottom left corner).
 *
 * @param x         The viewport x offset
 * @param y         The viewport y offset
 * @param width     The viewport width
 * @param height    The viewport height
 * @param yDown     Whether to put the origin in the top left corner
 */
void OrthographicCamera::set(float x, float y, float width, float height, bool yDown) {
    if (yDown) {
        _upwards.set(0, -1, 0);
        _direction.set(0, 0, 1);
    } else {
        _upwards.set(0, 1, 0);
        _direction.set(0, 0, -1);
    }
    if (!_initialized) {
        _position.set( width / (2.0f*_zoom), height / (2.0f*_zoom), 0);
    } else {
        Vec2 offset = _position - Vec3( _viewport.size.width / (2.0f*_zoom), _viewport.size.height / (2.0f*_zoom), 0);
        _position.set( width / (2.0f*_zoom), height / (2.0f*_zoom), 0);
        _position += offset;
    }
    _viewport.size.width  = width;
    _viewport.size.height = height;
    update();
}

/**
 * Recalculates the projection and view matrix of this camera.
 *
 * Use this after you've manipulated any of the attributes of the camera.
 */
void OrthographicCamera::update() {
    float invzoom = 1/_zoom;
    Mat4::createOrthographic(invzoom*_viewport.size.width,invzoom*_viewport.size.height,
                             _near,_far,&_projection);
    Mat4::createLookAt(_position,_position+_direction,_upwards,&_modelview);
    Mat4::multiply(_modelview,_projection,&_combined);
    Mat4::invert(_combined,&_inverse);
}

/**
 * Sets the magnification zoom of this camera.
 *
 * Large values make the images larger.  So, 2x magnification makes images
 * twices as large and effectively halves the viewport.
 *
 * Changing this value will have no effect on the underlying matrices
 * until you call the update() method.
 *
 * @param zoom  The magnification zoom
 */
void OrthographicCamera::setZoom(float zoom) {
    // Update the position first.
    Vec3 origin(_viewport.size.width / (2.0f*_zoom), _viewport.size.height / (2.0f*_zoom), 0);
    Vec3 offset = _position-origin;
    origin.set(_viewport.size.width / (2.0f*zoom), _viewport.size.height / (2.0f*zoom), 0);
    _position = origin+offset;
    _zoom = zoom;
}


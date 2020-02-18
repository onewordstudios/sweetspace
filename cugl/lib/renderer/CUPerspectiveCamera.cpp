//
//  CUPerspectiveCamera.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides an perspective camera class. While 3d is not a primary
//  use case for cugl, it is nice to have the support.
//
//  This module is based on the original file PerspectiveCamera by
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

#include <cugl/renderer/CUPerspectiveCamera.h>
#include <cugl/util/CUDebug.h>

using namespace cugl;

/**
 * Initializes a new PerspectiveCamera for the given viewport and field of view.
 *
 * The field of view given is for the height, and is measured in degrees.
 * The field of view for the width will be calculated according to the
 * aspect ratio. Furthermore, the aspect ratio is derived from the viewport
 * size.
 *
 * Offseting the viewport origin has no affect on the position attribute.
 * It only affects the coordinate conversion methods {@link Camera#project()}
 * and {@link Camera#unproject()}. It is supposed to represent the offset
 * of the viewport in a larger canvas.
 *
 * @param x             The viewport x offset
 * @param y             The viewport y offset
 * @param width         The viewport width
 * @param height        The viewport height
 * @param fieldOfView   The field of view of the height, in degrees
 *
 * @return true if initialization was successful.
 */
bool PerspectiveCamera::init(float x, float y, float width, float height, float fieldOfView) {
    if (_initialized) {
        CUAssertLog(false, "Camera has already been initialized");
        return false;   // In case asserts are off
    }

    set(x,y,width,height,fieldOfView);
    _initialized = true;
    return true;
}

/**
 * Returns this camera to the denegerate one, with all matrices the identity.
 */
void PerspectiveCamera::dispose() {
    Camera::dispose();
    _fieldOfView = 0; _initialized = false;
}

/**
 * Sets this PerspectiveCamera to have the given viewport and field of view.
 *
 * The field of view given is for the height, and is measured in degrees.
 * The field of view for the width will be calculated according to the
 * aspect ratio. Furthermore, the aspect ratio is derived from the viewport
 * size.
 *
 * Offseting the viewport origin has no affect on the position attribute.
 * It only affects the coordinate conversion methods {@link Camera#project()}
 * and {@link Camera#unproject()}. It is supposed to represent the offset
 * of the viewport in a larger canvas.
 *
 * @param x             The viewport x offset
 * @param y             The viewport y offset
 * @param width         The viewport width
 * @param height        The viewport height
 * @param fieldOfView   The field of view of the height, in degrees
 */
void PerspectiveCamera::set(float x, float y, float width, float height, float fieldOfView) {
    _fieldOfView = fieldOfView;
    _viewport.origin.x  = x;
    _viewport.origin.y  = y;
    _viewport.size.width  = width;
    _viewport.size.height = height;
    update();
}

/**
 * Recalculates the projection and view matrix of this camera.
 *
 * Use this after you've manipulated any of the attributes of the camera.
 */
void PerspectiveCamera::update() {
    float aspect = _viewport.size.width / _viewport.size.height;
    Mat4::createPerspective(_fieldOfView, aspect, _near, _far,  &_projection);
    Mat4::createLookAt(_position,_position+_direction,_upwards,&_modelview);
    Mat4::multiply(_modelview,_projection,&_combined);
    Mat4::invert(_combined,&_inverse);
}

/**
 * Sets the field of view for the given camera.
 *
 * The field of view given is for the height, and is measured in degrees.
 * The field of view for the width will be calculated according to the
 * aspect ratio. Furthermore, the aspect ratio is derived from the viewport
 * size.
 *
 * Changing this value will have no effect on the underlying matrices
 * until you call the update() method.
 *
 * @param fov   the field of view for the given camera.
 */
void PerspectiveCamera::setFieldOfView(float fov) {
    CUAssertLog(fov >= 0,   "Field of view is negative");
    CUAssertLog(fov <= 180, "Field of view is too large");
    _fieldOfView = fov;
}

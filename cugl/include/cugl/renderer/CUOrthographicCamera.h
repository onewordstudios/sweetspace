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

#ifndef __CU_ORTHOGRAPHIC_CAMERA_H__
#define __CU_ORTHOGRAPHIC_CAMERA_H__

#include "CUCamera.h"

namespace cugl {
    
/** 
 * This is a class for a camera with orthographic projection.
 *
 * This camera is used by the Scene class.
 */
class OrthographicCamera : public Camera {
#pragma mark Values
protected:
    /** The magnification zoom of the camera */
    float _zoom;

    /** Whether or not the camera has been initialized */
    bool _initialized;
    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates a degenerate camera.
     *
     * All of the matrices are the identity and the viewport is empty.
     */
    OrthographicCamera() : Camera() {
        _zoom = 1; _near = 0; _initialized = false;
    }
    
    /**
     * Deletes this camera, disposing all resources.
     */
    ~OrthographicCamera() { dispose(); }
    
    /**
     * Returns this camera to the denegerate one, with all matrices the identity.
     */
    void dispose() override;
    
    /** 
     * Initializes an OrthographicCamera for the given viewport.
     *
     * The viewport origin is assumed to be (0,0).
     *
     * If yDown is true, the origin will be at the top left corner (similar
     * to screen coordinates).  Otherwise, it will place the origin at the
     * traditional OpenGL origin (bottom left corner).
     *
     * @param size      The viewport size
     * @param yDown     Whether to put the origin in the top left corner
     *
     * @return true if initialization was successful.
     */
    bool init(const Size& size, bool yDown=false) {
        return initOffset(0,0,size.width,size.height,yDown);
    }

    /**
     * Initializes an OrthographicCamera for the given viewport.
     *
     * The viewport origin is assumed to be (0,0).
     *
     * If yDown is true, the origin will be at the top left corner (similar
     * to screen coordinates).  Otherwise, it will place the origin at the
     * traditional OpenGL origin (bottom left corner).
     *
     * @param width     The viewport width
     * @param height    The viewport height
     * @param yDown     Whether to put the origin in the top left corner
     *
     * @return true if initialization was successful.
     */
    bool init(float width, float height, bool yDown=false) {
        return initOffset(0,0,width,height,yDown);
    }

    /**
     * Initializes an OrthographicCamera for the given viewport.
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
     * @param rect      The viewport bounding box
     * @param yDown     Whether to put the origin in the top left corner
     *
     * @return true if initialization was successful.
     */
    bool initOffset(const Rect& rect, bool yDown=false) {
        return initOffset(rect.origin.x,rect.origin.y,rect.size.width,rect.size.height,yDown);
    }

    /**
     * Initializes an OrthographicCamera for the given viewport.
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
     * @param origin    The viewport offset
     * @param size      The viewport size
     * @param yDown     Whether to put the origin in the top left corner
     *
     * @return true if initialization was successful.
     */
    bool initOffset(const Vec2& origin, const Size& size, bool yDown=false) {
        return initOffset(origin.x,origin.y,size.width,size.height,yDown);
    }

    /**
     * Initializes an OrthographicCamera for the given viewport.
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
    bool initOffset(float x, float y, float width, float height, bool yDown=false);
    
#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a newly allocated OrthographicCamera for the given viewport.
     *
     * The viewport origin is assumed to be (0,0).
     *
     * If yDown is true, the origin will be at the top left corner (similar
     * to screen coordinates).  Otherwise, it will place the origin at the
     * traditional OpenGL origin (bottom left corner).
     *
     * @param size      The viewport size
     * @param yDown     Whether to put the origin in the top left corner
     *
     * @return a newly allocated OrthographicCamera for the given viewport.
     */
    static std::shared_ptr<OrthographicCamera> alloc(const Size& size, bool yDown=false) {
        std::shared_ptr<OrthographicCamera> result = std::make_shared<OrthographicCamera>();
        return(result->init(size,yDown) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated OrthographicCamera for the given viewport.
     *
     * The viewport origin is assumed to be (0,0).
     *
     * If yDown is true, the origin will be at the top left corner (similar
     * to screen coordinates).  Otherwise, it will place the origin at the
     * traditional OpenGL origin (bottom left corner).
     *
     * @param width     The viewport width
     * @param height    The viewport height
     * @param yDown     Whether to put the origin in the top left corner
     *
     * @return a newly allocated OrthographicCamera for the given viewport.
     */
    static std::shared_ptr<OrthographicCamera> alloc(float width, float height, bool yDown=false) {
        std::shared_ptr<OrthographicCamera> result = std::make_shared<OrthographicCamera>();
        return (result->init(width,height,yDown) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated OrthographicCamera for the given viewport.
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
     * @param rect      The viewport bounding box
     * @param yDown     Whether to put the origin in the top left corner
     *
     * @return a newly allocated OrthographicCamera for the given viewport.
     */
    static std::shared_ptr<OrthographicCamera> allocOffset(const Rect& rect, bool yDown=false) {
        std::shared_ptr<OrthographicCamera> result = std::make_shared<OrthographicCamera>();
        return (result->initOffset(rect,yDown) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated OrthographicCamera for the given viewport.
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
     * @param origin    The viewport offset
     * @param size      The viewport size
     * @param yDown     Whether to put the origin in the top left corner
     *
     * @return a newly allocated OrthographicCamera for the given viewport.
     */
    static std::shared_ptr<OrthographicCamera> allocOffset(const Vec2& origin, const Size& size,
                                                           bool yDown=false) {
        std::shared_ptr<OrthographicCamera> result = std::make_shared<OrthographicCamera>();
        return (result->initOffset(origin,size,yDown) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated OrthographicCamera for the given viewport.
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
     * @return a newly allocated OrthographicCamera for the given viewport.
     */
    static std::shared_ptr<OrthographicCamera> allocOffset(float x, float y, float width, float height,
                                                           bool yDown=false) {
        std::shared_ptr<OrthographicCamera> result = std::make_shared<OrthographicCamera>();
        return (result->initOffset(x,y,width,height,yDown) ? result : nullptr);
    }
    
#pragma mark -
#pragma mark Setters
    /**
     * Sets this OrthographicCamera to have the given viewport.
     *
     * The viewport origin is assumed to be (0,0).
     *
     * If yDown is true, the origin will be at the top left corner (similar
     * to screen coordinates).  Otherwise, it will place the origin at the
     * traditional OpenGL origin (bottom left corner).
     *
     * @param size      The viewport size
     * @param yDown     Whether to put the origin in the top left corner
     */
    void set(const Size& size, bool yDown=false) {
        set(0,0,size.width,size.height,yDown);
    }
    
    /**
     * Sets this OrthographicCamera to have the given viewport.
     *
     * The viewport origin is assumed to be (0,0).
     *
     * If yDown is true, the origin will be at the top left corner (similar
     * to screen coordinates).  Otherwise, it will place the origin at the
     * traditional OpenGL origin (bottom left corner).
     *
     * @param width     The viewport width
     * @param height    The viewport height
     * @param yDown     Whether to put the origin in the top left corner
     */
    void set(float width, float height, bool yDown=false) {
        set(0,0,width,height,yDown);
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
     * @param rect      The viewport bounding box
     * @param yDown     Whether to put the origin in the top left corner
     */
    void set(const Rect& rect, bool yDown=false) {
        set(rect.origin.x,rect.origin.y,rect.size.width,rect.size.height,yDown);
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
     * @param origin    The viewport offset
     * @param size      The viewport size
     * @param yDown     Whether to put the origin in the top left corner
     */
    void set(const Vec2& origin, const Size& size, bool yDown=false) {
        set(origin.x,origin.y,size.width,size.height,yDown);
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
    void set(float x, float y, float width, float height, bool yDown=false);

    /**
     * Recalculates the projection and view matrix of this camera.
     *
     * Use this after you've manipulated any of the attributes of the camera.
     */
    void update() override;
    
    
#pragma mark -
#pragma mark Attributes
    /**
     * Returns the magnification zoom of this camera.
     *
     * Large values make the images larger.  So, 2x magnification makes images
     * twices as large and effectively halves the viewport.
     *
     * @return the magnification zoom of this camera.
     */
    float getZoom() const { return _zoom; }

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
    void setZoom(float zoom);
    
};
    
}

#endif /* __CU_ORTHOGRAPHIC_CAMERA_H__ */

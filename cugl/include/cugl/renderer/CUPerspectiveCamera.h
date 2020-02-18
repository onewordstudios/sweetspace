//
//  CUPerspectiveCamera.h
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

#ifndef __CU_PERSPECTIVE_CAMERA_H__
#define __CU_PERSPECTIVE_CAMERA_H__


#endif /* __CU_PERSPECTIVE_CAMERA_H__ */

#define DEFAULT_CAMERA_FOV 67

#include "CUCamera.h"

namespace cugl {
    
/**
 * This is a class for a camera with a perspective projection.
 *
 * This camera is used for 3d rendering.
 */
class PerspectiveCamera : public Camera {
protected:
    /** The field of view of the height, in degrees **/
    float _fieldOfView = 67;
 
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
    PerspectiveCamera() : Camera() {
        _fieldOfView = 0; _initialized = false;
    }
    
    /**
     * Deletes this camera, disposing all resources.
     */
    ~PerspectiveCamera() { dispose(); }
    
    
    /**
     * Returns this camera to the denegerate one, with all matrices the identity.
     */
    void dispose() override;
    
    /**
     * Initializes a new PerspectiveCamera for the given viewport and field of view.
     *
     * The field of view given is for the height, and is measured in degrees.
     * The field of view for the width will be calculated according to the
     * aspect ratio. Furthermore, the aspect ratio is derived from the viewport
     * size.
     *
     * The viewport origin is assumed to be (0,0).
     *
     * @param size          The viewport size
     * @param fieldOfView   The field of view of the height, in degrees
     *
     * @return true if initialization was successful.
     */
    bool init(const Size& size, float fieldOfView = DEFAULT_CAMERA_FOV) {
        return init(0,0,size.width,size.height,fieldOfView);
    }

    /**
     * Initializes a new PerspectiveCamera for the given viewport and field of view.
     *
     * The field of view given is for the height, and is measured in degrees.
     * The field of view for the width will be calculated according to the
     * aspect ratio. Furthermore, the aspect ratio is derived from the viewport
     * size.
     *
     * The viewport origin is assumed to be (0,0).
     *
     * @param width         The viewport width
     * @param height        The viewport height
     * @param fieldOfView   The field of view of the height, in degrees
     *
     * @return true if initialization was successful.
     */
    bool init(float width, float height, float fieldOfView = DEFAULT_CAMERA_FOV) {
        return init(0,0,width,height,fieldOfView);
    }

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
     * @param rect          The viewport bounding box
     * @param fieldOfView   The field of view of the height, in degrees
     *
     * @return true if initialization was successful.
     */
    bool init(const Rect& rect, float fieldOfView = DEFAULT_CAMERA_FOV) {
        return init(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height,fieldOfView);
    }

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
     * @param origin        The viewport offset
     * @param size          The viewport size
     * @param fieldOfView   The field of view of the height, in degrees
     *
     * @return true if initialization was successful.
     */
    bool init(const Vec2& origin, const Size& size, float fieldOfView = DEFAULT_CAMERA_FOV) {
        return init(origin.x, origin.y, size.width, size.height,fieldOfView);
    }
    
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
    bool init(float x, float y, float width, float height, float fieldOfView = DEFAULT_CAMERA_FOV);
    
#pragma mark -
#pragma mark Static Constructors
    /**
     * Initializes a new PerspectiveCamera for the given viewport and field of view.
     *
     * The field of view given is for the height, and is measured in degrees.
     * The field of view for the width will be calculated according to the
     * aspect ratio. Furthermore, the aspect ratio is derived from the viewport
     * size.
     *
     * The viewport origin is assumed to be (0,0).
     *
     * @param size          The viewport size
     * @param fieldOfView   The field of view of the height, in degrees
     *
     * @return true if initialization was successful.
     */
    static std::shared_ptr<PerspectiveCamera> alloc(const Size& size, float fieldOfView = DEFAULT_CAMERA_FOV) {
        std::shared_ptr<PerspectiveCamera> result = std::make_shared<PerspectiveCamera>();
        return (result->init(size,fieldOfView) ? result : nullptr);
    }
    
    /**
     * Initializes a new PerspectiveCamera for the given viewport and field of view.
     *
     * The field of view given is for the height, and is measured in degrees.
     * The field of view for the width will be calculated according to the
     * aspect ratio. Furthermore, the aspect ratio is derived from the viewport
     * size.
     *
     * The viewport origin is assumed to be (0,0).
     *
     * @param width         The viewport width
     * @param height        The viewport height
     * @param fieldOfView   The field of view of the height, in degrees
     *
     * @return true if initialization was successful.
     */
    static std::shared_ptr<PerspectiveCamera> alloc(float width, float height,
                                                    float fieldOfView = DEFAULT_CAMERA_FOV) {
        std::shared_ptr<PerspectiveCamera> result = std::make_shared<PerspectiveCamera>();
        return (result->init(width,height,fieldOfView) ? result : nullptr);
    }
    
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
     * @param rect          The viewport bounding box
     * @param fieldOfView   The field of view of the height, in degrees
     *
     * @return true if initialization was successful.
     */
    static std::shared_ptr<PerspectiveCamera> alloc(const Rect& rect, float fieldOfView = DEFAULT_CAMERA_FOV) {
        std::shared_ptr<PerspectiveCamera> result = std::make_shared<PerspectiveCamera>();
        return (result->init(rect,fieldOfView) ? result : nullptr);
    }
    
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
     * @param origin        The viewport offset
     * @param size          The viewport size
     * @param fieldOfView   The field of view of the height, in degrees
     *
     * @return true if initialization was successful.
     */
    static std::shared_ptr<PerspectiveCamera> alloc(const Vec2& origin, const Size& size,
                                                    float fieldOfView = DEFAULT_CAMERA_FOV) {
        std::shared_ptr<PerspectiveCamera> result = std::make_shared<PerspectiveCamera>();
        return (result->init(origin,size,fieldOfView) ? result : nullptr);
    }
    
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
    static std::shared_ptr<PerspectiveCamera> alloc(float x, float y, float width, float height,
                                                    float fieldOfView = DEFAULT_CAMERA_FOV) {
        std::shared_ptr<PerspectiveCamera> result = std::make_shared<PerspectiveCamera>();
        return (result->init(x,y,width,height,fieldOfView) ? result : nullptr);
    }
    
#pragma mark -
#pragma mark Setters
    /**
     * Sets this PerspectiveCamera to have the given viewport and field of view.
     *
     * The field of view given is for the height, and is measured in degrees.
     * The field of view for the width will be calculated according to the
     * aspect ratio. Furthermore, the aspect ratio is derived from the viewport
     * size.
     *
     * The viewport origin is assumed to be (0,0).
     *
     * @param size          The viewport size
     * @param fieldOfView   The field of view of the height, in degrees
     */
    void set(const Size& size, float fieldOfView = DEFAULT_CAMERA_FOV) {
        set(0, 0, size.width, size.height, fieldOfView);
    }
    
    /**
     * Sets this PerspectiveCamera to have the given viewport and field of view.
     *
     * The field of view given is for the height, and is measured in degrees.
     * The field of view for the width will be calculated according to the
     * aspect ratio. Furthermore, the aspect ratio is derived from the viewport
     * size.
     *
     * The viewport origin is assumed to be (0,0).
     *
     * @param width         The viewport width
     * @param height        The viewport height
     * @param fieldOfView   The field of view of the height, in degrees
     */
    void set(float width, float height, float fieldOfView = DEFAULT_CAMERA_FOV) {
        set(0, 0, width, height, fieldOfView);
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
     * @param rect          The viewport bounding box
     * @param fieldOfView   The field of view of the height, in degrees
     */
    void set(const Rect& rect, float fieldOfView = DEFAULT_CAMERA_FOV) {
        set(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height, fieldOfView);
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
     * @param origin        The viewport offset
     * @param size          The viewport size
     * @param fieldOfView   The field of view of the height, in degrees
     */
    void set(const Vec2& origin, const Size& size, float fieldOfView = DEFAULT_CAMERA_FOV) {
        set(origin.x, origin.y, size.width, size.height, fieldOfView);
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
    void set(float x, float y, float width, float height, float fieldOfView = DEFAULT_CAMERA_FOV);
    
    /**
     * Recalculates the projection and view matrix of this camera.
     *
     * Use this after you've manipulated any of the attributes of the camera.
     */
    void update() override;
    
#pragma mark -
#pragma mark Attributes
    /**
     * Returns the field of view for the given camera.
     *
     * The field of view given is for the height, and is measured in degrees.
     * The field of view for the width will be calculated according to the
     * aspect ratio. Furthermore, the aspect ratio is derived from the viewport
     * size.
     *
     * @return the field of view for the given camera.
     */
    float getFieldOfView() const { return _fieldOfView; }

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
    void setFieldOfView(float fov);
    
};

}

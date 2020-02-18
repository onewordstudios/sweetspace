//
//  CUAnimationNode.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a straight-forward filmstrip API, similar to what we
//  use in the intro class.  Note that this class extends PolygonNode, as it
//  simply treats the node texture as a sprite sheet. Therefore, it is possible
//  to animate the filmstrip over polygons.  However, this can have undesirable
//  effects if the polygon coordinates extend beyond a single animation frame.
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
//  Version: 12/1/16
//
#ifndef __CU_ANIMATION_NODE_H__
#define __CU_ANIMATION_NODE_H__

#include <cugl/2d/CUPolygonNode.h>
#include <cugl/math/CURect.h>
#include <cugl/renderer/CUTexture.h>


namespace cugl {

#pragma mark -
#pragma mark AnimationNode

/**
 * Class to support simple film strip animation
 *
 * The API for this class is very similar to {@link PolygonNode}, except that it
 * treats the texture as a sprite sheet.  This means that you must specify the 
 * rows and columns in the sprite sheet so that it can break up the images for 
 * you.
 *
 * The basic constructors always set this object equal to a rectangle the same 
 * size as a single frame in the sprite sheet.  However, you could conceivably 
 * animate the filmstrip over polygons, simpy by changing the shape via 
 * {@link setPolygon}.  This can have undesirable effects if the polygon 
 * coordinate extend beyond a single animation frame.  The basic renderer does 
 * not allow us to wrap a single frame of a texture atlas.
 *
 * For example, suppose you have a filmstrip where each frame has a given width
 * and height.  Setting the polygon to a triangle with vertices (0,0), 
 * (width/2, height), and (width,height) is okay.  However, the vertices (0,0), 
 * (width, 2*height), and (2*width, height) are not okay.
 */
class AnimationNode : public PolygonNode {
protected:
    /** The number of columns in this filmstrip */
    int _cols;
    /** The number of frames in this filmstrip */
    int _size;
    /** The active animation frame */
    int _frame;
    /** The size of a single animation frame (different from active polygon) */
    Rect _bounds;
   
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Constructs a ActionNode with no filmstrip
     *
     * You must initialize this object before use.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    AnimationNode();
    
    /**
     * Releases all resources allocated with this node.
     *
     * This will release, but not necessarily delete the associated texture.
     * However, the polygon and drawing commands will be deleted and no
     * longer safe to use.
     */
    ~AnimationNode() { dispose(); }

    /**
     * Initializes the film strip with the given texture.
     *
     * This initializer assumes that the filmstrip is rectangular, and that
     * there are no unused frames.
     *
     * The size of the node is equal to the size of a single frame in the
     * filmstrip. To resize the node, scale it up or down.  Do NOT change the
     * polygon, as that will interfere with the animation.
     *
     * @param texture   The texture image to use
     * @param rows      The number of rows in the filmstrip
     * @param cols      The number of columns in the filmstrip
     *
     * @return  true if the filmstrip is initialized properly, false otherwise.
     */
    bool initWithFilmstrip(const std::shared_ptr<Texture>& texture, int rows, int cols) {
        return initWithFilmstrip(texture,rows,cols,rows*cols);
    }
    
    /**
     * Initializes the film strip with the given texture.
     *
     * The parameter size is to indicate that there are unused frames in the
     * filmstrip.  The value size must be less than or equal to rows*cols, or
     * this constructor will raise an error.
     *
     * The size of the node is equal to the size of a single frame in the 
     * filmstrip. To resize the node, scale it up or down.  Do NOT change the 
     * polygon, as that will interfere with the animation.
     *
     * @param texture   The texture image to use
     * @param rows      The number of rows in the filmstrip
     * @param cols      The number of columns in the filmstrip
     * @param size      The number of frames in the filmstrip
     *
     * @return  true if the filmstrip is initialized properly, false otherwise.
     */
    bool initWithFilmstrip(const std::shared_ptr<Texture>& texture, int rows, int cols, int size);
    
    /**
     * Initializes a node with the given JSON specificaton.
     *
     * This initializer is designed to receive the "data" object from the
     * JSON passed to {@link SceneLoader}.  This JSON format supports all
     * of the attribute values of its parent class.  In addition, it supports
     * the following additional attributes:
     *
     *      "span":     The number of frames in the filmstrip
     *      "cols":     An int specifying the number of columns
     *      "frame":    The initial starting frame.
     *
     * All attributes are optional.  However, if nothing is specified, it
     * assumes that this is a degenerate filmstrip with just one frame.  If
     * only span is specified, it assumes that it is just one row.
     *
     * @param loader    The scene loader passing this JSON file
     * @param data      The JSON object specifying the node
     *
     * @return true if initialization was successful.
     */
    virtual bool initWithData(const SceneLoader* loader, const std::shared_ptr<JsonValue>& data) override;

    
#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a newly allocated filmstrip node from the given texture.
     *
     * This constructor assumes that the filmstrip is rectangular, and that
     * there are no unused frames.
     *
     * The size of the node is equal to the size of a single frame in the
     * filmstrip. To resize the node, scale it up or down.  Do NOT change the
     * polygon, as that will interfere with the animation.
     *
     * @param texture   The texture image to use
     * @param rows      The number of rows in the filmstrip
     * @param cols      The number of columns in the filmstrip
     *
     * @return a newly allocated filmstrip node from the given texture.
     */
    static std::shared_ptr<AnimationNode> alloc(const std::shared_ptr<Texture>& texture,
                                                int rows, int cols) {
        std::shared_ptr<AnimationNode> node = std::make_shared<AnimationNode>();
        return (node->initWithFilmstrip(texture,rows,cols) ? node : nullptr);

    }
    
    /**
     * Returns a newly allocated filmstrip node from the given texture.
     *
     * The parameter size is to indicate that there are unused frames in the
     * filmstrip.  The value size must be less than or equal to rows*cols, or
     * this constructor will raise an error.
     *
     * The size of the node is equal to the size of a single frame in the
     * filmstrip. To resize the node, scale it up or down.  Do NOT change the
     * polygon, as that will interfere with the animation.
     *
     * @param texture   The texture image to use
     * @param rows      The number of rows in the filmstrip
     * @param cols      The number of columns in the filmstrip
     * @param size      The number of frames in the filmstrip
     *
     * @return a newly allocated filmstrip node from the given texture.
     */
    static std::shared_ptr<AnimationNode> alloc(const std::shared_ptr<Texture>& texture,
                                                int rows, int cols, int size) {
        std::shared_ptr<AnimationNode> node = std::make_shared<AnimationNode>();
        return (node->initWithFilmstrip(texture,rows,cols,size) ? node : nullptr);
    }
    
    /**
     * Returns a newly allocated node with the given JSON specificaton.
     *
     * This initializer is designed to receive the "data" object from the
     * JSON passed to {@link SceneLoader}.  This JSON format supports all
     * of the attribute values of its parent class.  In addition, it supports
     * the following additional attributes:
     *
     *      "span":     The number of frames in the filmstrip
     *      "cols":     An int specifying the number of columns
     *      "frame":    The initial starting frame.
     *
     * All attributes are optional.  However, if nothing is specified, it
     * assumes that this is a degenerate filmstrip with just one frame.  If
     * only span is specified, it assumes that it is just one row.
     *
     * @param loader    The scene loader passing this JSON file
     * @param data      The JSON object specifying the node
     *
     * @return a newly allocated node with the given JSON specificaton.
     */
    static std::shared_ptr<Node> allocWithData(const SceneLoader* loader,
                                               const std::shared_ptr<JsonValue>& data) {
        std::shared_ptr<AnimationNode> result = std::make_shared<AnimationNode>();
        if (!result->initWithData(loader,data)) { result = nullptr; }
        return std::dynamic_pointer_cast<Node>(result);
    }

    
#pragma mark -
#pragma mark Attribute Accessors
    /**
     * Returns the number of frames in this filmstrip.
     *
     * @return the number of frames in this filmstrip.
     */
    int getSize() const { return _size; }
    
    /**
     * Returns the current active frame.
     *
     * @return the current active frame.
     */
    unsigned int getFrame() const { return _frame; }
    
    /**
     * Sets the active frame as the given index.
     *
     * If the frame index is invalid, an error is raised.
     *
     * @param frame the index to make the active frame
     */
    void setFrame(int frame);

    
};

}

#endif /* __CU_ANIMATION_NODE_H__ */

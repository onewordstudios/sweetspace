//
//  CUWireNode.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a scene graph node that supports wireframes.  The
//  primary use case is to have a node that outlines physics bodies.
//
//  This class is loosely coupled with PathOutliner.  You can use PathOutliner
//  independent of the WireNode, but all functionality is present in this class.
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
//  Version: 6/27/16

#ifndef __CU_WIRE_NODE_H__
#define __CU_WIRE_NODE_H__

#include <string>
#include <cugl/2d/CUTexturedNode.h>
#include <cugl/math/CUPoly2.h>
#include <cugl/math/polygon/CUPathOutliner.h>

/** The number of segments in a wireframe ellipse */
#define WIRE_SEGMENTS 8

namespace cugl {
    
/**
 * This is a scene graph node to represent a wireframe
 *
 * The wireframes are lines, but they can still be textured.  However, generally
 * you will only want to create a wireframe with the degenerate texture (to draw
 * a solid, colored line). Hence, none of the static constructors take a texture.
 * You are free to update the texture after creation, if you wish.
 *
 * The node shape is stored as polygon. The wireframe shape is determined by the
 * polygon traversal.  There are three options, defined in {@link PathOutliner}.
 *
 *     OPEN:     The traversal is in order, but does not close the ends.
 *
 *     CLOSED:   The traversal is in order, and closes the ends.
 *
 *     INTERIOR: The traverse will outline the default triangulation.
 *
 * The default is traversal is CLOSED.
 *
 * The polygon is specified in image coordinates. Image coordinates are different
 * from texture coordinates. Their origin is at the bottom-left corner of the file,
 * and each pixel is one unit. This makes specifying to polygon more natural for
 * irregular shapes.
 *
 * This means that a polygon with vertices (0,0), (width,0), (width,height),
 * and (0,height) would be identical to a sprite node. However, a polygon with
 * vertices (0,0), (2*width,0), (2*width,2*height), and (0,2*height) would tile
 * the sprite (given the wrap settings) twice both horizontally and vertically.
 *
 * The content size of this node is defined by the size (but not the offset)
 * of the bounding box.  The anchor point is relative to this content size.
 * The default anchor point in TexturedNode is (0.5, 0.5).  This means that a
 * uniform translation of the polygon (in contrast to the node itself) will not
 * move the shape on the the screen.  Instead, it will just change the part of
 * the texture it uses.
 *
 * For example, suppose the texture has given width and height.  We have one
 * polygon with vertices (0,0), (width/2,0), (width/2,height/2), and (0,height/2).
 * We have another polygon with vertices (width/2,height/2), (width,height/2),
 * (width,height), and (width/2,height).  Both polygons would create a rectangle
 * of size (width/2,height/2). centered at the node position.  However, the
 * first would use the bottom left part of the texture, while the second would
 * use the top right.
 *
 * You can disable these features at any time by setting the attribute 
 * absolute to true.  Do this will place the polygon vertices in their 
 * absolute positions in Node space.  This will also disable anchor functions 
 * (setting the anchor as the bottom left corner), since anchors do not make 
 * sense when we are drawing vertices directly into the coordinate space.
 */
class WireNode : public TexturedNode {
#pragma mark Values
protected:
    /** An outliner for those incomplete polygons */
    static PathOutliner _outliner;
    
    /** The current (known) traversal of this wireframe */
    PathTraversal _traversal;

public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates an empty wireframe with the degenerate texture.
     *
     * You must initialize this WireNode before use.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    WireNode() : TexturedNode(), _traversal(PathTraversal::CLOSED) {
        _classname = "WireNode";
        _name = "WireNode";
    }
    
    /**
     * Releases all resources allocated with this node.
     *
     * This will release, but not necessarily delete the associated texture.
     * However, the polygon and drawing commands will be deleted and no
     * longer safe to use.
     */
    ~WireNode() { dispose(); }
    
    /**
     * Intializes a solid polygon with the given vertices and traversal
     *
     * You do not need to set the texture; rendering this into a SpriteBatch
     * will simply use the blank texture. Hence the wireframe will have a solid
     * color.
     *
     * The polygon will be outlined using the given traversal in PathOutliner.
     * WireNode objects share a single path outliner, so this initializer is
     * not thread safe.
     *
     * @param vertices  The vertices to texture (expressed in image space)
     * @param traversal The path traversal for index generation
     *
     * @return  true if the wireframe is initialized properly, false otherwise.
     */
    bool initWithVertices(const std::vector<Vec2>& vertices, PathTraversal traversal);
    
    /**
     * Initializes a wireframe that is a line from origin to destination.
     *
     * @param   origin  The line origin.
     * @param   dest    The line destination.
     *
     * @return true if the wireframe is initialized properly, false otherwise.
     */
    bool initWithLine(const Vec2 &origin, const Vec2 &dest);
    
    /**
     * Initializes a wireframe that is an ellipse with given the center and dimensions.
     *
     * The wireframe will show the boundary, not the circle tesselation.
     *
     * @param   center      The ellipse center point.
     * @param   size        The size of the ellipse.
     * @param   segments    The number of segments to use.
     *
     * @return true if the wireframe is initialized properly, false otherwise.
     */
    bool initWithEllipse(const Vec2& center, const Size& size, unsigned int segments = WIRE_SEGMENTS);
    
    /**
     * Initializes a node with the given JSON specificaton.
     *
     * This initializer is designed to receive the "data" object from the
     * JSON passed to {@link SceneLoader}.  This JSON format supports all
     * of the attribute values of its parent class.  In addition, it supports
     * the following additional attributes:
     *
     *      "texture":  The name of a previously loaded texture asset
     *      "polygon":  An even array of polygon vertices (numbers)
     *      "traveral": One of 'open', 'closed', or 'interior'
     *      "indices":  An array of unsigned ints defining triangles from the
     *                  the vertices. The array size should be a multiple of 3.
     *
     * All attributes are optional.  However, it is generally a good idea to
     * specify EITHER the texture or the polygon.  If you specify the indices,
     * then the traversal will be ignored.
     *
     * @param loader    The scene loader passing this JSON file
     * @param data      The JSON object specifying the node
     *
     * @return true if initialization was successful.
     */
    bool initWithData(const SceneLoader* loader, const std::shared_ptr<JsonValue>& data) override;

    
#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns an empty wireframe node.
     *
     * The underlying polygon is empty, and must be set via setPolygon.
     *
     * @return an empty wireframe node.
     */
    static std::shared_ptr<WireNode> alloc() {
        std::shared_ptr<WireNode> node = std::make_shared<WireNode>();
        return (node->init() ? node : nullptr);
    }
    
    /**
     * Returns a (closed) wireframe with the given vertices.
     *
     * The polygon will be outlined using a CLOSED traversal in PathOutliner.
     * To create a different traversal, use the alternate allocWithVertices()
     * constructor. All WireNode objects share a single path outliner, so this
     * method is not thread safe.
     *
     * @param vertices  The vertices forming the wireframe path
     *
     * @return a (closed) wireframe with the given vertices.
     */
    static std::shared_ptr<WireNode> allocWithVertices(const std::vector<Vec2>& vertices) {
        std::shared_ptr<WireNode> node = std::make_shared<WireNode>();
        return (node->init(vertices) ? node : nullptr);
    }
    
    /**
     * Returns a (closed) wireframe with the given vertices.
     *
     * The polygon will be outlined using the given traversal in PathOutliner.
     * WireNode objects share a single path outliner, so this constructor is
     * not thread safe.
     *
     * @param vertices  The vertices forming the wireframe path
     * @param traversal The path traversal for index generation
     *
     * @return a (closed) wireframe with the given vertices.
     */
    static std::shared_ptr<WireNode> allocWithVertices(const std::vector<Vec2>& vertices,
                                                       PathTraversal traversal) {
        std::shared_ptr<WireNode> node = std::make_shared<WireNode>();
        return (node->initWithVertices(vertices,traversal) ? node : nullptr);
    }
    
    /**
     * Returns a wireframe with the given polygon.
     *
     * The wireframe is a sequence of lines that is determined by the polygon 
     * indices. To create a specific traversal, use the {@link PathOutliner}
     * factory.
     *
     * @param   poly      The polygon for the wireframe path
     *
     * @return a wireframe with the given polygon.
     */
    static std::shared_ptr<WireNode> allocWithPoly(const Poly2& poly) {
        std::shared_ptr<WireNode> node = std::make_shared<WireNode>();
        return (node->init(poly) ? node : nullptr);
    }
    
    /**
     * Creates a wireframe with the given rect.
     *
     * The rectangle will be converted into a Poly2, and the traversal is closed.
     *
     * @param   rect    The rectangle for the wireframe path
     *
     * @return  An autoreleased wireframe node
     */
    static std::shared_ptr<WireNode> allocWithRect(const Rect& rect) {
        std::shared_ptr<WireNode> node = std::make_shared<WireNode>();
        return (node->init(rect) ? node : nullptr);
    }
    
    /**
     * Creates a wireframe that is a line from origin to destination.
     *
     * @param   origin  The line origin.
     * @param   dest    The line destination.
     *
     * @return  An autoreleased wireframe node
     */
    static std::shared_ptr<WireNode> allocWithLine(const Vec2 &origin, const Vec2 &dest) {
        std::shared_ptr<WireNode> node = std::make_shared<WireNode>();
        return (node->initWithLine(origin,dest) ? node : nullptr);
    }
    
    /**
     * Creates a wireframe that is an ellipse with given the center and dimensions.
     *
     * The wireframe will show the boundary, not the circle tesselation.
     *
     * @param   center      The ellipse center point.
     * @param   size        The size of the ellipse.
     * @param   segments    The number of segments to use.
     *
     * @return  An autoreleased wireframe node
     */
    static std::shared_ptr<WireNode> allocWithEllipse(const Vec2& center, const Size& size,
                                                      unsigned int segments = WIRE_SEGMENTS) {
        std::shared_ptr<WireNode> node = std::make_shared<WireNode>();
        return (node->initWithEllipse(center,size,segments) ? node : nullptr);
    }
    
    /**
     * Returns a newly allocated node with the given JSON specificaton.
     *
     * This initializer is designed to receive the "data" object from the
     * JSON passed to {@link SceneLoader}.  This JSON format supports all
     * of the attribute values of its parent class.  In addition, it supports
     * the following additional attributes:
     *
     *      "texture":  The name of a previously loaded texture asset
     *      "polygon":  An even array of polygon vertices (numbers)
     *      "traveral": One of 'open', 'closed', or 'interior'
     *      "indices":  An array of unsigned ints defining triangles from the
     *                  the vertices. The array size should be a multiple of 3.
     *
     * All attributes are optional.  However, it is generally a good idea to
     * specify EITHER the texture or the polygon.  If you specify the indices,
     * then the traversal will be ignored.
     *
     * @param loader    The scene loader passing this JSON file
     * @param data      The JSON object specifying the node
     *
     * @return a newly allocated with the given JSON specificaton.
     */
    static std::shared_ptr<Node> allocWithData(const SceneLoader* loader,
                                               const std::shared_ptr<JsonValue>& data) {
        std::shared_ptr<WireNode> result = std::make_shared<WireNode>();
        if (!result->initWithData(loader,data)) { result = nullptr; }
        return std::dynamic_pointer_cast<Node>(result);
    }
    
#pragma mark -
#pragma mark Attributes
    /**
     * Sets the traversal of this path.
     *
     * If the traversal is different from the current known traversal, it will
     * recompute the traveral using the PathOutliner. All WireNode objects share 
     * a single path outliner, so this method is not thread safe.
     *
     * @param traversal The new wireframe traversal
     */
    void setTraversal(PathTraversal traversal);

    /**
     * Returns the current traversal of this path.
     *
     * If the traversal is unknown (e.g. it is user-defined), this method 
     * returns NONE.
     *
     * @return the current traversal of this path.
     */
    PathTraversal getTraversal() const { return _traversal; }

    /**
     * Sets the wireframe polgon to the vertices expressed in texture space.
     *
     * The polygon will be outlined using a CLOSED traversal in PathOutliner.
     * To create a different traversal, use the alternate setPolygon()  method. 
     * All WireNode objects share a single path outliner, so this method is not
     * thread safe.
     *
     * @param vertices  The vertices to draw
     */
    virtual void setPolygon(const std::vector<Vec2>& vertices) override;

    /**
     * Sets the wireframe polgon to the vertices expressed in texture space.
     *
     * The polygon will be outlined using the given traversal in PathOutliner.
     * All WireNode objects share a single path outliner, so this method is not
     * thread safe.
     *
     * @param vertices  The vertices to draw
     * @param traversal The vertex traversal to use
     */
    void setPolygon(const std::vector<Vec2>& vertices, PathTraversal traversal);

    /**
     * Sets the wireframe polygon to the given one in texture space.
     *
     * This method confirms that the polygon is a PATH.  However, it cannot
     * learn the traversal, so the traveral will be NONE.
     *
     * @param poly  The polygon to draw
     */
    virtual void setPolygon(const Poly2& poly) override;
    
    /**
     * Sets the wireframe polygon to one equivalent to the given rect.
     *
     * The rectangle will be converted into a Poly2, using the standard outline.
     * This is the same as passing Poly2(rect,false).  The traversal will be
     * CLOSED.
     *
     * @param rect  The rectangle to draw
     */
    virtual void setPolygon(const Rect& rect) override;

    /**
     * Sets the wireframe polygon to be the line from origin to destination.
     *
     * The traversal will be OPEN.
     *
     * @param   origin  The line origin.
     * @param   dest    The line destination.
     */
    void setLine(const Vec2 &origin, const Vec2 &dest);
    
    /**
     * Sets the wireframe polygon to be an ellipse with given the center and dimensions.
     *
     * The wireframe will show the boundary, not the circle tesselation. The
     * traversal will be closed.
     *
     * @param   center      The ellipse center point.
     * @param   size        The size of the ellipse.
     * @param   segments    The number of segments to use.
     */
    void setEllipse(const Vec2& center, const Size& size, unsigned int segments = WIRE_SEGMENTS);

#pragma mark -
#pragma mark Rendering
    /**
     * Draws this Node via the given SpriteBatch.
     *
     * This method only worries about drawing the current node.  It does not
     * attempt to render the children.
     *
     * This is the method that you should override to implement your custom
     * drawing code.  You are welcome to use any OpenGL commands that you wish.
     * You can even skip use of the SpriteBatch.  However, if you do so, you
     * must flush the SpriteBatch by calling end() at the start of the method.
     * in addition, you should remember to call begin() at the start of the
     * method.
     *
     * This method provides the correct transformation matrix and tint color.
     * You do not need to worry about whether the node uses relative color.
     * This method is called by render() and these values are guaranteed to be
     * correct.  In addition, this method does not need to check for visibility,
     * as it is guaranteed to only be called when the node is visible.
     *
     * @param batch     The SpriteBatch to draw with.
     * @param transform The global transformation matrix.
     * @param tint      The tint to blend with the Node color.
     */
    virtual void draw(const std::shared_ptr<SpriteBatch>& batch, const Mat4& transform, Color4 tint) override;
    

private:
    /** This macro disables the copy constructor (not allowed on scene graphs) */
    CU_DISALLOW_COPY_AND_ASSIGN(WireNode);
};
    
}


#endif /* __CU_WIRE_NODE_H__ */

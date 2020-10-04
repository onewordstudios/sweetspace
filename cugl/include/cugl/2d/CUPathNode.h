//
//  CUPathNode.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a scene graph node that supports extruded paths. When
//  extruding paths, this node is better than PolygonNode, because it will align
//  the extruded path to the original wireframe.
//
//  This class is loosely coupled with PathExtruder.  You can use PathExtruder
//  independent of the PathNode, but all functionality is present in this class.
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

#ifndef __CU_PATH_NODE_H__
#define __CU_PATH_NODE_H__

#include <string>
#include <cugl/2d/CUTexturedNode.h>
#include <cugl/math/CUPoly2.h>
#include <cugl/math/polygon/CUPathExtruder.h>
#include <cugl/math/polygon/CUPathOutliner.h>

/** The number of segments in a wireframe ellipse */
#define PATH_SEGMENTS 8

namespace cugl {
    
/**
 * This is a scene graph node to represent a path with width.
 *
 * At first glance, it would appear that this class is unnecessary.  A path
 * with width, produced by {@link PathExtruder} is a solid polygon.  This 
 * polygon can, in turn, be used in conjunction with {@link PolygonNode}.
 *
 * However, there are some subtle issues.  In particular, mitres and joints
 * may mean that a PathNode and {@link WireNode} at the same position may
 * not line up with one another.  This is undesirable.  Hence this is a special
 * polygon node that takes into account that it is an extruded path.
 *
 * One of the side effects of this is that the content size of the node is
 * defined by the wireframe path, NOT the extruded path.  If you want the 
 * bounds of the extruded path (relative to Node space), you should use the 
 * method {@link getExtrudedContentBounds()}. Additionally, the anchor point is 
 * relative to the content size not the extruded size.  This means that the 
 * extruded path may be to the left of the origin even when the anchor is (0,0).
 *
 * Because paths have width, it is natural to texture them.  However, generally
 * you will only want to create a path with the degenerate texture (to draw
 * a solid, colored path).  Hence, none of the static constructors take a 
 * texture. You are free to update the texture after creation, if you wish.
 *
 * The polygon is specified in image coordinates. Image coordinates are different
 * from texture coordinates. Their origin is at the bottom-left corner of the 
 * file, and each pixel is one unit. This makes specifying to polygon more 
 * natural for irregular shapes.
 *
 * This means that a polygon with vertices (0,0), (width,0), (width,height),
 * and (0,height) would be identical to a sprite node. However, a polygon with
 * vertices (0,0), (2*width,0), (2*width,2*height), and (0,2*height) would tile
 * the sprite (given the wrap settings) twice both horizontally and vertically.
 * A uniform translation of the polygon (in contrast to the node itself) will
 * not move the shape on the the screen.  Instead, it will just change the part
 * or the texture it uses.
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
class PathNode : public TexturedNode {
#pragma mark Values
protected:
    /** An extruder for those incomplete polygons */
    static PathExtruder _extruder;
    /** An outliner for those incomplete polygons */
    static PathOutliner _outliner;

    /** The extrusion polygon, when the stroke > 0 */
    Poly2 _extrusion;
    /** The bounds of the extruded shape */
    RectCugl _extrbounds;
    
    /** Whether or not the polygon path is closed. */
    bool _closed;
    /** The stroke width of this path. */
    float _stroke;
    /** The joint between segments of the path. */
    PathJoint _joint;
    /** The shape of the two end caps of the path. */
    PathCap _endcap;

public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates an empty path node.
     *
     * You must initialize this PathNode before use.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    PathNode();
    
    /**
     * Releases all resources allocated with this node.
     *
     * This will release, but not necessarily delete the associated texture.
     * However, the polygon and drawing commands will be deleted and no
     * longer safe to use.
     */
    ~PathNode() { dispose(); }

    /**
     * Intializes a path with the given vertices and stroke width.
     *
     * You do not need to set the texture; rendering this into a SpriteBatch
     * will simply use the blank texture. Hence the wireframe will have a solid
     * color.
     *
     * The polygon will be extruded using the given sequence of vertices. 
     * First it will traverse the vertices using either a closed or open
     * traveral.  Then it will extrude that polygon with the given joint
     * and cap. PathNode objects share a single extruder, so this initializer 
     * is not thread safe.
     *
     * @param vertices  The vertices to texture (expressed in image space)
     * @param stroke    The stroke width of the extruded path.
     * @param joint     The joint between extrusion line segments.
     * @param cap       The end caps of the extruded paths.
     * @param closed    The whether the vertex path is open or closed.
     *
     * @return  true if the path node is initialized properly, false otherwise.
     */
    bool initWithVertices(const std::vector<Vec2>& vertices, float stroke,
                          PathJoint joint = PathJoint::NONE, PathCap cap = PathCap::NONE,
                          bool closed=true);

    /**
     * Intializes a path node with the given polygon and stroke width.
     *
     * You do not need to set the texture; rendering this into a SpriteBatch
     * will simply use the blank texture. Hence the wireframe will have a solid
     * color.
     *
     * The polygon will be extruded using the given polygon, assuming that it
     * is a (connected) path. It will extrude that polygon with the given joint
     * and cap.  It will assume the polygon is closed if the number of indices
     * is twice the number of vertices. PathNode objects share a single extruder, 
     * so this initializer is not thread safe.
     *
     * @param poly      The polygon to texture (expressed in image space)
     * @param stroke    The stroke width of the extruded path.
     * @param joint     The joint between extrusion line segments.
     * @param cap       The end caps of the extruded paths.
     *
     * @return  true if the path node is initialized properly, false otherwise.
     */
    bool initWithPoly(const Poly2& poly, float stroke,
                      PathJoint joint = PathJoint::NONE, PathCap cap = PathCap::NONE);

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
     *      "indices":  An array of unsigned ints defining triangles from the
     *                  the vertices. The array size should be a multiple of 3.
     *      'stroke':   A number specifying the stroke width.
     *      'joint':    One of 'mitre', 'bevel', or 'round'.
     *      'cap':      One of 'square' or 'round'.
     *      'closed':   A boolean specifying if the path is closed.
     *
     * All attributes are optional.  However, it is generally a good idea to
     * specify EITHER the texture or the polygon.
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
     * Returns an empty path node.
     *
     * The underlying polygon is empty, and must be set via setPolygon().
     *
     * @return an empty path node.
     */
    static std::shared_ptr<PathNode> alloc() {
        std::shared_ptr<PathNode> node = std::make_shared<PathNode>();
        return (node->init() ? node : nullptr);
    }
    
    /**
     * Returns a path node with the given vertices and stroke width.
     *
     * The polygon will be extruded using the given sequence of vertices.
     * First it will traverse the vertices using either a closed or open
     * traveral.  Then it will extrude that polygon with the given joint
     * and cap. PathNode objects share a single extruder, so this constructor
     * is not thread safe.
     *
     * @param vertices  The vertices to texture (expressed in image space)
     * @param stroke    The stroke width of the extruded path.
     * @param joint     The joint between extrusion line segments.
     * @param cap       The end caps of the extruded paths.
     * @param closed    The whether the vertex path is open or closed.
     *
     * @return a path node with the given vertices and stroke width.
     */
    static std::shared_ptr<PathNode> allocWithVertices(const std::vector<Vec2>& vertices, float stroke,
                                                       PathJoint joint = PathJoint::NONE,
                                                       PathCap cap = PathCap::NONE, bool closed = true) {
        std::shared_ptr<PathNode> node = std::make_shared<PathNode>();
        return (node->initWithVertices(vertices,stroke,joint,cap,closed) ? node : nullptr);
    }
    
    /**
     * Returns a path node with the given polygon and stroke width.
     *
     * The polygon will be extruded using the given polygon, assuming that it
     * is a (connected) path. It will extrude that polygon with the given joint
     * and cap.  It will assume the polygon is closed if the number of indices
     * is twice the number of vertices. PathNode objects share a single extruder, 
     * so this constructor is not thread safe.
     *
     * @param poly      The polygon to texture (expressed in image space)
     * @param stroke    The stroke width of the extruded path.
     * @param joint     The joint between extrusion line segments.
     * @param cap       The end caps of the extruded paths.
     *
     * @return a path node with the given polygon and stroke width.
     */
    static std::shared_ptr<PathNode> allocWithPoly(const Poly2& poly, float stroke,
                                                   PathJoint joint = PathJoint::NONE,
                                                   PathCap cap = PathCap::NONE) {
        std::shared_ptr<PathNode> node = std::make_shared<PathNode>();
        return (node->initWithPoly(poly,stroke,joint,cap) ? node : nullptr);
    }
    
    /**
     * Returns a path node with the given rect and stroke width.
     *
     * The rectangle will be converted into a Poly2, using the standard outline.
     * This is the same as passing Poly2(rect,false).  The traversal will be
     * CLOSED. It will then be extruded with the current joint and cap. PathNode
     * objects share a single extruder, so this constructor is not thread safe.
     *
     * @param rect      The rectangle for to texture.
     * @param stroke    The stroke width of the extruded path.
     * @param joint     The joint between extrusion line segments.
     * @param cap       The end caps of the extruded paths.
     *
     * @return a path with the given rect and stroke width.
     */
    static std::shared_ptr<PathNode> allocWithRect(const RectCugl& rect, float stroke,
                                                   PathJoint joint = PathJoint::NONE,
                                                   PathCap cap = PathCap::NONE) {
        std::shared_ptr<PathNode> node = std::make_shared<PathNode>();
        return (node->initWithPoly(Poly2(rect,false),stroke,joint,cap) ? node : nullptr);
    }
    
    /**
     * Returns a path node that is a line from origin to destination.
     *
     * The path will be OPEN. PathNode objects share a single extruder, so this
     * constructor is not thread safe.
     *
     * @param origin    The line origin
     * @param dest      The line destination
     * @param stroke    The width of the path
     * @param joint     The joint between extrusion line segments.
     * @param cap       The end caps of the extruded paths.
     *
     * @return  An autoreleased path node
     */
    static std::shared_ptr<PathNode> allocWithLine(const Vec2 &origin, const Vec2 &dest, float stroke,
                                                   PathJoint joint = PathJoint::NONE,
                                                   PathCap cap = PathCap::NONE) {
        std::shared_ptr<PathNode> node = std::make_shared<PathNode>();
        return (node->initWithPoly(Poly2::createLine(origin, dest),stroke,joint,cap) ? node : nullptr);
    }
    
    /**
     * Returns a path node that is an ellipse with given the center and dimensions.
     *
     * The path node will draw around the boundary of the ellipse, and will be
     * CLOSED. PathNode objects share a single extruder, so this constructor is 
     * not thread safe.
     *
     * @param   center      The ellipse center point
     * @param   size        The size of the ellipse
     * @param   stroke      The width of the path
     * @param   segments    The number of segments to use
     * @param   joint       The joint between extrusion line segments.
     * @param   cap         The end caps of the extruded paths.
     *
     * @return  a path node that is an ellipse with given the center and dimensions.
     */
    static std::shared_ptr<PathNode> allocWithEllipse( const Vec2& center, const Size& size, float stroke,
                                                      unsigned int segments = PATH_SEGMENTS,
                                                      PathJoint joint = PathJoint::NONE,
                                                      PathCap cap = PathCap::NONE) {
        std::shared_ptr<PathNode> node = std::make_shared<PathNode>();
        return (node->initWithPoly(Poly2::createEllipse(center,size,segments,false),stroke,joint,cap) ?
                node : nullptr);
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
     *      "indices":  An array of unsigned ints defining triangles from the
     *                  the vertices. The array size should be a multiple of 3.
     *      'stroke':   A number specifying the stroke width.
     *      'joint':    One of 'mitre', 'bevel', or 'round'.
     *      'cap':      One of 'square' or 'round'.
     *      'closed':   A boolean specifying if the path is closed.
     *
     * All attributes are optional.  However, it is generally a good idea to
     * specify EITHER the texture or the polygon.
     *
     * @param loader    The scene loader passing this JSON file
     * @param data      The JSON object specifying the node
     *
     * @return a newly allocated node with the given JSON specificaton.
     */
    static std::shared_ptr<Node> allocWithData(const SceneLoader* loader,
                                               const std::shared_ptr<JsonValue>& data) {
        std::shared_ptr<PathNode> result = std::make_shared<PathNode>();
        if (!result->initWithData(loader,data)) { result = nullptr; }
        return std::dynamic_pointer_cast<Node>(result);
    }
    
    
#pragma mark -
#pragma mark Attributes
    /**
     * Sets the stroke width of the path.
     *
     * This method affects the extruded polygon, but not the original path
     * polygon.
     *
     * @param stroke    The stroke width of the path
     */
    void setStroke(float stroke);
    
    /**
     * Returns the stroke width of the path.
     *
     * @return the stroke width of the path.
     */
    float getStroke() const { return _stroke; }
    
    /**
     * Sets whether the path is closed.
     *
     * This method affects both the extruded polygon and the original path
     * polygon.
     *
     * @param closed  Whether the path is closed.
     */
    void setClosed(bool closed);
    
    /**
     * Returns whether the path is closed.
     *
     * @return whether the path is closed.
     */
    bool getClosed() const { return _closed; }
    
    /**
     * Sets the joint type between path segments.
     *
     * This method affects the extruded polygon, but not the original path
     * polygon.
     *
     * @param joint The joint type between path segments
     */
    void setJoint(PathJoint joint);
    
    /**
     * Returns the joint type between path segments.
     *
     * @return the joint type between path segments.
     */
    PathJoint getJoint() const { return _joint; }
    
    /**
     * Sets the cap shape at the ends of the path.
     *
     * This method affects the extruded polygon, but not the original path
     * polygon.
    *
     * @param cap   The cap shape at the ends of the path.
     */
    void setCap(PathCap cap);
    
    /**
     * Returns the cap shape at the ends of the path.
     *
     * @return the cap shape at the ends of the path.
     */
    PathCap getCap() const { return _endcap; }
    
#pragma mark -
#pragma mark Polygons
    /**
     * Sets the polgon to the vertices expressed in texture space.
     *
     * The polygon will be extruded using the given sequence of vertices.
     * First it will traverse the vertices using the current traversal. Then
     * it will extrude that polygon with the current joint and cap. PathNode 
     * objects share a single extruder, so this method is not thread safe.
     *
     * @param vertices  The vertices to texture
     */
    virtual void setPolygon(const std::vector<Vec2>& vertices) override;
    
    /**
     * Sets the polygon to the given one in texture space.
     *
     * This method will extrude that polygon with the current joint and cap.
     * The polygon is assumed to be closed if the number of indices is twice
     * the number of vertices. PathNode objects share a single extruder, so
     * this method is not thread safe.
     *
     * @param poly  The polygon to texture
     */
    virtual void setPolygon(const Poly2& poly) override;
    
    /**
     * Sets the texture polygon to one equivalent to the given rect.
     *
     * The rectangle will be converted into a Poly2, using the standard outline.
     * This is the same as passing Poly2(rect,false). It will then be extruded 
     * with the current joint and cap. PathNode objects share a single extruder,
     * so this method is not thread safe.
     *
     * @param rect  The rectangle to texture
     */
    virtual void setPolygon(const RectCugl& rect) override;
    
    /**
     * Returns the width of the extruded content.
     *
     * This method is an alternative to {@link getContentWidth()}. That method
     * only returns the content width of the path polygon; it does not include
     * the stroke width, mitres, and caps.  This method includes the extra
     * width of the extruded path.
     *
     * @return the width of the extruded content.
     */
    float getExtrudedContentWidth() const { return _extrbounds.size.width; }
    
    /**
     * Returns the height of the extruded content.
     *
     * This method is an alternative to {@link getContentHeight()}. That method
     * only returns the content height of the path polygon; it does not include
     * the stroke width, mitres, and caps.  This method includes the extra
     * height of the extruded path.
     *
     * @return the height of the extruded content.
     */
    float getExtrudedContentHeight() const { return _extrbounds.size.height; }
    
    /**
     * Returns the size of the extruded content.
     *
     * This method is an alternative to {@link getContentSize()}. That method
     * only returns the content size of the path polygon; it does not include
     * the stroke width, mitres, and caps.  This method includes the extra
     * size of the extruded path.
     *
     * @return the size of the extruded content.
     */
    const Size& getExtrudedContentSize() const { return _extrbounds.size; }
    
    /**
     * Returns the bounding box of the extruded content.
     *
     * This method is similar to {@link getExtrudedContentSize()} in that it
     * returns the extra content size created by the stroke width, mitres, and 
     * caps. In addition, it returns the actual bounds of the path within
     * Node space.
     *
     * Unlike {@link getBoundingBox()}, this method is unaffected by any 
     * transforms on this node.
     *
     * @return the bounding box of the extruded content.
     */
    const RectCugl& getExtrudedContentBounds() const { return _extrbounds; }

    
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
    

    
#pragma mark Internal Helpers
private:
    /**
     * Allocate the render data necessary to render this node.
     */
    virtual void generateRenderData() override;
    
    /**
     * Updates the extrusion polygon, based on the current settings.
     */
    void updateExtrusion();

    /** This macro disables the copy constructor (not allowed on scene graphs) */
    CU_DISALLOW_COPY_AND_ASSIGN(PathNode);
};

}

#endif /* __CU_PATH_NODE_H__ */

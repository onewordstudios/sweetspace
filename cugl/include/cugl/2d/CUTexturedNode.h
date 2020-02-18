//
//  CUTexturedNode.h
//  Cornell University Game Library (CUGL)
//
//  This module provides an abstract class for textured scene graph nodes. It
//  is the core scene graph node in CUGL.
//
//  You should never instantiate an object of this class.  Instead, you should
//  use one of the concrete subclasses: WireNode, PolygonNode, or PathNode.
//  Because it is abstract, it has no allocators.  It only has initializers.
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

#ifndef __CU_TEXTURED_NODE_H__
#define __CU_TEXTURED_NODE_H__

#include <string>
#include <cugl/math/CUPoly2.h>
#include <cugl/2d/CUNode.h>
#include <cugl/renderer/CUTexture.h>
#include <cugl/renderer/CUVertex.h>

namespace cugl {
    
/**
 * This class is an abstract scene graph node representing a textured shape.
 *
 * This class cannot be instantiated directly. Instead, you must use one of the
 * subclasses: {@link PolygonNode}, {@link PathNode}, {@link WireNode}.
 *
 * The node shape is stored as polygon.  This is true regardless of whether the
 * node is displaying solid shapes or a wireframe.  The polygon is itself 
 * specified in image coordinates. Image coordinates are different from texture 
 * coordinates. Their origin is at the bottom-left corner of the file, and each 
 * pixel is one unit. This makes specifying a polygon more natural for irregular
 * shapes.
 *
 * This means that a polygon with vertices (0,0), (width,0), (width,height),
 * and (0,height) would be identical to a sprite node. However, a polygon with
 * vertices (0,0), (2*width,0), (2*width,2*height), and (0,2*height) would tile
 * the sprite (given the wrap settings) twice both horizontally and vertically.
 *
 * The content size of this node is defined by the size (but not the offset)
 * of the bounding box. The anchor point is relative to this content size.
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
class TexturedNode : public Node {
#pragma mark Values
protected:
    /** The class name for the specific subclass */
    std::string _classname;
    
    /** Texture to be applied to the polygon */
    std::shared_ptr<Texture>    _texture;
    
    /** The polygon defining this node, with vertices in texture space */
    Poly2   _polygon;
    
    /** Whether to disable anchors and draw the polygon in absolute coordinates */
    bool _absolute;
    /** Whether to stretch the image to match the content size */
    bool _stretch;

    /** Whether we have generated render data for this node */
    bool _rendered;
    /** The render data for this node */
    std::vector<Vertex2> _vertices;
    
    /** The blending equation for this texture */
    GLenum _blendEquation;
    /** The source factor for the blend function */
    GLenum _srcFactor;
    /** The destination factor for the blend function */
    GLenum _dstFactor;
    
    /** Whether or not to flip the texture horizontally */
    bool _flipHorizontal;
    /** Whether or not to flip the texture vertically */
    bool _flipVertical;
    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates an empty polygon with the degenerate texture.
     *
     * This constructor should never be called directly, as this is an abstract 
     * class.
     */
    TexturedNode();
    
    /**
     * Deletes this node, releasing all resources.
     */
    ~TexturedNode() { dispose(); }
    
    /**
     * Disposes all of the resources used by this node.
     *
     * A disposed Node can be safely reinitialized. Any children owned by this
     * node will be released.  They will be deleted if no other object owns them.
     *
     * It is unsafe to call this on a Node that is still currently inside of
     * a scene graph.
     */
    virtual void dispose() override;
    
    /**
     * Intializes an empty polygon with the degenerate texture.
     *
     * You do not need to set the texture; rendering this into a SpriteBatch
     * will simply use the blank texture. The polygon, however, will also be
     * empty, and must be set via setPolygon.
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    virtual bool init() override {
        return initWithTexture(nullptr, Rect::ZERO);
    }
    
    /**
     * Intializes a solid polygon with the given vertices.
     *
     * You do not need to set the texture; rendering this into a SpriteBatch
     * will simply use the blank texture. Hence the polygon will have a solid 
     * color.
     *
     * The polygon indices will be subclass specific.
     *
     * @param vertices  The vertices to texture (expressed in image space)
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    bool init(const std::vector<Vec2>& vertices) {
        return initWithTexture(nullptr, vertices);
    }
    
    /**
     * Intializes a solid polygon given polygon shape.
     *
     * You do not need to set the texture; rendering this into a SpriteBatch
     * will simply use the blank texture. Hence the polygon will have a solid
     * color.
     *
     * @param poly      The polygon to texture
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    bool  init(const Poly2& poly) {
        return initWithTexture(nullptr, poly);
    }
    
    /**
     * Intializes a solid polygon with the given rect.
     *
     * You do not need to set the texture; rendering this into a SpriteBatch
     * will simply use the blank texture. Hence the polygon will have a solid
     * color.
     *
     * The conversion of rectangle to polygon is subclass specific.
     *
     * @param rect      The rectangle to texture
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    bool  init(const Rect& rect) {
        return initWithTexture(nullptr, rect);
    }
    
    /**
     * Intializes a textured polygon from the image filename.
     *
     * After creation, the polygon will be a rectangle.  The vertices of this
     * polygon will be the corners of the image.
     *
     * @param filename  A path to image file, e.g., "scene1/earthtile.png"
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    bool initWithFile(const std::string& filename);
    
    /**
     * Initializes a textured polygon from the image filename and the given vertices.
     *
     * The polygon indices will be subclass specific.
     *
     * @param filename  A path to image file, e.g., "scene1/earthtile.png"
     * @param vertices  The vertices to texture (expressed in image space)
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    bool initWithFile(const std::string& filename, const std::vector<Vec2>& vertices) {
        _polygon.set(vertices);
        return initWithFile(filename, _polygon);
    }
    
    /**
     * Initializes a textured polygon from the image filename and the given polygon.
     *
     * @param filename  A path to image file, e.g., "scene1/earthtile.png"
     * @param poly      The polygon to texture
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    bool initWithFile(const std::string& filename, const Poly2& poly);
    
    /**
     * Initializes a textured polygon from the image filename and the given rect.
     *
     * The conversion of rectangle to polygon is subclass specific.
     *
     * @param filename  A path to image file, e.g., "scene1/earthtile.png"
     * @param rect      The rectangle to texture
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    bool initWithFile(const std::string& filename, const Rect& rect);
    
    /**
     * Initializes a textured polygon from a Texture object.
     *
     * After creation, the polygon will be a rectangle. The vertices of this
     * polygon will be the corners of the texture.
     *
     * @param texture   A shared pointer to a Texture object.
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    bool initWithTexture(const std::shared_ptr<Texture>& texture);
    
    /**
     * Initializes a textured polygon from a Texture object and the given vertices.
     *
     * The polygon indices will be subclass specific.
     *
     * @param texture    A shared pointer to a Texture object.
     * @param vertices   The vertices to texture (expressed in image space)
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    bool initWithTexture(const std::shared_ptr<Texture>&  texture, const std::vector<Vec2>& vertices) {
        setPolygon(vertices);
        return initWithTexture(texture, _polygon);
    }
    
    /**
     * Initializes a textured polygon from a Texture object and the given polygon.
     *
     * @param texture   A shared pointer to a Texture object.
     * @param poly      The polygon to texture
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    bool initWithTexture(const std::shared_ptr<Texture>& texture, const Poly2& poly);
    
    /**
     * Initializes a textured polygon from a Texture object and the given rect.
     *
     * The conversion of rectangle to polygon is subclass specific.
     *
     * @param texture   A shared pointer to a Texture object.
     * @param rect      The rectangle to texture
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    bool initWithTexture(const std::shared_ptr<Texture>& texture, const Rect& rect);
    
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
     *
     * All attributes are optional.  However, it is generally a good idea to
     * specify EITHER the texture or the polygon
     *
     * @param loader    The scene loader passing this JSON file
     * @param data      The JSON object specifying the node
     *
     * @return true if initialization was successful.
     */
    virtual bool initWithData(const SceneLoader* loader, const std::shared_ptr<JsonValue>& data) override;

#pragma mark -
#pragma mark Attributes
    /**
     * Sets the node texture to a new one allocated from a filename.
     *
     * This method will have no effect on the polygon vertices.  This class
     * decouples the geometry from the texture.  That is because we do not 
     * expect the vertices to match the texture perfectly.
     *
     * @param filename  A path to image file, e.g., "scene1/earthtile.png"
     */
    void setTexture(const std::string &filename) {
        std::shared_ptr<Texture> texture =  Texture::allocWithFile(filename);
        setTexture(texture);
    }
    
    /**
     * Sets the node texture to the one specified.
     *
     * This method will have no effect on the polygon vertices.  This class
     * decouples the geometry from the texture.  That is because we do not
     * expect the vertices to match the texture perfectly.
     *
     * @param texture   A shared pointer to a Texture object.
     */
    void setTexture(const std::shared_ptr<Texture>& texture);
    
    /**
     * Returns the texture used by this node.
     *
     * @return the texture used by this node
     */
    std::shared_ptr<Texture>& getTexture() { return _texture; }

    /**
     * Returns the texture used by this node.
     *
     * @return the texture used by this node
     */
    const std::shared_ptr<Texture>& getTexture() const { return _texture; }

    /**
     * Sets the polgon to the vertices expressed in texture space.
     *
     * The polygon indices will be subclass specific.
     *
     * @param   vertices The vertices to texture
     */
    virtual void setPolygon(const std::vector<Vec2>& vertices) = 0;
    
    /**
     * Sets the polygon to the given one in texture space.
     *
     * @param poly  The polygon to texture
     */
    virtual void setPolygon(const Poly2& poly);
    
    /**
     * Sets the texture polygon to one equivalent to the given rect.
     *
     * The conversion of rectangle to polygon is subclass specific.
     *
     * @param rect  The rectangle to texture
     */
    virtual void setPolygon(const Rect& rect) = 0;
    
    /**
     * Returns the texture polygon for this scene graph node
     *
     * @returns the texture polygon for this scene graph node
     */
    const Poly2& getPolygon() const { return _polygon; }
    
    /**
     * Translates the polygon by the given amount.
     *
     * Remember that translating the polygon has no effect on the shape or
     * position.  Because the polygon is expressed in image coordinates, all 
     * it does is shift the texture coords of the polygon.  Hence this method 
     * can be used for animation and filmstrips.
     *
     * Calling this method is faster than changing the polygon and
     * resetting it.
     *
     * @param dx    The amount to shift horizontally.
     * @param dy    The amount to shift horizontally.
     */
    virtual void shiftPolygon(float dx, float dy);
    
    /**
     * Returns the rect of the Polygon in points
     *
     * The bounding rect is the smallest rectangle containing all
     * of the points in the polygon.
     *
     * This value also defines the content size of the node.  The
     * polygon will be shifted so that its bounding rect is centered
     * at the node center.
     */
    const Rect& getBoundingRect() const { return _polygon.getBounds(); }
    
    /**
     * Sets the blending function for this texture node.
     *
     * The enums are the standard ones supported by OpenGL.  See
     *
     *      https://www.opengl.org/sdk/docs/man/html/glBlendFunc.xhtml
     *
     * However, this setter does not do any error checking to verify that
     * the enums are valid.  By default, srcFactor is GL_SRC_ALPHA while
     * dstFactor is GL_ONE_MINUS_SRC_ALPHA. This corresponds to non-premultiplied
     * alpha blending.
     *
     * This blending factor only affects the texture of the current node.  It
     * does not affect any children of the node.
     *
     * @param srcFactor Specifies how the source blending factors are computed
     * @param dstFactor Specifies how the destination blending factors are computed.
     */
    void setBlendFunc(GLenum srcFactor, GLenum dstFactor) { _srcFactor = srcFactor; _dstFactor = dstFactor; }
    
    /**
     * Returns the source blending factor
     *
     * By default this value is GL_SRC_ALPHA. For other options, see
     *
     *      https://www.opengl.org/sdk/docs/man/html/glBlendFunc.xhtml
     *
     * This blending factor only affects the texture of the current node.  It
     * does not affect any children of the node.
     *
     * @return the source blending factor
     */
    GLenum getSourceBlendFactor() const { return _srcFactor; }
    
    /**
     * Returns the destination blending factor
     *
     * By default this value is GL_ONE_MINUS_SRC_ALPHA. For other options, see
     *
     *      https://www.opengl.org/sdk/docs/man/html/glBlendFunc.xhtml
     *
     * This blending factor only affects the texture of the current node.  It
     * does not affect any children of the node.
     *
     * @return the destination blending factor
     */
    GLenum getDestinationBlendFactor() const { return _srcFactor; }
    
    /**
     * Sets the blending equation for this textured node
     *
     * The enum must be a standard ones supported by OpenGL.  See
     *
     *      https://www.opengl.org/sdk/docs/man/html/glBlendEquation.xhtml
     *
     * However, this setter does not do any error checking to verify that
     * the input is valid.  By default, the equation is GL_FUNC_ADD.
     *
     * This blending equation only affects the texture of the current node.  It
     * does not affect any children of the node.
     *
     * @param equation  Specifies how source and destination colors are combined
     */
    void setBlendEquation(GLenum equation) { _blendEquation = equation; }
    
    /**
     * Returns the blending equation for this textured node
     *
     * By default this value is GL_FUNC_ADD. For other options, see
     *
     *      https://www.opengl.org/sdk/docs/man/html/glBlendEquation.xhtml
     *
     * This blending equation only affects the texture of the current node.  It
     * does not affect any children of the node.
     *
     * @return the blending equation for this sprite batch
     */
    GLenum getBlendEquation() const { return _blendEquation; }
    
    /**
     * Flips the texture coordinates horizontally if flag is true.
     *
     * Flipping the texture coordinates replaces each u coordinate with
     * 1-u.  Hence this operation is defined even if the texture coordinates
     * are outside the range 0..1.
     *
     * @param  flag whether to flip the coordinates horizontally
     */
    void flipHorizontal(bool flag) { _flipHorizontal = flag; updateTextureCoords(); }
    
    /**
     * Returns true if the texture coordinates are flipped horizontally.
     *
     * @return true if the texture coordinates are flipped horizontally.
     */
    bool isFlipHorizontal() const { return _flipHorizontal; }
    
    /**
     * Flips the texture coordinates vertically if flag is true.
     *
     * Flipping the texture coordinates replaces each v coordinate with
     * 1-v.  Hence this operation is defined even if the texture coordinates
     * are outside the range 0..1.
     *
     * @param  flag whether to flip the coordinates vertically
     */
    void flipVertical(bool flag) { _flipVertical = flag; updateTextureCoords(); }
    
    /**
     * Returns true if the texture coordinates are flipped vertically.
     *
     * @return true if the texture coordinates are flipped vertically.
     */
    bool isFlipVertical() const { return _flipVertical; }

    /**
     * Returns a string representation of this node for debugging purposes.
     *
     * If verbose is true, the string will include class information.  This
     * allows us to unambiguously identify the class.
     *
     * @param verbose Whether to include class information
     *
     * @return a string representation of this node for debuggging purposes.
     */
    virtual std::string toString(bool verbose=false) const override;
    
    
#pragma mark -
#pragma mark Scaling Attributes
    /**
     * Returns true if this node is using absolute positioning.
     *
     * In absolute positioning, the vertices are draw in their correct
     * position with respect to the node origin.  We no longer try to 
     * offset the polygon with respect to the anchors.  This is useful
     * when you need a scene graph node that has an external relationship
     * to a non-child node.
     *
     * Setting this value to true will disable anchor functions (and set
     * the anchor to the bottom left).  That is because anchors do not 
     * make sense when we are using absolute positioning.
     *
     * @return true if this node is using absolute positioning.
     */
    bool isAbsolute() const { return _absolute; }
    
    /**
     * Sets whether this node is using absolute positioning.
     *
     * In absolute positioning, the vertices are draw in their correct
     * position with respect to the node origin.  We no longer try to
     * offset the polygon with respect to the anchors.  This is useful
     * when you need a scene graph node that has an external relationship
     * to a non-child node.
     *
     * Setting this value to true will disable anchor functions (and set
     * the anchor to the bottom left).  That is because anchors do not
     * make sense when we are using absolute positioning.
     *
     * @param flag  Whether if this node is using absolute positioning.
     */
    void setAbsolute(bool flag) {
        _absolute = flag;
        _anchor = Vec2::ANCHOR_BOTTOM_LEFT;
    }
    
    /**
     * Sets the anchor point in percentages.
     *
     * The anchor point defines the relative origin of Node with respect to its
     * parent.  It is a "pin" where the Node is attached to its parent.  In
     * effect, the translation of a Node is defined by its position plus
     * anchor point.
     *
     * The anchorPoint is normalized, like a percentage. (0,0) means the
     * bottom-left corner and (1,1) means the top-right corner. There are many
     * anchor point constants defined in {@link Vec2}.  However, there is
     * nothing preventing an anchor point higher than (1,1) or lower than (0,0).
     *
     * The default anchorPoint is (0.5,0.5), so it starts in the center of
     * the node. Changing the anchor will not move the contents of the node in
     * the parent space, but it will change the value of the Node position.
     *
     * This function is disabled if the node is using absolute positioning.  
     * That is because anchors do not make sense when we are drawing polygons
     * directly to the screen.
     *
     * @param anchor    The anchor point of node.
     */
    virtual void setAnchor(const Vec2& anchor) override {
        if (!_absolute) { Node::setAnchor(anchor); }
    }

    /**
     * Sets the untransformed size of the node.
     *
     * The content size remains the same no matter how the node is scaled or
     * rotated. All nodes must have a size, though it may be degenerate (0,0).
     *
     * By default, the content size of a TexturedNode is the size of the
     * bounding box of the defining polygon.  Resizing a texture node will
     * stretch the image to fill in the new size.
     *
     * @param size  The untransformed size of the node.
     */
    virtual void setContentSize(const Size& size) override;
    
    /**
     * Sets the untransformed size of the node.
     *
     * The content size remains the same no matter how the node is scaled or
     * rotated. All nodes must have a size, though it may be degenerate (0,0).
     *
     * By default, the content size of a TexturedNode is the size of the
     * bounding box of the defining polygon.  Resizing a texture node will
     * stretch the image to fill in the new size.
     *
     * @param width     The untransformed width of the node.
     * @param height    The untransformed height of the node.
     */
    virtual void setContentSize(float width, float height) override {
        setContentSize(Size(width, height));
    }
    
    
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
    virtual void draw(const std::shared_ptr<SpriteBatch>& batch,
                      const Mat4& transform, Color4 tint) override = 0;
    
    /**
     * Refreshes this node to restore the render data.
     */
    void refresh() { clearRenderData(); generateRenderData(); }
    
#pragma mark -
#pragma mark Internal Helpers
protected:
    /**
     * Allocates the render data necessary to render this node.
     */
    virtual void generateRenderData();
    
    /**
     * Clears the render data, releasing all vertices and indices.
     */
    void clearRenderData();

    /**
     * Updates the texture coordinates for this polygon
     *
     * The texture coordinates are computed assuming that the polygon is
     * defined in image space, with the origin in the bottom left corner
     * of the texture.
     */
    void updateTextureCoords();

    /** This macro disables the copy constructor (not allowed on scene graphs) */
    CU_DISALLOW_COPY_AND_ASSIGN(TexturedNode);

};

}

#endif /* __CU_TEXTURED_NODE_H__ */

//
//  CUPolygonNode.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a scene graph node that supports basic sprite graphics.
//  The sprites do not have to be rectangular.  They may be any shape represented
//  by Poly2.
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

#ifndef __CU_POLYGON_NODE_H__
#define __CU_POLYGON_NODE_H__

#include <string>
#include <cugl/2d/CUTexturedNode.h>
#include <cugl/math/polygon/CUSimpleTriangulator.h>

namespace cugl {
    
/**
 * This is a scene graph node representing a solid 2D polygon textured by a sprite.
 *
 * The polygon is specified in image coordinates. Image coordinates are different
 * from texture coordinates. Their origin is at the bottom-left corner of the file,
 * and each pixel is one unit. This makes specifying a polygon more natural for
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
 */
class PolygonNode : public TexturedNode {
#pragma mark Values
protected:
    /** A triangulator for those incomplete polygons */
    static SimpleTriangulator _triangulator;

public:
#pragma mark -
#pragma mark Constructor
    /**
     * Creates an empty polygon with the degenerate texture.
     *
     * You must initialize this PolygonNode before use.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    PolygonNode() : TexturedNode() {
        _classname = "PolygonNode";
        _name = "PolygonNode";
    }

    /**
     * Releases all resources allocated with this node.
     *
     * This will release, but not necessarily delete the associated texture.
     * However, the polygon and drawing commands will be deleted and no
     * longer safe to use.
     */
    ~PolygonNode() { dispose(); }

#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns an empty polygon with the degenerate texture.
     *
     * You do not need to set the texture; rendering this into a SpriteBatch
     * will simply use the blank texture. The polygon, however, will also be
     * empty, and must be set via setPolygon.
     *
     * @return an empty polygon with the degenerate texture.
     */
    static std::shared_ptr<PolygonNode> alloc() {
        std::shared_ptr<PolygonNode> node = std::make_shared<PolygonNode>();
        return (node->init() ? node : nullptr);
    }
    
    /**
     * Returns a solid polygon with the given vertices.
     *
     * You do not need to set the texture; rendering this into a SpriteBatch
     * will simply use the blank texture. Hence the polygon will have a solid
     * color.
     *
     * The polygon will be triangulated using the rules of SimpleTriangulator.
     * All PolygonNode objects share a single triangulator, so this allocator is
     * not thread safe.
     *
     * @param vertices  The vertices to texture (expressed in image space)
     *
     * @return a solid polygon with the given vertices.
     */
    static std::shared_ptr<PolygonNode> alloc(const std::vector<Vec2>& vertices) {
        std::shared_ptr<PolygonNode> node = std::make_shared<PolygonNode>();
        return (node->init(vertices) ? node : nullptr);
    }
    
    /**
     * Returns a solid polygon given polygon shape.
     *
     * You do not need to set the texture; rendering this into a SpriteBatch
     * will simply use the blank texture. Hence the polygon will have a solid
     * color.
     *
     * @param   poly     The polygon to texture
     *
     * @return a solid polygon given polygon shape.
     */
    static std::shared_ptr<PolygonNode> alloc(const Poly2& poly) {
        std::shared_ptr<PolygonNode> node = std::make_shared<PolygonNode>();
        return (node->init(poly) ? node : nullptr);
    }
    
    /**
     * Returns a solid polygon with the given rect.
     *
     * You do not need to set the texture; rendering this into a SpriteBatch
     * will simply use the blank texture. Hence the polygon will have a solid
     * color.
     *
     * The rectangle will be converted into a Poly2, using the standard (solid)
     * triangulation.  This is the same as passing Poly2(rect,true).
     *
     * @param   rect     The rectangle to texture
     *
     * @return a solid polygon with the given rect.
     */
    static std::shared_ptr<PolygonNode> alloc(const Rect& rect) {
        std::shared_ptr<PolygonNode> node = std::make_shared<PolygonNode>();
        return (node->init(rect) ? node : nullptr);
    }
    
    /**
     * Returns a textured polygon from the image filename.
     *
     * After creation, the polygon will be a rectangle.  The vertices of this
     * polygon will be the corners of the image.
     *
     * @param   filename A path to image file, e.g., "scene1/earthtile.png"
     *
     * @return  a textured polygon from the image filename.
     */
    static std::shared_ptr<PolygonNode> allocWithFile(const std::string& filename) {
        std::shared_ptr<PolygonNode> node = std::make_shared<PolygonNode>();
        return (node->initWithFile(filename) ? node : nullptr);
    }
    
    /**
     * Returns a textured polygon from the image filename and the given vertices.
     *
     * The polygon will be triangulated using the rules of SimpleTriangulator.
     * All PolygonNode objects share a single triangulator, so this allocator is
     * not thread safe.
     *
     * @param filename  A path to image file, e.g., "scene1/earthtile.png"
     * @param vertices  The vertices to texture (expressed in image space)
     *
     * @return a textured polygon from the image filename and the given vertices.
     */
    static std::shared_ptr<PolygonNode> allocWithFile(const std::string& filename,
                                                      const std::vector<Vec2>& vertices) {
        std::shared_ptr<PolygonNode> node = std::make_shared<PolygonNode>();
        return (node->initWithFile(filename,vertices) ? node : nullptr);
    }
    
    /**
     * Returns a textured polygon from the image filename and the given polygon.
     *
     * @param filename  A path to image file, e.g., "scene1/earthtile.png"
     * @param poly      The polygon to texture
     *
     * @return a textured polygon from the image filename and the given polygon.
     */
    static std::shared_ptr<PolygonNode> allocWithFile(const std::string& filename, const Poly2& poly) {
        std::shared_ptr<PolygonNode> node = std::make_shared<PolygonNode>();
        return (node->initWithFile(filename,poly) ? node : nullptr);
    }
    
    /**
     * Returns a textured polygon from the image filename and the given rect.
     *
     * The rectangle will be converted into a Poly2, using the standard (solid)
     * triangulation.  This is the same as passing Poly2(rect,true).
     *
     * @param filename  A path to image file, e.g., "scene1/earthtile.png"
     * @param rect      The rectangle to texture
     *
     * @return a textured polygon from the image filename and the given rect.
     */
    static std::shared_ptr<PolygonNode> allocWithFile(const std::string& filename, const Rect& rect) {
        std::shared_ptr<PolygonNode> node = std::make_shared<PolygonNode>();
        return (node->initWithFile(filename,rect) ? node : nullptr);
    }
    
    /**
     * Returns a textured polygon from a Texture object.
     *
     * After creation, the polygon will be a rectangle. The vertices of this
     * polygon will be the corners of the texture.
     *
     * @param texture   A shared pointer to a Texture object.
     *
     * @return a textured polygon from a Texture object.
     */
    static std::shared_ptr<PolygonNode> allocWithTexture(const std::shared_ptr<Texture>& texture) {
        std::shared_ptr<PolygonNode> node = std::make_shared<PolygonNode>();
        return (node->initWithTexture(texture) ? node : nullptr);
    }
    
    /**
     * Returns a textured polygon from a Texture object and the given vertices.
     *
     * The polygon will be triangulated using the rules of SimpleTriangulator.
     * All PolygonNode objects share a single triangulator, so this method is
     * not thread safe.
     *
     * @param texture   A shared pointer to a Texture object.
     * @param vertices  The vertices to texture (expressed in image space)
     *
     * @return a textured polygon from a Texture object and the given vertices.
     */
    static std::shared_ptr<PolygonNode> allocWithTexture(const std::shared_ptr<Texture>& texture,
                                                         const std::vector<Vec2>& vertices) {
        std::shared_ptr<PolygonNode> node = std::make_shared<PolygonNode>();
        return (node->initWithTexture(texture,vertices) ? node : nullptr);
    }
    /**
     * Returns a textured polygon from a Texture object and the given polygon.
     *
     * @param texture   A shared pointer to a Texture object.
     * @param poly      The polygon to texture
     *
     * @return a textured polygon from a Texture object and the given polygon.
     */
    static std::shared_ptr<PolygonNode> allocWithTexture(const std::shared_ptr<Texture>& texture,
                                                         const Poly2& poly) {
        std::shared_ptr<PolygonNode> node = std::make_shared<PolygonNode>();
        return (node->initWithTexture(texture,poly) ? node : nullptr);
    }
    
    /**
     * Returns a textured polygon from a Texture object and the given rect.
     *
     * The rectangle will be converted into a Poly2, using the standard (solid)
     * triangulation.  This is the same as passing Poly2(rect,true).
     *
     * @param texture   A shared pointer to a Texture object.
     * @param rect      The rectangle to texture
     *
     * @return a textured polygon from a Texture object and the given rect.
     */
    static std::shared_ptr<PolygonNode> allocWithTexture(const std::shared_ptr<Texture>& texture,
                                                         const Rect& rect)  {
        std::shared_ptr<PolygonNode> node = std::make_shared<PolygonNode>();
        return (node->initWithTexture(texture,rect) ? node : nullptr);
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
     *
     * All attributes are optional.  However, it is generally a good idea to
     * specify EITHER the texture or the polygon
     *
     * @param loader    The scene loader passing this JSON file
     * @param data      The JSON object specifying the node
     *
     * @return a newly allocated node with the given JSON specificaton.
     */
    static std::shared_ptr<Node> allocWithData(const SceneLoader* loader,
                                               const std::shared_ptr<JsonValue>& data) {
        std::shared_ptr<PolygonNode> result = std::make_shared<PolygonNode>();
        if (!result->initWithData(loader,data)) { result = nullptr; }
        return std::dynamic_pointer_cast<Node>(result);
    }
    
#pragma mark -
#pragma mark Attributes
    /**
     * Sets the polgon to the vertices expressed in texture space.
     *
     * The polygon will be triangulated using the rules of SimpleTriangulator.
     * All PolygonNode objects share a single triangulator, so this method is
     * not thread safe.
     *
     * @param vertices  The vertices to texture
     */
    virtual void setPolygon(const std::vector<Vec2>& vertices) override;
    
    /**
     * Sets the polygon to the given one in texture space.
     *
     * This method confirms that the polygon is SOLID.
     *
     * @param poly  The polygon to texture
     */
    virtual void setPolygon(const Poly2& poly) override;
    
    /**
     * Sets the texture polygon to one equivalent to the given rect.
     *
     * The rectangle will be converted into a Poly2, using the standard (solid)
     * triangulation.  This is the same as passing Poly2(rect,true). This will
     * not size the image to fit the rectangle.  Instead, it uses the rectangle
     * to define the portion of the image that will be displayed.
     *
     * @param rect  The rectangle to texture
     */
    virtual void setPolygon(const Rect& rect) override;
    
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

    
#pragma mark -
#pragma mark Internal Helpers
private:
    /** This macro disables the copy constructor (not allowed on scene graphs) */
    CU_DISALLOW_COPY_AND_ASSIGN(PolygonNode);

};
    
}

#endif /* __CU_POLYGON_NODE_H__ */

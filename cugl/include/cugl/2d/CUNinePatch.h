//
//  CUNinePatch.h
//  Cornell University Game Library (CUGL)
//
//  This module implements a ninepatch for expandable UI elements.  A ninepatch
//  breaks up an image into nine parts.  It expands the middle elements while
//  preserving the corners.  This allows us to arbitrarily stretch an image
//  like a beveled button without distorting it. Ninepatches are used heavily
//  in mobile UI development.
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
//  Version: 11/8/17
//
#ifndef __CU_NINEPATCH_H__
#define __CU_NINEPATCH_H__

#include <string>
#include <cugl/math/CURect.h>
#include <cugl/2d/CUNode.h>
#include <cugl/renderer/CUTexture.h>
#include <cugl/renderer/CUVertex.h>
#include <cugl/renderer/CUSpriteBatch.h>

namespace cugl {

/**
 * Class to support a ninepatch image
 *
 * Even though a ninepatch is composed of nine parts, we assume that the
 * ninepatch is specified by a single texture (it would be far too inefficient
 * to spread a ninepatch across multiple textures).  Instead, the user
 * specifies the interior of the texture with a rectangle, and the class
 * interpolates the ninepatch from that. If no rectangle is specified,
 * it assumes a degenerate ninepatch with a one-pixel middle.
 *
 * The interior rectangle is specified in pixel coordinates. As with
 * {@link PolygonNode}, we assume that the pixel origin is the bottom-left
 * corner of the image.
 */
class NinePatch : public Node {
#pragma mark Values
protected:
    /** The NinePatch texture */
    std::shared_ptr<Texture> _texture;

    /** The internal grid square in pixel space */
    Rect _interior;
    
    /** Whether we have generated render data for this node */
    bool _rendered;
    /** The render vertices for this node */
    std::vector<Vertex2> _vertices;
    /** The render indices for this node */
    std::vector<unsigned short> _indices;
    
    /** The blending equation for this texture */
    GLenum _blendEquation;
    /** The source factor for the blend function */
    GLenum _srcFactor;
    /** The destination factor for the blend function */
    GLenum _dstFactor;
    
    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates a NinePatch with the degenerate texture.
     *
     * You must initialize this NinePatch before use.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    NinePatch();
    
    /**
     * Deletes this node, releasing all resources.
     */
    ~NinePatch() { dispose(); }
    
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
     * Intializes a simple NinePatch with the degenerate texture.
     *
     * You do not need to set the texture; rendering this into a SpriteBatch
     * will simply use the blank texture. However, the result will just
     * be rectangle the same size as the blank texture, and so will not be
     * very interesting.
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    virtual bool init() override {
        return initWithTexture(SpriteBatch::getBlankTexture());
    }

    /**
     * Intializes a degenerate NinePatch from the image filename.
     *
     * After creation, this will be a degenerate NinePatch.  It will identify
     * the center pixel of the image as the interior.  All other pixels will
     * either be a corner or side.
     *
     * @param filename  A path to image file, e.g., "scene1/earthtile.png"
     *
     * @return  true if the node is initialized properly, false otherwise.
     */
    bool initWithFile(const std::string& filename);
    
    /**
     * Initializes a NinePatch with the given interior from the image filename.
     *
     * The interior rectangle is specified in pixel coordinates. As with
     * {@link PolygonNode}, we assume that the pixel origin is the bottom-left
     * corner of the image.
     *
     * The interior rectangle fully defines the NinePatch. For example, suppose
     * the rectangle has origin (2,3) and size (4,2).  Then (1,1) is in the
     * bottom left corner, while (3,1) is in the bottom middle, and so on.
     *
     * @param filename  A path to image file, e.g., "scene1/earthtile.png"
     * @param interior  The rectangle (in pixel coordinates) defining the interior.
     *
     * @return  true if the node is initialized properly, false otherwise.
     */
    bool initWithFile(const std::string& filename, const Rect& interior);
    
    /**
     * Initializes a degenerate NinePatch from a Texture object.
     *
     * After creation, this will be a degenerate NinePatch.  It will identify
     * the center pixel of the image as the interior.  All other pixels will
     * either be a corner or side.
     *     *
     * @param texture   A shared pointer to a Texture object.
     *
     * @return  true if the node is initialized properly, false otherwise.
     */
    bool initWithTexture(const std::shared_ptr<Texture>& texture);
    
    /**
     * Initializes a NinePatch with the given interior from a Texture object.
     *
     * The interior rectangle is specified in pixel coordinates. As with
     * {@link PolygonNode}, we assume that the pixel origin is the bottom-left
     * corner of the image.
     *
     * The interior rectangle fully defines the NinePatch. For example, suppose
     * the rectangle has origin (2,3) and size (4,2).  Then (1,1) is in the
     * bottom left corner, while (3,1) is in the bottom middle, and so on.
     *
     * @param texture    A shared pointer to a Texture object.
     * @param interior  The rectangle (in pixel coordinates) defining the interior.
     *
     * @return  true if the node is initialized properly, false otherwise.
     */
    bool initWithTexture(const std::shared_ptr<Texture>&  texture, const Rect& interior);
    
    /**
     * Initializes a node with the given JSON specificaton.
     *
     * This initializer is designed to receive the "data" object from the
     * JSON passed to {@link SceneLoader}.  This JSON format supports all
     * of the attribute values of its parent class.  In addition, it supports
     * the following additional attributes:
     *
     *      "texture":  The name of a previously loaded texture asset
     *      "interior": An four-element array of numbers (x,y,width,height)
     *
     * All attributes are optional.  However, it is generally a good idea to
     * specify both to take full advantage of NinePatch features.
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
     * Returns a simple NinePatch with the degenerate texture.
     *
     * You do not need to set the texture; rendering this into a SpriteBatch
     * will simply use the blank texture. However, the result will just
     * be rectangle the same size as the blank texture, and so will not be
     * very interesting.
     *
     * @return a simple NinePatch with the degenerate texture.
     */
    static std::shared_ptr<NinePatch> alloc() {
        std::shared_ptr<NinePatch> node = std::make_shared<NinePatch>();
        return (node->init() ? node : nullptr);
    }
    
    /**
     * Returns a newly allocated NinePatch with the given interior from the image filename.
     *
     * The interior rectangle is specified in pixel coordinates. As with
     * {@link PolygonNode}, we assume that the pixel origin is the bottom-left
     * corner of the image.
     *
     * The interior rectangle fully defines the NinePatch. For example, suppose
     * the rectangle has origin (2,3) and size (4,2).  Then (1,1) is in the
     * bottom left corner, while (3,1) is in the bottom middle, and so on.
     *
     * @param filename  A path to image file, e.g., "scene1/earthtile.png"
     *
     * @return a newly allocated NinePatch with the given interior from the image filename.
     */
    static std::shared_ptr<NinePatch> allocWithFile(const std::string& filename) {
        std::shared_ptr<NinePatch> node = std::make_shared<NinePatch>();
        return (node->initWithFile(filename) ? node : nullptr);
    }
    
    /**
     * Returns a newly allocated NinePatch with the given interior from the image filename.
     *
     * The interior rectangle is specified in pixel coordinates. As with
     * {@link PolygonNode}, we assume that the pixel origin is the bottom-left
     * corner of the image.
     *
     * The interior rectangle fully defines the NinePatch.  Any pixel to the
     * left and above the rectangle is in the top right corner, and so on.
     *
     * @param filename  A path to image file, e.g., "scene1/earthtile.png"
     * @param interior  The rectangle (in pixel coordinates) defining the interior.
     *
     * @return a newly allocated NinePatch with the given interior from the image filename.
     */
    static std::shared_ptr<NinePatch> allocWithFile(const std::string& filename, const Rect& interior) {
        std::shared_ptr<NinePatch> node = std::make_shared<NinePatch>();
        return (node->initWithFile(filename,interior) ? node : nullptr);
    }
    
    /**
     * Returns a newly allocated degenerate NinePatch from a Texture object.
     *
     * After creation, this will be a degenerate NinePatch.  It will identify
     * the center pixel of the image as the interior.  All other pixels will
     * either be a corner or side.
     *
     * @param texture   A shared pointer to a Texture object.
     *
     * @return a newly allocated degenerate NinePatch from a Texture object.
     */
    static std::shared_ptr<NinePatch> allocWithTexture(const std::shared_ptr<Texture>& texture) {
        std::shared_ptr<NinePatch> node = std::make_shared<NinePatch>();
        return (node->initWithTexture(texture) ? node : nullptr);
    }

    /**
     * Returns a newly allocated NinePatch with the given interior from a Texture object.
     *
     * The interior rectangle is specified in pixel coordinates. As with
     * {@link PolygonNode}, we assume that the pixel origin is the bottom-left
     * corner of the image.
     *
     * The interior rectangle fully defines the NinePatch. For example, suppose
     * the rectangle has origin (2,3) and size (4,2).  Then (1,1) is in the
     * bottom left corner, while (3,1) is in the bottom middle, and so on.
     *
     * @param texture    A shared pointer to a Texture object.
     * @param interior  The rectangle (in pixel coordinates) defining the interior.
     *
     * @return a newly allocated NinePatch with the given interior from a Texture object.
     */
    static std::shared_ptr<NinePatch> allocWithTexture(const std::shared_ptr<Texture>& texture,
                                                       const Rect& interior) {
        std::shared_ptr<NinePatch> node = std::make_shared<NinePatch>();
        return (node->initWithTexture(texture,interior) ? node : nullptr);
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
     *      "interior": An four-element array of numbers (x,y,width,height)
     *
     * All attributes are optional.  However, it is generally a good idea to
     * specify both to take full advantage of NinePatch features.
     *
     * @param loader    The scene loader passing this JSON file
     * @param data      The JSON object specifying the node
     *
     * @return a newly allocated node with the given JSON specificaton.
     */
    static std::shared_ptr<Node> allocWithData(const SceneLoader* loader,
                                               const std::shared_ptr<JsonValue>& data) {
        std::shared_ptr<NinePatch> result = std::make_shared<NinePatch>();
        if (!result->initWithData(loader,data)) { result = nullptr; }
        return std::dynamic_pointer_cast<Node>(result);
    }
    

#pragma mark -
#pragma mark Attributes
    /**
     * Sets the untransformed size of the node.
     *
     * The content size remains the same no matter how the node is scaled or
     * rotated. All nodes must have a size, though it may be degenerate (0,0).
     *
     * Changing the size of a rectangle will not change the position of the
     * node.  However, if the anchor is not the bottom-left corner, it will
     * change the origin.  The Node will grow out from an anchor on an edge,
     * and equidistant from an anchor in the center.
     *
     * @param size  The untransformed size of the node.
     */
    virtual void setContentSize(const Size& size) override;
    
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
     * Sets interior rectangle defining the NinePatch.
     *
     * The interior rectangle is specified in pixel coordinates. As with
     * {@link PolygonNode}, we assume that the pixel origin is the bottom-left
     * corner of the image.
     *
     * The interior rectangle fully defines the NinePatch. For example, suppose
     * the rectangle has origin (2,3) and size (4,2).  Then (1,1) is in the
     * bottom left corner, while (3,1) is in the bottom middle, and so on.
     *
     * @param interior The NinePatch interior
     */
    void setInterior(const Rect& interior);
    
    /**
     * Returns interior rectangle defining the NinePatch.
     *
     * The interior rectangle is specified in pixel coordinates. As with
     * {@link PolygonNode}, we assume that the pixel origin is the bottom-left
     * corner of the image.
     *
     * The interior rectangle fully defines the NinePatch. For example, suppose
     * the rectangle has origin (2,3) and size (4,2).  Then (1,1) is in the
     * bottom left corner, while (3,1) is in the bottom middle, and so on.
     *
     * @returns interior rectangle defining the NinePatch.
     */
    const Rect& getInterior() const { return _interior; }
    
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
                      const Mat4& transform, Color4 tint) override;
    
    /**
     * Refreshes this node to restore the render data.
     */
    void refresh() { clearRenderData(); generateRenderData(); }
    
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
#pragma mark Internal Helpers
protected:
    /**
     * Allocate the render data necessary to render this node.
     */
    virtual void generateRenderData();
    
    /**
     * Clears the render data, releasing all vertices and indices.
     */
    void clearRenderData();

    /**
     * Generates the textured quad for one of the nine patches.
     *
     * This function generates a quad for the rectangle dst, using the
     * subtexture identified by src.  If dst is larger than src, the image
     * is stretched to fit.  The vertices are added to _vertices and the
     * indices are added to _indices.
     *
     * The value offset specifies the initial index to use for _indices.
     * The indices for all created vertices are start from this value.
     * The value returned is the next index available.
     *
     * @param src       The subtexture region to use
     * @param dst       The rectangle to texture and to add to _vertices
     * @param offset    The first available vertex index
     *
     * @return the next available vertex index
     */
    unsigned short generatePatch(const Rect& src, const Rect& dst, unsigned short offset);
    
    /** This macro disables the copy constructor (not allowed on scene graphs) */
    CU_DISALLOW_COPY_AND_ASSIGN(NinePatch);
};

}
#endif /* __CU_NINEPATCH_H__ */

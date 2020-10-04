//
//  CUSpriteBatch.h
//  Cornell University Game Library (CUGL)
//
//  This module provides one-stop shopping for basic 2d graphics.  Despite the
//  name, it is also capable of drawing solid shapes, as well as wireframes.
//  The only time you might want a separate class is when you need another
//  shader.  And even then, you can swap out the shader for this class.
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
//  Version: 6/23/16

#ifndef __CU_SPRITE_BATCH_H__
#define __CU_SPRITE_BATCH_H__

#include <SDL/SDL.h>
#include <cugl/math/CUMathBase.h>
#include <cugl/math/CUMat4.h>
#include <cugl/renderer/CUVertex.h>
#include <vector>

#define DEFAULT_CAPACITY 8192

namespace cugl {

/** Forward references */
class SpriteShader;
class Affine2;
class Texture;
class RectCugl;
class Poly2;
    
/**
 * This class is a sprite batch for drawing 2d graphics.
 *
 * A sprite batch gathers together sprites and draws them as a single mesh
 * whenever possible.  Changing the active texture requires that the sprite 
 * batch flush the mesh.  Hence, using a single texture atlas can significantly
 * improve drawing speed.
 *
 * This is sprite batch is different from a classic sprite batch in that it 
 * can draw both solid shapes and outlines. Outlines use the same texturing 
 * rules that solids do.  
 *
 * In addition, this sprite batch is capable of drawing without an active 
 * texture.  In that case, the shape will be drawn with a solid color.
 */
class SpriteBatch {
#pragma mark Values
private:
    /** The shader for this sprite batch */
    std::shared_ptr<SpriteShader> _shader;
    /** The vertex capacity of the mesh */
    unsigned int _capacity;
    
    /** The OpenGL vertex array object */
    GLuint _vertArray;
    /** The OpenGL vertex buffer object */
    GLuint _vertBuffer;
    /** The OpenGL index buffer object */
    GLuint _indxBuffer;
    
    /** The sprite batch vertex mesh */
    Vertex2* _vertData;
    /** The indices for the vertex mesh */
    GLuint*  _indxData;
    
    /** The size of the vertex mesh */
    unsigned int _vertMax;
    /** The number of vertices in the current mesh */
    unsigned int _vertSize;

    /** The size of the associated index array */
    unsigned int _indxMax;
    /** The number of indices in the current mesh */
    unsigned int _indxSize;

    /** The active texture */
    std::shared_ptr<Texture> _texture;
    /** The active color */
    Color4 _color;
    /** The active drawing command */
    GLenum _command;
    /** The active perspective matrix */
    Mat4 _perspective;
    
    /** The blending equation for this sprite batch */
    GLenum _blendEquation;
    /** The source factor for the blend function */
    GLenum _srcFactor;
    /** The destination factor for the blend function */
    GLenum _dstFactor;
    
    /** The number of vertices drawn in this pass (so far) */
    unsigned int _vertTotal;
    /** The number of OpenGL calls in this pass (so far) */
    unsigned int _callTotal;
    
    /** Whether this sprite batch has been initialized yet */
    bool _initialized;
    /** Whether this sprite batch is currently active */
    bool _active;

    /** The blank (nullptr) texture */
    static std::shared_ptr<Texture> _blank;

    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates a degenerate sprite batch with no buffers.
     *
     * You must initialize the buffer before using it.
     */
    SpriteBatch();
    
    /**
     * Deletes the sprite batch, disposing all resources
     */
    ~SpriteBatch() { dispose(); }

    /**
     * Deletes the vertex buffers and resets all attributes.
     *
     * You must reinitialize the sprite batch to use it.
     */
    void dispose();
    
    /** 
     * Initializes a sprite batch with the default vertex capacity.
     *
     * The default vertex capacity is 8192 vertices and 8192*3 = 24576 indices.
     * If the mesh exceeds these values, the sprite batch will flush before
     * before continuing to draw.  If you wish to increase (or decrease) the
     * capacity, use the alternate initializer.
     *
     * The sprite batch begins with the default blank texture, and color white.
     * The perspective matrix is the identity.
     *
     * @return true if initialization was successful.
     */
    bool init();
    
    /**
     * Initializes a sprite batch with the default vertex capacity and given shader
     *
     * The default vertex capacity is 8192 vertices and 8192*3 = 24576 indices.
     * If the mesh exceeds these values, the sprite batch will flush before
     * before continuing to draw.  If you wish to increase (or decrease) the
     * capacity, use the alternate initializer.
     *
     * The sprite batch begins with the default blank texture, and color white.
     * The perspective matrix is the identity.
     *
     * @return true if initialization was successful.
     */
    bool init(std::shared_ptr<SpriteShader> shader) {
        return init(DEFAULT_CAPACITY,shader);
    }
    
    /**
     * Initializes a sprite batch with the given vertex capacity.
     *
     * The index capacity will be 3 times the vertex capacity.  The maximum
     * number of possible indices is the maximum size_t, so the vertex size
     * must be a third that.
     *
     * If the mesh exceeds the capacity, the sprite batch will flush before
     * before continuing to draw. You should tune your system to have the 
     * appropriate capacity.  To small a capacity will cause the system to
     * thrash.  However, too large a capacity could stall on memory transfers.
     *
     * The sprite batch begins with the default blank texture, and color white.
     * The perspective matrix is the identity.
     *
     * @return true if initialization was successful.
     */
    bool init(unsigned int capacity);
    
    /**
     * Initializes a sprite batch with the given vertex capacity and shader
     *
     * The index capacity will be 3 times the vertex capacity.  The maximum
     * number of possible indices is the maximum size_t, so the vertex size
     * must be a third that.
     *
     * If the mesh exceeds the capacity, the sprite batch will flush before
     * before continuing to draw. You should tune your system to have the
     * appropriate capacity.  To small a capacity will cause the system to
     * thrash.  However, too large a capacity could stall on memory transfers.
     *
     * The sprite batch begins with the default blank texture, and color white.
     * The perspective matrix is the identity.
     *
     * @return true if initialization was successful.
     */
    bool init(unsigned int capacity, std::shared_ptr<SpriteShader> shader);
    
#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a new sprite batch with the default vertex capacity.
     *
     * The default vertex capacity is 8192 vertices and 8192*3 = 24576 indices.
     * If the mesh exceeds these values, the sprite batch will flush before
     * before continuing to draw.  If you wish to increase (or decrease) the
     * capacity, use the alternate initializer.
     *
     * The sprite batch begins with the default blank texture, and color white.
     * The perspective matrix is the identity.
     *
     * @return a new sprite batch with the default vertex capacity.
     */
    static std::shared_ptr<SpriteBatch> alloc() {
        std::shared_ptr<SpriteBatch> result = std::make_shared<SpriteBatch>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a new sprite batch with the given vertex capacity.
     *
     * The index capacity will be 3 times the vertex capacity.  The maximum
     * number of possible indices is the maximum size_t, so the vertex size
     * must be a third that.
     *
     * If the mesh exceeds the capacity, the sprite batch will flush before
     * before continuing to draw. You should tune your system to have the
     * appropriate capacity.  To small a capacity will cause the system to
     * thrash.  However, too large a capacity could stall on memory transfers.
     *
     * The sprite batch begins with the default blank texture, and color white.
     * The perspective matrix is the identity.
     *
     * @return a new sprite batch with the given vertex capacity.
     */
    static std::shared_ptr<SpriteBatch> alloc(unsigned int capacity) {
        std::shared_ptr<SpriteBatch> result = std::make_shared<SpriteBatch>();
        return (result->init(capacity) ? result : nullptr);
    }
    
#pragma mark -
#pragma mark Attributes
    /**
     * Returns true if this sprite batch has been initialized and is ready for use.
     *
     * @return true if this sprite batch has been initialized and is ready for use.
     */
    bool isReady() const { return _initialized; }
    
    /**
     * Returns whether this sprite batch is actively drawing.
     *
     * A sprite batch is in use if begin() has been called without the
     * requisite end() to flush the pipeline.
     *
     * @return whether this sprite batch is actively drawing.
     */
    bool isDrawing() const { return _active; }

    /**
     * Returns the number of vertices drawn in the latest pass (so far).
     *
     * This value will be reset to 0 whenever begin() is called.
     *
     * @return the number of vertices drawn in the latest pass (so far).
     */
    unsigned int getVerticesDrawn() const { return _vertTotal; }

    /**
     * Returns the number of OpenGL calls in the latest pass (so far).
     *
     * This value will be reset to 0 whenever begin() is called.
     *
     * @return the number of OpenGL calls in the latest pass (so far).
     */
    unsigned int getCallsMade() const { return _callTotal; }

    /**
     * Sets the shader for this sprite batch
     *
     * This value may NOT be changed during a drawing pass.
     *
     * @param shader The active color for this sprite batch
     */
    void setShader(const std::shared_ptr<SpriteShader>& shader);
    
    /**
     * Returns the shader for this sprite batch
     *
     * This value may NOT be changed during a drawing pass.
     *
     * @return the shader for this sprite batch
     */
    const std::shared_ptr<SpriteShader>& getShader() const { return _shader; }
    
    /**
     * Sets the active color of this sprite batch 
     *
     * All subsequent shapes and outlines drawn by this sprite batch will be
     * tinted by this color.  This color is white by default.
     *
     * Changing this value will not cause the sprite batch to flush.
     *
     * @param color The active color for this sprite batch
     */
    void setColor(Color4 color) { _color = color; }

    /**
     * Returns the active color of this sprite batch
     *
     * All subsequent shapes and outlines drawn by this sprite batch will be
     * tinted by this color.  This color is white by default.
     *
     * @return the active color of this sprite batch
     */
    Color4 getColor() const { return _color; }
    
    /**
     * Sets the active texture of this sprite batch
     *
     * All subsequent shapes and outlines drawn by this sprite batch will use 
     * this texture.  If the value is nullptr, all shapes and outlines will be
     * draw with a solid color instead.  This value is nullptr by default.
     *
     * Changing this value will cause the sprite batch to flush.  However, a
     * subtexture will not cause a pipeline flush.  This is an important 
     * argument for using texture atlases.
     *
     * @param texture The active texture for this sprite batch
     */
    void setTexture(const std::shared_ptr<Texture>& texture);

    /**
     * Returns the active texture of this sprite batch
     *
     * All subsequent shapes and outlines drawn by this sprite batch will use
     * this texture.  If the value is nullptr, all shapes and outlines will be
     * drawn with a solid color instead.  This value is nullptr by default.
     *
     * @return the active texture of this sprite batch
     */
    const std::shared_ptr<Texture>& getTexture() const { return _texture; }
    
    /**
     * Returns the blank texture used to make solid shapes
     *
     * This is the texture used when the active texture is nullptr.  Using this
     * texture means that all shapes and outlines will be drawn with a solid 
     * color instead.
     *
     * @return the blank texture used to make solid shapes
     */
    static const std::shared_ptr<Texture>& getBlankTexture();

    // TODO: Orthographic support from query
    /**
     * Sets the active perspective matrix of this sprite batch
     *
     * The perspective matrix is the combined modelview-projection from the
     * camera. By default, this is the identity matrix. Changing this value
     * will cause the sprite batch to flush.
     *
     * @param perspective   The active perspective matrix for this sprite batch
     */
    void setPerspective(const Mat4& perspective);
    
    /**
     * Returns the active perspective matrix of this sprite batch
     *
     * The perspective matrix is the combined modelview-projection from the
     * camera.  By default, this is the identity matrix.
     *
     * @return the active perspective matrix of this sprite batch
     */
    const Mat4& getPerspective() const { return _perspective; }
    
    /**
     * Sets the blending function for this sprite batch
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
     * Changing this value will cause the sprite batch to flush.
     *
     * @param srcFactor Specifies how the source blending factors are computed
     * @param dstFactor Specifies how the destination blending factors are computed.
     */
    void setBlendFunc(GLenum srcFactor, GLenum dstFactor);
    
    /** 
     * Returns the source blending factor
     *
     * By default this value is GL_SRC_ALPHA. For other options, see
     *
     *      https://www.opengl.org/sdk/docs/man/html/glBlendFunc.xhtml
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
     * @return the destination blending factor
     */
    GLenum getDestinationBlendFactor() const { return _srcFactor; }
    
    /**
     * Sets the blending equation for this sprite batch
     *
     * The enum must be a standard ones supported by OpenGL.  See
     *
     *      https://www.opengl.org/sdk/docs/man/html/glBlendEquation.xhtml
     *
     * However, this setter does not do any error checking to verify that
     * the input is valid.  By default, the equation is GL_FUNC_ADD.
     *
     * Changing this value will cause the sprite batch to flush.
     *
     * @param equation  Specifies how source and destination colors are combined
     */
    void setBlendEquation(GLenum equation);

    /**
     * Returns the blending equation for this sprite batch
     *
     * By default this value is GL_FUNC_ADD. For other options, see
     *
     *      https://www.opengl.org/sdk/docs/man/html/glBlendEquation.xhtml
     *
     * @return the blending equation for this sprite batch
     */
    GLenum getBlendEquation() const { return _blendEquation; }
    
#pragma mark -
#pragma mark Rendering
    /**
     * Starts drawing with the current perspective matrix.
     *
     * This call will disable depth buffer writing. It enables blending and
     * texturing. You must call end() to complete drawing.
     *
     * Calling this method will reset the vertex and OpenGL call counters to 0.
     */
    void begin();
    
    /**
     * Starts drawing with the given perspective matrix.
     *
     * This call will disable depth buffer writing. It enables blending and
     * texturing. You must call {@link end()} to complete drawing.
     *
     * Calling this method will reset the vertex and OpenGL call counters to 0.
     *
     * @param perspective   The perspective matrix to draw with.
     */
    void begin(const Mat4& perspective) {
        setPerspective(perspective); begin();
    }
    
    /**
     * Completes the drawing pass for this sprite batch, flushing the buffer.
     *
     * This method enables depth writes and disables blending and texturing. It
     * Must always be called after a call to {@link #begin()}.
     */
    void end();
    
    /**
     * Flushes the current mesh without completing the drawing pass.
     *
     * This method is called whenever you change any attribute other than color
     * mid-pass. It prevents the attribute change from retoactively affecting
     * previuosly drawn shapes.
     */
    void flush();

#pragma mark -
#pragma mark Solid Shapes
    /**
     * Draws the given rectangle filled with the current color and texture.
     *
     * The texture will fill the entire rectangle with texture coordinate 
     * (0,1) at the bottom left corner identified by rect,origin. To draw only
     * part of a texture, use a subtexture to fill the rectangle with the
     * region [minS,maxS]x[min,maxT]. Alternatively, you can use a {@link Poly2}
     * for more fine-tuned control.
     *
     * @param rect      The rectangle to draw
     */
    void fill(const RectCugl& rect);
    
    /**
     * Draws the given rectangle filled with the current color and texture.
     *
     * The rectangle will be scaled first, then rotated, and finally offset
     * by the given position. Rotation is measured in radians and is counter
     * clockwise from the x-axis.  Rotation will be about the provided origin, 
     * which is specified in world coordinates (not relative to the rectangle).
     * A rectangle should be centered on the origin to rotate properly.
     *
     * The texture will fill the entire rectangle before being transformed. 
     * Texture coordinate (0,1) will at the bottom left corner identified by 
     * rect,origin. To draw only part of a texture, use a subtexture to fill 
     * the rectangle with the region [minS,maxS]x[min,maxT]. Alternatively, you 
     * can use a {@link Poly2} for more fine-tuned control.
     *
     * @param rect      The rectangle to draw
     * @param origin    The rotation origin
     * @param scale     The amount to scale the rectangle
     * @param angle     The amount to rotate the rectangle
     * @param offset    The rectangle offset
     */
    void fill(const RectCugl& rect, const Vec2& origin, const Vec2& scale, float angle, const Vec2& offset);
    
    /**
     * Draws the given rectangle filled with the current color and texture.
     *
     * The rectangle will transformed by the given matrix. The transform will 
     * be applied assuming the given origin, which is specified in world 
     * world coordinates (but which should ideally be inside the rectangle 
     * bounds).
     *
     * The texture will fill the entire rectangle with texture coordinate
     * (0,1) at the bottom left corner identified by rect,origin. To draw only
     * part of a texture, use a subtexture to fill the rectangle with the
     * region [minS,maxS]x[min,maxT]. Alternatively, you can use a {@link Poly2}
     * for more fine-tuned control.
     *
     * @param rect      The rectangle to draw
     * @param origin    The coordinate origin
     * @param transform The coordinate transform
     */
    void fill(const RectCugl& rect, const Vec2& origin, const Mat4& transform);
    
    /**
     * Draws the given rectangle filled with the current color and texture.
     *
     * The rectangle will transformed by the given matrix. The transform will
     * be applied assuming the given origin, which is specified in world
     * world coordinates (but which should ideally be inside the rectangle
     * bounds).
     *
     * The texture will fill the entire rectangle with texture coordinate
     * (0,1) at the bottom left corner identified by rect,origin. To draw only
     * part of a texture, use a subtexture to fill the rectangle with the
     * region [minS,maxS]x[min,maxT]. Alternatively, you can use a {@link Poly2}
     * for more fine-tuned control.
     *
     * @param rect      The rectangle to draw
     * @param origin    The coordinate origin
     * @param transform The coordinate transform
     */
    void fill(const RectCugl& rect, const Vec2& origin, const Affine2& transform);

    /**
     * Draws the given polygon filled with the current color and texture.
     *
     * The polygon tesselation will be determined by the indices in poly. If
     * the polygon has not been triangulated (by one of the triangulation 
     * factories {@link SimpleTriangulator} or {@link DelaunayTriangulator},
     * it may not draw properly.
     *
     * The vertex coordinates will be determined by polygon vertex position.
     * A horizontal position x has texture coordinate x/texture.width. A 
     * vertical coordinate has texture coordinate 1-y/texture.height. As a 
     * result, a rectangular polygon that has the same dimensions as the texture
     * is the same as simply drawing the texture.  
     *
     * One way to think of the polygon is as a "cookie cutter".  Treat the 
     * polygon coordinates as pixel coordinates in the texture filed, and use
     * that to determine how the texture fills the polygon. This may make the
     * polygon larger than you like in order to get the appropriate texturing.
     * You should use one of the transform methods to fix this.
     *
     * @param poly      The polygon to draw
     */
    void fill(const Poly2& poly);

    /**
     * Draws the given polygon filled with the current color and texture.
     *
     * The polygon will be offset by the given position.
     *
     * The polygon tesselation will be determined by the indices in poly. If
     * the polygon has not been triangulated (by one of the triangulation
     * factories {@link SimpleTriangulator} or {@link DelaunayTriangulator},
     * it may not draw properly.
     *
     * The vertex coordinates will be determined by polygon vertex position.
     * A horizontal position x has texture coordinate x/texture.width. A
     * vertical coordinate has texture coordinate 1-y/texture.height. As a
     * result, a rectangular polygon that has the same dimensions as the texture
     * is the same as simply drawing the texture.
     *
     * One way to think of the polygon is as a "cookie cutter".  Treat the
     * polygon coordinates as pixel coordinates in the texture filed, and use
     * that to determine how the texture fills the polygon. This may make the
     * polygon larger than you like in order to get the appropriate texturing.
     * You should use one of the transform methods to fix this.
     *
     * @param poly      The polygon to draw
     * @param offset    The polygon offset
     */
    void fill(const Poly2& poly, const Vec2& offset);
    
    /**
     * Draws the given polygon filled with the current color and texture.
     *
     * The polygon will be scaled first, then rotated, and finally offset
     * by the given position. Rotation is measured in radians and is counter
     * clockwise from the x-axis.  Rotation will be about the provided origin,
     * which is specified in world coordinates (not relative to the rectangle)
     * a rectangle should be centered on the origin to rotate properly.
     *
     * The polygon tesselation will be determined by the indices in poly. If
     * the polygon has not been triangulated (by one of the triangulation
     * factories {@link SimpleTriangulator} or {@link DelaunayTriangulator},
     * it may not draw properly.
     *
     * The vertex coordinates will be determined by polygon vertex position.
     * A horizontal position x has texture coordinate x/texture.width. A
     * vertical coordinate has texture coordinate 1-y/texture.height. As a
     * result, a rectangular polygon that has the same dimensions as the texture
     * is the same as simply drawing the texture.
     *
     * One way to think of the polygon is as a "cookie cutter".  Treat the
     * polygon coordinates as pixel coordinates in the texture filed, and use
     * that to determine how the texture fills the polygon. This may make the
     * polygon larger than you like in order to get the appropriate texturing.
     * You should use one of the transform methods to fix this.
     *
     * @param poly      The polygon to draw
     * @param origin    The rotation origin
     * @param scale     The amount to scale the polygon
     * @param angle     The amount to rotate the polygon
     * @param offset    The polygon offset
     */
    void fill(const Poly2& poly, const Vec2& origin, const Vec2& scale, float angle, const Vec2& offset);

    /**
     * Draws the given polygon filled with the current color and texture.
     *
     * The polygon will transformed by the given matrix. The transform will
     * be applied assuming the given origin, which is specified in world
     * world coordinates (but which should ideally be inside the polygon
     * bounds).
     *
     * The polygon tesselation will be determined by the indices in poly. If
     * the polygon has not been triangulated (by one of the triangulation
     * factories {@link SimpleTriangulator} or {@link DelaunayTriangulator},
     * it may not draw properly.
     *
     * The vertex coordinates will be determined by polygon vertex position.
     * A horizontal position x has texture coordinate x/texture.width. A
     * vertical coordinate has texture coordinate 1-y/texture.height. As a
     * result, a rectangular polygon that has the same dimensions as the texture
     * is the same as simply drawing the texture.
     *
     * One way to think of the polygon is as a "cookie cutter".  Treat the
     * polygon coordinates as pixel coordinates in the texture filed, and use
     * that to determine how the texture fills the polygon. This may make the
     * polygon larger than you like in order to get the appropriate texturing.
     * You should use one of the transform methods to fix this.
     *
     * @param poly      The polygon to draw
     * @param origin    The coordinate origin
     * @param transform The coordinate transform
     */
    void fill(const Poly2& poly, const Vec2& origin, const Mat4& transform);
    
    /**
     * Draws the given polygon filled with the current color and texture.
     *
     * The polygon will transformed by the given matrix. The transform will
     * be applied assuming the given origin, which is specified in world
     * world coordinates (but which should ideally be inside the polygon
     * bounds).
     *
     * The polygon tesselation will be determined by the indices in poly. If
     * the polygon has not been triangulated (by one of the triangulation
     * factories {@link SimpleTriangulator} or {@link DelaunayTriangulator},
     * it may not draw properly.
     *
     * The vertex coordinates will be determined by polygon vertex position.
     * A horizontal position x has texture coordinate x/texture.width. A
     * vertical coordinate has texture coordinate 1-y/texture.height. As a
     * result, a rectangular polygon that has the same dimensions as the texture
     * is the same as simply drawing the texture.
     *
     * One way to think of the polygon is as a "cookie cutter".  Treat the
     * polygon coordinates as pixel coordinates in the texture filed, and use
     * that to determine how the texture fills the polygon. This may make the
     * polygon larger than you like in order to get the appropriate texturing.
     * You should use one of the transform methods to fix this.
     *
     * @param poly      The polygon to draw
     * @param origin    The coordinate origin
     * @param transform The coordinate transform
     */
    void fill(const Poly2& poly, const Vec2& origin, const Affine2& transform);

    /**
     * Fills the triangulated vertices with the current texture.
     *
     * This method provides more fine tuned control over texture coordinates
     * that the other fill methods.  The texture no longer needs to be
     * drawn uniformly over the shape. The transform will be applied to the
     * vertex positions directly in world space.
     *
     * The triangulation will be determined by the given indices. If necessary,
     * these can be generated via one of the triangulation factories
     * {@link SimpleTriangulator} or {@link DelaunayTriangulator}.
     *
     * The vertices use their own color values.  However, if tint is true, these
     * values will be tinted (i.e. multiplied) by the current active color.
     *
     * @param vertices  The list of vertices
     * @param indices   The triangulation list
     * @param transform The coordinate transform
     * @param tint      Whether to tint with the active color
     */
    void fill(const std::vector<Vertex2>& vertices, const std::vector<unsigned short>& indices,
              const Mat4& transform, bool tint = true) {
        fill(vertices.data(),(unsigned int)vertices.size(),0,indices.data(),(unsigned int)indices.size(),0,transform,tint);
    }

    /**
     * Fills the triangulated vertices with the current texture.
     *
     * This method provides more fine tuned control over texture coordinates
     * that the other fill methods.  The texture no longer needs to be
     * drawn uniformly over the shape. The transform will be applied to the
     * vertex positions directly in world space.
     *
     * The triangulation will be determined by the given indices. If necessary,
     * these can be generated via one of the triangulation factories
     * {@link SimpleTriangulator} or {@link DelaunayTriangulator}.
     *
     * The vertices use their own color values.  However, if tint is true, these
     * values will be tinted (i.e. multiplied) by the current active color.
     *
     * @param vertices  The list of vertices
     * @param indices   The triangulation list
     * @param transform The coordinate transform
     * @param tint      Whether to tint with the active color
     */
    void fill(const std::vector<Vertex2>& vertices, const std::vector<unsigned short>& indices,
              const Affine2& transform, bool tint = true) {
        fill(vertices.data(),(unsigned int)vertices.size(),0,indices.data(),(unsigned int)indices.size(),0,transform,tint);
    }

    /**
     * Fills the triangulated vertices with the current texture.
     *
     * This method provides more fine tuned control over texture coordinates
     * that the other fill methods.  The texture no longer needs to be
     * drawn uniformly over the shape. The transform will be applied to the
     * vertex positions directly in world space.
     *
     * The triangulation will be determined by the given indices. If necessary,
     * these can be generated via one of the triangulation factories
     * {@link SimpleTriangulator} or {@link DelaunayTriangulator}.
     *
     * The vertices use their own color values.  However, if tint is true, these
     * values will be tinted (i.e. multiplied) by the current active color.
     *
     * @param vertices  The array of vertices
     * @param vsize     The size of the vertex array
     * @param voffset   The first element of the vertex array
     * @param indices   The triangulation array
     * @param isize     The size of the index array
     * @param ioffset   The first element of the index array
     * @param transform The coordinate transform
     * @param tint      Whether to tint with the active color
     */
    void fill(const Vertex2* vertices, unsigned int vsize, unsigned int voffset,
              const unsigned short* indices, unsigned int isize, unsigned int ioffset,
              const Mat4& transform, bool tint = true);

    /**
     * Fills the triangulated vertices with the current texture.
     *
     * This method provides more fine tuned control over texture coordinates
     * that the other fill methods.  The texture no longer needs to be
     * drawn uniformly over the shape. The transform will be applied to the
     * vertex positions directly in world space.
     *
     * The triangulation will be determined by the given indices. If necessary,
     * these can be generated via one of the triangulation factories
     * {@link SimpleTriangulator} or {@link DelaunayTriangulator}.
     *
     * The vertices use their own color values.  However, if tint is true, these
     * values will be tinted (i.e. multiplied) by the current active color.
     *
     * @param vertices  The array of vertices
     * @param vsize     The size of the vertex array
     * @param voffset   The first element of the vertex array
     * @param indices   The triangulation array
     * @param isize     The size of the index array
     * @param ioffset   The first element of the index array
     * @param transform The coordinate transform
     * @param tint      Whether to tint with the active color
     */
    void fill(const Vertex2* vertices, unsigned int vsize, unsigned int voffset,
              const unsigned short* indices, unsigned int isize, unsigned int ioffset,
              const Affine2& transform, bool tint = true);

#pragma mark -
#pragma mark Outlines
    /**
     * Outlines the given rectangle with the current color and texture.
     *
     * The drawing will be a wireframe of a rectangle.  The wireframe will
     * be textured with Texture coordinate (0,1) at the bottom left corner 
     * identified by rect,origin. The remaining edges will correspond to the
     * edges of the texture. To draw only part of a texture, use a subtexture
     * to outline the edges with [minS,maxS]x[min,maxT]. Alternatively, you can 
     * use a {@link Poly2} for more fine-tuned control.
     *
     * @param rect      The rectangle to outline
     */
    void outline(const RectCugl& rect);
 
    /**
     * Outlines the given rectangle with the current color and texture.
     *
     * The rectangle will be scaled first, then rotated, and finally offset
     * by the given position. Rotation is measured in radians and is counter
     * clockwise from the x-axis.  Rotation will be about the provided origin,
     * which is specified in world coordinates (not relative to the rectangle)
     * a rectangle should be centered on the origin to rotate properly.
     *
     * The drawing will be a wireframe of a rectangle.  The wireframe will
     * be textured with Texture coordinate (0,1) at the bottom left corner
     * identified by rect,origin. The remaining edges will correspond to the
     * edges of the texture. To draw only part of a texture, use a subtexture
     * to outline the edges with [minS,maxS]x[min,maxT]. Alternatively, you can
     * use a {@link Poly2} for more fine-tuned control.
     *
     * @param rect      The rectangle to outline
     * @param origin    The rotation origin
     * @param scale     The amount to scale the rectangle
     * @param angle     The amount to rotate the rectangle
     * @param offset    The rectangle offset
     */
    void outline(const RectCugl& rect, const Vec2& origin, const Vec2& scale, float angle, const Vec2& offset);
    
    /**
     * Outlines the given rectangle with the current color and texture.
     *
     * The rectangle will transformed by the given matrix. The transform will
     * be applied assuming the given origin, which is specified in world
     * world coordinates (but which should ideally be inside the rectangle
     * bounds).
     *
     * The drawing will be a wireframe of a rectangle.  The wireframe will
     * be textured with Texture coordinate (0,1) at the bottom left corner
     * identified by rect,origin. The remaining edges will correspond to the
     * edges of the texture. To draw only part of a texture, use a subtexture
     * to outline the edges with [minS,maxS]x[min,maxT]. Alternatively, you can
     * use a {@link Poly2} for more fine-tuned control.
     *
     * @param rect      The rectangle to outline
     * @param origin    The coordinate origin
     * @param transform The coordinate transform
     */
    void outline(const RectCugl& rect, const Vec2& origin, const Mat4& transform);
    
    /**
     * Outlines the given rectangle with the current color and texture.
     *
     * The rectangle will transformed by the given matrix. The transform will
     * be applied assuming the given origin, which is specified in world
     * world coordinates (but which should ideally be inside the rectangle
     * bounds).
     *
     * The drawing will be a wireframe of a rectangle.  The wireframe will
     * be textured with Texture coordinate (0,1) at the bottom left corner
     * identified by rect,origin. The remaining edges will correspond to the
     * edges of the texture. To draw only part of a texture, use a subtexture
     * to outline the edges with [minS,maxS]x[min,maxT]. Alternatively, you can
     * use a {@link Poly2} for more fine-tuned control.
     *
     * @param rect      The rectangle to outline
     * @param origin    The coordinate origin
     * @param transform The coordinate transform
     */
    void outline(const RectCugl& rect, const Vec2& origin, const Affine2& transform);
    
    /**
     * Outlines the given polygon with the current color and texture.
     *
     * The polygon path will be determined by the indices in poly. The indices
     * should be a multiple of two, preferably generated by the factories
     * {@link PathOutliner} or {@link CubicSplineApproximator}.
     *
     * The drawing will be a wireframe of a polygon, but the lines are textured.
     * The vertex coordinates will be determined by polygon vertex position.
     * A horizontal position x has texture coordinate x/texture.width. A
     * vertical coordinate has texture coordinate 1-y/texture.height. As a
     * result, a rectangular polygon that has the same dimensions as the texture
     * is the same as simply outlines the rectangle.
     *
     * One way to think of the polygon is as a "cookie cutter".  Treat the
     * polygon coordinates as pixel coordinates in the texture filed, and use
     * that to determine how the texture fills the polygon. This may make the
     * polygon larger than you like in order to get the appropriate texturing.
     * You should use one of the transform methods to fix this.
     *
     * @param poly      The polygon to outline
     */
    void outline(const Poly2& poly);
    
    /**
     * Outlines the given polygon with the current color and texture.
     *
     * The polygon will be offset by the given position.
     *
     * The polygon path will be determined by the indices in poly. The indices
     * should be a multiple of two, preferably generated by the factories
     * {@link PathOutliner} or {@link CubicSplineApproximator}.
     *
     * The drawing will be a wireframe of a polygon, but the lines are textured.
     * The vertex coordinates will be determined by polygon vertex position.
     * A horizontal position x has texture coordinate x/texture.width. A
     * vertical coordinate has texture coordinate 1-y/texture.height. As a
     * result, a rectangular polygon that has the same dimensions as the texture
     * is the same as simply outlines the rectangle.
     *
     * One way to think of the polygon is as a "cookie cutter".  Treat the
     * polygon coordinates as pixel coordinates in the texture filed, and use
     * that to determine how the texture fills the polygon. This may make the
     * polygon larger than you like in order to get the appropriate texturing.
     * You should use one of the transform methods to fix this.
     *
     * @param poly      The polygon to outline
     * @param offset    The polygon offset
     */
    void outline(const Poly2& poly, const Vec2& offset);
    
    /**
     * Outlines the given polygon with the current color and texture.
     *
     * The polygon will be scaled first, then rotated, and finally offset
     * by the given position. Rotation is measured in radians and is counter
     * clockwise from the x-axis.  Rotation will be about the provided origin,
     * which is specified in world coordinates (not relative to the rectangle)
     * a rectangle should be centered on the origin to rotate properly.
     *
     * The polygon path will be determined by the indices in poly. The indices
     * should be a multiple of two, preferably generated by the factories
     * {@link PathOutliner} or {@link CubicSplineApproximator}.
     *
     * The drawing will be a wireframe of a polygon, but the lines are textured.
     * The vertex coordinates will be determined by polygon vertex position.
     * A horizontal position x has texture coordinate x/texture.width. A
     * vertical coordinate has texture coordinate 1-y/texture.height. As a
     * result, a rectangular polygon that has the same dimensions as the texture
     * is the same as simply outlines the rectangle.
     *
     * One way to think of the polygon is as a "cookie cutter".  Treat the
     * polygon coordinates as pixel coordinates in the texture filed, and use
     * that to determine how the texture fills the polygon. This may make the
     * polygon larger than you like in order to get the appropriate texturing.
     * You should use one of the transform methods to fix this.
     *
     * @param poly      The polygon to outline
     * @param origin    The rotation origin
     * @param scale     The amount to scale the polygon
     * @param angle     The amount to rotate the polygon
     * @param offset    The polygon offset
     */
    void outline(const Poly2& poly, const Vec2& origin, const Vec2& scale, float angle, const Vec2& offset);
    
    /**
     * Outlines the given polygon with the current color and texture.
     *
     * The polygon will transformed by the given matrix. The transform will
     * be applied assuming the given origin, which is specified in world
     * world coordinates (but which should ideally be inside the polygon
     * bounds).
     *
     * The polygon path will be determined by the indices in poly. The indices
     * should be a multiple of two, preferably generated by the factories
     * {@link PathOutliner} or {@link CubicSplineApproximator}.
     *
     * The drawing will be a wireframe of a polygon, but the lines are textured.
     * The vertex coordinates will be determined by polygon vertex position.
     * A horizontal position x has texture coordinate x/texture.width. A
     * vertical coordinate has texture coordinate 1-y/texture.height. As a
     * result, a rectangular polygon that has the same dimensions as the texture
     * is the same as simply outlines the rectangle.
     *
     * One way to think of the polygon is as a "cookie cutter".  Treat the
     * polygon coordinates as pixel coordinates in the texture filed, and use
     * that to determine how the texture fills the polygon. This may make the
     * polygon larger than you like in order to get the appropriate texturing.
     * You should use one of the transform methods to fix this.
     *
     * @param poly      The polygon to outline
     * @param origin    The coordinate origin
     * @param transform The coordinate transform
     */
    void outline(const Poly2& poly, const Vec2& origin, const Mat4& transform);
    
    /**
     * Outlines the given polygon with the current color and texture.
     *
     * The polygon will transformed by the given matrix. The transform will
     * be applied assuming the given origin, which is specified in world
     * world coordinates (but which should ideally be inside the polygon
     * bounds).
     *
     * The polygon path will be determined by the indices in poly. The indices
     * should be a multiple of two, preferably generated by the factories
     * {@link PathOutliner} or {@link CubicSplineApproximator}.
     *
     * The drawing will be a wireframe of a polygon, but the lines are textured.
     * The vertex coordinates will be determined by polygon vertex position.
     * A horizontal position x has texture coordinate x/texture.width. A
     * vertical coordinate has texture coordinate 1-y/texture.height. As a
     * result, a rectangular polygon that has the same dimensions as the texture
     * is the same as simply outlines the rectangle.
     *
     * One way to think of the polygon is as a "cookie cutter".  Treat the
     * polygon coordinates as pixel coordinates in the texture filed, and use
     * that to determine how the texture fills the polygon. This may make the
     * polygon larger than you like in order to get the appropriate texturing.
     * You should use one of the transform methods to fix this.
     *
     * @param poly      The polygon to outline
     * @param origin    The coordinate origin
     * @param transform The coordinate transform
     */
    void outline(const Poly2& poly, const Vec2& origin, const Affine2& transform);
    
    /**
     * Outlines the vertex path with the current texture.
     *
     * This method provides more fine tuned control over texture coordinates
     * that the other outline methods.  The texture no longer needs to be
     * drawn uniformly over the wireframe. The transform will be applied to the
     * vertex positions directly in world space.
     *
     * The vertex path will be determined by the provided indices. The indices
     * should be a multiple of two, preferably generated by the factories
     * {@link PathOutliner} or {@link CubicSplineApproximator}.
     *
     * The vertices use their own color values.  However, if tint is true, these
     * values will be tinted (i.e. multiplied) by the current active color.
     *
     * @param vertices  The list of vertices
     * @param indices   The triangulation list
     * @param transform The coordinate transform
     * @param tint      Whether to tint with the active color
     */
    void outline(const std::vector<Vertex2>& vertices, const std::vector<unsigned short>& indices,
                 const Mat4& transform, bool tint = true) {
        outline(vertices.data(),(unsigned int)vertices.size(),0,indices.data(),(unsigned int)indices.size(),0,transform,tint);
    }
    
    /**
     * Outlines the vertex path with the current texture.
     *
     * This method provides more fine tuned control over texture coordinates
     * that the other outline methods.  The texture no longer needs to be
     * drawn uniformly over the wireframe. The transform will be applied to the
     * vertex positions directly in world space.
     *
     * The vertex path will be determined by the provided indices. The indices
     * should be a multiple of two, preferably generated by the factories
     * {@link PathOutliner} or {@link CubicSplineApproximator}.
     *
     * The vertices use their own color values.  However, if tint is true, these
     * values will be tinted (i.e. multiplied) by the current active color.
     *
     * @param vertices  The list of vertices
     * @param indices   The triangulation list
     * @param transform The coordinate transform
     * @param tint      Whether to tint with the active color
     */
    void outline(const std::vector<Vertex2>& vertices, const std::vector<unsigned short>& indices,
                 const Affine2& transform, bool tint = true) {
        outline(vertices.data(),(unsigned int)vertices.size(),0,indices.data(),(unsigned int)indices.size(),0,transform,tint);
    }
    
    /**
     * Outlines the vertex path with the current texture.
     *
     * This method provides more fine tuned control over texture coordinates
     * that the other outline methods.  The texture no longer needs to be
     * drawn uniformly over the wireframe. The transform will be applied to the
     * vertex positions directly in world space.
     *
     * The vertex path will be determined by the provided indices. The indices
     * should be a multiple of two, preferably generated by the factories
     * {@link PathOutliner} or {@link CubicSplineApproximator}.
     *
     * The vertices use their own color values.  However, if tint is true, these
     * values will be tinted (i.e. multiplied) by the current active color.
     *
     * @param vertices  The array of vertices
     * @param vsize     The size of the vertex array
     * @param voffset   The first element of the vertex array
     * @param indices   The triangulation array
     * @param isize     The size of the index array
     * @param ioffset   The first element of the index array
     * @param transform The coordinate transform
     * @param tint      Whether to tint with the active color
     */
    void outline(const Vertex2* vertices, unsigned int vsize, unsigned int voffset,
                 const unsigned short* indices, unsigned int isize, unsigned int ioffset,
                 const Mat4& transform, bool tint = true);
    
    /**
     * Outlines the vertex path with the current texture.
     *
     * This method provides more fine tuned control over texture coordinates
     * that the other outline methods.  The texture no longer needs to be
     * drawn uniformly over the wireframe. The transform will be applied to the
     * vertex positions directly in world space.
     *
     * The vertex path will be determined by the provided indices. The indices
     * should be a multiple of two, preferably generated by the factories
     * {@link PathOutliner} or {@link CubicSplineApproximator}.
     *
     * The vertices use their own color values.  However, if tint is true, these
     * values will be tinted (i.e. multiplied) by the current active color.
     *
     * @param vertices  The array of vertices
     * @param vsize     The size of the vertex array
     * @param voffset   The first element of the vertex array
     * @param indices   The triangulation array
     * @param isize     The size of the index array
     * @param ioffset   The first element of the index array
     * @param transform The coordinate transform
     * @param tint      Whether to tint with the active color
     */
    void outline(const Vertex2* vertices, unsigned int vsize, unsigned int voffset,
                 const unsigned short* indices, unsigned int isize, unsigned int ioffset,
                 const Affine2& transform, bool tint = true);
    
#pragma mark -
#pragma mark Convenience Methods
    /**
     * Draws the texture (without tint) at the given position
     *
     * This is a convenience method that calls the appropriate fill method.
     * It sets both the texture and color (removing the previous active values).
     * It then draws a rectangle of the size of the texture, with bottom left
     * corner at the given position.
     *
     * @param texture   The new active texture
     * @param position  The bottom left corner of the texture
     */
    void draw(const std::shared_ptr<Texture>& texture, const Vec2& position);
    
    /**
     * Draws the tinted texture at the given position
     *
     * This is a convenience method that calls the appropriate fill method.
     * It sets both the texture and color (removing the previous active values).  
     * It then draws a rectangle of the size of the texture, with bottom left 
     * corner at the given position.
     *
     * @param texture   The new active texture
     * @param color     The new active color
     * @param position  The bottom left corner of the texture
     */
    void draw(const std::shared_ptr<Texture>& texture, Color4 color, const Vec2& position);
    
    /**
     * Draws the texture (without tint) inside the given bounds
     *
     * This is a convenience method that calls the appropriate fill method.
     * It sets both the texture and color (removing the previous active values).
     * It then draws the specified rectangle filled with the texture.
     *
     * @param texture   The new active texture
     * @param bounds    The rectangle to texture
     */
    void draw(const std::shared_ptr<Texture>& texture, const RectCugl& bounds) {
        setTexture(texture); setColor(Color4::WHITE);
        fill(bounds);
    }

    /**
     * Draws the tinted texture at the given position
     *
     * This is a convenience method that calls the appropriate fill method.
     * It sets both the texture and color (removing the previous active values).
     * It then draws the specified rectangle filled with the texture.
     *
     * @param texture   The new active texture
     * @param color     The new active color
     * @param bounds    The rectangle to texture
     */
    void draw(const std::shared_ptr<Texture>& texture, Color4 color, const RectCugl& bounds) {
        setTexture(texture); setColor(color);
        fill(bounds);
    }

    /**
     * Draws the texture (without tint) transformed by the given parameters
     *
     * This is a convenience method that calls the appropriate fill method.
     * It sets both the texture and color (removing the previous active values).
     * It then draws a texture-sized rectangle centered at the given origin, and
     * transformed by the given parameters.
     *
     * The rectangle will be scaled first, then rotated, and finally offset
     * by the given position. Rotation is measured in radians and is counter
     * clockwise from the x-axis.  Rotation will be about the provided origin,
     * which is specified in texture pixel coordinates (e.g from the bottom)
     * left corner).
     *
     * @param texture   The new active texture
     * @param origin    The rotation origin
     * @param scale     The amount to scale the texture
     * @param angle     The amount to rotate the texture
     * @param offset    The texture origin offset
     */
    void draw(const std::shared_ptr<Texture>& texture,
              const Vec2& origin, const Vec2& scale, float angle, const Vec2& offset);
    
    /**
     * Draws the tinted texture transformed by the given parameters
     *
     * This is a convenience method that calls the appropriate fill method.
     * It sets both the texture and color (removing the previous active values).
     * It then draws a texture-sized rectangle centered at the given origin, and
     * transformed by the given parameters.
     *
     * The rectangle will be scaled first, then rotated, and finally offset
     * by the given position. Rotation is measured in radians and is counter
     * clockwise from the x-axis.  Rotation will be about the provided origin,
     * which is specified in texture pixel coordinates (e.g from the bottom)
     * left corner).
     *
     * @param texture   The new active texture
     * @param color     The new active color
     * @param origin    The rotation origin
     * @param scale     The amount to scale the texture
     * @param angle     The amount to rotate the texture
     * @param offset    The texture origin offset
     */
    void draw(const std::shared_ptr<Texture>& texture, Color4 color,
              const Vec2& origin, const Vec2& scale, float angle, const Vec2& offset);
    
    /**
     * Draws the texture (without tint) transformed by the given parameters
     *
     * This is a convenience method that calls the appropriate fill method.
     * It sets both the texture and color (removing the previous active values).
     * It then draws the rectangle centered at the given origin, and transformed 
     * by the given parameters.
     *
     * The rectangle will be scaled first, then rotated, and finally offset
     * by the given position. Rotation is measured in radians and is counter
     * clockwise from the x-axis.  Rotation will be about the provided origin,
     * which is specified in world coordinates (not relative to the rectangle).
     * A rectangle should be centered on the origin to rotate properly.
     *
     * @param texture   The new active texture
     * @param bounds    The rectangle to texture
     * @param origin    The rotation origin
     * @param scale     The amount to scale the texture
     * @param angle     The amount to rotate the texture
     * @param offset    The texture origin offset
     */
    void draw(const std::shared_ptr<Texture>& texture, const RectCugl& bounds,
              const Vec2& origin, const Vec2& scale, float angle, const Vec2& offset) {
        setTexture(texture); setColor(Color4::WHITE);
        fill(bounds, origin, scale, angle, offset);
    }

    /**
     * Draws the tinted texture transformed by the given parameters
     *
     * This is a convenience method that calls the appropriate fill method.
     * It sets both the texture and color (removing the previous active values).
     * It then draws the rectangle centered at the given origin, and transformed
     * by the given parameters.
     *
     * The rectangle will be scaled first, then rotated, and finally offset
     * by the given position. Rotation is measured in radians and is counter
     * clockwise from the x-axis.  Rotation will be about the provided origin,
     * which is specified in world coordinates (not relative to the rectangle).
     * A rectangle should be centered on the origin to rotate properly.
     *
     * @param texture   The new active texture
     * @param color     The new active color
     * @param bounds    The rectangle to texture
     * @param origin    The rotation origin
     * @param scale     The amount to scale the texture
     * @param angle     The amount to rotate the texture
     * @param offset    The texture origin offset
     */
    void draw(const std::shared_ptr<Texture>& texture, Color4 color, const RectCugl& bounds,
              const Vec2& origin, const Vec2& scale, float angle, const Vec2& offset) {
        setTexture(texture); setColor(color);
        fill(bounds, origin, scale, angle, offset);
    }

    /**
     * Draws the texture (without tint) transformed by the matrix
     *
     * This is a convenience method that calls the appropriate fill method.
     * It sets both the texture and color (removing the previous active values).
     * It then draws a texture-sized rectangle centered at the given origin, and
     * transformed by the given matrix.
     *
     * The transform will be applied assuming the given origin, which is 
     * specified in world world coordinates (but which should ideally be inside 
     * the rectangle bounds).
     *
     * @param texture   The new active texture
     * @param origin    The coordinate origin
     * @param transform The coordinate transform
     */
    void draw(const std::shared_ptr<Texture>& texture, const Vec2& origin, const Mat4& transform);
    
    /**
     * Draws the tinted texture transformed by the matrix
     *
     * This is a convenience method that calls the appropriate fill method.
     * It sets both the texture and color (removing the previous active values).
     * It then draws a texture-sized rectangle centered at the given origin, and
     * transformed by the given matrix.
     *
     * The transform will be applied assuming the given origin, which is
     * specified in world world coordinates (but which should ideally be inside
     * the rectangle bounds).
     *
     * @param texture   The new active texture
     * @param color     The new active color
     * @param origin    The coordinate origin
     * @param transform The coordinate transform
     */
    void draw(const std::shared_ptr<Texture>& texture, Color4 color, const Vec2& origin, const Mat4& transform);
    
    /**
     * Draws the texture (without tint) transformed by the matrix
     *
     * This is a convenience method that calls the appropriate fill method.
     * It sets both the texture and color (removing the previous active values).
     * It then draws the rectangle centered at the given origin, and transformed
     * by the given matrix.
     *
     * The transform will be applied assuming the given origin, which is
     * specified in world world coordinates (but which should ideally be inside
     * the rectangle bounds).
     *
     * @param texture   The new active texture
     * @param bounds    The rectangle to texture
     * @param origin    The coordinate origin
     * @param transform The coordinate transform
     */
    void draw(const std::shared_ptr<Texture>& texture, const RectCugl& bounds,
              const Vec2& origin, const Mat4& transform) {
        setTexture(texture); setColor(Color4::WHITE);
        fill(bounds, origin, transform);
    }

    /**
     * Draws the tinted texture transformed by the matrix
     *
     * This is a convenience method that calls the appropriate fill method.
     * It sets both the texture and color (removing the previous active values).
     * It then draws the rectangle centered at the given origin, and transformed
     * by the given matrix.
     *
     * The transform will be applied assuming the given origin, which is
     * specified in world world coordinates (but which should ideally be inside
     * the rectangle bounds).
     *
     * @param texture   The new active texture
     * @param color     The new active color
     * @param bounds    The rectangle to texture
     * @param origin    The coordinate origin
     * @param transform The coordinate transform
     */
    void draw(const std::shared_ptr<Texture>& texture, Color4 color, const RectCugl& bounds,
              const Vec2& origin, const Mat4& transform) {
        setTexture(texture); setColor(color);
        fill(bounds, origin, transform);
    }

    /**
     * Draws the texture (without tint) transformed by the affine matrix
     *
     * This is a convenience method that calls the appropriate fill method.
     * It sets both the texture and color (removing the previous active values).
     * It then draws a texture-sized rectangle centered at the given origin, and
     * transformed by the given affine matrix.
     *
     * The transform will be applied assuming the given origin, which is
     * specified in world world coordinates (but which should ideally be inside
     * the rectangle bounds).
     *
     * @param texture   The new active texture
     * @param origin    The coordinate origin
     * @param transform The coordinate transform
     */
    void draw(const std::shared_ptr<Texture>& texture, const Vec2& origin, const Affine2& transform);
    
    /**
     * Draws the tinted texture transformed by the affine matrix
     *
     * This is a convenience method that calls the appropriate fill method.
     * It sets both the texture and color (removing the previous active values).
     * It then draws a texture-sized rectangle centered at the given origin, and
     * transformed by the given affine matrix.
     *
     * The transform will be applied assuming the given origin, which is
     * specified in world world coordinates (but which should ideally be inside
     * the rectangle bounds).
     *
     * @param texture   The new active texture
     * @param color     The new active color
     * @param origin    The coordinate origin
     * @param transform The coordinate transform
     */
    void draw(const std::shared_ptr<Texture>& texture, Color4 color,
              const Vec2& origin, const Affine2& transform);
    
    /**
     * Draws the texture (without tint) transformed by the affine matrix
     *
     * This is a convenience method that calls the appropriate fill method.
     * It sets both the texture and color (removing the previous active values).
     * It then draws the rectangle centered at the given origin, and transformed
     * by the given affine matrix.
     *
     * The transform will be applied assuming the given origin, which is
     * specified in world world coordinates (but which should ideally be inside
     * the rectangle bounds).
     *
     * @param texture   The new active texture
     * @param bounds    The rectangle to texture
     * @param origin    The coordinate origin
     * @param transform The coordinate transform
     */
    void draw(const std::shared_ptr<Texture>& texture, const RectCugl& bounds,
              const Vec2& origin, const Affine2& transform) {
        setTexture(texture); setColor(Color4::WHITE);
        fill(bounds, origin, transform);
    }
    
    /**
     * Draws the tinted texture transformed by the affine matrix
     *
     * This is a convenience method that calls the appropriate fill method.
     * It sets both the texture and color (removing the previous active values).
     * It then draws the rectangle centered at the given origin, and transformed
     * by the given affine matrix.
     *
     * The transform will be applied assuming the given origin, which is
     * specified in world world coordinates (but which should ideally be inside
     * the rectangle bounds).
     *
     * @param texture   The new active texture
     * @param color     The new active color
     * @param bounds    The rectangle to texture
     * @param origin    The coordinate origin
     * @param transform The coordinate transform
     */
    void draw(const std::shared_ptr<Texture>& texture, Color4 color, const RectCugl& bounds,
              const Vec2& origin, const Affine2& transform) {
        setTexture(texture); setColor(color);
        fill(bounds, origin, transform);
    }

    /**
     * Draws the textured polygon (without tint) at the given position
     *
     * This is a convenience method that calls the appropriate fill method.
     * It sets both the texture and color (removing the previous active values).
     * It then draws the polygon, offset by the given value.
     *
     * The polygon tesselation will be determined by the indices in poly. If
     * the polygon has not been triangulated (by one of the triangulation
     * factories {@link SimpleTriangulator} or {@link DelaunayTriangulator},
     * it may not draw properly.
     *
     * The vertex coordinates will be determined by polygon vertex position.
     * A horizontal position x has texture coordinate x/texture.width. A
     * vertical coordinate has texture coordinate 1-y/texture.height. As a
     * result, a rectangular polygon that has the same dimensions as the texture
     * is the same as simply drawing the texture.
     *
     * One way to think of the polygon is as a "cookie cutter".  Treat the
     * polygon coordinates as pixel coordinates in the texture filed, and use
     * that to determine how the texture fills the polygon. This may make the
     * polygon larger than you like in order to get the appropriate texturing.
     * You should use one of the transform methods to fix this.
     *
     * @param texture   The new active texture
     * @param poly      The polygon to texture
     * @param offset    The polygon offset
     */
    void draw(const std::shared_ptr<Texture>& texture, const Poly2& poly, const Vec2& offset) {
        setTexture(texture); setColor(Color4::WHITE);
        fill(poly, offset);
    }

    /**
     * Draws the tinted, textured polygon at the given position
     *
     * This is a convenience method that calls the appropriate fill method.
     * It sets both the texture and color (removing the previous active values).
     * It then draws the polygon, offset by the given value.
     *
     * The polygon tesselation will be determined by the indices in poly. If
     * the polygon has not been triangulated (by one of the triangulation
     * factories {@link SimpleTriangulator} or {@link DelaunayTriangulator},
     * it may not draw properly.
     *
     * The vertex coordinates will be determined by polygon vertex position.
     * A horizontal position x has texture coordinate x/texture.width. A
     * vertical coordinate has texture coordinate 1-y/texture.height. As a
     * result, a rectangular polygon that has the same dimensions as the texture
     * is the same as simply drawing the texture.
     *
     * One way to think of the polygon is as a "cookie cutter".  Treat the
     * polygon coordinates as pixel coordinates in the texture filed, and use
     * that to determine how the texture fills the polygon. This may make the
     * polygon larger than you like in order to get the appropriate texturing.
     * You should use one of the transform methods to fix this.
     *
     * @param texture   The new active texture
     * @param color     The new active color
     * @param poly      The polygon to texture
     * @param offset    The polygon offset
     */
    void draw(const std::shared_ptr<Texture>& texture, Color4 color, const Poly2& poly, const Vec2& offset) {
        setTexture(texture); setColor(color);
        fill(poly, offset);
    }
    
    /**
     * Draws the textured polygon (without tint) transformed by the given parameters
     *
     * This is a convenience method that calls the appropriate fill method.
     * It sets both the texture and color (removing the previous active values).
     * It then draws the polygon, translated by the given parameters.
     *
     * The polygon will be scaled first, then rotated, and finally offset
     * by the given position. Rotation is measured in radians and is counter
     * clockwise from the x-axis.  Rotation will be about the provided origin,
     * which is specified in world coordinates (not relative to the rectangle)
     * a rectangle should be centered on the origin to rotate properly.
     *
     * The polygon tesselation will be determined by the indices in poly. If
     * the polygon has not been triangulated (by one of the triangulation
     * factories {@link SimpleTriangulator} or {@link DelaunayTriangulator},
     * it may not draw properly.
     *
     * The vertex coordinates will be determined by polygon vertex position.
     * A horizontal position x has texture coordinate x/texture.width. A
     * vertical coordinate has texture coordinate 1-y/texture.height. As a
     * result, a rectangular polygon that has the same dimensions as the texture
     * is the same as simply drawing the texture.
     *
     * One way to think of the polygon is as a "cookie cutter".  Treat the
     * polygon coordinates as pixel coordinates in the texture filed, and use
     * that to determine how the texture fills the polygon. This may make the
     * polygon larger than you like in order to get the appropriate texturing.
     * You should use one of the transform methods to fix this.
     *
     * @param texture   The new active texture
     * @param poly      The polygon to texture
     * @param origin    The rotation origin
     * @param scale     The amount to scale the polygon
     * @param angle     The amount to rotate the polygon
     * @param offset    The polygon offset
     */
    void draw(const std::shared_ptr<Texture>& texture, const Poly2& poly,
              const Vec2& origin, const Vec2& scale, float angle, const Vec2& offset) {
        setTexture(texture); setColor(Color4::WHITE);
        fill(poly, origin, scale, angle, offset);
    }

    /**
     * Draws the tinted, textured polygon transformed by the given parameters
     *
     * This is a convenience method that calls the appropriate fill method.
     * It sets both the texture and color (removing the previous active values).
     * It then draws the polygon, translated by the given parameters.
     *
     * The polygon will be scaled first, then rotated, and finally offset
     * by the given position. Rotation is measured in radians and is counter
     * clockwise from the x-axis.  Rotation will be about the provided origin,
     * which is specified in world coordinates (not relative to the rectangle)
     * a rectangle should be centered on the origin to rotate properly.
     *
     * The polygon tesselation will be determined by the indices in poly. If
     * the polygon has not been triangulated (by one of the triangulation
     * factories {@link SimpleTriangulator} or {@link DelaunayTriangulator},
     * it may not draw properly.
     *
     * The vertex coordinates will be determined by polygon vertex position.
     * A horizontal position x has texture coordinate x/texture.width. A
     * vertical coordinate has texture coordinate 1-y/texture.height. As a
     * result, a rectangular polygon that has the same dimensions as the texture
     * is the same as simply drawing the texture.
     *
     * One way to think of the polygon is as a "cookie cutter".  Treat the
     * polygon coordinates as pixel coordinates in the texture filed, and use
     * that to determine how the texture fills the polygon. This may make the
     * polygon larger than you like in order to get the appropriate texturing.
     * You should use one of the transform methods to fix this.
     *
     * @param texture   The new active texture
     * @param color     The new active color
     * @param poly      The polygon to texture
     * @param origin    The rotation origin
     * @param scale     The amount to scale the polygon
     * @param angle     The amount to rotate the polygon
     * @param offset    The polygon offset
     */
    void draw(const std::shared_ptr<Texture>& texture, Color4 color, const Poly2& poly,
              const Vec2& origin, const Vec2& scale, float angle, const Vec2& offset) {
        setTexture(texture); setColor(color);
        fill(poly, origin, scale, angle, offset);
    }

    /**
     * Draws the textured polygon (without tint) transformed by the given matrix
     *
     * This is a convenience method that calls the appropriate fill method.
     * It sets both the texture and color (removing the previous active values).
     * It then draws the polygon, translated by the given matrix.
     *
     * The transform will be applied assuming the given origin, which is
     * specified in world world coordinates (but which should ideally be inside
     * the polygon bounds).
     *
     * The polygon tesselation will be determined by the indices in poly. If
     * the polygon has not been triangulated (by one of the triangulation
     * factories {@link SimpleTriangulator} or {@link DelaunayTriangulator},
     * it may not draw properly.
     *
     * The vertex coordinates will be determined by polygon vertex position.
     * A horizontal position x has texture coordinate x/texture.width. A
     * vertical coordinate has texture coordinate 1-y/texture.height. As a
     * result, a rectangular polygon that has the same dimensions as the texture
     * is the same as simply drawing the texture.
     *
     * One way to think of the polygon is as a "cookie cutter".  Treat the
     * polygon coordinates as pixel coordinates in the texture filed, and use
     * that to determine how the texture fills the polygon. This may make the
     * polygon larger than you like in order to get the appropriate texturing.
     * You should use one of the transform methods to fix this.
     *
     * @param texture   The new active texture
     * @param poly      The polygon to texture
     * @param origin    The coordinate origin
     * @param transform The coordinate transform
     */
    void draw(const std::shared_ptr<Texture>& texture, const Poly2& poly,
              const Vec2& origin, const Mat4& transform) {
        setTexture(texture); setColor(Color4::WHITE);
        fill(poly, origin, transform);
    }
    
    /**
     * Draws the tinted, textured polygon transformed by the given matrix
     *
     * This is a convenience method that calls the appropriate fill method.
     * It sets both the texture and color (removing the previous active values).
     * It then draws the polygon, translated by the given matrix.
     *
     * The transform will be applied assuming the given origin, which is
     * specified in world world coordinates (but which should ideally be inside
     * the polygon bounds).
     *
     * The polygon tesselation will be determined by the indices in poly. If
     * the polygon has not been triangulated (by one of the triangulation
     * factories {@link SimpleTriangulator} or {@link DelaunayTriangulator},
     * it may not draw properly.
     *
     * The vertex coordinates will be determined by polygon vertex position.
     * A horizontal position x has texture coordinate x/texture.width. A
     * vertical coordinate has texture coordinate 1-y/texture.height. As a
     * result, a rectangular polygon that has the same dimensions as the texture
     * is the same as simply drawing the texture.
     *
     * One way to think of the polygon is as a "cookie cutter".  Treat the
     * polygon coordinates as pixel coordinates in the texture filed, and use
     * that to determine how the texture fills the polygon. This may make the
     * polygon larger than you like in order to get the appropriate texturing.
     * You should use one of the transform methods to fix this.
     *
     * @param texture   The new active texture
     * @param color     The new active color
     * @param poly      The polygon to texture
     * @param origin    The coordinate origin
     * @param transform The coordinate transform
    */
    void draw(const std::shared_ptr<Texture>& texture, Color4 color, const Poly2& poly,
              const Vec2& origin, const Mat4& transform) {
        setTexture(texture); setColor(color);
        fill(poly, origin, transform);
    }

    /**
     * Draws the textured polygon (without tint) transformed by the given affine matrix
     *
     * This is a convenience method that calls the appropriate fill method.
     * It sets both the texture and color (removing the previous active values).
     * It then draws the polygon, translated by the given affine matrix.
     *
     * The transform will be applied assuming the given origin, which is
     * specified in world world coordinates (but which should ideally be inside
     * the polygon bounds).
     *
     * The polygon tesselation will be determined by the indices in poly. If
     * the polygon has not been triangulated (by one of the triangulation
     * factories {@link SimpleTriangulator} or {@link DelaunayTriangulator},
     * it may not draw properly.
     *
     * The vertex coordinates will be determined by polygon vertex position.
     * A horizontal position x has texture coordinate x/texture.width. A
     * vertical coordinate has texture coordinate 1-y/texture.height. As a
     * result, a rectangular polygon that has the same dimensions as the texture
     * is the same as simply drawing the texture.
     *
     * One way to think of the polygon is as a "cookie cutter".  Treat the
     * polygon coordinates as pixel coordinates in the texture filed, and use
     * that to determine how the texture fills the polygon. This may make the
     * polygon larger than you like in order to get the appropriate texturing.
     * You should use one of the transform methods to fix this.
     *
     * @param texture   The new active texture
     * @param poly      The polygon to texture
     * @param origin    The coordinate origin
     * @param transform The coordinate transform
     */
    void draw(const std::shared_ptr<Texture>& texture, const Poly2& poly,
              const Vec2& origin, const Affine2& transform) {
        setTexture(texture); setColor(Color4::WHITE);
        fill(poly, origin, transform);
    }
    
    /**
     * Draws the tinted, textured polygon transformed by the given affine matrix
     *
     * This is a convenience method that calls the appropriate fill method.
     * It sets both the texture and color (removing the previous active values).
     * It then draws the polygon, translated by the given affine matrix.
     *
     * The transform will be applied assuming the given origin, which is
     * specified in world world coordinates (but which should ideally be inside
     * the polygon bounds).
     *
     * The polygon tesselation will be determined by the indices in poly. If
     * the polygon has not been triangulated (by one of the triangulation
     * factories {@link SimpleTriangulator} or {@link DelaunayTriangulator},
     * it may not draw properly.
     *
     * The vertex coordinates will be determined by polygon vertex position.
     * A horizontal position x has texture coordinate x/texture.width. A
     * vertical coordinate has texture coordinate 1-y/texture.height. As a
     * result, a rectangular polygon that has the same dimensions as the texture
     * is the same as simply drawing the texture.
     *
     * One way to think of the polygon is as a "cookie cutter".  Treat the
     * polygon coordinates as pixel coordinates in the texture filed, and use
     * that to determine how the texture fills the polygon. This may make the
     * polygon larger than you like in order to get the appropriate texturing.
     * You should use one of the transform methods to fix this.
     *
     * @param texture   The new active texture
     * @param color     The new active color
     * @param poly      The polygon to texture
     * @param origin    The coordinate origin
     * @param transform The coordinate transform
     */
    void draw(const std::shared_ptr<Texture>& texture, Color4 color, const Poly2& poly,
              const Vec2& origin, const Affine2& transform) {
        setTexture(texture); setColor(color);
        fill(poly, origin, transform);
    }

#pragma mark -
#pragma mark Internal Helpers
private:
    /**
     * Sets the current drawing command.
     *
     * The value must be one of GL_TRIANGLES or GL_LINES.  Changing this value
     * during a drawing pass will flush the buffer.
     *
     * @param command   The new drawing command
     */
    void setCommand(GLenum command);
    
    /**
     * Returns the current drawing command.
     *
     * The value must be one of GL_TRIANGLES or GL_LINES.  Changing this value
     * during a drawing pass will flush the buffer.
     *
     * @return the current drawing command.
     */
    GLenum getCommand() const { return _command; }
    
    /**
     * Returns true if the vertex buffer was successfully allocated.
     *
     * If compilation fails, it will display error messages on the log.
     *
     * @param buffer    The buffer to test
     * @param message   The message to display on failure
     *
     * @return true if the vertex buffer was successfully allocated.
     */
    bool validateBuffer(GLuint buffer, const char* message);
    
    /**
     * Returns the number of vertices added to the drawing buffer.
     *
     * This method adds the given rectangle to the drawing buffer, but does not 
     * draw it.  You must call flush() to draw the rectangle.
     *
     * @param rect  The rectangle to add to the buffer
     * @param solid Whether the rectangle is to be filled
     *
     * @return the number of vertices added to the drawing buffer.
     */
    unsigned int prepare(const RectCugl& rect,  bool solid);

    /**
     * Returns the number of vertices added to the drawing buffer.
     *
     * This method adds the given polygon to the drawing buffer, but does not
     * draw it.  You must call flush() to draw the polygon.
     *
     * @param poly  The polygon to add to the buffer
     * @param solid Whether the polygon is to be filled
     *
     * @return the number of vertices added to the drawing buffer.
     */
    unsigned int prepare(const Poly2& poly, bool solid);

    /**
     * Returns the number of vertices added to the drawing buffer.
     *
     * This method adds the given vertices and indices to the drawing buffer, 
     * but does not draw them.  You must call flush() to draw the mesh.
     *
     * @param vertices  The vertices to add to the buffer
     * @param vsize     The number of vertices to add
     * @param voffset   The position of the first vertex to add
     * @param indices   The indices to add to the buffer
     * @param isize     The number of indices to add
     * @param ioffset   The position of the first index to add
     * @param solid     Whether the vertex mesh is to be filled
     * @param tint      Whether to tint with the active color
     *
     * @return the number of vertices added to the drawing buffer.
     */
    unsigned int prepare(const Vertex2* vertices, unsigned int vsize, unsigned int voffset,
                         const unsigned short* indices, unsigned int isize, unsigned int ioffset,
                         bool solid, bool tint = true);

};

}

#endif /* __CU_SPRITE_BATCH_H__ */

//
//  CUSpriteBatch.cpp
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
#include <cugl/renderer/CUSpriteBatch.h>
#include <cugl/renderer/CUSpriteShader.h>
#include <cugl/renderer/CUTexture.h>
#include <cugl/math/CUAffine2.h>
#include <cugl/math/CUPoly2.h>
#include <cugl/util/CUDebug.h>
#include <SDL/SDL_image.h>

using namespace cugl;


/**
 * This array is the data of a white image with 2 by 2 dimension.
 * It's used for creating a default texture when the texture is a nullptr.
 */
static unsigned char cu_2x2_white_image[] = {
    // RGBA8888
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF
};

/** The blank texture corresponding to cu_2x2_white_image */
std::shared_ptr<Texture> SpriteBatch::_blank;

#pragma mark Constructors
/**
 * Creates a degenerate sprite batch with no buffers.
 *
 * You must initialize the buffer before using it.
 */
SpriteBatch::SpriteBatch() :
_capacity(0),
_vertData(nullptr),
_indxData(nullptr),
_vertArray(0),
_vertBuffer(0),
_indxBuffer(0),
_vertMax(0),
_vertSize(0),
_indxMax(0),
_indxSize(0),
_color(Color4::WHITE),
_perspective(Mat4::IDENTITY),
_command(GL_TRIANGLES),
_blendEquation(GL_FUNC_ADD),
_srcFactor(GL_SRC_ALPHA),
_dstFactor(GL_ONE_MINUS_SRC_ALPHA),
_texture(nullptr),
_vertTotal(0),
_callTotal(0),
_initialized(false),
_active(false) {
}

/**
 * Deletes the vertex buffers and resets all attributes.
 *
 * You must reinitialize the sprite batch to use it.
 */
void SpriteBatch::dispose() {
    if (_vertData) { delete[] _vertData; _vertData = nullptr; }
    if (_indxData) { delete[] _indxData; _indxData = nullptr; }
    if (_vertArray) { glDeleteVertexArrays(1,&_vertArray); _vertArray = 0; }
    if (_indxBuffer) { glDeleteBuffers(1,&_indxBuffer); _indxBuffer = 0; }
    if (_vertBuffer) { glDeleteBuffers(1,&_vertBuffer); _vertBuffer = 0; }
    if (_shader != nullptr) { _shader = nullptr; }
    if (_texture != nullptr) { _texture = nullptr; }
    
    _capacity = 0;
    _vertMax  = 0;
    _vertSize = 0;
    _indxMax  = 0;
    _indxSize = 0;
    _color = Color4::WHITE;
    _perspective = Mat4::IDENTITY;
    _command = GL_TRIANGLES;
    _blendEquation = GL_FUNC_ADD;
    _srcFactor = GL_SRC_ALPHA;
    _dstFactor = GL_ONE_MINUS_SRC_ALPHA;
    
    _vertTotal = 0;
    _callTotal = 0;

    _initialized = false;
    _active = false;
}

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
bool SpriteBatch::init() {
    return init(DEFAULT_CAPACITY,SpriteShader::alloc());
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
bool SpriteBatch::init(unsigned int capacity) {
    return init(capacity,SpriteShader::alloc());
}

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
bool SpriteBatch::init(unsigned int capacity, std::shared_ptr<SpriteShader> shader) {
    if (_initialized) {
        CUAssertLog(false, "SpriteBatch is already initialized");
        return false; // If asserts are turned off.
    }
    
    // Set up a default shader
    _shader = shader;
    if (_shader == nullptr) {
        return false;
    }

    _capacity = capacity;
    
    // Set up data arrays;
    _vertMax = _capacity;
    _vertData = new Vertex2[_vertMax];
    _indxMax = _capacity*3;
    _indxData = new GLuint[_indxMax];
    
    // Generate the buffers
    glGenBuffers(1, &_vertBuffer);
    if (!validateBuffer(_vertBuffer, "Unable to unable to generate Vertex Buffer Object")) {
        dispose();
        return false;
    }
    
    glGenVertexArrays (1, &_vertArray);
    if (!validateBuffer(_vertArray, "Unable to unable to generate Vertex Array Object")) {
        dispose();
        return false;
    }
    
    glGenBuffers(1, &_indxBuffer);
    if (!validateBuffer(_indxBuffer, "Unable to unable to generate Index Buffer Object")) {
        dispose();
        return false;
    }
    
    // Bind and link the buffers
    glBindBuffer( GL_ARRAY_BUFFER, _vertBuffer );
    glBindVertexArray(_vertArray);
    glBufferData( GL_ARRAY_BUFFER, _vertSize * sizeof(Vertex2), _vertData, GL_DYNAMIC_DRAW );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, _indxBuffer );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, _indxSize * sizeof(GLuint), _indxData, GL_DYNAMIC_DRAW );
    _texture = SpriteBatch::getBlankTexture();
    return true;
}

#pragma mark -
#pragma mark Attributes

/**
 * Sets the shader for this sprite batch
 *
 * This value may NOT be changed during a drawing pass.
 *
 * @param color The active color for this sprite batch
 */
void SpriteBatch::setShader(const std::shared_ptr<SpriteShader>& shader) {
    CUAssertLog(_active, "Attempt to reassign shader while drawing is active");
    _shader = shader;
}

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
 * @param color The active texture for this sprite batch
 */
void SpriteBatch::setTexture(const std::shared_ptr<Texture>& texture) {
    if (texture == nullptr) {
        if (_texture != nullptr && _texture->getBuffer() != getBlankTexture()->getBuffer()) {
            if (_active) { flush(); }
            _shader->setTexture(getBlankTexture());
            _texture = getBlankTexture();
        }
    } else if (_texture->getBuffer() != texture->getBuffer()) {  // Both must be not nullptr
        if (_active) { flush(); }
        _shader->setTexture(texture);
        _texture = texture;
    }
}

/**
 * Returns the blank texture used to make solid shapes
 *
 * This is the texture used when the active texture is nullptr.  Using this
 * texture means that all shapes and outlines will be drawn with a solid
 * color instead.
 *
 * @return the blank texture used to make solid shapes
 */
const std::shared_ptr<Texture>& SpriteBatch::getBlankTexture() {
    if (_blank == nullptr) {
        _blank = Texture::allocWithData(cu_2x2_white_image, 2, 2);
        _blank->bind();
        _blank->setWrapS(GL_REPEAT);
        _blank->setWrapT(GL_REPEAT);
        _blank->unbind();
    }
    return _blank;
}

/**
 * Sets the active perspective matrix of this sprite batch
 *
 * The perspective matrix is the combined modelview-projection from the
 * camera. By default, this is the identity matrix. Changing this value
 * will cause the sprite batch to flush.
 *
 * @param perspective   The active perspective matrix for this sprite batch
 */
void SpriteBatch::setPerspective(const Mat4& perspective) {
    if (_active && _perspective != perspective) {
        flush();
        _shader->setPerspective(perspective);
    }
    _perspective = perspective;
}

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
void SpriteBatch::setBlendFunc(GLenum srcFactor, GLenum dstFactor) {
    if (_active && (_srcFactor != srcFactor || _dstFactor != dstFactor)) {
        flush();
        glBlendFunc(srcFactor, dstFactor);
    }
    
    _srcFactor = srcFactor;
    _dstFactor = dstFactor;
}

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
void SpriteBatch::setBlendEquation(GLenum equation) {
    if (_active && _blendEquation != equation) {
        flush();
        glBlendEquation(equation);
    }
    
    _blendEquation = equation;
}

/**
 * Returns the current drawing command.
 *
 * The value must be one of GL_TRIANGLES or GL_LINES.  Changing this value
 * during a drawing pass will flush the buffer.
 *
 * @return the current drawing command.
 */
void SpriteBatch::setCommand(GLenum command) {
    if (_active && command != _command) {
        flush();
    }
    _command = command;
}



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
void SpriteBatch::begin() {
    glDisable(GL_CULL_FACE);
    glDepthMask(false);
    glEnable(GL_BLEND);
    glBlendEquation(_blendEquation);
    glBlendFunc(_srcFactor, _dstFactor);
    
    // DO NOT CLEAR.  This responsibility lies elsewhere
    
    _shader->bind();
    _shader->setPerspective(_perspective);
    _shader->setTexture(_texture);
    _shader->attach(_vertArray, _vertBuffer);
    _active = true;
}

/**
 * Completes the drawing pass for this sprite batch, flushing the buffer.
 *
 * This method enables depth writes and disables blending and texturing. It
 * Must always be called after a call to {@link #begin()}.
 */
void SpriteBatch::end() {
    flush();
    _shader->unbind();
    _active = false;

}


/**
 * Flushes the current mesh without completing the drawing pass.
 *
 * This method is called whenever you change any attribute other than color
 * mid-pass. It prevents the attribute change from retoactively affecting
 * previuosly drawn shapes.
 */
void SpriteBatch::flush() {
    if (_indxSize == 0 || _vertSize == 0) {
        _vertSize = _indxSize = 0;
        return;
    }
    
    glBindVertexArray (_vertArray);
    glBindBuffer( GL_ARRAY_BUFFER, _vertBuffer );
    glBufferData( GL_ARRAY_BUFFER, _vertSize * sizeof(Vertex2), _vertData, GL_DYNAMIC_DRAW );
    
    // Set index data and render
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, _indxBuffer );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, _indxSize * sizeof(GLuint), _indxData, GL_DYNAMIC_DRAW );
    glDrawElements(_command, _indxSize, GL_UNSIGNED_INT, NULL );
    
    // Increment the counters
    _vertTotal += _indxSize;
    _callTotal++;
    
    _vertSize = _indxSize = 0;
}

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
void SpriteBatch::fill(const Rect& rect) {
    setCommand(GL_TRIANGLES);
    prepare(rect,true);
}

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
void SpriteBatch::fill(const Rect& rect,  const Vec2& origin, const Vec2& scale,
                       float angle, const Vec2& offset) {
    setCommand(GL_TRIANGLES);
    unsigned int count = prepare(rect,true);
    
    Mat4 transform;
    Mat4::createTranslation(-origin.x,-origin.y,0,&transform);
    transform.scale(scale);
    transform.rotateZ(angle);
    transform.translate((Vec3)(origin+offset));
    
    for(int ii = 1; ii <= count; ii++) {
        _vertData[_vertSize-ii].position *= transform;
    }
}

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
 * region [minS,maxS]x[min,maxT]. Alternatively, you can use a {@limk Poly2}
 * for more fine-tuned control.
 *
 * @param rect      The rectangle to draw
 * @param origin    The coordinate origin
 * @param transform The coordinate transform
 */
void SpriteBatch::fill(const Rect& rect, const Vec2& origin, const Mat4& transform) {
    setCommand(GL_TRIANGLES);
    unsigned int count = prepare(rect,true);

    Mat4 matrix;
    Mat4::createTranslation(-origin.x,-origin.y,0,&matrix);
    matrix *= transform;
    matrix.translate(origin.x,origin.y,0);

    for(int ii = 1; ii <= count; ii++) {
        _vertData[_vertSize-ii].position *= matrix;
    }
}

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
 * region [minS,maxS]x[min,maxT]. Alternatively, you can use a {@limk Poly2}
 * for more fine-tuned control.
 *
 * @param rect      The rectangle to draw
 * @param origin    The coordinate origin
 * @param transform The coordinate transform
 */
void SpriteBatch::fill(const Rect& rect, const Vec2& origin, const Affine2& transform) {
    setCommand(GL_TRIANGLES);
    unsigned int count = prepare(rect,true);
    
    Affine2 matrix;
    Affine2::createTranslation(-origin.x,0,&matrix);
    matrix *= transform;
    matrix.translate(origin);
    
    for(int ii = 1; ii <= count; ii++) {
        _vertData[_vertSize-ii].position *= matrix;
    }
}

/**
 * Draws the given polygon filled with the current color and texture.
 *
 * The polygon tesselation will be determined by the indices in poly. If
 * the polygon has not been triangulated (by one of the triangulation
 * factories {@link SimpleTriangulator} or {@link ComplexTriangulator},
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
void SpriteBatch::fill(const Poly2& poly) {
    setCommand(GL_TRIANGLES);
    prepare(poly,true);
}

/**
 * Draws the given polygon filled with the current color and texture.
 *
 * The polygon will be offset by the given position.
 *
 * The polygon tesselation will be determined by the indices in poly. If
 * the polygon has not been triangulated (by one of the triangulation
 * factories {@link SimpleTriangulator} or {@link ComplexTriangulator},
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
void SpriteBatch::fill(const Poly2& poly, const Vec2& offset) {
    setCommand(GL_TRIANGLES);
    unsigned int count = prepare(poly,true);
    for(int ii = 1; ii <= count; ii++) {
        _vertData[_vertSize-ii].position += offset;
    }
}

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
 * factories {@link SimpleTriangulator} or {@link ComplexTriangulator},
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
void SpriteBatch::fill(const Poly2& poly, const Vec2& origin, const Vec2& scale,
                       float angle, const Vec2& offset) {
    setCommand(GL_TRIANGLES);
    unsigned int count = prepare(poly,true);

    Mat4 transform;
    Mat4::createTranslation(-origin.x,-origin.y,0,&transform);
    transform.scale(scale);
    transform.rotateZ(angle);
    transform.translate((Vec3)(origin+offset));
    
    for(int ii = 1; ii <= count; ii++) {
        _vertData[_vertSize-ii].position *= transform;
    }
}

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
 * factories {@link SimpleTriangulator} or {@link ComplexTriangulator},
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
void SpriteBatch::fill(const Poly2& poly, const Vec2& origin, const Mat4& transform) {
    setCommand(GL_TRIANGLES);
    unsigned int count = prepare(poly,true);
    
    Mat4 matrix;
    Mat4::createTranslation(-origin.x,-origin.y,0,&matrix);
    matrix *= transform;
    matrix.translate(origin.x,origin.y,0);

    for(int ii = 1; ii <= count; ii++) {
        _vertData[_vertSize-ii].position *= matrix;
    }
}

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
 * factories {@link SimpleTriangulator} or {@link ComplexTriangulator},
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
void SpriteBatch::fill(const Poly2& poly, const Vec2& origin, const Affine2& transform) {
    setCommand(GL_TRIANGLES);
    unsigned int count = prepare(poly,true);
    
    Affine2 matrix;
    Affine2::createTranslation(-origin.x,0,&matrix);
    matrix *= transform;
    matrix.translate(origin);

    for(int ii = 1; ii <= count; ii++) {
        _vertData[_vertSize-ii].position *= matrix;
    }
}

/**
 * Fills the triangulated vertices with the current texture.
 *
 * This method provides more fine tuned control over texture coordinates
 * that the other fill methods.  The texture no longer needs to be
 * drawn uniformly over the shape.
 *
 * The triangulation will be determined by the given indices. If necessary,
 * these can be generated via one of the triangulation factories
 * {@link SimpleTriangulator} or {@link ComplexTriangulator}.
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
void SpriteBatch::fill(const Vertex2* vertices, unsigned int vsize, unsigned int voffset,
                       const unsigned short* indices, unsigned int isize, unsigned int ioffset,
                       const Mat4& transform, bool tint) {
    setCommand(GL_TRIANGLES);
    unsigned int count = prepare(vertices,vsize,voffset,indices,isize,ioffset,true,tint);
    
    for(int ii = 1; ii <= count; ii++) {
        _vertData[_vertSize-ii].position *= transform;
    }
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
 * {@link SimpleTriangulator} or {@link ComplexTriangulator}.
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
void SpriteBatch::fill(const Vertex2* vertices, unsigned int vsize, unsigned int voffset,
                       const unsigned short* indices, unsigned int isize, unsigned int ioffset,
                       const Affine2& transform, bool tint) {
    setCommand(GL_TRIANGLES);
    unsigned int count = prepare(vertices,vsize,voffset,indices,isize,ioffset,true,tint);
    
    for(int ii = 1; ii <= count; ii++) {
        _vertData[_vertSize-ii].position *= transform;
    }
}

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
void SpriteBatch::outline(const Rect& rect) {
    setCommand(GL_LINES);
    prepare(rect,false);
    
}

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
void SpriteBatch::outline(const Rect& rect, const Vec2& origin, const Vec2& scale,
                          float angle, const Vec2& offset) {
    setCommand(GL_LINES);
    unsigned int count = prepare(rect,false);
    
    Mat4 transform;
    Mat4::createTranslation(-origin.x,-origin.y,0,&transform);
    transform.scale(scale);
    transform.rotateZ(angle);
    transform.translate((Vec3)(origin+offset));
    
    for(int ii = 1; ii <= count; ii++) {
        _vertData[_vertSize-ii].position *= transform;
    }
}

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
void SpriteBatch::outline(const Rect& rect, const Vec2& origin, const Mat4& transform) {
    setCommand(GL_LINES);
    unsigned int count = prepare(rect,false);
    
    Mat4 matrix;
    Mat4::createTranslation(-origin.x,-origin.y,0,&matrix);
    matrix *= transform;
    matrix.translate(origin.x,origin.y,0);
    
    for(int ii = 1; ii <= count; ii++) {
        _vertData[_vertSize-ii].position *= matrix;
    }
}

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
void SpriteBatch::outline(const Rect& rect, const Vec2& origin, const Affine2& transform) {
    setCommand(GL_LINES);
    unsigned int count = prepare(rect,false);
    
    Affine2 matrix;
    Affine2::createTranslation(-origin.x,-origin.y,&matrix);
    matrix *= transform;
    matrix.translate(origin.x,origin.y);

    for(int ii = 1; ii <= count; ii++) {
        _vertData[_vertSize-ii].position *= matrix;
    }
}

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
void SpriteBatch::outline(const Poly2& poly) {
    setCommand(GL_LINES);
    prepare(poly,false);
}

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
void SpriteBatch::outline(const Poly2& poly, const Vec2& offset) {
    setCommand(GL_LINES);
    unsigned int count = prepare(poly,false);
    for(int ii = 1; ii <= count; ii++) {
        _vertData[_vertSize-ii].position += offset;
    }
}

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
void SpriteBatch::outline(const Poly2& poly, const Vec2& origin, const Vec2& scale,
                          float angle, const Vec2& offset) {
    setCommand(GL_LINES);
    unsigned int count = prepare(poly,false);
    
    Mat4 transform;
    Mat4::createTranslation(-origin.x,-origin.y,0,&transform);
    transform.scale(scale);
    transform.rotateZ(angle);
    transform.translate((Vec3)(origin+offset));
    
    for(int ii = 1; ii <= count; ii++) {
        _vertData[_vertSize-ii].position *= transform;
    }

}

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
void SpriteBatch::outline(const Poly2& poly, const Vec2& origin, const Mat4& transform) {
    setCommand(GL_LINES);
    unsigned int count = prepare(poly,false);
    
    Mat4 matrix;
    Mat4::createTranslation(-origin.x,-origin.y,0,&matrix);
    matrix *= transform;
    matrix.translate(origin.x,origin.y,0);

    for(int ii = 1; ii <= count; ii++) {
        _vertData[_vertSize-ii].position *= matrix;
    }
}

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
void SpriteBatch::outline(const Poly2& poly, const Vec2& origin, const Affine2& transform) {
    setCommand(GL_LINES);
    unsigned int count = prepare(poly,false);
    
    Affine2 matrix;
    Affine2::createTranslation(-origin.x,-origin.y,&matrix);
    matrix *= transform;
    matrix.translate(origin.x,origin.y);

    for(int ii = 1; ii <= count; ii++) {
        _vertData[_vertSize-ii].position *= matrix;
    }
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
void SpriteBatch::outline(const Vertex2* vertices, unsigned int vsize, unsigned int voffset,
                          const unsigned short* indices, unsigned int isize, unsigned int ioffset,
                          const Mat4& transform, bool tint) {
    setCommand(GL_LINES);
    unsigned int count = prepare(vertices,vsize,voffset,indices,isize,ioffset,false,tint);
    
    for(int ii = 1; ii <= count; ii++) {
        _vertData[_vertSize-ii].position *= transform;
    }
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
void SpriteBatch::outline(const Vertex2* vertices, unsigned int vsize, unsigned int voffset,
                          const unsigned short* indices, unsigned int isize, unsigned int ioffset,
                          const Affine2& transform, bool tint) {
    setCommand(GL_LINES);
    unsigned int count = prepare(vertices,vsize,voffset,indices,isize,ioffset,false,tint);
    
    for(int ii = 1; ii <= count; ii++) {
        _vertData[_vertSize-ii].position *= transform;
    }
}

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
void SpriteBatch::draw(const std::shared_ptr<Texture>& texture, const Vec2& position) {
    setTexture(texture); setColor(Color4::WHITE);
    fill(Rect(position.x,position.y, (float)texture->getWidth(), (float)texture->getHeight()));
}

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
void SpriteBatch::draw(const std::shared_ptr<Texture>& texture, Color4 color, const Vec2& position) {
    setTexture(texture); setColor(color);
    fill(Rect(position.x,position.y, (float)texture->getWidth(), (float)texture->getHeight()));
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
void SpriteBatch::draw(const std::shared_ptr<Texture>& texture,
                       const Vec2& origin, const Vec2& scale, float angle, const Vec2& offset) {
    setTexture(texture); setColor(Color4::WHITE);
    fill(Rect(0,0, (float)texture->getWidth(), (float)texture->getHeight()), origin, scale, angle, offset);
}

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
void SpriteBatch::draw(const std::shared_ptr<Texture>& texture, Color4 color,
                       const Vec2& origin, const Vec2& scale, float angle, const Vec2& offset) {
    setTexture(texture); setColor(color);
    fill(Rect(0,0, (float)texture->getWidth(), (float)texture->getHeight()), origin, scale, angle, offset);
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
void SpriteBatch::draw(const std::shared_ptr<Texture>& texture, const Vec2& origin, const Mat4& transform) {
    setTexture(texture); setColor(Color4::WHITE);
    fill(Rect(0,0, (float)texture->getWidth(), (float)texture->getHeight()), origin, transform);
}

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
void SpriteBatch::draw(const std::shared_ptr<Texture>& texture, Color4 color,
                       const Vec2& origin, const Mat4& transform)  {
    setTexture(texture); setColor(color);
    fill(Rect(0,0, (float)texture->getWidth(), (float)texture->getHeight()), origin, transform);
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
void SpriteBatch::draw(const std::shared_ptr<Texture>& texture, const Vec2& origin, const Affine2& transform) {
    setTexture(texture); setColor(Color4::WHITE);
    fill(Rect(0,0, (float)texture->getWidth(), (float)texture->getHeight()), origin, transform);
}

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
void SpriteBatch::draw(const std::shared_ptr<Texture>& texture, Color4 color,
                       const Vec2& origin, const Affine2& transform)  {
    setTexture(texture); setColor(color);
    fill(Rect(0,0, (float)texture->getWidth(), (float)texture->getHeight()), origin, transform);
}

#pragma mark -
#pragma mark Internal Helpers
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
bool SpriteBatch::validateBuffer(GLuint buffer, const char* message) {
    if (!buffer) {
        CULogGLError();
        return false;
    }
    return true;
}

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
unsigned int SpriteBatch::prepare(const Rect& rect, bool solid) {
    if (_vertSize+4 > _vertMax ||  _indxSize+8 > _indxMax) {
        flush();
    }
    
    Poly2 poly(rect, solid);
    unsigned int vstart = _vertSize;
    int ii = 0;
    for(auto it = poly.getVertices().begin(); it != poly.getVertices().end(); ++it) {
        Vec2 point = (*it);
        _vertData[vstart+ii].position = point;
        _vertData[vstart+ii].color = _color;
        
        float value = (point.x-rect.origin.x)/rect.size.width;
        _vertData[vstart+ii].texcoord.x = value*_texture->getMaxS()+(1-value)*_texture->getMinS();
        value = 1-(point.y-rect.origin.y)/rect.size.height;
        _vertData[vstart+ii].texcoord.y = value*_texture->getMaxT()+(1-value)*_texture->getMinT();
        ii++;
    }
    
    int jj = 0;
    unsigned int istart = _indxSize;
    for(auto it = poly.getIndices().begin(); it != poly.getIndices().end(); ++it) {
        _indxData[istart+jj] = vstart+(*it);
        jj++;
    }
    
    _vertSize += ii;
    _indxSize += jj;
    return ii;
}

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
unsigned int SpriteBatch::prepare(const Poly2& poly, bool solid) {
    CUAssertLog((solid ? poly.getIndices().size() % 3 : poly.getIndices().size() % 2) == 0,
                "Polynomial has the wrong number of indices: %d", (int)poly.getIndices().size());
    if (_vertSize+poly.getVertices().size() > _vertMax ||
        _indxSize+poly.getIndices().size()  > _indxMax) {
        flush();
    }
    
    unsigned int vstart = _vertSize;
    int ii = 0;
    for(auto it = poly.getVertices().begin(); it != poly.getVertices().end(); ++it) {
        Vec2 point = (*it);
        _vertData[vstart+ii].position = point;
        _vertData[vstart+ii].color = _color;
        
        float value = point.x/_texture->getWidth();
        _vertData[vstart+ii].texcoord.x = value*_texture->getMaxS()+(1-value)*_texture->getMinS();
        value = 1-point.y/_texture->getHeight();
        _vertData[vstart+ii].texcoord.y = value*_texture->getMaxT()+(1-value)*_texture->getMinT();
        ii++;
    }
    
    int jj = 0;
    unsigned int istart = _indxSize;
    for(auto it = poly.getIndices().begin(); it != poly.getIndices().end(); ++it) {
        _indxData[istart+jj] = vstart+(*it);
        jj++;
    }
    
    _vertSize += ii;
    _indxSize += jj;
    return ii;
}

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
unsigned int SpriteBatch::prepare(const Vertex2* vertices, unsigned int vsize, unsigned int voffset,
                                  const unsigned short* indices, unsigned int isize, unsigned int ioffset,
                                  bool solid, bool tint) {
    CUAssertLog((solid ? isize % 3 : isize % 2) == 0,
                "Vertex mesh has the wrong number of indices: %d", isize);
    if (_vertSize+vsize > _vertMax || _indxSize+isize  > _indxMax) {
        flush();
    }
    
    int ii = 0;
    unsigned int vstart = _vertSize;
    Vertex2 temp;
    for(int kk = voffset; ii < vsize; ii++) {
        _vertData[vstart+ii] = vertices[kk+ii];
        if (tint) {
            _vertData[vstart+ii].color *= _color;
        }
    }
    
    int jj = 0;
    unsigned int istart = _indxSize;
    for(int kk = ioffset; jj < isize; jj++) {
        _indxData[istart+jj] = vstart+indices[kk+jj];
    }
    
    _vertSize += ii;
    _indxSize += jj;
    return ii;
}





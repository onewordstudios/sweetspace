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

#include <algorithm>
#include <cugl/2d/CUPolygonNode.h>

using namespace cugl;


/**
 * Sets the texture polgon to the vertices expressed in image space.
 *
 * The polygon will be triangulated using the rules of SimpleTriangulator.
 * All PolygonNode objects share a single triangulator, so this method is
 * not thread safe.
 *
 * @param   vertices The vertices to texture
 * @param   offset   The offset in vertices
 * @param   size     The number of elements in vertices
 */
void PolygonNode::setPolygon(const std::vector<Vec2>& vertices) {
    _polygon.set(vertices);
    _polygon.getIndices().clear();
    _triangulator.set(vertices);
    _triangulator.calculate();
    _triangulator.getTriangulation(_polygon.getIndices());
    TexturedNode::setPolygon(_polygon);
}

/**
 * Sets the polygon to the given one in texture space.
 *
 * @param poly  The polygon to texture
 */
void PolygonNode::setPolygon(const Poly2& poly) {
    if (&_polygon != &poly) {
        CUAssertLog(poly.getType() == Poly2::Type::SOLID,
                    "The polygon is not solid");
        _polygon.set(poly);
    }
    
    setContentSize(_polygon.getBounds().size);
}

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
void PolygonNode::setPolygon(const Rect& rect) {
    _polygon.set(rect);
    setContentSize(_polygon.getBounds().size);
}

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
 * @param matrix    The global transformation matrix.
 * @param tint      The tint to blend with the Node color.
 */
void PolygonNode::draw(const std::shared_ptr<SpriteBatch>& batch, const Mat4& transform, Color4 tint) {
    if (!_rendered) {
        generateRenderData();
    }
    
    batch->setColor(tint);
    batch->setTexture(_texture);
    batch->setBlendEquation(_blendEquation);
    batch->setBlendFunc(_srcFactor, _dstFactor);
    batch->fill(_vertices.data(),(unsigned int)_vertices.size(),0,
                _polygon.getIndices().data(),(unsigned int)_polygon.getIndices().size(),0,
                transform);
}

/** A triangulator for those incomplete polygons */
SimpleTriangulator PolygonNode::_triangulator;


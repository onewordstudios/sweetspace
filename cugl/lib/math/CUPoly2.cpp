//
//  CUPoly2.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides a class that represents a simple polygon.  The purpose
//  of this class is to separate the geometry (and math) of a polygon from the
//  rendering data of a pipeline.  The class provides the index data necessary
//  for rendering. In addition, it has some primitive geometry methods such as
//  containment.
//
//  In OpenGL style, the object is defined by a set of vertices, and a set of
//  indices that define a mesh over the vertices.  This class is intentionally
//  (based on experience in previous semesters) lightweight.  There is no
//  verification that indices are properly defined.  It is up to the user to
//  verify and specify the components.  If you need help with triangulation
//  or path extrusion, use one the the related factory classes.
//
//  Because math objects are intended to be on the stack, we do not provide
//  any shared pointer support in this class.
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
//  Version: 6/20/16

#include <cugl/math/CUPoly2.h>
#include <cugl/util/CUDebug.h>
#include <cugl/util/CUStrings.h>
#include <algorithm>
#include <vector>
#include <stack>
#include <sstream>
#include <cmath>
#include <iterator>
#include <cugl/math/CUMat4.h>
#include <cugl/math/CUAffine2.h>

using namespace std;
using namespace cugl;

#pragma mark Convex Hull Algorithm

/**
 * Returns -1, 0, or 1 indicating the orientation of a -> b -> c
 *
 * If the function returns -1, this is a counter-clockwise turn.  If 1, it is
 * a clockwise turn.  If 0, it is colinear.
 *
 * @param a The first point
 * @param b The second point
 * @param c The third point
 *
 * @return -1, 0, or 1 indicating the orientation of a -> b -> c
 */
static int orientation(Vec2 a, Vec2 b, Vec2 c) {
    float val = (b.y - a.y) * (c.x - a.x) - (b.x - a.x) * (c.y - a.y);
        
    if (-CU_MATH_EPSILON < val && val < CU_MATH_EPSILON) return 0;  // colinear
    return (val > 0)? 1: -1; // clock or counterclock wise
}

/**
 * Returns true if the given point is incident to the given line segment.
 *
 * The variance specifies the tolerance that we allow for begin off the line
 * segment.
 *
 * @param point     The point to check
 * @param a         The start of the line segment
 * @param b         The end of the line segment
 * @param variance  The distance tolerance
 *
 * @return true if the given point is incident to the given line segment.
 */
static bool onsegment(const Vec2& point, const Vec2& a, const Vec2& b, float variance) {
    float d1 = point.distance(a);
    float d2 = point.distance(b);
    float d3 = a.distance(b);
    return fabsf(d3-d2-d1) <= variance;
}

/**
 * This class implements a pivot for the Graham Scan convex hull algorithm.
 *
 * This pivot allows us to have a relative comparison function to an
 * anchor point.
 */
class GSPivot {
public:
    /** The pivot anchor */
    Vec2* anchor;
    
    /** Constructs an empty pivot */
    GSPivot() : anchor(nullptr) {}
    
    /**
     * Returns true if a < b. in the polar order with respect to the pivot.
     *
     * The polar order is computed relative to the pivot anchor.
     *
     * @param a     The first point
     * @param b     The second point
     *
     * @return rue if a < b. in the polar order with respect to the pivot.
     */
    bool compare(const Vec2& a, const Vec2& b) {
        int order = orientation(*anchor, a, b);
        if (order == 0) {
            float d1 = anchor->distanceSquared(a);
            float d2 = anchor->distanceSquared(b);
            return (d1 < d2 || (d1 == d2 && a < b));
        }
        return (order == -1);
    }
};


#pragma mark -
#pragma mark Static Constructors

/**
 * Creates a polygon that represents a line segment from origin to dest.
 *
 * The polygon will have indices and the type will be PATH
 *
 * @param   origin  The line origin.
 * @param   dest    The line destination.
 *
 * @return A new polygon representing a line segment.
 */
Poly2 Poly2::createLine(const Vec2& origin, const Vec2& dest) {
    Poly2 result;
    result._vertices.push_back(origin);
    result._vertices.push_back(dest);
    
    result._indices.push_back(0);
    result._indices.push_back(1);
    
    result._type = Type::PATH;
    result.computeBounds();
    return result;
}

/**
 * Creates a polygon that represents a line segment from origin to dest.
 *
 * This alternate constructor does not allocate the Poly2 object.  Instead,
 * it sets the contents of Poly2 pointer.
 *
 * The polygon will have indices and the type will be PATH
 *
 * @param  origin   The line origin.
 * @param  dest     The line destination.
 * @param  dst      A polygon to store the result in.
 *
 * @return A reference to dst for chaining
 */
Poly2* Poly2::createLine(const Vec2& origin, const Vec2& dest, Poly2* dst) {
    CUAssertLog(dst, "Assignment polygon is null");
    dst->_vertices.clear();
    dst->_vertices.push_back(origin);
    dst->_vertices.push_back(dest);
    
    dst->_indices.clear();
    dst->_indices.push_back(0);
    dst->_indices.push_back(1);
    
    dst->_type = Type::PATH;
    dst->computeBounds();
    return dst;
}

/**
 * Creates a polygon that represents a simple triangle.
 *
 * The indices will be generated automatically. If the value solid is true,
 * then the type will be SOLID.  Otherwise, the type will be PATH.
 *
 * @param  a        The first vertex.
 * @param  b        The second vertex.
 * @param  c        The third vertex.
 * @param  solid    If true, treat this triangle as a SOLID.
 *
 * @return A new polygon representing a triangle.
 */
Poly2 Poly2::createTriangle(const Vec2& a, const Vec2& b, const Vec2& c, bool solid) {
    Poly2 result;
    result._vertices.push_back(a);
    result._vertices.push_back(b);
    result._vertices.push_back(c);
    
    if (solid) {
        result._indices.resize(3,0);
        result._indices[0] = 0;
        result._indices[1] = 1;
        result._indices[2] = 2;
        result._type = Type::SOLID;
    } else {
        result._indices.resize(6,0);
        result._indices[0] = 0;
        result._indices[1] = 1;
        result._indices[2] = 1;
        result._indices[3] = 2;
        result._indices[4] = 2;
        result._indices[5] = 0;
        result._type = Type::PATH;
    }

    result.computeBounds();
    return result;
}

/**
 * Creates a polygon that represents a simple triangle.
 *
 * This alternate constructor does not allocate the Poly2 object.  Instead,
 * it sets the contents of Poly2 pointer.
 *
 * The indices will be generated automatically. If the value solid is true,
 * then the type will be SOLID.  Otherwise, the type will be PATH.
 *
 * @param  a        The first vertex.
 * @param  b        The second vertex.
 * @param  c        The third vertex.
 * @param  dst      A polygon to store the result in.
 * @param  solid    If true, treat this triangle as a SOLID.
 *
 * @return A reference to dst for chaining
 */
Poly2* Poly2::createTriangle(const Vec2& a, const Vec2& b, const Vec2& c, Poly2* dst, bool solid) {
    CUAssertLog(dst, "Assignment polygon is null");
    dst->_vertices.clear();
    dst->_vertices.push_back(a);
    dst->_vertices.push_back(b);
    dst->_vertices.push_back(c);
    
    if (solid) {
        dst->_indices.resize(3,0);
        dst->_indices[0] = 0;
        dst->_indices[1] = 1;
        dst->_indices[2] = 2;
        dst->_type = Type::SOLID;
    } else {
        dst->_indices.resize(6,0);
        dst->_indices[0] = 0;
        dst->_indices[1] = 1;
        dst->_indices[2] = 1;
        dst->_indices[3] = 2;
        dst->_indices[4] = 2;
        dst->_indices[5] = 0;
        dst->_type = Type::PATH;
    }
    
    dst->computeBounds();
    return dst;
}

/**
 * Creates a polygon that represents an ellipse of the given dimensions.
 *
 * The indices will be generated automatically. If the value solid is true,
 * then the type will be SOLID.  Otherwise, the type will be PATH.
 *
 * @param  center   The ellipse center point.
 * @param  size     The size of the ellipse.
 * @param  segments The number of segments to use.
 * @param  solid    If true, treat this ellipse as a SOLID.
 *
 * @return A new polygon representing an ellipse.
 */
Poly2 Poly2::createEllipse(const Vec2& center, const Size& size, unsigned int segments, bool solid) {
    const float coef = 2.0f * (float)M_PI/segments;
    
    Poly2 result;
    Vec2 vert;
    result._vertices.resize(segments,Vec2::ZERO);
    for(unsigned int ii = 0; ii < segments; ii++) {
        float rads = ii*coef;
        vert.x = 0.5f * size.width  * cosf(rads) + center.x;
        vert.y = 0.5f * size.height * sinf(rads) + center.y;
        result._vertices[ii] = vert;
    }
    
    if (solid) {
        result._vertices.push_back(center);
        result._indices.resize(3*segments,0);
        for(unsigned int ii = 0; ii < segments-1; ii++) {
            result._indices[3*ii  ] = ii;
            result._indices[3*ii+1] = ii+1;
            result._indices[3*ii+2] = segments;
        }
        result._indices[3*segments-3] = segments-1;
        result._indices[3*segments-2] = 0;
        result._indices[3*segments-1] = segments;
        result._type = Type::SOLID;
    } else {
        result._indices.resize(2*segments,0);
        for(unsigned int ii = 0; ii < segments-1; ii++) {
            result._indices[2*ii  ] = ii;
            result._indices[2*ii+1] = ii+1;
        }
        result._indices[2*segments-2] = segments-1;
        result._indices[2*segments-1] = 0;
        result._type = Type::PATH;
    }

    result.computeBounds();
    return result;
}

/**
 * Creates a polygon that represents an ellipse of the given dimensions.
 *
 * The indices will be generated automatically. If the value solid is true,
 * then the type will be SOLID.  Otherwise, the type will be PATH.
 *
 * @param  center   The ellipse center point.
 * @param  size     The size of the ellipse.
 * @param  segments The number of segments to use.
 * @param  dst      A polygon to store the result in.
 * @param  solid    If true, treat this ellipse as a SOLID.
 *
 * @return A reference to dst for chaining
 */
Poly2* Poly2::createEllipse(const Vec2& center, const Size& size, unsigned int segments,
                            Poly2* dst, bool solid) {
    CUAssertLog(dst, "Assignment polygon is null");
    const float coef = 2.0f * (float)M_PI/segments;
    
    Vec2 vert;
    dst->_vertices.resize(segments,Vec2::ZERO);
    for(unsigned int ii = 0; ii < segments; ii++) {
        float rads = ii*coef;
        vert.x = 0.5f * size.width  * cosf(rads) + center.x;
        vert.y = 0.5f * size.height * sinf(rads) + center.y;
        dst->_vertices[ii] = vert;
    }
    
    if (solid) {
        dst->_vertices.push_back(center);
        dst->_indices.resize(3*segments,0);
        for(unsigned int ii = 0; ii < segments-1; ii++) {
            dst->_indices[3*ii  ] = ii;
            dst->_indices[3*ii+1] = ii+1;
            dst->_indices[3*ii+2] = segments;
        }
        dst->_indices[3*segments-3] = segments-1;
        dst->_indices[3*segments-2] = 0;
        dst->_indices[3*segments-1] = segments;
        dst->_type = Type::SOLID;
    } else {
        dst->_indices.resize(2*segments,0);
        for(unsigned int ii = 0; ii < segments-1; ii++) {
            dst->_indices[2*ii  ] = ii;
            dst->_indices[2*ii+1] = ii+1;
        }
        dst->_indices[2*segments-2] = segments-1;
        dst->_indices[2*segments-1] = 0;
        dst->_type = Type::PATH;
    }
    
    dst->computeBounds();
    return dst;
}

#pragma mark -
#pragma mark Setters
/**
 * Sets the polygon to have the given vertices
 *
 * The resulting polygon has no indices and the type is UNDEFINED.
 *
 * This method returns a reference to this polygon for chaining.
 *
 * @param vertices  The vector of vertices (as Vec2) in this polygon
 *
 * @return This polygon, returned for chaining
 */
Poly2& Poly2::set(const vector<Vec2>& vertices) {
    _vertices.assign(vertices.begin(),vertices.end());
    _indices.clear();
    _type = Type::UNDEFINED;
    computeBounds();
    return *this;
}

/**
 * Sets the polygon to have the given vertices and indices.
 *
 * A valid list of indices must only refer to vertices in the vertex array.
 * That is, the indices should all be non-negative, and each value should be
 * less than the number of vertices.
 *
 * This assignment will also assign a type according to the multiplicity of the
 * indices.  If the number of indices is three times the number of vertices,
 * the type will be SOLID.  If it is two times the number of vertices, the
 * type will be PATH.  Otherwise, the type will be UNDEFINED.
 *
 * @param vertices  The vector of vertices (as Vec2) in this polygon
 * @param indices   The vector of indices for the rendering
 *
 * @return This polygon, returned for chaining
 */
Poly2& Poly2::set(const vector<Vec2>& vertices, const vector<unsigned short>& indices) {
    _vertices.assign(vertices.begin(),vertices.end());
    _indices.assign(indices.begin(),indices.end());
    computeType();
    computeBounds();
    return *this;
}

/**
 * Sets the polygon to have the given vertices
 *
 * The float array should have an even number of elements.  The number of
 * vertices is half of the size of the array. For each value ii, 2*ii and
 * 2*ii+1 are the coordinates of a single vertex.
 *
 * The resulting polygon has no indices and the type is UNDEFINED.
 *
 * This method returns a reference to this polygon for chaining.
 *
 * @param vertices  The vector of vertices (as floats) in this polygon
 *
 * @return This polygon, returned for chaining
 */
Poly2& Poly2::set(const vector<float>& vertices) {
    vector<Vec2>* ref = (vector<Vec2>*)&vertices;
    _vertices.assign(ref->begin(),ref->end());
    _indices.clear();
    _type = Type::UNDEFINED;
    computeBounds();
    return *this;
}

/**
 * Sets a polygon to have the given vertices and indices.
 *
 * The float array should have an even number of elements.  The number of
 * vertices is half of the size of the array. For each value ii, 2*ii and
 * 2*ii+1 are the coordinates of a single vertex.
 *
 * A valid list of indices must only refer to vertices in the vertex array.
 * That is, the indices should all be non-negative, and each value should be
 * less than the number of vertices.
 *
 * This assignment will also assign a type according to the multiplicity of the
 * indices.  If the number of indices is three times the number of vertices,
 * the type will be SOLID.  If it is two times the number of vertices, the
 * type will be PATH.  Otherwise, the type will be UNDEFINED.
 *
 * This method returns a reference to this polygon for chaining.
 *
 * @param vertices  The vector of vertices (as floats) in this polygon
 * @param indices   The vector of indices for the rendering
 *
 * @return This polygon, returned for chaining
 */
Poly2& Poly2::set(const vector<float>& vertices, const vector<unsigned short>& indices) {
    vector<Vec2>* ref = (vector<Vec2>*)&vertices;
    _vertices.assign(ref->begin(),ref->end());
    _indices.assign(indices.begin(),indices.end());
    computeType();
    computeBounds();
    return *this;
}

/**
 * Sets the polygon to have the given vertices.
 *
 * The resulting polygon has no indices and the type is UNDEFINED.
 *
 * This method returns a reference to this polygon for chaining.
 *
 * @param vertices  The array of vertices (as Vec2) in this polygon
 * @param vertsize  The number of elements to use from vertices
 * @param voffset   The offset in vertices to start the polygon
 *
 * @return This polygon, returned for chaining
 */
Poly2& Poly2::set(Vec2* vertices, int vertsize, int voffset) {
    _vertices.assign(vertices+voffset,vertices+voffset+vertsize);
    _indices.clear();
    _type = Type::UNDEFINED;
    computeBounds();
    return *this;
}

/**
 * Sets the polygon to have the given vertices and indices.
 *
 * A valid list of indices must only refer to vertices in the vertex array.
 * That is, the indices should all be non-negative, and each value should be
 * less than the number of vertices.
 *
 * This assignment will also assign a type according to the multiplicity of the
 * indices.  If the number of indices is three times the number of vertices,
 * the type will be SOLID.  If it is two times the number of vertices, the
 * type will be PATH.  Otherwise, the type will be UNDEFINED.
 *
 * This method returns a reference to this polygon for chaining.
 *
 * @param vertices  The array of vertices (as Vec2) in this polygon
 * @param vertsize  The number of elements to use from vertices
 * @param indices   The array of indices for the rendering
 * @param indxsize  The number of elements to use for the indices
 * @param voffset   The offset in vertices to start the polygon
 * @param ioffset   The offset in indices to start from
 *
 * @return This polygon, returned for chaining
 */
Poly2& Poly2::set(Vec2* vertices, int vertsize, unsigned short* indices, int indxsize,
                  int voffset, int ioffset) {
    _vertices.assign(vertices+voffset,vertices+voffset+vertsize);
    _indices.assign(indices+ioffset, indices+ioffset+indxsize);
    computeType();
    computeBounds();
    return *this;
}

/**
 * Creates a copy of the given polygon.
 *
 * Both the vertices and the indices are copied.  No references to the
 * original polygon are kept.
 *
 * @param poly  The polygon to copy
 */
Poly2& Poly2::set(const Poly2& poly) {
    _vertices.assign(poly._vertices.begin(),poly._vertices.end());
    _indices.assign(poly._indices.begin(),poly._indices.end());
    _bounds = poly._bounds;
    _type = poly._type;
    return *this;
}

/**
 * Sets the polygon to represent the given rectangle.
 *
 * The polygon will have four vertices, one for each corner of the rectangle.
 * This optional argument (which is true by default) will initialize the
 * indices with a triangulation of the rectangle.  In other words, the type
 * will be SOLID. This is faster than using one of the more heavy-weight
 * triangulators.
 *
 * If solid is false, it will still generate indices, but will be a PATH
 * instead.
 *
 * @param rect  The rectangle to copy
 * @param solid Whether to treat this rectangle as a solid polygon
 *
 * @return This polygon, returned for chaining
 */
Poly2& Poly2::set(const Rect& rect, bool solid) {
    _vertices.resize(4,Vec2::ZERO);
    
    _vertices[0] = rect.origin;
    _vertices[1] = Vec2(rect.origin.x+rect.size.width, rect.origin.y);
    _vertices[2] = Vec2(rect.origin.x+rect.size.width, rect.origin.y+rect.size.height);
    _vertices[3] = Vec2(rect.origin.x, rect.origin.y+rect.size.height);
    
    if (solid) {
        _indices.resize(6,0);
        _indices[0] = 0;
        _indices[1] = 1;
        _indices[2] = 2;
        _indices[3] = 0;
        _indices[4] = 2;
        _indices[5] = 3;
        _type = Type::SOLID;
    } else {
        _indices.resize(8,0);
        _indices[0] = 0;
        _indices[1] = 1;
        _indices[2] = 1;
        _indices[3] = 2;
        _indices[4] = 2;
        _indices[5] = 3;
        _indices[6] = 3;
        _indices[7] = 0;
        _type = Type::PATH;
    }
    _bounds = rect;
    return *this;
}

#pragma mark -
#pragma mark Index Methods
/**
 * Sets the indices for this polygon to the ones given.
 *
 * A valid list of indices must only refer to vertices in the vertex array.
 * That is, the indices should all be non-negative, and each value should be
 * less than the number of vertices.
 *
 * This assignment will also assign a type according to the multiplicity of the
 * indices.  If the number of indices is three times the number of vertices,
 * the type will be SOLID.  If it is two times the number of vertices, the
 * type will be PATH.  Otherwise, the type will be UNDEFINED.
 *
 * This method returns a reference to this polygon for chaining.
 *
 * @param indices   The vector of indices for the shape
 *
 * @return This polygon, returned for chaining
 */
Poly2& Poly2::setIndices(const vector<unsigned short>& indices) {
    _indices.assign(indices.begin(), indices.end());
    computeType();
    return *this;
}

/**
 * Sets the indices for this polygon to the ones given.
 *
 * A valid list of indices must only refer to vertices in the vertex array.
 * That is, the indices should all be non-negative, and each value should be
 * less than the number of vertices.
 *
 * The provided array is copied.  The polygon does not retain a reference.
 *
 * This assignment will also assign a type according to the multiplicity of the
 * indices.  If the number of indices is three times the number of vertices,
 * the type will be SOLID.  If it is two times the number of vertices, the
 * type will be PATH.  Otherwise, the type will be UNDEFINED.
 *
 * This method returns a reference to this polygon for chaining.
 *
 * @param indices   The array of indices for the rendering
 * @param indxsize  The number of elements to use for the indices
 * @param ioffset   The offset in indices to start from
 *
 * @return This polygon, returned for chaining
 */
Poly2& Poly2::setIndices(unsigned short* indices, int indxsize, int ioffset) {
    _indices.assign(indices+ioffset, indices+ioffset+indxsize);
    computeType();
    return *this;
}

/**
 * Returns true if the indices are in the proper normal form.
 *
 * If the polygon is SOLID, this method will return true if the number of
 * indices is divisible by three.
 *
 * If the polygon is PATH, this method will return true if the number of
 * indices is divisible by two.
 *
 * If the polygon is UNDEFINED, this method will return true if there are
 * no indices.
 *
 * This method does not validate that the indices are with in range.
 * See {@link isValid()}.
 *
 * @return true if the indices are in the proper normal form.
 */
bool Poly2::isStandardized() {
    bool result;
    if (_type == Type::SOLID) {
        result = (_indices.size() % 3 == 0);
    } else if (_type == Type::PATH) {
        result = (_indices.size() % 2 == 0);
    } else {
        result = _indices.empty();
    }
    return result;
}

/**
 * Returns true if the indices are all valid.
 *
 * This method is a heavier-weight version of {@link isStandardized()}.
 * It verifies that the number of indices is correct, and that they are
 * all in range.
 *
 * If the polygon is SOLID, this method will return true if the number of
 * indices is divisible by three and if the index values are all in range.
 *
 * If the polygon is PATH, this method will return true if the number of
 * indices is divisible by two and if the index values are all in range.
 *
 * If the polygon is UNDEFINED, this method will return true if there are
 * no indices.
 *
 * @return true if the indices are all valid.
 */
bool Poly2::isValid() {
    bool result;
    if (_type == Type::SOLID) {
        result = (_indices.size() % 3 == 0);
    } else if (_type == Type::PATH) {
        result = (_indices.size() % 2 == 0);
    } else {
        result = _indices.empty();
    }
    
    if (result) {
        for(int ii = 0; result && ii < _indices.size(); ii++) {
            result = (_indices[ii] < _vertices.size());
        }
    }
    
    return result;
}


#pragma mark -
#pragma mark Polygon Operations

/**
 * Uniformly scales all of the vertices of this polygon.
 *
 * The vertices are scaled from the origin of the coordinate space.  This
 * means that if the origin is not in the interior of this polygon, the
 * polygon will be effectively translated by the scaling.
 *
 * @param scale The uniform scaling factor
 *
 * @return This polygon, scaled uniformly.
 */
Poly2& Poly2::operator*=(float scale) {
    for(int ii = 0; ii < _vertices.size(); ii++) {
        _vertices[ii] *= scale;
    }
    
    computeBounds();
    return *this;
}

/**
 * Nonuniformly scales all of the vertices of this polygon.
 *
 * The vertices are scaled from the origin of the coordinate space.  This
 * means that if the origin is not in the interior of this polygon, the
 * polygon will be effectively translated by the scaling.
 *
 * @param scale The non-uniform scaling factor
 *
 * @return This polygon, scaled non-uniformly.
 */
Poly2& Poly2::operator*=(const Vec2& scale) {
    for(int ii = 0; ii < _vertices.size(); ii++) {
        _vertices[ii].x *= scale.x;
        _vertices[ii].y *= scale.y;
    }
    
    computeBounds();
    return *this;
}

/**
 * Transforms all of the vertices of this polygon.
 *
 * @param transform The affine transform
 *
 * @return This polygon with the vertices transformed
 */
Poly2& Poly2::operator*=(const Affine2& transform) {
    Vec2 tmp;
    for(int ii = 0; ii < _vertices.size(); ii++) {
        Affine2::transform(transform,_vertices[ii], &tmp);
        _vertices[ii] = tmp;
    }
    
    computeBounds();
    return *this;
}

/**
 * Transforms all of the vertices of this polygon.
 *
 * The vertices are transformed as points. The z-value is 0.
 *
 * @param transform The transform matrix
 *
 * @return This polygon with the vertices transformed
 */
Poly2& Poly2::operator*=(const Mat4& transform) {
    Vec2 tmp;
    for(int ii = 0; ii < _vertices.size(); ii++) {
        Mat4::transform(transform,_vertices[ii], &tmp);
        _vertices[ii] = tmp;
    }
    
    computeBounds();
    return *this;
}

/**
 * Uniformly scales all of the vertices of this polygon.
 *
 * The vertices are scaled from the origin of the coordinate space.  This
 * means that if the origin is not in the interior of this polygon, the
 * polygon will be effectively translated by the scaling.
 *
 * @param scale The inverse of the uniform scaling factor
 *
 * @return This polygon, scaled uniformly.
 */
Poly2& Poly2::operator/=(float scale) {
    CUAssertLog(scale != 0, "Division by 0");
    for(int ii = 0; ii < _vertices.size(); ii++) {
        _vertices[ii].x /= scale;
        _vertices[ii].y /= scale;
    }
    
    computeBounds();
    return *this;
}

/**
 * Nonuniformly scales all of the vertices of this polygon.
 *
 * The vertices are scaled from the origin of the coordinate space.  This
 * means that if the origin is not in the interior of this polygon, the
 * polygon will be effectively translated by the scaling.
 *
 * @param scale The inverse of the non-uniform scaling factor
 *
 * @return This polygon, scaled non-uniformly.
 */
Poly2& Poly2::operator/=(const Vec2& scale) {
    for(int ii = 0; ii < _vertices.size(); ii++) {
        _vertices[ii].x /= scale.x;
        _vertices[ii].y /= scale.y;
    }
    
    computeBounds();
    return *this;
}

/**
 * Uniformly translates all of the vertices of this polygon.
 *
 * @param offset The uniform translation amount
 *
 * @return This polygon, translated uniformly.
 */
Poly2& Poly2::operator+=(float offset) {
    for(int ii = 0; ii < _vertices.size(); ii++) {
        _vertices[ii].x += offset;
        _vertices[ii].y += offset;
    }
    
    computeBounds();
    return *this;
}

/**
 * Non-uniformly translates all of the vertices of this polygon.
 *
 * @param offset The non-uniform translation amount
 *
 * @return This polygon, translated non-uniformly.
 */
Poly2& Poly2::operator+=(const Vec2& offset) {
    for(int ii = 0; ii < _vertices.size(); ii++) {
        _vertices[ii] += offset;
    }
    
    computeBounds();
    return *this;
}

/**
 * Uniformly translates all of the vertices of this polygon.
 *
 * @param offset The inverse of the uniform translation amount
 *
 * @return This polygon, translated uniformly.
 */
Poly2& Poly2::operator-=(float offset) {
    for(int ii = 0; ii < _vertices.size(); ii++) {
        _vertices[ii].x -= offset;
        _vertices[ii].y -= offset;
    }
    
    computeBounds();
    return *this;
}

/**
 * Non-uniformly translates all of the vertices of this polygon.
 *
 * @param offset The inverse of the non-uniform translation amount
 *
 * @return This polygon, translated non-uniformly.
 */
Poly2& Poly2::operator-=(const Vec2& offset) {
    for(int ii = 0; ii < _vertices.size(); ii++) {
        _vertices[ii] -= offset;
    }
    
    computeBounds();
    return *this;
}

#pragma mark -
#pragma mark Geometry Methods

/**
 * Returns the set of points forming the convex hull of this polygon.
 *
 * The returned set of points is guaranteed to be a counter-clockwise traversal
 * of the hull.
 *
 * The points on the convex hull define the "border" of the shape.  In addition
 * to minimizing the number of vertices, this is useful for determining whether
 * or not a point lies on the boundary.
 *
 * This implementation is adapted from the example at
 *
 *   http://www.geeksforgeeks.org/convex-hull-set-2-graham-scan/ 
 *
 * @return the set of points forming the convex hull of this polygon.
 */
std::vector<Vec2> Poly2::convexHull() const {
    std::vector<Vec2> points;
    std::copy(_vertices.begin(), _vertices.end(), std::back_inserter(points));
    std::vector<Vec2> hull;

    // Find the bottommost point (or chose the left most point in case of tie)
    int n = (int)points.size();
    int ymin = 0;
    for (int ii = 1; ii < n; ii++) {
        float y1 = points[ii].y;
        float y2 = points[ymin].y;
        if (y1 < y2 || (y1 == y2 && points[ii].x < points[ymin].x)) {
            ymin = ii;
        }
    }
    
    // Place the bottom-most point at first position
    Vec2 temp = points[0];
    points[0] = points[ymin];
    points[ymin] = temp;

    // Set the pivot at this first point
    GSPivot pivot;
    pivot.anchor = &points[0];
    
    // Sort the remaining points by polar angle.
    // This creates a counter-clockwise traversal of the points.
    std::sort(points.begin()+1, points.end(),
              [&](const Vec2& a, const Vec2& b) { return pivot.compare(a,b); });
    
    
    // Remove the colinear points.
    int m = 1;
    for (int ii = 1; ii < n; ii++) {
        // Keep removing i while angle of i and i+1 is same with respect to pivot
        while (ii < n-1 && orientation(*(pivot.anchor), points[ii], points[ii+1]) == 0) {
            ii++;
        }
        
        points[m] = points[ii];
        m++;   // Update size of modified array
    }
    points.resize(m);

    // If modified array of points has less than 3 points, convex hull is not possible
    if (m < 3) {
        return hull;
    }
    
    // Push first three points to the vector (as a stack).
    hull.push_back(points[0]);
    hull.push_back(points[1]);
    hull.push_back(points[2]);
    
    // Process remaining n-3 points
    for (int ii = 3; ii < m; ii++) {
        // Keep removing back whenever we make a non-left turn
        Vec2* atback   = &(hull[hull.size()-1]);
        Vec2* nextback = &(hull[hull.size()-2]);
        
        while (orientation(*nextback, *atback, points[ii]) != -1) {
            hull.pop_back();
            atback = nextback;
            nextback = &(hull[hull.size()-2]);
        }
        hull.push_back(points[ii]);
    }
    
    return hull;
}

/**
 * Returns true if this polygon contains the given point.
 *
 * This method returns false is the polygon is not SOLID.  If it is solid,
 * it checks for containment within the associated triangles.  It includes
 * points on the polygon border.
 *
 * @param  point    The point to test
 *
 * @return true if this polygon contains the given point.
 */
bool Poly2::contains(const Vec2& point) const {
    if (_type != Type::SOLID) {
        return false;
    }
    bool inside = false;
    for(int ii = 0; !inside && 3*ii < _indices.size(); ii++) {
        Vec3 tmp = getBarycentric(point,ii);
        inside = (0 <= tmp.x && tmp.x <= 1 && 0 <= tmp.y && tmp.y <= 1 && 0 <= tmp.z && tmp.z <= 1);
    }
    return inside;
}

/**
 * Returns true if the given point is on the boundary of this polygon.
 *
 * This method returns false is the polygon is not SOLID or PATH.  If it is
 * a path, it checks that the point is within variance of the a line segment
 * on the path.
 *
 * If it is solid, it checks that the point is within variance of the convex
 * hull.  The convex hull is not a fast computation, so this method should
 * be used with care.
 *
 * @param  point    The point to test
 * @param  variance The distance tolerance
 *
 * @return true if the given point is on the boundary of this polygon.
 */
bool Poly2::incident(const Vec2& point, float variance) const {
    if (_type == Type::UNDEFINED) {
        return false;
    }
    
    if (_type == Type::PATH) {
        bool touches = false;
        for(int ii = 0; !touches && 2*ii < _indices.size(); ii++) {
            touches = onsegment(point, _vertices[_indices[2*ii]], _vertices[_indices[2*ii+1]], variance);
        }
        return touches;
    }
    
    // SOLID
    std::vector<Vec2> hull = convexHull();
    
    bool touches = false;
    for(int ii = 0; !touches && ii+1 < hull.size(); ii++) {
        touches = onsegment(point, hull[ii], hull[ii+1], variance);
    }
    touches = touches || onsegment(point, hull[-1+(int)hull.size()], hull[0], variance);
    return touches;
}


#pragma mark -
#pragma mark Internal Helpers

/**
 * Compute the bounding box for this polygon.
 *
 * The bounding box is the minimal rectangle that contains all of the vertices in
 * this polygon.  It is recomputed whenever the vertices are set.
 */
void Poly2::computeBounds() {
    float minx, maxx;
    float miny, maxy;
    
    minx = _vertices[0].x;
    maxx = _vertices[0].x;
    miny = _vertices[0].y;
    maxy = _vertices[0].y;
    for(auto it = _vertices.begin()+1; it != _vertices.end(); ++it) {
        if (it->x < minx) {
            minx = it->x;
        } else if (it->x > maxx) {
            maxx = it->x;
        }
        if (it->y < miny) {
            miny = it->y;
        } else if (it->y > maxy) {
            maxy = it->y;
        }
    }
    
    _bounds.origin.x = minx;
    _bounds.origin.y = miny;
    _bounds.size.width  = maxx-minx;
    _bounds.size.height = maxy-miny;
}

/**
 * Compute the bounding box for this polygon.
 *
 * This type is assigned according to the multiplicity of the indices.  If
 * the number of indices is three times the number of vertices, the type
 * will be SOLID.  If it is two times the number of vertices, the
 * type will be PATH.  Otherwise, the type will be UNDEFINED.
 */
void Poly2::computeType() {
    int n = (int)_indices.size();
    int k = (int)_vertices.size();
    if (n % 2 == 0 && (n == 2*k || n == 2*k-2)) {
        _type = Type::PATH;
    } else if (n % 3 == 0) {
        _type = Type::SOLID;
    } else if (n % 2 == 0) {
        _type = Type::PATH;
    } else {
        _type = Type::UNDEFINED;
    }
}

/**
 * Returns the barycentric coordinates for a point relative to a triangle.
 *
 * The triangle is identified by the given index.  For index ii, it is the
 * triangle defined by indices 3*ii, 3*ii+1, and 3*ii+2.
 *
 * This method is not defined if the polygon is not SOLID.
 */
Vec3 Poly2::getBarycentric(const Vec2& point, unsigned short index) const {
    Vec2 a = _vertices[_indices[3*index  ]];
    Vec2 b = _vertices[_indices[3*index+1]];
    Vec2 c = _vertices[_indices[3*index+2]];
    
    float det = (b.y-c.y)*(a.x-c.x)+(c.x-b.x)*(a.y-c.y);
    Vec3 result;
    result.x = (b.y-c.y)*(point.x-c.x)+(c.x-b.x)*(point.y-c.y);
    result.y = (c.y-a.y)*(point.x-c.x)+(a.x-c.x)*(point.y-c.y);
    result.x /= det;
    result.y /= det;
    result.z = 1 - result.x - result.y;
    return result;
}

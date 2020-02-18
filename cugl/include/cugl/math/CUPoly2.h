//
//  CUPoly2.h
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

#ifndef __CU_POLY2_H__
#define __CU_POLY2_H__

#include <vector>
#include "CUVec2.h"
#include "CURect.h"

namespace cugl {

// Forward references
class Mat4;
class Affine2;
    
/**
 * Class to represent a simple polygon.
 *
 * This class is intended to represent any polygon (including non-convex polygons) 
 * that does not have holes or self-interections.  This class performs no 
 * verification. It will not check for holes or self-intersections; those are 
 * the responsibility of the programmer.
 *
 * When instantiating this class, the user should provide a set of indices which 
 * will be used in rendering.  These indices can either represent a triangulation 
 * of the polygon, or they can represent a traversal (for a wireframe).  Provided
 * that these indices are in "normal" form, it can provide support a for basic
 * geometry queries, such as wether a point is inside the polygon.
 *
 * There are two normal forms, depending on the value of the type attribute.
 * If the polygon is SOLID, then the indices are a set of triangles.  That is,
 * there are three times as many indices as triangles, and a point is in the
 * polygon if it is contained in any one of the triangles.  Solid polygons do
 * not support triangle fans or strips.
 *
 * If the polygon is a PATH, then the indices are a set of lines.  That is,
 * there are two times as many indices as triangles, and a point is in the
 * polygon if it is incident on any of the lines. Non-solid (or path) polygons 
 * do not support line strips.
 *
 * Generating indices for a Poly2 can be nontrivial.  While this class has
 * standard constructors, allowing the programmer full control, most Poly2
 * objects are created through alternate means.  For simple shapes, like lines,
 * triangles, and ellipses, this class has several static constructors.
 *
 * For more complex shapes, we have several Poly2 factories.  These factories
 * allow for delegating index computation to a separate thread, if it takes
 * too long.  These factories are as follows:
 *
 * {@link SimpleTriangulator}: This is a simple earclipping-triangulator for
 * tesselating simple, solid polygons (e.g. no holes or self-intersections).
 *
 * {@link DelaunayTriangulator}: This is a Delaunay Triangular that gives a 
 * more uniform triangulation in accordance to the Vornoi diagram.
 * 
 * {@link PathOutliner}: This is a tool is used to generate indices for a
 * path polygon.  It has several options, that allow it to make useful 
 * wireframes for debugging.
 *
 * {@link CubicSplineApproximator}: This is a tool is used to generate a Poly2 
 * object from a Cubic Bezier curve.
 *
 * {@link PathExtruder}: This is a tool can take a path polygon and convert it
 * into a solid polygon.  This solid polygon is the same as the path, except 
 * that the path now has a width and a mitre at the joints.  This tool is 
 * different from the other tools in that it generates new vertices in addition 
 * to the indices.
 */
class Poly2 {
#pragma mark Values
public:
    /** 
     * This enum is used to determine the normal form for the indices. Any
     * rendering classes should use this type as a hint for how to render 
     * the polygon.
     */
    enum class Type {
        /** 
         * This polygon either has no indices, or they are not in a normal form. 
         */
        UNDEFINED,
        /** 
         * This polygon represents a solid shape. The indices are a sequence of 
         * triangles.  That is, the number of indices is divisible by three, 
         * with each triplet forming a triangle.
         */
        SOLID,
        /**
         * This polygon represents a path outline. The indices are a sequence of
         * line segments.  That is, the number of indices is divisible by two,
         * with each pair forming a segment.
         */
        PATH
    };
    
private:
    /** The vector of vertices in this polygon */
    std::vector<Vec2> _vertices;
    /** The vector of indices in the triangulation */
    std::vector<unsigned short> _indices;
    /** The bounding box for this polygon */
    Rect _bounds;
    /** The indexing style of polygon (determines normal form) */
    Type _type;
    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates an empty polygon.
     *
     * The created polygon has no vertices and no triangulation.  The bounding 
     * box is trivial.
     */
    Poly2() : _type(Type::UNDEFINED) { }
    
    /**
     * Creates a polygon with the given vertices
     *
     * The new polygon has no indices and the type is UNDEFINED.
     *
     * @param vertices  The vector of vertices (as Vec2) in this polygon
     */
    Poly2(const std::vector<Vec2>& vertices) { set(vertices); }
    
    /**
     * Creates a polygon with the given vertices and indices.
     *
     * A valid list of indices must only refer to vertices in the vertex array.
     * That is, the indices should all be non-negative, and each value should be
     * less than the number of vertices.
     *
     * This constructor will assign a type accoring to the multiplicity of the
     * indices. If the number of indices n is correct for a closed or open path
     * of all vertices (e.g. 2n or 2n-2), then the type will be PATH. Otherwise,
     * if n is divisible by 3 it will be SOLID. All other values will be 
     * UNDEFINED, and the user must manually set the type.
     *
     * @param vertices  The vector of vertices (as Vec2) in this polygon
     * @param indices   The vector of indices for the rendering
     */
    Poly2(const std::vector<Vec2>& vertices, const std::vector<unsigned short>& indices) {
        set(vertices, indices);
    }
    
    /**
     * Creates a polygon with the given vertices
     *
     * The new polygon has no indices. 
     *
     * The float array should have an even number of elements.  The number of
     * vertices is half of the size of the array. For each value ii, 2*ii and
     * 2*ii+1 are the coordinates of a single vertex.
     *
     * The new polygon has no indices and the type is UNDEFINED.
     *
     * @param vertices  The vector of vertices (as floats) in this polygon
     */
    Poly2(const std::vector<float>& vertices)    { set(vertices); }
    
    /**
     * Creates a polygon with the given vertices and indices.
     *
     * The float array should have an even number of elements.  The number of 
     * vertices is half of the size of the array. For each value ii, 2*ii and
     * 2*ii+1 are the coordinates of a single vertex.
     *
     * A valid list of indices must only refer to vertices in the vertex array.
     * That is, the indices should all be non-negative, and each value should be
     * less than the number of vertices.
     *
     * This constructor will assign a type accoring to the multiplicity of the
     * indices. If the number of indices n is correct for a closed or open path
     * of all vertices (e.g. 2n or 2n-2), then the type will be PATH. Otherwise,
     * if n is divisible by 3 it will be SOLID. All other values will be
     * UNDEFINED, and the user must manually set the type.
     *
     * @param vertices  The vector of vertices (as floats) in this polygon
     * @param indices   The vector of indices for the rendering
     */
    Poly2(const std::vector<float>& vertices, const std::vector<unsigned short>& indices) {
        set(vertices, indices);
    }
    
    /**
     * Creates a polygon with the given vertices
     *
     * The new polygon has no indices and the type is UNDEFINED.
     *
     * @param vertices  The array of vertices (as Vec2) in this polygon
     * @param vertsize  The number of elements to use from vertices
     * @param voffset   The offset in vertices to start the polygon
     */
    Poly2(Vec2* vertices,  int vertsize, int voffset=0) {
        set(vertices, vertsize, voffset);
    }
    
    /**
     * Creates a polygon with the given vertices
     *
     * The float array should have an even number of elements.  The number of
     * vertices is half of the size of the array. For each value ii, 2*ii and
     * 2*ii+1 are the coordinates of a single vertex.
     *
     * The new polygon has no indices and the type is UNDEFINED.
     *
     * @param vertices  The array of vertices (as floats) in this polygon
     * @param vertsize  The number of elements to use from vertices
     * @param voffset   The offset in vertices to start the polygon
     */
    Poly2(float* vertices,  int vertsize, int voffset=0) {
        set(vertices, vertsize, voffset);
    }
    
    /**
     * Creates a polygon with the given vertices and indices.
     *
     * A valid list of indices must only refer to vertices in the vertex array.
     * That is, the indices should all be non-negative, and each value should be
     * less than the number of vertices.
     *
     * This constructor will assign a type accoring to the multiplicity of the
     * indices. If the number of indices n is correct for a closed or open path
     * of all vertices (e.g. 2n or 2n-2), then the type will be PATH. Otherwise,
     * if n is divisible by 3 it will be SOLID. All other values will be
     * UNDEFINED, and the user must manually set the type.
     *
     * @param vertices  The array of vertices (as Vec2) in this polygon
     * @param vertsize  The number of elements to use from vertices
     * @param indices   The array of indices for the rendering
     * @param indxsize  The number of elements to use for the indices
     * @param voffset   The offset in vertices to start the polygon
     * @param ioffset   The offset in indices to start from
     */
    Poly2(Vec2* vertices,  int vertsize, unsigned short* indices, int indxsize,
          int voffset=0, int ioffset=0) {
        set(vertices, vertsize, indices, indxsize, voffset, ioffset);
    }
    
    /**
     * Creates a polygon with the given vertices and indices.
     *
     * The float array should have an even number of elements.  The number of
     * vertices is half of the size of the array. For each value ii, 2*ii and
     * 2*ii+1 are the coordinates of a single vertex.
     *
     * A valid list of indices must only refer to vertices in the vertex array.
     * That is, the indices should all be non-negative, and each value should be
     * less than the number of vertices.
     *
     * This constructor will assign a type accoring to the multiplicity of the
     * indices. If the number of indices n is correct for a closed or open path
     * of all vertices (e.g. 2n or 2n-2), then the type will be PATH. Otherwise,
     * if n is divisible by 3 it will be SOLID. All other values will be
     * UNDEFINED, and the user must manually set the type.
     *
     * @param vertices  The array of vertices (as floats) in this polygon
     * @param vertsize  The number of elements to use from vertices
     * @param indices   The array of indices for the rendering
     * @param indxsize  The number of elements to use for the indices
     * @param voffset   The offset in vertices to start the polygon
     * @param ioffset   The offset in indices to start from
     */
    Poly2(float* vertices,  int vertsize, unsigned short* indices, int indxsize,
          int voffset=0, int ioffset=0) {
        set(vertices, vertsize, indices, indxsize, voffset, ioffset);
    }
    
    /**
     * Creates a copy of the given polygon.
     *
     * Both the vertices and the indices are copied.  No references to the
     * original polygon are kept.
     *
     * @param poly  The polygon to copy
     */
    Poly2(const Poly2& poly) { set(poly); }

    /**
     * Creates a copy with the resource of the given polygon.
     *
     * @param poly  The polygon to take from
     */
    Poly2(Poly2&& poly) :
        _vertices(std::move(poly._vertices)), _indices(std::move(poly._indices)),
        _bounds(std::move(poly._bounds)), _type(poly._type) {}
    
    /**
     * Creates a polygon for the given rectangle.
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
     */
    Poly2(const Rect& rect, bool solid=true) { set(rect,solid); }
    
    /**
     * Deletes the given polygon, freeing all resources.
     */
    ~Poly2() { }
    

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
    static Poly2 createLine(const Vec2& origin, const Vec2& dest);
    
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
    static Poly2* createLine(const Vec2& origin, const Vec2& dest, Poly2* dst);
    
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
    static Poly2 createTriangle(const Vec2& a, const Vec2& b, const Vec2& c, bool solid=true);

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
    static Poly2* createTriangle(const Vec2& a, const Vec2& b, const Vec2& c, Poly2* dst, bool solid=true);

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
    static Poly2 createEllipse(const Vec2& center, const Size& size, unsigned int segments, bool solid=true);
    
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
    static Poly2* createEllipse(const Vec2& center, const Size& size, unsigned int segments, Poly2* dst,
                                bool solid=true);
    
#pragma mark -
#pragma mark Setters
    /**
     * Sets this polygon to be a copy of the given one.
     *
     * All of the contents are copied, so that this polygon does not hold any
     * references to elements of the other polygon. This method returns
     * a reference to this polygon for chaining.
     *
     * @param other  The polygon to copy
     *
     * @return This polygon, returned for chaining
     */
    Poly2& operator= (const Poly2& other) { return set(other); }

    /**
     * Sets this polygon to be have the resources of the given one.
     *
     * @param other  The polygon to take from
     *
     * @return This polygon, returned for chaining
     */
    Poly2& operator=(Poly2&& other) {
        _vertices = std::move(other._vertices);
        _indices = std::move(other._indices);
        _bounds = std::move(other._bounds);
        _type = other._type;
        return *this;
    }

    
    /**
     * Sets this polygon to be a copy of the given rectangle.
     *
     * The polygon will have four vertices, one for each corner of the rectangle.
     * In addition, this assignment will initialize the indices with a simple 
     * triangulation of the rectangle. The type will be SOLID.
     *
     * @param rect  The rectangle to copy
     *
     * @return This polygon, returned for chaining
     */
    Poly2& operator= (const Rect& rect) { return set(rect); }

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
    Poly2& set(const std::vector<Vec2>& vertices);
    
    /**
     * Sets the polygon to have the given vertices and indices.
     *
     * A valid list of indices must only refer to vertices in the vertex array.
     * That is, the indices should all be non-negative, and each value should be
     * less than the number of vertices.
     *
     * This method will assign a type accoring to the multiplicity of the
     * indices. If the number of indices n is correct for a closed or open path
     * of all vertices (e.g. 2n or 2n-2), then the type will be PATH. Otherwise,
     * if n is divisible by 3 it will be SOLID. All other values will be
     * UNDEFINED, and the user must manually set the type.
     *
     * @param vertices  The vector of vertices (as Vec2) in this polygon
     * @param indices   The vector of indices for the rendering
     *
     * @return This polygon, returned for chaining
     */
    Poly2& set(const std::vector<Vec2>& vertices, const std::vector<unsigned short>& indices);
    
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
    Poly2& set(const std::vector<float>& vertices);
    
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
     * This method will assign a type accoring to the multiplicity of the
     * indices. If the number of indices n is correct for a closed or open path
     * of all vertices (e.g. 2n or 2n-2), then the type will be PATH. Otherwise,
     * if n is divisible by 3 it will be SOLID. All other values will be
     * UNDEFINED, and the user must manually set the type.
     *
     * This method returns a reference to this polygon for chaining.
     *
     * @param vertices  The vector of vertices (as floats) in this polygon
     * @param indices   The vector of indices for the rendering
     *
     * @return This polygon, returned for chaining
     */
    Poly2& set(const std::vector<float>& vertices, const std::vector<unsigned short>& indices);
    
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
    Poly2& set(Vec2* vertices, int vertsize, int voffset=0);
    
    /**
     * Sets the polygon to have the given vertices.
     *
     * The float array should have an even number of elements.  The number of
     * vertices is half of the size of the array. For each value ii, 2*ii and
     * 2*ii+1 are the coordinates of a single vertex.
     *
     * The resulting polygon has no indices and the type is UNDEFINED.
     *
     * This method returns a reference to this polygon for chaining.
     *
     * @param vertices  The array of vertices (as floats) in this polygon
     * @param vertsize  The number of elements to use from vertices
     * @param voffset   The offset in vertices to start the polygon
     *
     * @return This polygon, returned for chaining
     */
    Poly2& set(float* vertices,  int vertsize, int voffset=0) {
        return set((Vec2*)vertices, vertsize/2, voffset/2);
    }
    
    /**
     * Sets the polygon to have the given vertices and indices.
     *
     * A valid list of indices must only refer to vertices in the vertex array.
     * That is, the indices should all be non-negative, and each value should be
     * less than the number of vertices.
     *
     * This method will assign a type accoring to the multiplicity of the
     * indices. If the number of indices n is correct for a closed or open path
     * of all vertices (e.g. 2n or 2n-2), then the type will be PATH. Otherwise,
     * if n is divisible by 3 it will be SOLID. All other values will be
     * UNDEFINED, and the user must manually set the type.
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
    Poly2& set(Vec2* vertices, int vertsize, unsigned short* indices, int indxsize,
               int voffset=0, int ioffset=0);
    
    /**
     * Sets the polygon to have the given vertices and indices.
     *
     * The float array should have an even number of elements.  The number of
     * vertices is half of the size of the array. For each value ii, 2*ii and
     * 2*ii+1 are the coordinates of a single vertex.
     *
     * A valid list of indices must only refer to vertices in the vertex array.
     * That is, the indices should all be non-negative, and each value should be
     * less than the number of vertices.
     *
     * This method will assign a type accoring to the multiplicity of the
     * indices. If the number of indices n is correct for a closed or open path
     * of all vertices (e.g. 2n or 2n-2), then the type will be PATH. Otherwise,
     * if n is divisible by 3 it will be SOLID. All other values will be
     * UNDEFINED, and the user must manually set the type.
     *
     * This method returns a reference to this polygon for chaining.
     *
     * @param vertices  The array of vertices (as floats) in this polygon
     * @param vertsize  The number of elements to use from vertices
     * @param indices   The array of indices for the rendering
     * @param indxsize  The number of elements to use for the indices
     * @param voffset   The offset in vertices to start the polygon
     * @param ioffset   The offset in indices to start from
     *
     * @return This polygon, returned for chaining
     */
    Poly2& set(float* vertices, int vertsize, unsigned short* indices, int indxsize,
               int voffset=0, int ioffset=0) {
        return set((Vec2*)vertices, vertsize/2, indices, indxsize, voffset/2, ioffset);
    }
    
    /**
     * Sets this polygon to be a copy of the given one.
     *
     * All of the contents are copied, so that this polygon does not hold any
     * references to elements of the other polygon. This method returns
     * a reference to this polygon for chaining.
     *
     * This method returns a reference to this polygon for chaining.
     *
     * @param poly  The polygon to copy
     *
     * @return This polygon, returned for chaining
     */
    Poly2& set(const Poly2& poly);
    
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
    Poly2& set(const Rect& rect, bool solid=true);
    
    /**
     * Clears the contents of this polygon and sets the type to UNDEFINED
     *
     * @return This polygon, returned for chaining
     */
    Poly2& clear() {
        _vertices.clear();
        _indices.clear();
        _type = Type::UNDEFINED;
        _bounds = Rect::ZERO;
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
     * This method will assign a type accoring to the multiplicity of the
     * indices. If the number of indices n is correct for a closed or open path
     * of all vertices (e.g. 2n or 2n-2), then the type will be PATH. Otherwise,
     * if n is divisible by 3 it will be SOLID. All other values will be
     * UNDEFINED, and the user must manually set the type.
     *
     * This method returns a reference to this polygon for chaining.
     *
     * @param indices   The vector of indices for the shape
     *
     * @return This polygon, returned for chaining
     */
    Poly2& setIndices(const std::vector<unsigned short>& indices);
    
    /**
     * Sets the indices for this polygon to the ones given.
     *
     * A valid list of indices must only refer to vertices in the vertex array.
     * That is, the indices should all be non-negative, and each value should be
     * less than the number of vertices.
     *
     * The provided array is copied.  The polygon does not retain a reference.
     *
     * This method will assign a type accoring to the multiplicity of the
     * indices. If the number of indices n is correct for a closed or open path
     * of all vertices (e.g. 2n or 2n-2), then the type will be PATH. Otherwise,
     * if n is divisible by 3 it will be SOLID. All other values will be
     * UNDEFINED, and the user must manually set the type.
     *
     * This method returns a reference to this polygon for chaining.
     *
     * @param indices   The array of indices for the rendering
     * @param indxsize  The number of elements to use for the indices
     * @param ioffset   The offset in indices to start from
     *
     * @return This polygon, returned for chaining
     */
    Poly2& setIndices(unsigned short* indices, int indxsize, int ioffset=0);
    
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
    bool isStandardized();
    
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
    bool isValid();
    

#pragma mark -
#pragma mark Polygon Attributes
    /**
     * Returns the number of vertices in a polygon.
     *
     * @return the number of vertices in a polygon.
     */
    size_t size() const { return _vertices.size(); }

    /**
     * Returns the number of indices in a polygon.
     *
     * @return the number of indices in a polygon.
     */
    size_t indexSize() const { return _indices.size(); }

    /**
     * Returns a reference to the attribute at the given index.
     *
     * This accessor will allow you to change the (singular) vertex.  It is
     * intended to allow minor distortions to the polygon without changing
     * the underlying mesh.
     *
     * @param index  The attribute index
     *
     * @return a reference to the attribute at the given index.
     */
    Vec2& at(int index) { return _vertices.at(index); }

    /**
     * Returns the list of vertices
     *
     * This accessor will not permit any changes to the vertex array.  To change
     * the array, you must change the polygon via a set() method.
     *
     * @return a reference to the vertex array
     */
    const std::vector<Vec2>& getVertices() const { return _vertices; }

    /**
     * Returns a reference to list of indices.
     *
     * This accessor will not permit any changes to the index array.  To change
     * the array, you must change the polygon via a set() method.
     *
     * @return a reference to the vertex array
     */
    const std::vector<unsigned short>& getIndices() const  { return _indices; }

    /**
     * Returns a reference to list of indices.
     *
     * This accessor will not permit any changes to the index array.  To change
     * the array, you must change the polygon via a set() method.
     *
     * This non-const version of the method is used by triangulators.
     *
     * @return a reference to the vertex array
     */
    std::vector<unsigned short>& getIndices()  { return _indices; }

    /**
     * Returns the bounding box for the polygon
     *
     * The bounding box is the minimal rectangle that contains all of the vertices in
     * this polygon.  It is recomputed whenever the vertices are set.
     *
     * @return the bounding box for the polygon
     */
    const Rect& getBounds() const { return _bounds; }
    
    /**
     * Returns the type of this polygon.
     * 
     * The type determines the proper form of the indices.
     *
     * If the polygon is SOLID, there should be three times as many indices as
     * vertices.  Each triplet should define a triangle over the vertices.
     *
     * If the polygon is PATH, there should be two times as many indices as
     * vertices.  Each pair should define a line segment over the vertices.   
     *
     * If the polygon is UNDEFINED, the index list should be empty.
     *
     * @return the type of this polygon.
     */
    Type getType() const { return _type; }
    
    /**
     * Sets the type of this polygon.
     *
     * The type determines the proper form of the indices.
     *
     * If the polygon is SOLID, there should be three times as many indices as
     * vertices.  Each triplet should define a triangle over the vertices.
     *
     * If the polygon is PATH, there should be two times as many indices as
     * vertices.  Each pair should define a line segment over the vertices.
     *
     * If the polygon is UNDEFINED, the index list should be empty.
     *
     * @param type  The type of this polygon.
     */
    void setType(Type type) { _type = type; }
    
    
#pragma mark -
#pragma mark Operators
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
    Poly2& operator*=(float scale);
    
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
    Poly2& operator*=(const Vec2& scale);

    /**
     * Transforms all of the vertices of this polygon.
     *
     * @param transform The affine transform
     *
     * @return This polygon with the vertices transformed
     */
    Poly2& operator*=(const Affine2& transform);
    
    /**
     * Transforms all of the vertices of this polygon.
     *
     * The vertices are transformed as points. The z-value is 0.
     *
     * @param transform The transform matrix
     *
     * @return This polygon with the vertices transformed
     */
    Poly2& operator*=(const Mat4& transform);
    
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
    Poly2& operator/=(float scale);
    
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
    Poly2& operator/=(const Vec2& scale);
    
    /**
     * Uniformly translates all of the vertices of this polygon.
     *
     * @param offset The uniform translation amount
     *
     * @return This polygon, translated uniformly.
     */
    Poly2& operator+=(float offset);
    
    /**
     * Non-uniformly translates all of the vertices of this polygon.
     *
     * @param offset The non-uniform translation amount
     *
     * @return This polygon, translated non-uniformly.
     */
    Poly2& operator+=(const Vec2& offset);
    
    /**
     * Uniformly translates all of the vertices of this polygon.
     *
     * @param offset The inverse of the uniform translation amount
     *
     * @return This polygon, translated uniformly.
     */
    Poly2& operator-=(float offset);
    
    /**
     * Non-uniformly translates all of the vertices of this polygon.
     *
     * @param offset The inverse of the non-uniform translation amount
     *
     * @return This polygon, translated non-uniformly.
     */
    Poly2& operator-=(const Vec2& offset);
    
    /**
     * Returns a new polygon by scaling the vertices uniformly.
     *
     * The vertices are scaled from the origin of the coordinate space.  This
     * means that if the origin is not in the interior of this polygon, the
     * polygon will be effectively translated by the scaling.
     *
     * Note: This method does not modify the polygon.
     *
     * @param scale The uniform scaling factor
     *
     * @return The scaled polygon
     */
    Poly2 operator*(float scale) const { return Poly2(*this) *= scale; }
    
    /**
     * Returns a new polygon by scaling the vertices non-uniformly.
     *
     * The vertices are scaled from the origin of the coordinate space.  This
     * means that if the origin is not in the interior of this polygon, the
     * polygon will be effectively translated by the scaling.
     *
     * Note: This method does not modify the polygon.
     *
     * @param scale The non-uniform scaling factor
     *
     * @return The scaled polygon
     */
    Poly2 operator*(const Vec2& scale) const { return Poly2(*this) *= scale; }
    

    /**
     * Returns a new polygon by transforming all of the vertices of this polygon.
     *
     * Note: This method does not modify the polygon.
     *
     * @param transform The affine transform
     *
     * @return The transformed polygon
     */
    Poly2 operator*=(const Affine2& transform) const { return Poly2(*this) *= transform; }

    /**
     * Returns a new polygon by transforming all of the vertices of this polygon.
     *
     * The vertices are transformed as points. The z-value is 0.
     *
     * Note: This method does not modify the polygon.
     *
     * @param transform The transform matrix
     *
     * @return The transformed polygon
     */
    Poly2 operator*(const Mat4& transform) const { return Poly2(*this) *= transform; }
    
    /**
     * Returns a new polygon by scaling the vertices uniformly.
     *
     * The vertices are scaled from the origin of the coordinate space.  This
     * means that if the origin is not in the interior of this polygon, the
     * polygon will be effectively translated by the scaling.
     *
     * Note: This method does not modify the polygon.
     *
     * @param scale The inverse of the uniform scaling factor
     *
     * @return The scaled polygon
     */
    Poly2 operator/(float scale) const { return Poly2(*this) /= scale; }
    
    /**
     * Returns a new polygon by scaling the vertices non-uniformly.
     *
     * The vertices are scaled from the origin of the coordinate space.  This
     * means that if the origin is not in the interior of this polygon, the
     * polygon will be effectively translated by the scaling.
     *
     * Note: This method does not modify the polygon.
     *
     * @param scale The inverse of the non-uniform scaling factor
     *
     * @return The scaled polygon
     */
    Poly2 operator/(const Vec2& scale) const { return Poly2(*this) /= scale; }
    
    /**
     * Returns a new polygon by translating the vertices uniformly.
     *
     * Note: This method does not modify the polygon.
     *
     * @param offset The uniform translation amount
     *
     * @return The translated polygon
     */
    Poly2 operator+(float offset) const { return Poly2(*this) += offset; }
    
    /**
     * Returns a new polygon by translating the vertices non-uniformly.
     *
     * Note: This method does not modify the polygon.
     *
     * @param offset The non-uniform translation amount
     *
     * @return The translated polygon
     */
    Poly2 operator+(const Vec2& offset) const { return Poly2(*this) += offset; }
    
    /**
     * Returns a new polygon by translating the vertices uniformly.
     *
     * Note: This method does not modify the polygon.
     *
     * @param offset The inverse of the uniform translation amount
     *
     * @return The translated polygon
     */
    Poly2 operator-(float offset) { return Poly2(*this) -= offset; }
    
    /**
     * Returns a new polygon by translating the vertices non-uniformly.
     *
     * Note: This method does not modify the polygon.
     *
     * @param offset The inverse of the non-uniform translation amount
     *
     * @return The translated polygon
     */
    Poly2 operator-(const Vec2& offset) { return Poly2(*this) -= offset; }
    
    /**
     * Returns a new polygon by scaling the vertices uniformly.
     *
     * The vertices are scaled from the origin of the coordinate space.  This
     * means that if the origin is not in the interior of this polygon, the
     * polygon will be effectively translated by the scaling.
     *
     * @param scale The uniform scaling factor
     * @param poly 	The polygon to scale
     *
     * @return The scaled polygon
     */
    friend Poly2 operator*(float scale, const Poly2& poly) { return poly*scale; }
    
    /**
     * Returns a new polygon by scaling the vertices non-uniformly.
     *
     * The vertices are scaled from the origin of the coordinate space.  This
     * means that if the origin is not in the interior of this polygon, the
     * polygon will be effectively translated by the scaling.
     *
     * @param scale The non-uniform scaling factor
     * @param poly 	The polygon to scale
     *
     * @return The scaled polygon
     */
    friend Poly2 operator*(const Vec2& scale, const Poly2& poly) { return poly*scale; }
    
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
    std::vector<Vec2> convexHull() const;
    
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
    bool contains(const Vec2& point) const;
    
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
    bool incident(const Vec2& point, float variance=CU_MATH_EPSILON) const;

    
#pragma mark -
#pragma mark Internal Helper Methods
private:
    /**
     * Compute the type for this polygon.
     *
     * The bounding box is the minimal rectangle that contains all of the vertices in
     * this polygon.  It is recomputed whenever the vertices are set.
     */
    void computeBounds();

    /**
     * Compute the bounding box for this polygon.
     *
     * This type is assigned according to the multiplicity of the indices. If
     * the number of indices n is correct for a closed or open path of all
     * vertices (e.g. 2n or 2n-2), then the type will be PATH. Otherwise,
     * if n is divisible by 3 it will be SOLID. All other values will be
     * UNDEFINED, and the user must manually set the type.
     */
    void computeType();

    /**
     * Returns the barycentric coordinates for a point relative to a triangle.
     *
     * The triangle is identified by the given index.  For index ii, it is the
     * triangle defined by indices 3*ii, 3*ii+1, and 3*ii+2.
     *
     * This method is not defined if the polygon is not SOLID.
     *
     * @param point The point to convert
     * @param index The triangle index in this polygon
     *
     * @return the barycentric coordinates for a point relative to a triangle.
     */
    Vec3 getBarycentric(const Vec2& point, unsigned short index) const;

    // Make friends with the factory classes
    friend class CubicSplineApproximator;
    friend class SimpleTriangulator;
    friend class DelaunayTriangulator;
    friend class PathOutliner;
    friend class PathExtruder;
};

}
#endif /* __CU_POLY2_H__ */

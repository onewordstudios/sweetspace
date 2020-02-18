//
//  CUSimpleTriangulator.h
//  Cornell University Game Library (CUGL)
//
//  This module is a factory for a very lightweight triangulator.  While we
//  do have access to the very powerful Pol2Tri, that API has a lot of overhead
//  with it.  If you polygon is simple (no holes or self-intersections), this
//  is good enough for you.
//
//  Because math objects are intended to be on the stack, we do not provide
//  any shared pointer support in this class.
//
//  This implementation is largely inspired by the LibGDX implementation from
//  Nicolas Gramlich, Eric Spits, Thomas Cate, and Nathan Sweet.
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
//  Version: 6/22/16

#ifndef __CU_SIMPLE_TRIANGULATOR_H__
#define __CU_SIMPLE_TRIANGULATOR_H__

#include <cugl/math/CUPoly2.h>
#include <cugl/math/CUVec2.h>
#include <vector>

namespace cugl {
    
/**
 * This class is a factory for producing solid Poly2 objects from a set of vertices.
 *
 * For all but the simplist of shapes, it is important to have a triangulator
 * that can divide up the polygon into triangles for drawing.  This is a simple
 * implementation of the the ear cutting algorithm to triangulate simple
 * polygons.  It will not handle polygons with holes or with self intersections.
 *
 * As with all factories, the methods are broken up into three phases:
 * initialization, calculation, and materialization.  To use the factory, you
 * first set the data (in this case a set of vertices or another Poly2) with the
 * initialization methods.  You then call the calculation method.  Finally,
 * you use the materialization methods to access the data in several different
 * ways.
 *
 * This division allows us to support multithreaded calculation if the data
 * generation takes too long.  However, note that this factory is not thread
 * safe in that you cannot access data while it is still in mid-calculation.
 */
class SimpleTriangulator {
#pragma mark Values
private:
    /**
     * Enumeration of vertex types (for triangulation purposes)
     *
     * A vertex type is classified by the area spanned by this vertex and its adjacent
     * neighbors.  If the interior angle is outside of the polygon, it is CONCAVE.  If it
     * is inside the polygon, it is CONVEX.
     */
    enum VertexType {
        /** Vertex and its immediate neighbors form a concave polygon */
        CONCAVE    = -1,
        /** Vertex is colinear with its immediate neighbors */
        TANGENTIAL = 0,
        /** Vertex and its immediate neighbors form a convex polygon */
        CONVEX     = 1
    };

    /** The set of vertices to use in the calculation */
    std::vector<Vec2> _input;
    /** The classification type of each vertex in the triangulation */
    std::vector<VertexType> _types;
    /** A naive, intermediate triangulation.  The final triangulation builds from this */
    std::vector<unsigned short> _naive;
    /** The output results of the triangulation */
    std::vector<unsigned short> _output;
    /** Whether or not the calculation has been run */
    bool _calculated;

    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates a triangulator with no vertex data.
     */
    SimpleTriangulator() {}

    /**
     * Creates a triangulator with the given vertex data.
     *
     * The vertex data is copied.  The triangulator does not retain any
     * references to the original data.
     * 
     * @param points    The vertices to triangulate
     */
    SimpleTriangulator(const std::vector<Vec2>& points) : _calculated(false) { _input = points; }

    /**
     * Creates a triangulator with the given vertex data.
     *
     * The triangulator only uses the vertex data from the polygon.  It ignores
     * any existing indices.
     *
     * The vertex data is copied.  The triangulator does not retain any
     * references to the original data.
     *
     * @param poly    The vertices to triangulate
     */
    SimpleTriangulator(const Poly2& poly) :  _calculated(false) { _input = poly._vertices; }

    /**
     * Deletes this triangulator, releasing all resources.
     */
    ~SimpleTriangulator() {}

#pragma mark -
#pragma mark Initialization
    /**
     * Sets the vertex data for this triangulator..
     *
     * The triangulator only uses the vertex data from the polygon.  It ignores
     * any existing indices.
     *
     * The vertex data is copied.  The triangulator does not retain any
     * references to the original data.
     *
     * This method resets all interal data.  You will need to reperform the
     * calculation before accessing data.
     *
     * @param poly    The vertices to triangulate
     */
    void set(const Poly2& poly) {
        reset();
        _input = poly._vertices;
    }

    /**
     * Sets the vertex data for this triangulator..
     *
     * The vertex data is copied.  The triangulator does not retain any
     * references to the original data.
     *
     * This method resets all interal data.  You will need to reperform the
     * calculation before accessing data.
     *
     * @param points    The vertices to triangulate
     */
    void set(const std::vector<Vec2>& points) {
        reset();
        _input = points;
    }
    /**
     * Clears all internal data, but still maintains the initial vertex data.
     */
    void reset() {
        _calculated = false;
        _output.clear(); _naive.clear(); _types.clear();
    }
    
    /**
     * Clears all internal data, the initial vertex data.
     *
     * When this method is called, you will need to set a new vertices before
     * calling calculate.
     */
    void clear() {
        _calculated = false;
        _input.clear(); _output.clear(); _naive.clear(); _types.clear();
    }
    
#pragma mark -
#pragma mark Calculation
    /**
     * Performs a triangulation of the current vertex data.
     */
    void calculate();
    
#pragma mark -
#pragma mark Materialization
    /**
     * Returns a list of indices representing the triangulation.
     *
     * The indices represent positions in the original vertex list.  If you
     * have modified that list, these indices may no longer be valid.
     *
     * The triangulator does not retain a reference to the returned list; it
     * is safe to modify it.
     *
     * If the calculation is not yet performed, this method will return the
     * empty list.
     *
     * @return a list of indices representing the triangulation.
     */
    std::vector<unsigned short> getTriangulation() const;

    /**
     * Stores the triangulation indices in the given buffer.
     *
     * The indices represent positions in the original vertex list.  If you
     * have modified that list, these indices may no longer be valid.
     *
     * The indices will be appended to the provided vector. You should clear 
     * the vector first if you do not want to preserve the original data.
     *
     * If the calculation is not yet performed, this method will do nothing.
     *
     * @return the number of elements added to the buffer
     */
    size_t getTriangulation(std::vector<unsigned short>& buffer) const;

    /**
     * Returns a polygon representing the triangulation.
     *
     * The polygon contains the original vertices together with the new
     * indices defining a solid shape.  The triangulator does not maintain
     * references to this polygon and it is safe to modify it.
     *
     * If the calculation is not yet performed, this method will return the
     * empty polygon.
     *
     * @return a polygon representing the triangulation.
     */
    Poly2 getPolygon() const;
    
    /**
     * Stores the triangulation in the given buffer.
     *
     * This method will add both the original vertices, and the corresponding
     * indices to the new buffer.  If the buffer is not empty, the indices
     * will be adjusted accordingly. You should clear the buffer first if
     * you do not want to preserve the original data.
     *
     * If the calculation is not yet performed, this method will do nothing.
     *
     * @param buffer    The buffer to store the triangulated polygon
     *
     * @return a reference to the buffer for chaining.
     */
    Poly2* getPolygon(Poly2* buffer) const;

#pragma mark -
#pragma mark Internal Data Generation
private:
    /**
     * Classifies the vertex p2 according to its immediate neighbors.
     *
     * If the interior angle is outside of the polygon, it is CONCAVE.  If it is inside
     * the polygon, it is CONVEX.
     *
     * @param p1    The previous vertex
     * @param p2    The current vertex
     * @param p3    The next vertex
     *
     * @return the VertexType of vertex p2
     */
    VertexType computeSpannedAreaType(const Vec2& p1, const Vec2& p2, const Vec2& p3);
    
    /**
     * Returns true if the vertices are arranged clockwise about the interior.
     *
     * @param vertices  The array of vertices to check
     *
     * @return true if the vertices are arranged clockwise about the interior
     */
    bool areVerticesClockwise (const std::vector<Vec2>& vertices);
    
    /**
     * Removes an ear tip from the naive triangulation, adding it to the output.
     *
     * This function modifies both indices and types, removing the clipped triangle.
     * The triangle is defined by the given index and its immediate neighbors on
     * either side.
     *
     * @param earTipIndex  The index indentifying the triangle
     */
    void cutEarTip(int earTipIndex);
    
    /**
     * Returns true if the specified triangle is an eartip.
     *
     * The triangle is defined by the given index and its immediate neighbors on
     * either side.
     *
     * @param earTipIndex  The index indentifying the triangle
     *
     * @return true if the specified triangle is an eartip
     */
    bool isEarTip (int earTipIndex);
    
    /**
     * Returns a candidate ear-tip triangle
     *
     * The triangle is defined by the given index and its immediate neighbors on
     * either side.  A triangle is a candidate if the defining vertex is convex
     * or tangential.
     *
     * @return a candidate ear-tip triangle
     */
    int findEarTip();
    
    /**
     * Returns the classification for the vertex at the given index.
     *
     * A vertex type is classified by the area spanned by this vertex and its 
     * adjacent neighbors.  If the interior angle is outside of the polygon, it 
     * is CONCAVE.  If it is inside the polygon, it is CONVEX.
     *
     * @return the classification for the vertex at the given index
     */
    VertexType classifyVertex (int index);
    
    /**
     * Computes the indices for a triangulation of the given vertices.
     *
     * The indices are references to vertices in the given vertex array. Vertex 
     * 0 is the one with coordinates vertices[offset] and vertices[offset+1]. 
     * There will be three times as many indices as triangles.
     *
     * This function uses ear-clipping triangulation. The function culls all 
     * degenerate triangles from the result before returning.
     */
    void computeTriangulation();
    
    /**
     * Removes colinear vertices from the current triangulation.
     *
     * Because we permit tangential vertices as ear-clips, this triangulator 
     * will occasionally return colinear vertices.  This will crash OpenGL, so
     * we remove them.
     */
    void trimColinear();
    
};

}

#endif /* __CU_SIMPLE_TRIANGULATOR_H__ */


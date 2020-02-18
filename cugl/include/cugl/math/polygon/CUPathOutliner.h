//
//  CUPathOutliner.h
//  Cornell University Game Library (CUGL)
//
//  This module is a factory for outlining the boundary of a polygon.  While
//  this code is very straight-forward, previous semesters have shown that it
//  is best to factor this functionality out of Poly2.
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
//  Version: 6/22/16

#ifndef __CU_PATH_OUTLINER_H__
#define __CU_PATH_OUTLINER_H__

#include <cugl/math/CUPoly2.h>
#include <cugl/math/CUVec2.h>
#include <cugl/math/polygon/CUSimpleTriangulator.h>
#include <cugl/math/polygon/CUDelaunayTriangulator.h>
#include <vector>

namespace cugl {
    
/**! \public
 * This enum lists the types of path traversal that are supported. 
 *
 * This enumeration is used by both {@link PathOutliner} and {@link WireNode}.
 */
enum class PathTraversal : int {
    /** No traversal; the index list will be empty. */
    NONE = 0,
    /** Traverse the border, but do not close the ends. */
    OPEN = 1,
    /** Traverse the border, and close the ends. */
    CLOSED = 2,
    /** Traverse the individual triangles in the standard tesselation. */
    INTERIOR = 3
};
    
/**
 * This class is a factory for producing wireframe Poly2 objects from a set of vertices.
 *
 * This class provides three types of traversals: open, closed, and interior.
 * An interior traversal first triangulates the polygon, and then creates a
 * wireframe traversal of that triangulation.
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
class PathOutliner {
#pragma mark Values
private:
    /** The set of vertices to use in the calculation */
    std::vector<Vec2> _input;
    /** The output results of the path traversal */
    std::vector<unsigned short> _output;
    /** Whether or not the calculation has been run */
    bool _calculated;
    
    /** A triangulator for interior traversals */
    DelaunayTriangulator _triangulator;
    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates an outliner with no vertex data.
     */
    PathOutliner() {}
    
    /**
     * Creates an outliner with the given vertex data.
     *
     * The vertex data is copied.  The outliner does not retain any references 
     * to the original data.
     *
     * @param points    The vertices to outline
     */
    PathOutliner(const std::vector<Vec2>& points) : _calculated(false) { _input = points; }
    
    /**
     * Creates an outliner with the given vertex data.
     *
     * The outline only uses the vertex data from the polygon.  It ignores any
     * existing indices.
     *
     * The vertex data is copied.  The outliner does not retain any references
     * to the original data.
     *
     * @param poly    The vertices to outline
     */
    PathOutliner(const Poly2& poly) :  _calculated(false) { _input = poly._vertices; }
    
    /**
     * Deletes this outliner, releasing all resources.
     */
    ~PathOutliner() {}
    
#pragma mark -
#pragma mark Initialization
    /**
     * Sets the vertex data for this outliner.
     *
     * The outline only uses the vertex data from the polygon.  It ignores any
     * existing indices.
     *
     * The vertex data is copied.  The outliner does not retain any references
     * to the original data.
     *
     * This method resets all interal data.  You will need to reperform the
     * calculation before accessing data.
     *
     * @param poly    The vertices to outline
     */
    void set(const Poly2& poly) {
        reset();
        _input = poly._vertices;
    }
    
    /**
     * Sets the vertex data for this outliner.
     *
     * The vertex data is copied.  The outliner does not retain any references
     * to the original data.
     *
     * This method resets all interal data.  You will need to reperform the
     * calculation before accessing data.
     *
     * @param points    The vertices to outline
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
        _output.clear();
    }
    
    /**
     * Clears all internal data, the initial vertex data.
     *
     * When this method is called, you will need to set a new vertices before
     * calling calculate.
     */
    void clear() {
        _calculated = false;
        _input.clear(); _output.clear();
    }
    
#pragma mark -
#pragma mark Calculation
    /**
     * Performs a path calculation of the current vertex data.
     *
     * The path takes the provided traversal. See {@link PathTraversal} for 
     * more information.
     *
     * @param traversal The traversal type.
     */
    void calculate(PathTraversal traversal);
    
#pragma mark -
#pragma mark Materialization
    /**
     * Returns a list of indices representing the path outline.
     *
     * The indices represent positions in the original vertex list.  If you
     * have modified that list, these indices may no longer be valid.
     *
     * The outliner does not retain a reference to the returned list; it
     * is safe to modify it.
     *
     * If the calculation is not yet performed, this method will return the
     * empty list.
     *
     * @return a list of indices representing the path outline.
     */
    std::vector<unsigned short> getPath() const;
    
    /**
     * Stores the path outline indices in the given buffer.
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
    size_t getPath(std::vector<unsigned short>& buffer) const;
    
    /**
     * Returns a polygon representing the path outline.
     *
     * The polygon contains the original vertices together with the new
     * indices defining the wireframe path.  The outliner does not maintain
     * references to this polygon and it is safe to modify it.
     *
     * If the calculation is not yet performed, this method will return the
     * empty polygon.
     *
     * @return a polygon representing the path outline.
     */
    Poly2 getPolygon() const;
    
    /**
     * Stores the path outline in the given buffer.
     *
     * This method will add both the original vertices, and the corresponding
     * indices to the new buffer.  If the buffer is not empty, the indices
     * will be adjusted accordingly. You should clear the buffer first if
     * you do not want to preserve the original data.
     *
     * If the calculation is not yet performed, this method will do nothing.
     *
     * @param buffer    The buffer to store the outlined polygon
     *
     * @return a reference to the buffer for chaining.
     */
    Poly2* getPolygon(Poly2* buffer) const;
};
}

#endif /* __CU_PATH_OUTLINER_H__ */

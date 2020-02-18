//
//  CUDelaunayTriangulator.h
//  Cornell University Game Library (CUGL)
//
//  This module is a factory for a Delaunay triangulator.  Delaunay support is
//  not necessary for texture tesselation, but it is useful for applications
//  like HRTF support that require certain geometric guaranties on the
//  triangulation.  In addition, this triangulator can be used to extract the
//  Voronoi diagram as well.
//
//  Because math objects are intended to be on the stack, we do not provide
//  any shared pointer support in this class.
//
//  This implementation is based on the Bowyer-Watson algorithm detailed at
//
//      https://en.wikipedia.org/wiki/Bowyer–Watson_algorithm
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
//  Version: 2/8/18
//
#ifndef __CU_DELAUNAY_TRIANGULATOR_H__
#define __CU_DELAUNAY_TRIANGULATOR_H__

#include <cugl/math/CUPoly2.h>
#include <cugl/math/CUVec2.h>
#include <cugl/math/CUVec3.h>
#include <vector>
#include <unordered_map>

namespace cugl {

/**
 * This class is a factory for producing solid Poly2 objects from a set of vertices.
 *
 * For all but the simplist of shapes, it is important to have a triangulator
 * that can divide up the polygon into triangles for drawing.  This is triangulator
 * uses the Bowyer-Watson algorithm to perform a Delaunay triangulation.  This
 * triangulation minimizes sliver triangles, which are common with ear clipping
 * algorithms {@see SimpleTriangulator}.
 *
 * Because the Voronoi diagram is the dual of the Delaunay triangulation, this
 * factory can be used to extract this diagram. The Voronoi diagram can be
 * extracted as either a wireframe or a collection of regions.
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
class DelaunayTriangulator {
private:
#pragma mark Vertex
    /**
     * A vertex indexed for triangulation
     *
     * This data structure allows us to sort the vertices lexicographically,
     * for improved performance.  The vertex keeps an index as a back pointer
     * to its original position in the vertex list.
     */
    class Vertex {
    public:
        /** The vertex location */
        Vec2 point;
        /** The vertex index in the input list */
        Sint64 index;

        /**
         * Creates a vertex at the origin with index -1.
         *
         * This constructor is necessary to inline the hash support.
         */
        Vertex() : index(-1) {}

        /**
         * Creates a vertex with the given position and list index.
         *
         * @param p The vertex position
         * @param i The index of the vertex in the initial list
         */
        Vertex(const Vec2& p, Sint64 i);

        /**
         * Returns true if this vertex is equal to the given vertex.
         *
         * This comparison compares the coordinates (and inde) piecewise.
         *
         * @param v The vertex to compare against.
         *
         * @return True if this vertex is equal to the given vertex.
         */
        bool operator==(const Vertex& v) const;

        /**
         * Returns true if this vertex is not equal to the given vertex.
         *
         * This comparison compares the coordinates (and inde) piecewise.
         *
         * @param v The vertex to compare against.
         *
         * @return True if this vertex is not equal to the given vertex.
         */
        bool operator!=(const Vertex& v) const { return !(*this == v); }

        /**
         * Returns true if this vertex is less than the given vertex.
         *
         * This comparison uses the lexicographical order.  In the case of
         * ties, it compares against the index.
         *
         * @param v The vertex to compare against.
         *
         * @return True if this vertex is less than the given vertex.
         */
        bool operator<(const Vertex& v) const;
        
        /**
         * Returns true if this vertex is greater than the given vertex.
         *
         * This comparison uses the lexicographical order.  In the case of
         * ties, it compares against the index.
         *
         * @param v The vertex to compare against.
         *
         * @return True if this vertex is greater than the given vertex.
         */
        bool operator> (const Vertex& v) const { return v < *this; }
        
        /**
         * Returns true if this vertex is less than or equal to the given vertex.
         *
         * This comparison uses the lexicographical order.  In the case of
         * ties, it compares against the index.
         *
         * @param v The vertex to compare against.
         *
         * @return True if this vertex is less than or equal to the given vertex.
         */
        bool operator<=(const Vertex& v) const { return !(v < *this); }
        
        /**
         * Returns true if this vertex is greater than or equal to the given vertex.
         *
         * This comparison uses the lexicographical order.  In the case of
         * ties, it compares against the index.
         *
         * @param v The vertex to compare against.
         *
         * @return True if this vertex is greater than or equal to the given vertex.
         */
        bool operator>=(const Vertex& v) const { return !(*this < v); }

        /**
         * Returns the hash code for the specified vertex.
         *
         * This function is used to allow us to to inline the hash code into
         * the class definition (which is not typical C++).  This effectively
         * works as a static function.
         *
         * @param v The vertex to hash
         *
         * @return the hash code for the specified vertex.
         */
        size_t operator()(const Vertex& v) const;

    };
    
    
#pragma mark -
#pragma mark Edge
    /**
     * An internal representation of an edge.
     *
     * This class represents a single triangle in a triangulation. The Delaunay
     * triangulation requires us to preserve a lot more information than a
     * simple ear clipping algorithm.  This representation of a triangulation
     * allows us to extract either the triangulation or the Voronoi diagram
     * with no additional computation.
     */
    class Edge {
    public:
        /** The first edge vertex */
        Vertex v1;
        /** The second edge vertex */
        Vertex v2;
        
        /**
         * Creates a degnerate edge at the origin.
         *
         * This constructor is necessary to inline the hash support.
         */
        Edge() {}
        
        /**
         * Creates an edge with the given the vertices.
         *
         * This constructor does NOT check for degnerate edges.
         *
         * @param p1    The first vertex
         * @param p2    The second vertex
         */
        Edge(const Vertex& p1, const Vertex& p2);
        
        /**
         * Returns true if this edge is equal to the given edge.
         *
         * This comparison compares the vertices piecewise.  However, it
         * does recognize flipped edges.
         *
         * @param t The edge to compare against.
         *
         * @return True if this edge is equal to the given edge.
         */
        bool operator==(const Edge& t) const;
        
        /**
         * Returns true if this edge is not equal to the given edge.
         *
         * This comparison compares the vertices piecewise.  However, it
         * does recognize flipped edges.
         *
         * @param t The edge to compare against.
         *
         * @return True if this edge is not equal to the given edge.
         */
        bool operator!=(const Edge& t) const { return !(*this == t); }
        
        /**
         * Returns the hash code for the specified edge.
         *
         * This function is used to allow us to to inline the hash code into
         * the class definition (which is not typical C++).  This effectively
         * works as a static function.
         *
         * @param t The edge to hash
         *
         * @return the hash code for the specified edge.
         */
        size_t operator()(const Edge& t) const;
        
        /**
         * Returns true if the given vertex is one of the two in this edge
         *
         * @param v The vertex to check.
         *
         * @return True if the given vertex is one of the two in this edge
         */
        bool hasVertex(const Vec2& v) const;

        /**
         * Returns true if this edge is degenerate.
         *
         * A degenerate edge is one where the edges are identical
         *
         * @return True if this edge is degenerate.
         */
        bool isDegenerate() const;
    };
    
#pragma mark -
#pragma mark Triangle
    /**
     * An internal representation of a triangle.
     *
     * This class represents a single triangle in a triangulation. The Delaunay
     * triangulation requires us to preserve a lot more information than a
     * simple ear clipping algorithm.  This representation of a triangulation
     * allows us to extract either the triangulation or the Voronoi diagram
     * with no additional computation.
     */
    class Triangle {
    public:
        /** The first triangle vertex */
        Vertex v1;
        /** The second triangle vertex */
        Vertex v2;
        /** The third triangle vertex */
        Vertex v3;
        /** Whether the triangle has been marked as bad (for removal) */
        bool isbad;
        
        /**
         * Creates a degnerate triangle at the origin.
         *
         * This constructor is necessary to inline the hash support.
         */
        Triangle() : isbad(true) {}
        
        /**
         * Creates a triangle with the given the vertices.
         *
         * This constructor does NOT check for degnerate triangles.
         *
         * @param p1    The first vertex
         * @param p2    The second vertex
         * @param p3    The third vertex
         */
        Triangle(const Vertex& p1, const Vertex& p2, const Vertex& p3);
        
        /**
         * Returns true if this triangle is equal to the given triangle.
         *
         * This comparison compares the vertices piecewise.  However, it
         * does recognize both rotated and flipped triangles.
         *
         * @param t The triangle to compare against.
         *
         * @return True if this triangle is equal to the given triangle.
         */
        bool operator==(const Triangle& t);
        
        /**
         * Returns true if this triangle is not equal to the given triangle.
         *
         * This comparison compares the vertices piecewise.  However, it
         * does recognize both rotated and flipped triangles.
         *
         * @param t The triangle to compare against.
         *
         * @return True if this triangle is not equal to the given triangle.
         */
        bool operator!=(const Triangle& t) { return !(*this == t); }
        
        /**
         * Returns the hash code for the specified triangle.
         *
         * This function is used to allow us to to inline the hash code into
         * the class definition (which is not typical C++).  This effectively
         * works as a static function.
         *
         * @param t The triangle to hash
         *
         * @return the hash code for the specified triangle.
         */
        size_t operator()(const Triangle& t) const;

        /**
         * Returns true if the given vertex is one of the three in this triangle
         *
         * @param v The vertex to check.
         *
         * @return True if the given vertex is one of the three in this triangle
         */
        bool hasVertex(const Vec2& v);

        /**
         * Returns the barycentric coordinates for a point relative to the triangle.
         *
         * @param point The point to convert
         *
         * @return the barycentric coordinates for a point relative to the triangle.
         */
        Vec3 getBarycentric(const Vec2& point) const;

        /**
         * Returns the center of the circle circumscribed by this triangle
         *
         * @return The center of the circle circumscribed by this triangle
         */
        Vec2 getCircleCenter() const;
        
        /**
         * Returns the radius of the circle circumscribed by this triangle
         *
         * @return The radius of the circle circumscribed by this triangle
         */
        float getCircleRadius() const;

        /**
         * Returns true if the point is in the circle circumscribed by this triangle
         *
         * @param point The point to check
         *
         * @return True if the point is in the circle circumscribed by this triangle
         */
        bool containsInCircle(const Vec2& point) const;
        
        /**
         * Marks this triangle as bad, removing it from the triangulation
         *
         * @param bad Whether the triangle is bad
         */
        void setBad(bool bad);

        /**
         * Returns true if this triangle as bad and should be removed.
         *
         * @return True if this triangle as bad and should be removed.
         */
        bool isBad() const;
        
        /**
         * Returns true if this triangle is degenerate.
         *
         * A degenerate triangle is one where all of the vertices are colinear.
         *
         * @return True if this triangle is degenerate.
         */
        bool isDegenerate() const;
        
        /**
         * Returns true if this is an exterior triangle in the triangulation
         *
         * An exterior triangle has a vertex with negative index.
         *
         * @return true if this is an exterior triangle in the triangulation
         */
        bool isExterior() const;
    };
    
#pragma mark -
#pragma mark Delaunay
    /** The set of vertices to use in the calculation */
    std::vector<Vec2> _input;
    /** The final Delaunay triangulation (without external triangles) */
    std::vector<Triangle> _output;
    /** The dual points for the Voronoi diagram */
    std::vector<Vec2> _dual;
    /** The edges of the corresponding Voronoi diagram */
    std::vector<std::vector<Edge>> _voronoi;
    /** Whether or not the triangulation has been computed */
    bool _calculated;
    /** Whether or not the Voronoi has been computed */
    bool _dualated;

public:
    /**
     * Creates a triangulator with no vertex data.
     */
    DelaunayTriangulator() {}
    
    /**
     * Creates a triangulator with the given vertex data.
     *
     * The vertex data is copied.  The triangulator does not retain any
     * references to the original data.
     *
     * @param points    The vertices to triangulate
     */
    DelaunayTriangulator(const std::vector<Vec2>& points) : _calculated(false) { _input = points; }
    
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
    DelaunayTriangulator(const Poly2& poly) :  _calculated(false) { _input = poly._vertices; }
    
    /**
     * Deletes this triangulator, releasing all resources.
     */
    ~DelaunayTriangulator() {}
    
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
        _calculated = false; _dualated = false;
        _output.clear(); _voronoi.clear();
    }
    
    /**
     * Clears all internal data, the initial vertex data.
     *
     * When this method is called, you will need to set a new vertices before
     * calling calculate.
     */
    void clear() {
        _calculated = false; _dualated = false;
        _input.clear(); _output.clear(); _voronoi.clear();
    }
    
#pragma mark Calculation
    /**
     * Performs a triangulation of the current vertex data.
     *
     * This method does not automatically calculate the Voronoi diagram. Call
     * {@link calculateDual()} to do that.
     */
    void calculate();
    
    /**
     * Creates a Voronoi diagram from the current vertex data.
     *
     * If the method {@link calculate()} has not been called, this method will
     * call that first.  Then it will construct the Voronoi diagram.
     */
    void calculateDual();
    
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

    
#pragma mark Voronoization
    /**
     * Returns the Voronoi diagram as a list of polygons
     *
     * Each polygon represents a single Voronoi cell. A Voronoi cell is a
     * polygon whose vertices are the boundary of the cell. The returned Poly2
     * object does not have indices and is free to be triangulated later.
     * The triangulator does not maintain references to these polygons and it
     * is safe to modify them.
     *
     * Each Voronoi cell corresponds to a vertex in the original triangulation.
     * The cells are returned in the same order as the vertices.
     *
     * If the Voronoi diagram is not calculated, this method will do nothing.
     *
     * @return the Voronoi diagram as a list of polygons
     */
    std::vector<Poly2> getVoronoi() const;

    /**
     * Returns the Voronoi cell for the given index
     *
     * A Voronoi cell is a polygon whose vertices are the boundary of the cell.
     * The returned Poly2 object does not have indices and is free to be
     * triangulated later. The triangulator does not maintain references to
     * this polygon and it is safe to modify it.
     *
     * The index corresponds to the vertex in the original triangulation.
     * If the Voronoi diagram is not calculated, this method will do nothing.
     *
     * @param index The index of the vertex generating the cell
     *
     * @return he Voronoi cell for the given index
     */
    Poly2 getVoronoiCell(size_t index) const;
    
    /**
     * Stores the Voronoi cell in the given buffer.
     *
     * A Voronoi cell is a polygon whose vertices are the boundary of the cell.
     * This method will theses vertices to the new buffer. You should clear the
     * buffer first if you do not want to preserve the original data. No indices
     * will be added to the buffer, and it is free to be triangulated later.
     *
     * The index corresponds to the vertex in the original triangulation.
     * If the Voronoi diagram is not calculated, this method will do nothing.
     *
     * @param index     The index of the vertex generating the cell
     * @param buffer    The buffer to store the Voronoi cell
     *
     * @return a reference to the buffer for chaining.
     */
    Poly2* getVoronoiCell(size_t index, Poly2* buffer) const;
    
    /**
     * Returns a polygon with a wireframe of the Voronoi diagram.
     *
     * The polygon contains the vertices of the Voronoi diagram together with
     * the new indices connecting the vertices as edges. The triangulator does
     * not maintain references to this polygon and it is safe to modify it.
     *
     * If the Voronoi diagram is not calculated, this method will return the
     * empty polygon.
     *
     * @return a polygon with a wireframe of the Voronoi diagram.
     */
    Poly2 getVoronoiFrame() const;
    
    /**
     * Stores a wireframe of the Voronoi diagram in the given buffer.
     *
     * This method will add both the Voronoi vertices, and the corresponding
     * indices to the new buffer.  If the buffer is not empty, the indices
     * will be adjusted accordingly. You should clear the buffer first if
     * you do not want to preserve the original data.
     *
     * If the Voronoi diagram is not calculated, this method will return the
     * empty polygon.
     *
     * @param buffer    The buffer to store the wireframe of the Voronoi diagram
     *
     * @return a reference to the buffer for chaining.
     */
    Poly2* getVoronoiFrame(Poly2* buffer) const;

#pragma mark -
#pragma mark Internal Data Generation
    /**
     * Returns the bounding box for the input vertices
     *
     * @return the bounding box for the input vertices
     */
    Rect getBoundingBox() const;
    
    /**
     * Calculates the Delaunay triangulation.
     *
     * The provided bounding box guides the initial super triangle.
     *
     * @param rect the bounding box for the input vertices
     */
    void computeDelaunay(const Rect& rect);

    /**
     * Calculates the Voronoi diagram.
     *
     * The provided bounding box guides the boundary edges.
     *
     * @param rect the bounding box for the input vertices
     */
    void computeVoronoi(const Rect& rect);
    
    /**
     * Sorts the edges of the Voronoi cell so that they are adjacent.
     *
     * In addition to sorting the edges, this method fills in any missing
     * edges on the outside of the bounding box.
     *
     & @param index the index of the Voronoi cell
     * @param rect  the bounding box for the input vertices
     */
    void sortCell(size_t index, const Rect& rect);
};
  
    
}

#endif /* __CU_DELAUNAY_TRIANGULATOR_H__ */

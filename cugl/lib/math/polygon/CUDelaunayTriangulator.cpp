//
//  CUDelaunayTriangulator.cpp
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
#include <cugl/math/polygon/CUDelaunayTriangulator.h>
#include <cugl/util/CUDebug.h>
#include <unordered_set>
#include <iterator>

// Taken from Boost
#define HASH_CONST  0x9e3779b9
#define EPSILON_ADJ 1.000001

using namespace cugl;

/**
 * Returns the point of intersection of a ray with the bounding box.
 *
 * This function extends the ray with direction dir and anchor start, until
 * it intersects with one of the edges of the bounding box.  If there is
 * no intersection, it will return the origin (0,0).
 *
 * @param start the start anchor of the ray
 * @param dir   the direction vector of the ray
 * @param rect  the bounding box to intersect
 *
 * @return the point of intersection of a ray with the bounding box
 */
Vec2 get_intersection(const Vec2& start, const Vec2& dir, const Rect& box) {
    float s = -1;
    float t = -1;
    
    Vec2::doesLineIntersect(start,start+dir,box.origin,box.origin+Vec2(box.size.width,0),&s,&t);
    if (0 <= t && t <= 1 && s >= 0) {
        return box.origin+Vec2(box.size.width*t,0);
    }
    
    Vec2::doesLineIntersect(start,start+dir,box.origin,box.origin+Vec2(0,box.size.height),&s,&t);
    if (0 <= t && t <= 1 && s >= 0) {
        return box.origin+Vec2(0,box.size.height*t);
    }
    
    Vec2::doesLineIntersect(start,start+dir,box.origin+Vec2(box.size.width,0),box.origin+box.size,&s,&t);
    if (0 <= t && t <= 1 && s >= 0) {
        return box.origin+Vec2(box.size.width,box.size.height*t);
    }
    
    Vec2::doesLineIntersect(start,start+dir,box.origin+Vec2(0,box.size.height),box.origin+box.size,&s,&t);
    if (0 <= t && t <= 1 && s >= 0) {
        return box.origin+Vec2(box.size.width*t,box.size.height);
    }
    
    return Vec2(0,0);
}

/**
 * Returns the corner vertex between two boundary points.
 *
 * This function assumes the boundary points start and end are on two different
 * but adjacent edges of the bounding box.
 *
 * @param start the first boundary point
 * @param end   the second boundary point
 * @param rect  the bounding box to check
 *
 * @return he corner vertex between two boundary points.
 */
Vec2 get_interior(const Vec2& start, const Vec2& end, const Rect& box) {
    Vec2 result;
    if (start.x == box.origin.x || end.x == box.origin.x) {
        result.x = box.origin.x;
    } else {
        result.x = box.origin.x+box.size.width;
    }
    if (start.y == box.origin.y || end.y == box.origin.y) {
        result.y = box.origin.y;
    } else {
        result.y = box.origin.y+box.size.height;
    }
    return result;
}

#pragma mark -
#pragma mark Vertex
/**
 * Creates a vertex with the given position and list index.
 *
 * @param p The vertex position
 * @param i The index of the vertex in the initial list
 */
DelaunayTriangulator::Vertex::Vertex(const Vec2& p, Sint64 i) {
    point = p; index = i;
}

/**
 * Returns true if this vertex is equal to the given vertex.
 *
 * This comparison compares the coordinates (and inde) piecewise.
 *
 * @param v The vertex to compare against.
 *
 * @return True if this vertex is equal to the given vertex.
 */
bool DelaunayTriangulator::Vertex::operator==(const Vertex& v) const {
    return v.point == point && index == v.index;
}

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
bool DelaunayTriangulator::Vertex::operator<(const Vertex& v) const {
    return (v.point == point ? index < v.index : point < v.point);
}

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
size_t DelaunayTriangulator::Vertex::operator()(const Vertex& v) const {
    size_t one = std::hash<float>{}(v.point.x);
    size_t two = std::hash<float>{}(v.point.y);
    size_t tre = std::hash<Sint64>{}(v.index);
    one ^= two + HASH_CONST + (one<<6) + (one>>2);
    one ^= tre + HASH_CONST + (one<<6) + (one>>2);
    return one;
}

#pragma mark -
#pragma mark Edge
/**
 * Creates an edge with the given the vertices.
 *
 * This constructor does NOT check for degnerate edges.
 *
 * @param p1    The first vertex
 * @param p2    The second vertex
 */
DelaunayTriangulator::Edge::Edge(const Vertex& p1, const Vertex& p2) {
    v1 = p1; v2 = p2;
}

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
bool DelaunayTriangulator::Edge::operator==(const Edge& t) const {
    return (v1 == t.v1 && v2 == t.v2) || (v1 == t.v2 && v2 == t.v1);
}

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
size_t DelaunayTriangulator::Edge::operator()(const Edge& t) const {
    size_t one = v1(v1);
    size_t two = v2(v2);
    return one+two;
}

/**
 * Returns true if the given vertex is one of the two in this edge
 *
 * @param v The vertex to check.
 *
 * @return True if the given vertex is one of the two in this edge
 */
bool DelaunayTriangulator::Edge::hasVertex(const Vec2& v) const {
    return v == v1.point || v == v2.point;
}

/**
 * Returns true if this edge is degenerate.
 *
 * A degenerate edge is one where the edges are identical
 *
 * @return True if this edge is degenerate.
 */
bool DelaunayTriangulator::Edge::isDegenerate() const {
    return v1.point == v2.point;
}


#pragma mark -
#pragma mark Triangle
/**
 * Creates a triangle with the given the vertices.
 *
 * This constructor does NOT check for degnerate triangles.
 *
 * @param p1    The first vertex
 * @param p2    The second vertex
 * @param p3    The third vertex
 */
DelaunayTriangulator::Triangle::Triangle(const Vertex& p1, const Vertex& p2, const Vertex& p3) {
    v1 = p1; v2 = p2; v3 = p3;
    isbad = false;
}

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
bool DelaunayTriangulator::Triangle::operator==(const Triangle& t) {
    if (v1 == t.v1) {
        return (v2 == t.v2 && v3 == t.v3) || (v2 == t.v3 && v3 == t.v2);
    } else if (v1 == t.v2) {
        return (v2 == t.v1 && v3 == t.v3) || (v2 == t.v3 && v3 == t.v1);
    } else if (v1 == t.v3) {
        return (v2 == t.v1 && v3 == t.v2) || (v2 == t.v2 && v3 == t.v1);
    }
    
    return false;
}

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
size_t DelaunayTriangulator::Triangle::operator()(const Triangle& t) const {
    size_t one = v1(v1);
    size_t two = v2(v2);
    size_t tre = v3(v3);
    one ^= two + HASH_CONST + (one<<6) + (one>>2);
    one ^= tre + HASH_CONST + (one<<6) + (one>>2);
    return one;
}

/**
 * Returns true if the given vertex is one of the three in this triangle
 *
 * @param v The vertex to check.
 *
 * @return True if the given vertex is one of the three in this triangle
 */
bool DelaunayTriangulator::Triangle::hasVertex(const Vec2& v) {
    return v == v1.point || v == v2.point || v == v3.point;
}

/**
 * Returns the barycentric coordinates for a point relative to the triangle.
 *
 * @param point The point to convert
 *
 * @return the barycentric coordinates for a point relative to the triangle.
 */
Vec3 DelaunayTriangulator::Triangle::getBarycentric(const Vec2& point) const {
    float det = (v2.point.y-v3.point.y)*(v1.point.x-v3.point.x)+(v3.point.x-v2.point.x)*(v1.point.y-v3.point.y);

    Vec3 result;
    result.x = (v2.point.y-v3.point.y)*(point.x-v3.point.x)+(v3.point.x-v2.point.x)*(point.y-v3.point.y);
    result.y = (v3.point.y-v1.point.y)*(point.x-v3.point.x)+(v1.point.x-v3.point.x)*(point.y-v3.point.y);
    result.x /= det;
    result.y /= det;
    result.z = 1 - result.x - result.y;
    return result;
}

/**
 * Returns the center of the circle circumscribed by this triangle
 *
 * @return The center of the circle circumscribed by this triangle
 */
Vec2 DelaunayTriangulator::Triangle::getCircleCenter() const {
    float ab = (v1.point.x * v1.point.x) + (v1.point.y * v1.point.y);
    float cd = (v2.point.x * v2.point.x) + (v2.point.y * v2.point.y);
    float ef = (v3.point.x * v3.point.x) + (v3.point.y * v3.point.y);

    Vec2 result;
    result.x  = (ab * (v3.point.y - v2.point.y) + cd * (v1.point.y - v3.point.y) + ef * (v2.point.y - v1.point.y));
    result.x /= (v1.point.x * (v3.point.y - v2.point.y) + v2.point.x * (v1.point.y - v3.point.y) + v3.point.x * (v2.point.y - v1.point.y));
    result.x /= 2.0f;

    result.y  = (ab * (v3.point.x - v2.point.x) + cd * (v1.point.x - v3.point.x) + ef * (v2.point.x - v1.point.x));
    result.y /= (v1.point.y * (v3.point.x - v2.point.x) + v2.point.y * (v1.point.x - v3.point.x) + v3.point.y * (v2.point.x - v1.point.x));
    result.y /= 2.0f;

    return result;
}

/**
 * Returns the radius of the circle circumscribed by this triangle
 *
 * @return The radius of the circle circumscribed by this triangle
 */
float DelaunayTriangulator::Triangle::getCircleRadius() const {
    Vec2 center = getCircleCenter();
    return center.distance(v1.point);
}

/**
 * Returns true if the point is in the circle circumscribed by this triangle
 *
 * @param point The point to check
 *
 * @return True if the point is in the circle circumscribed by this triangle
 */
bool DelaunayTriangulator::Triangle::containsInCircle(const Vec2& point) const {
    Vec2 center  = getCircleCenter();
    float radius2 = center.distanceSquared(v1.point);
    
    return center.distanceSquared(point) < radius2;
}

/**
 * Marks this triangle as bad, removing it from the triangulation
 *
 * @param bad Whether the triangle is bad
 */
void DelaunayTriangulator::Triangle::setBad(bool bad) {
    isbad = bad;
}

/**
 * Returns true if this triangle as bad and should be removed.
 *
 * @return True if this triangle as bad and should be removed.
 */
bool DelaunayTriangulator::Triangle::isBad() const {
    return isbad;
}

/**
 * Returns true if this triangle is degenerate.
 *
 * A degenerate triangle is one where all of the vertices are colinear.
 *
 * @return True if this triangle is degenerate.
 */
bool DelaunayTriangulator::Triangle::isDegenerate() const {
    float det = (v1.point.x - v3.point.x) * (v2.point.y - v3.point.y);
    det -= (v1.point.y - v3.point.y) * (v2.point.x - v3.point.x);
    return det == 0;
}

/**
 * Returns true if this is an exterior triangle in the triangulation
 *
 * An exterior triangle has a vertex with negative index.
 *
 * @return true if this is an exterior triangle in the triangulation
 */
bool DelaunayTriangulator::Triangle::isExterior() const {
    return v1.index < 0 || v2.index < 0 || v3.index < 0;
}

#pragma mark -
#pragma mark Delaunay
/**
 * Performs a triangulation of the current vertex data.
 *
 * This method does not automatically calculate the Voronoi diagram. Call
 * {@link calculateDual()} to do that.
 */
void DelaunayTriangulator::calculate() {
    reset();
    if (_input.size() == 0) { return; }

    // Compute the bounding box and super triangle
    Rect box = getBoundingBox();
    computeDelaunay(box);
    _calculated = true;
}

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
std::vector<unsigned short> DelaunayTriangulator::getTriangulation() const {
    std::vector<unsigned short> result;
    if (_calculated) {
        for(auto it = _output.begin(); it != _output.end(); ++it) {
            result.push_back(it->v1.index);
            result.push_back(it->v2.index);
            result.push_back(it->v3.index);
        }
    }
    return result;
}

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
size_t DelaunayTriangulator::getTriangulation(std::vector<unsigned short>& buffer) const {
    if (_calculated) {
        buffer.reserve(buffer.size()+_output.size()*3);
        for(auto it = _output.begin(); it != _output.end(); ++it) {
            buffer.push_back(it->v1.index);
            buffer.push_back(it->v2.index);
            buffer.push_back(it->v3.index);
        }
        return _output.size()*3;
    }
    return 0;
}

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
Poly2 DelaunayTriangulator::getPolygon() const {
    Poly2 poly;
    if (_calculated) {
        poly._vertices = _input;
        poly._indices.reserve(_output.size()*3);
        for(auto it = _output.begin(); it != _output.end(); ++it) {
            poly._indices.push_back(it->v1.index);
            poly._indices.push_back(it->v2.index);
            poly._indices.push_back(it->v3.index);
        }
        poly._type = Poly2::Type::SOLID;
        poly.computeBounds();
    }
    return poly;
}

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
Poly2* DelaunayTriangulator::getPolygon(Poly2* buffer) const {
    CUAssertLog(buffer, "Destination buffer is null");
    if (_calculated) {
        if (buffer->_vertices.size() == 0) {
            buffer->_vertices = _input;
            buffer->_indices.reserve(_output.size()*3);
            for(auto it = _output.begin(); it != _output.end(); ++it) {
                buffer->_indices.push_back(it->v1.index);
                buffer->_indices.push_back(it->v2.index);
                buffer->_indices.push_back(it->v3.index);
            }
        } else {
            int offset = (int)buffer->_vertices.size();
            buffer->_vertices.reserve(offset+_input.size());
            std::copy(_input.begin(),_input.end(),std::back_inserter(buffer->_vertices));
            
            buffer->_indices.reserve(buffer->_indices.size()+_output.size()*3);
            for(auto it = _output.begin(); it != _output.end(); ++it) {
                buffer->_indices.push_back(offset+it->v1.index);
                buffer->_indices.push_back(offset+it->v2.index);
                buffer->_indices.push_back(offset+it->v3.index);
            }
        }
        buffer->_type = Poly2::Type::SOLID;
        buffer->computeBounds();
    }
    return buffer;
}

#pragma mark -
#pragma mark Voronoi
/**
 * Creates a Voronoi diagram from the current vertex data.
 *
 * If the method {@link calculate()} has not been called, this method will
 * call that first.  Then it will construct the Voronoi diagram.
 */
void DelaunayTriangulator::calculateDual() {
    if (!_calculated) {
        calculate();
    }
    
    // Compute the bounding box and super triangle
    Rect box = getBoundingBox();
    computeVoronoi(box);
    _dualated = true;
}

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
 * If the calculation is not yet performed, this method will do nothing.
 *
 * @return the Voronoi diagram as a list of polygons
 */
std::vector<Poly2> DelaunayTriangulator::getVoronoi() const {
    std::vector<Poly2> result;
    if (_dualated) {
        for(size_t ii = 0; ii < _voronoi.size(); ii++) {
            result.push_back(getVoronoiCell(ii));
        }
    }
    return result;
}

/**
 * Returns the Voronoi cell for the given index
 *
 * A Voronoi cell is a polygon whose vertices are the boundary of the cell.
 * The returned Poly2 object does not have indices and is free to be
 * triangulated later. The triangulator does not maintain references to
 * this polygon and it is safe to modify it.
 *
 * The index corresponds to the vertex in the original triangulation.
 * If the calculation is not yet performed, this method will do nothing.
 *
 * @param index The index of the vertex generating the cell
 *
 * @return he Voronoi cell for the given index
 */
Poly2 DelaunayTriangulator::getVoronoiCell(size_t index) const {
    Poly2 poly;
    if (_dualated) {
        poly._vertices.reserve(_voronoi[index].size());
        for(auto it = _voronoi[index].begin(); it != _voronoi[index].end(); ++it) {
            poly._vertices.push_back(it->v1.point);
        }
    }
    return poly;
}

/**
 * Stores the Voronoi cell in the given buffer.
 *
 * A Voronoi cell is a polygon whose vertices are the boundary of the cell.
 * This method will theses vertices to the new buffer. You should clear the
 * buffer first if you do not want to preserve the original data. No indices
 * will be added to the buffer, and it is free to be triangulated later.
 *
 * The index corresponds to the vertex in the original triangulation.
 * If the calculation is not yet performed, this method will do nothing.
 *
 * @param index     The index of the vertex generating the cell
 * @param buffer    The buffer to store the Voronoi cell
 *
 * @return a reference to the buffer for chaining.
 */
Poly2* DelaunayTriangulator::getVoronoiCell(size_t index, Poly2* buffer) const {
    CUAssertLog(buffer, "Destination buffer is null");
    CUAssertLog(index < _voronoi.size(), "Voronoi cell %lu is out of range", index);
    if (_dualated) {
        int offset = (int)buffer->_vertices.size();
        buffer->_vertices.reserve(offset+_voronoi[index].size());
        for(auto it = _voronoi[index].begin(); it != _voronoi[index].end(); ++it) {
            buffer->_vertices.push_back(it->v1.point);
        }
    }
    return buffer;
}

/**
 * Returns a polygon with a wireframe of the Voronoi diagram.
 *
 * The polygon contains the vertices of the Voronoi diagram together with
 * the new indices connecting the vertices as edges. The triangulator does
 * not maintain references to this polygon and it is safe to modify it.
 *
 * If the calculation is not yet performed, this method will return the
 * empty polygon.
 *
 * @return a polygon with a wireframe of the Voronoi diagram.
 */
Poly2 DelaunayTriangulator::getVoronoiFrame() const {
    Poly2 poly;
    if (_dualated) {
        poly._vertices = _dual;
        poly._indices.reserve(_voronoi.size()*2);
        for(auto it = _voronoi.begin(); it != _voronoi.end(); ++it) {
            for(auto jt = it->begin(); jt != it->end(); ++jt) {
                poly._indices.push_back(jt->v1.index);
                poly._indices.push_back(jt->v2.index);
            }
        }
        poly._type = Poly2::Type::PATH;
        poly.computeBounds();
    }
    return poly;
}

/**
 * Stores a wireframe of the Voronoi diagram in the given buffer.
 *
 * This method will add both the Voronoi vertices, and the corresponding
 * indices to the new buffer.  If the buffer is not empty, the indices
 * will be adjusted accordingly. You should clear the buffer first if
 * you do not want to preserve the original data.
 *
 * If the calculation is not yet performed, this method will return the
 * empty polygon.
 *
 * @param buffer    The buffer to store the wireframe of the Voronoi diagram
 *
 * @return a reference to the buffer for chaining.
 */
Poly2* DelaunayTriangulator::getVoronoiFrame(Poly2* buffer) const {
    CUAssertLog(buffer, "Destination buffer is null");
    if (_dualated) {
        int offset = (int)buffer->_vertices.size();
        buffer->_vertices.reserve(offset+_dual.size());
        std::copy(_dual.begin(),_dual.end(),std::back_inserter(buffer->_vertices));

        buffer->_indices.reserve(buffer->_indices.size()+_dual.size()*2);
        for(auto jt = _voronoi.begin(); jt != _voronoi.end(); ++jt) {
            for(auto kt = jt->begin(); kt != jt->end(); ++kt) {
                buffer->_indices.push_back(offset+kt->v1.index);
                buffer->_indices.push_back(offset+kt->v2.index);
            }
        }
        buffer->_type = Poly2::Type::PATH;
        buffer->computeBounds();
    }
    return buffer;
}


#pragma mark -
#pragma mark Internal Data Generation
/**
 * Returns the bounding box for the input vertices
 *
 * @return the bounding box for the input vertices
 */
Rect DelaunayTriangulator::getBoundingBox() const {
    CUAssertLog(_input.size() > 0, "Calculating bounding box on empty input");
    
    float minX = _input[0].x;
    float minY = _input[0].y;
    float maxX = minX;
    float maxY = minY;
    
    for(auto it = _input.begin(); it != _input.end(); ++it) {
        if (it->x < minX) minX = it->x;
        if (it->y < minY) minY = it->y;
        if (it->x > maxX) maxX = it->x;
        if (it->y > maxY) maxY = it->y;
    }
    
    return Rect(Vec2(minX,minY),Size(maxX-minX,maxY-minY));
}

/**
 * Calculates the Delaunay triangulation.
 *
 * The provided bounding box guides the initial super triangle.
 *
 * @param rect the bounding box for the input vertices
 */
void DelaunayTriangulator::computeDelaunay(const Rect& rect) {
    Vec2 one(rect.origin.x-rect.size.height*EPSILON_ADJ,rect.origin.y);
    Vec2 two(rect.origin.x+rect.size.width+rect.size.height*EPSILON_ADJ,rect.origin.y);
    Vec2 tre(rect.origin.x+rect.size.width/2.0f,rect.size.height+rect.size.width*(EPSILON_ADJ/2.0f));
    _output.push_back(Triangle(Vertex(one,-1),Vertex(two,-2),Vertex(tre,-3)));
    
    // Create an indexed list of vertices and sort
    std::vector<Vertex> points;
    for(int ii = 0; ii < _input.size(); ii++) {
        points.push_back(Vertex(_input[ii],ii));
    }
    std::sort(points.begin(), points.end());
    
    for(auto it = points.begin(); it != points.end(); ++it) {
        std::unordered_map<Edge,bool,Edge> polygon;
        for(auto jt = _output.begin(); jt != _output.end(); ++jt) {
            // INVARIANT: No triangle in output is degenerate
            if (jt->containsInCircle(it->point)) {
                jt->setBad(true);
                
                Edge e1(jt->v1,jt->v2);
                auto pt = polygon.find(e1);
                polygon[e1] = (pt == polygon.end());
                
                Edge e2(jt->v2,jt->v3);
                pt = polygon.find(e2);
                polygon[e2] = (pt == polygon.end());
                
                Edge e3(jt->v3,jt->v1);
                pt = polygon.find(e3);
                polygon[e3] = (pt == polygon.end());
            }
        }
        
        // Remove the bad triangles
        auto bt = std::remove_if(_output.begin(), _output.end(),[](const Triangle& t){return t.isBad();});
        _output.erase(bt, _output.end());
        
        // Fill the hole
        for(auto kt = polygon.begin(); kt != polygon.end(); ++kt) {
            Triangle candidate(kt->first.v1,kt->first.v2,*it);
            if (kt->second && !candidate.isDegenerate()) {
                _output.push_back(candidate);
            }
        }
    }
    
    // Remove exterior triangles
    auto et = std::remove_if(_output.begin(), _output.end(),[](const Triangle& t){return t.isExterior();});
    _output.erase(et, _output.end());
}

/**
 * Calculates the Voronoi diagram.
 *
 * The provided bounding box guides the boundary edges.
 *
 * @param rect the bounding box for the input vertices
 */
void DelaunayTriangulator::computeVoronoi(const Rect& rect) {
    std::vector<std::unordered_set<Sint64>> neighbors;
    neighbors.resize(_input.size());
    
    std::unordered_map<Triangle*, int> lookup;
    std::unordered_map<Edge, std::vector<Triangle*>, Edge> edges;
    int pos = 0;
    for(auto it = _output.begin(); it != _output.end(); it++) {
        neighbors[(int)it->v1.index].insert(it->v2.index);
        neighbors[(int)it->v1.index].insert(it->v3.index);
        neighbors[(int)it->v2.index].insert(it->v1.index);
        neighbors[(int)it->v2.index].insert(it->v3.index);
        neighbors[(int)it->v3.index].insert(it->v1.index);
        neighbors[(int)it->v3.index].insert(it->v2.index);
        _dual.push_back(it->getCircleCenter());
        lookup[&(*it)] = pos;
        
        
        Edge e1(it->v1,it->v2);
        auto jt = edges.find(e1);
        if (jt == edges.end()) {
            edges[e1] = std::vector<Triangle*>();
        }
        edges[e1].push_back(&(*it));

        Edge e2(it->v2,it->v3);
        jt = edges.find(e2);
        if (jt == edges.end()) {
            edges[e2] = std::vector<Triangle*>();
        }
        edges[e2].push_back(&(*it));

        
        Edge e3(it->v3,it->v1);
        jt = edges.find(e3);
        if (jt == edges.end()) {
            edges[e3] = std::vector<Triangle*>();
        }
        edges[e3].push_back(&(*it));
        
        pos++;
    }
    _voronoi.resize(_input.size());
    for(size_t ii = 0; ii < _input.size(); ii++) {
        for(auto jt = neighbors[ii].begin(); jt != neighbors[ii].end(); ++jt) {
            Sint64 val = *jt;
            Edge e(Vertex(_input[ii],(int)ii),Vertex(_input[(int)val],val));
            if (edges[e].size() > 1) {
                Triangle* t1 = edges[e][0];
                Triangle* t2 = edges[e][1];
                Edge f(Vertex(t1->getCircleCenter(),lookup[t1]),Vertex(t2->getCircleCenter(),lookup[t2]));
                _voronoi[ii].push_back(f);
            } else {
                // Compute the direction from the triangle center
                Triangle* t1 = edges[e][0];
                Vec2 center = t1->getCircleCenter();
                Vec2 dir = e.v1.point-e.v2.point;
                dir.perp();
            
                // Figure out if we need to flip
                float s = -1;
                float t = -1;
                Vec2::doesLineIntersect(e.v1.point,e.v2.point,center+dir,center,&s,&t);
                if (t < 0) { dir.negate(); }
                dir = get_intersection(center,dir,rect);
                _dual.push_back(dir);
                
                Edge f(Vertex(t1->getCircleCenter(),lookup[t1]),Vertex(dir,(int)_dual.size()-1));
                _voronoi[ii].push_back(f);
            }
        }
    }

    for(size_t ii = 0; ii < _input.size(); ii++) {
        sortCell(ii,rect);
    }
}

/**
 * Sorts the edges of the Voronoi cell so that they are adjacent.
 *
 * In addition to sorting the edges, this method fills in any missing
 * edges on the outside of the bounding box.
 *
 & @param index the index of the Voronoi cell
 * @param rect  the bounding box for the input vertices
 */
void DelaunayTriangulator::sortCell(size_t index, const Rect& rect) {
    
    // First edge determines direction
    bool goon = true;
    size_t fore = 0;
    for(size_t ii = 0; goon && ii < _voronoi[index].size(); ii++) {
        Sint64 pos = -1;
        for(size_t jj = ii+1; pos == -1 && jj < _voronoi[index].size(); jj++) {
            if (_voronoi[index][ii].v2 == _voronoi[index][jj].v1) {
                pos = (Sint64)jj;
            } else if (_voronoi[index][ii].v2 == _voronoi[index][jj].v2) {
                Vertex tmp = _voronoi[index][jj].v2;
                _voronoi[index][jj].v2 = _voronoi[index][jj].v1;
                _voronoi[index][jj].v1 = tmp;
                pos = (Sint64)jj;
            }
        }
        if (pos != -1) {
            Edge tmp = _voronoi[index][(int)pos];
            _voronoi[index][(int)pos]  = _voronoi[index][ii+1];
            _voronoi[index][ii+1] = tmp;
        } else {
            goon = false;
            fore = ii+1;
        }
    }

    // Now move everyone to the front.
    for(size_t ii = 0; ii < fore; ii++) {
        int pos = (int)_voronoi[index].size();
        Edge tmp = _voronoi[index][pos-ii-1];
        _voronoi[index][pos-ii-1]  = _voronoi[index][fore-ii-1];
        _voronoi[index][fore-ii-1] = tmp;
    }
    
    // Go backwards
    goon = true;
    size_t back = 0;
    for(Sint64 ii = _voronoi[index].size()-fore; goon && ii >= 0; ii--) {
        Sint64 pos = -1;
        for(Sint64 jj = ii-1; pos == -1 && jj >= 0; jj--) {
            if (_voronoi[index][(int)ii].v1 == _voronoi[index][(int)jj].v2) {
                pos = jj;
            } else if (_voronoi[index][(int)ii].v1 == _voronoi[index][(int)jj].v1) {
                Vertex tmp = _voronoi[index][(int)jj].v2;
                _voronoi[index][(int)jj].v2 = _voronoi[index][(int)jj].v1;
                _voronoi[index][(int)jj].v1 = tmp;
                pos = jj;
            }
        }
        if (pos != -1) {
            Edge tmp = _voronoi[index][(int)pos];
            _voronoi[index][(int)pos]  = _voronoi[index][(int)ii-1];
            _voronoi[index][(int)ii-1] = tmp;
        } else {
            goon = false;
            back = (size_t)ii;
        }
    }
    
    // Now close the ends
    Vertex* v1 = &(_voronoi[index][0].v1);
    Vertex* v2 = &(_voronoi[index].back().v2);
    if (v1->point.x == v2->point.x || v1->point.y == v2->point.y) {
        _voronoi[index].push_back(Edge(*v2,*v1));
    } else {
        Vec2 corner = get_interior(v1->point, v2->point, rect);
        Edge e1(*v2,Vertex(corner,_dual.size()));
        Edge e2(Vertex(corner,_dual.size()),*v1);

        _dual.push_back(corner);
        _voronoi[index].push_back(e1);
        _voronoi[index].push_back(e2);
    }
}


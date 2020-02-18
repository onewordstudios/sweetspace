//
//  CUSimpleTriangulator.cpp
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

#include <cugl/math/polygon/CUSimpleTriangulator.h>
#include <cugl/util/CUDebug.h>
#include <iterator>

/** Computes the previous index in a vector, treating it as a circular queue */
#define PREV(i,idx) ((i == 0 ? (int)idx.size() : i) - 1)
/** Computes the next index in a vector, treating it as a circular queue */
#define NEXT(i,idx) ((i + 1) % (int)idx.size())

using namespace cugl;

#pragma mark -
#pragma mark Calculation
/**
 * Performs a triangulation of the current vertex data.
 */
void SimpleTriangulator::calculate() {
    reset();
    int vcount = (int)_input.size();
    
    _naive.reserve(vcount);
    _naive.resize(vcount,0);
    
    if (areVerticesClockwise(_input)) {
        for (short i = 0; i < vcount; i++) {
            _naive[i] = i;
        }
    } else {
        for (int i = 0, n = vcount - 1; i < vcount; i++) {
            _naive[i] = (short)(n - i); // Reversed.
        }
    }
    
    _types.reserve(vcount);
    for (int ii = 0; ii < vcount; ++ii) {
        _types.push_back(classifyVertex(ii));
    }
    
    // A polygon with n vertices has a triangulation of n-2 triangles.
    int size = (vcount-2 > 0 ? vcount-2 : 0)*3;
    _output.reserve(size);
    computeTriangulation();
    trimColinear();
    _calculated = true;
}

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
SimpleTriangulator::VertexType
    SimpleTriangulator::computeSpannedAreaType(const Vec2& p1, const Vec2& p2, const Vec2& p3) {
    float area = p1.x * (p3.y - p2.y);
    area += p2.x * (p1.y - p3.y);
    area += p3.x * (p2.y - p1.y);
    VertexType result = (area < 0 ? VertexType::CONCAVE :
                         (area > 0 ? VertexType::CONVEX : VertexType::TANGENTIAL));
    return result;
}

/**
 * Returns true if the vertices are arranged clockwise about the interior.
 *
 * @param vertices  The array of vertices to check
 *
 * @return true if the vertices are arranged clockwise about the interior
 */
bool SimpleTriangulator::areVerticesClockwise (const std::vector<Vec2>& vertices) {
    if (vertices.size() <= 2) {
        return false;
    }
    
    float area = 0;
    Vec2 p1, p2;
    for (int i = 0, n = (int)vertices.size() - 3; i < n; i += 2) {
        p1 = vertices[i  ];
        p2 = vertices[i+1];
        area += p1.x * p2.y - p2.x * p1.y;
    }
    p1 = vertices[vertices.size()-1];
    p2 = vertices[0];
    return area + p1.x * p2.y - p2.x * p1.y < 0;
}

/**
 * Removes an ear tip from the naive triangulation, adding it to the output.
 *
 * This function modifies both indices and types, removing the clipped triangle.
 * The triangle is defined by the given index and its immediate neighbors on
 * either side.
 *
 * @param earTipIndex  The index indentifying the triangle
 */
void SimpleTriangulator::cutEarTip(int earTipIndex) {
    _output.push_back(_naive[PREV(earTipIndex, _naive)]);
    _output.push_back(_naive[earTipIndex]);
    _output.push_back(_naive[NEXT(earTipIndex, _naive)]);
    
    _naive.erase(_naive.begin() + earTipIndex);
    _types.erase(_types.begin() + earTipIndex);
}

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
bool SimpleTriangulator::isEarTip(int earTipIndex) {
    if (_types[earTipIndex] == VertexType::CONCAVE) {
        return false;
    }
    
    int prevIndex = PREV(earTipIndex, _naive);
    int nextIndex = NEXT(earTipIndex, _naive);
    
    int p1 = _naive[prevIndex];
    int p2 = _naive[earTipIndex];
    int p3 = _naive[nextIndex];
    
    Vec2 v1 = _input[p1];
    Vec2 v2 = _input[p2];
    Vec2 v3 = _input[p3];
    
    // Check if any point is inside the triangle formed by previous, current and next vertices.
    // Only consider vertices that are not part of this triangle, or else we'll always find one inside.
    for (int i = NEXT(nextIndex, _naive); i != prevIndex; i = NEXT(i, _naive)) {
        // Concave vertices can obviously be inside the candidate ear, but so can tangential vertices
        // if they coincide with one of the triangle's vertices.
        if (_types[i] != VertexType::CONVEX) {
            int v = _naive[i];
            Vec2 vt = _input[v];
            // Because the polygon has clockwise winding order, the area sign will be positive if the point
            // is strictly inside. It will be 0 on the edge, which we want to include as well.
            // note: check the edge defined by p1->p3 first since this fails _far_ more then the other 2 checks.
            if (computeSpannedAreaType(v3, v1, vt) >= 0) {
                if (computeSpannedAreaType(v1, v2, vt) >= 0) {
                    if (computeSpannedAreaType(v2, v3, vt) >= 0) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

/**
 * Returns a candidate ear-tip triangle
 *
 * The triangle is defined by the given index and its immediate neighbors on
 * either side.  A triangle is a candidate if the defining vertex is convex
 * or tangential.
 *
 * @return a candidate ear-tip triangle
 */
int SimpleTriangulator::findEarTip() {
    for (int ii = 0; ii < _naive.size(); ii++) {
        if (isEarTip(ii)) {
            return ii;
        }
    }
    
    // Desperate mode: if no vertex is an ear tip, we are dealing with a degenerate polygon
    // (e.g. nearly collinear). Note that the input was not necessarily degenerate, but we could
    // have made it so by clipping some valid ears.
    
    // Idea taken from Martin Held, "FIST: Fast industrial-strength triangulation of polygons",
    // Algorithmica (1998), http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.115.291
    
    // Return a convex or tangential vertex if one exists.
    for (int ii = 0; ii < _naive.size(); ii++) {
        if (_types[ii] != VertexType::CONCAVE) {
            return ii;
        }
    }
    
    return 0; // If all vertices are concave, just return the first one.
}

/**
 * Returns the classification for the vertex at the given index.
 *
 * A vertex type is classified by the area spanned by this vertex and its
 * adjacent neighbors.  If the interior angle is outside of the polygon, it
 * is CONCAVE.  If it is inside the polygon, it is CONVEX.
 *
 * @return the classification for the vertex at the given index
 */
SimpleTriangulator::VertexType SimpleTriangulator::classifyVertex (int index) {
    int prev = _naive[PREV(index, _naive)];
    int curr = _naive[index];
    int next = _naive[NEXT(index, _naive)];
    VertexType result = computeSpannedAreaType(_input[prev],_input[curr],_input[next]);
    return result;
}

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
void SimpleTriangulator::computeTriangulation() {
    while (_naive.size() > 3) {
        int earTipIndex = findEarTip();
        cutEarTip(earTipIndex);
        
        // The type of the two vertices adjacent to the clipped vertex may have changed.
        int prevIndex = PREV(earTipIndex, _naive);
        int nextIndex = earTipIndex == _naive.size() ? 0 : earTipIndex;
        _types[prevIndex] = classifyVertex(prevIndex);
        _types[nextIndex] = classifyVertex(nextIndex);
    }
    
    if (_naive.size() == 3) {
        _output.push_back(_naive[0]);
        _output.push_back(_naive[1]);
        _output.push_back(_naive[2]);
    }
}

/**
 * Removes colinear vertices from the given triangulation.
 *
 * Because we permit tangential vertices as ear-clips, this triangulator will occasionally
 * return colinear vertices.  This will crash OpenGL, so we remove them.
 *
 * @param vertices The polygon vertices
 * @param indices The triangulation indices
 *
 * @return reference to the triangulation indices
 */
void SimpleTriangulator::trimColinear() {
    int colinear = 0;
    for(int ii = 0; ii < _naive.size()/3-colinear; ii++) {
        float t1 = _input[_naive[3*ii  ]].x*(_input[_naive[3*ii+1]].y-_input[_naive[3*ii+2]].y);
        float t2 = _input[_naive[3*ii+1]].x*(_input[_naive[3*ii+2]].y-_input[_naive[3*ii  ]].y);
        float t3 = _input[_naive[3*ii+2]].x*(_input[_naive[3*ii  ]].y-_input[_naive[3*ii+1]].y);
        if (fabs(t1+t2+t3) < 0.0000001f) {
            iter_swap(_naive.begin()+(3*ii  ), _naive.end()-(3*colinear+3));
            iter_swap(_naive.begin()+(3*ii+1), _naive.end()-(3*colinear+2));
            iter_swap(_naive.begin()+(3*ii+2), _naive.end()-(3*colinear+1));
            colinear++;
        }
    }
    
    if (colinear > 0) {
        _naive.erase(_naive.end()-3*colinear,_naive.end());
    }
}

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
 * @param buffer    The buffer to store the index data
 *
 * @return a list of indices representing the triangulation.
 */
std::vector<unsigned short> SimpleTriangulator::getTriangulation() const {
    std::vector<unsigned short> result;
    if (_calculated) {
        result.assign(_output.begin(), _output.end());
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
size_t SimpleTriangulator::getTriangulation(std::vector<unsigned short>& buffer) const {
    if (_calculated) {
        buffer.reserve(buffer.size()+_output.size());
        std::copy(_output.begin(), _output.end(),std::back_inserter(buffer));
        return _output.size();
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
Poly2 SimpleTriangulator::getPolygon() const {
    Poly2 poly;
    if (_calculated) {
        poly._vertices = _input;
        poly._indices  = _output;
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
Poly2* SimpleTriangulator::getPolygon(Poly2* buffer) const {
    CUAssertLog(buffer, "Destination buffer is null");
    if (_calculated) {
        if (buffer->_vertices.size() == 0) {
            buffer->_vertices = _input;
            buffer->_indices  = _output;
        } else {
            int offset = (int)buffer->_vertices.size();
            buffer->_vertices.reserve(offset+_input.size());
            std::copy(_input.begin(),_input.end(),std::back_inserter(buffer->_vertices));
            
            buffer->_indices.reserve(buffer->_indices.size()+_output.size());
            for(auto it = _output.begin(); it != _output.end(); ++it) {
                buffer->_indices.push_back(offset+*it);
            }
        }
        buffer->_type = Poly2::Type::SOLID;
        buffer->computeBounds();
    }
    return buffer;
}

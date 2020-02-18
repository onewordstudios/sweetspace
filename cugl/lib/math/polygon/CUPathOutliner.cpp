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

#include <cugl/math/polygon/CUPathOutliner.h>
#include <cugl/math/polygon/CUDelaunayTriangulator.h>
#include <cugl/util/CUDebug.h>
#include <iterator>

using namespace cugl;

#pragma mark Calculation
/**
 * Performs a triangulation of the current vertex data.
 */
void PathOutliner::calculate(PathTraversal traversal) {
    reset();
    int vcount = (int)_input.size();
    _output.clear();
    switch (traversal) {
        case PathTraversal::OPEN:
        {
            _output.reserve(2*(vcount-1));
            for(int ii = 0; ii < vcount-1; ii++) {
                _output.push_back(ii  );
                _output.push_back(ii+1);
            }
            break;
        }
        case PathTraversal::CLOSED:
        {
            _output.reserve(2*vcount);
            for(int ii = 0; ii < vcount-1; ii++) {
                _output.push_back(ii  );
                _output.push_back(ii+1);
            }
            _output.push_back(vcount-1);
            _output.push_back(0);
            break;
        }
        case PathTraversal::INTERIOR:
        {
            std::vector<unsigned short> indx;
            _triangulator.set(_input);
            _triangulator.calculate();
            _triangulator.getTriangulation(indx);
            _output.reserve((int)(2*indx.size()));
            for(int ii = 0; ii < indx.size(); ii++) {
                int next = (ii % 3 == 2 ? ii-2 : ii+1);
                _output.push_back(indx[ii  ]);
                _output.push_back(indx[next]);
            }

            break;
        }
        case PathTraversal::NONE:
            // Do nothing
            break;
    }

    _calculated = true;
}

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
 * @param buffer    The buffer to store the index data
 *
 * @return a list of indices representing the path outline.
 */

std::vector<unsigned short> PathOutliner::getPath() const {
    std::vector<unsigned short> result;
    if (_calculated) {
        result.assign(_output.begin(), _output.end());
    }
    return result;
}

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
size_t PathOutliner::getPath(std::vector<unsigned short>& buffer) const {
    if (_calculated) {
        buffer.reserve(buffer.size()+_output.size());
        std::copy(_output.begin(), _output.end(),std::back_inserter(buffer));
        return _output.size();
    }
    return 0;
}

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
Poly2 PathOutliner::getPolygon() const {
    Poly2 poly;
    if (_calculated) {
        poly._vertices = _input;
        poly._indices  = _output;
        poly._type = Poly2::Type::PATH;
        poly.computeBounds();
    }
    return poly;
}

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
Poly2* PathOutliner::getPolygon(Poly2* buffer) const {
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
        buffer->_type = Poly2::Type::PATH;
        buffer->computeBounds();
    }
    return buffer;
}

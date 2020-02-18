//
//  CUPathExtruder.h
//  Cornell University Game Library (CUGL)
//
//  This module is a factory for extruding a path polgyon into a stroke with
//  width. It has support for joints and end caps.
//
//  The code in this factory is ported from the Kivy implementation of Line in
//  package kivy.vertex_instructions.  We believe that this port is acceptable
//  within the scope of the Kivy license.  There are no specific credits in that
//  file, so there is no one specific to credit.  However, thanks to the Kivy
//  team for doing the heavy lifting on this method.
//
//  Because they did all the hard work, we will recommend their picture of how
//  joints and end caps work:
//
//      http://kivy.org/docs/_images/line-instruction.png
//
//  Since math objects are intended to be on the stack, we do not provide
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

#ifndef __CU_PATH_EXTRUDER_H__
#define __CU_PATH_EXTRUDER_H__

#include <cugl/math/CUPoly2.h>
#include <cugl/math/CUVec2.h>
#include <vector>

namespace cugl {

// Forward reference to the opaque data class.
class KivyData;
    
/**
 * The types of joints supported in an extrusion.
 *
 * A joint is the rule for how to connect two extruded line segments.
 * If there is not joint, the path will look like a sequence of overlapping
 * rectangles.
 *
 * This enumeration is used by both {@link PathExtruder} and {@link PathNode}.
 */
enum class PathJoint : int {
    /** No joint; the path will look like a sequence of links */
    NONE = 0,
    /** Mitre joint; ideal for paths with sharp corners */
    MITRE = 1,
    /** Bevel joint; ideal for smoother paths */
    BEVEL = 2,
    /** Round joint; used to smooth out paths with sharp corners */
    ROUND = 3
};
        
/**
 * The types of caps supported in an extrusion.
 *
 * A cap is the rule for how to end an extruded line segment that has no
 * neighbor on that end.  If there is no cap, the path terminates at the
 * end vertices.
 *
 * This enumeration is used by both {@link PathExtruder} and {@link PathNode}.
 */
enum class PathCap :int {
    /** No end cap; the path terminates at the end vertices */
    NONE = 0,
    /** Square cap; like no cap, except the ends are padded by stroke width */
    SQUARE = 1,
    /** Round cap; the ends are half circles whose radius is the stroke width */
    ROUND = 2
};
    
/**
 * This class is a factory for extruding wireframe paths into a solid path.
 *
 * An extrusion of a path is a second polygon that follows the path of
 * the first one, but gives it width.  Hence it takes a path and turns it
 * into a solid shape. This is more complicated than simply triangulating
 * the original polygon.  The new polygon has more vertices, depending on
 * the choice of joint (shape at the corners) and cap (shape at the end).
 *
 * Unlike the traverse option, this method cannot be used to extrude an
 * internal polygon tesselation. it assumes that the path is continuous.
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
 *
 * CREDITS: This code is ported from the Kivy implementation of Line in package
 * kivy.vertex_instructions.  We believe that this port is acceptable within
 * the scope of the Kivy license.  There are no specific credits in that file,
 * so there is no one specific to credit.  However, thanks to the Kivy team for
 * doing the heavy lifting on this method.
 *
 * Because they did all the hard work, we will recommend their picture of how
 * joints and end caps work:
 *
 *      http://kivy.org/docs/_images/line-instruction.png
 *
 */
class PathExtruder {
#pragma mark Values
private:
    /** The set of vertices to use in the calculation */
    std::vector<Vec2> _input;
    /** Whether the path is closed */
    bool _closed;
    
    /** The output results of extruded vertices */
    std::vector<Vec2> _outverts;
    /** The output results of extruded indices */
    std::vector<unsigned short> _outindx;
    /** Whether or not the calculation has been run */
    bool _calculated;
    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates an extruder with no vertex data.
     */
    PathExtruder() {}
    
    /**
     * Creates an extruder with the given vertex data.
     *
     * The vertex data is copied.  The extruder does not retain any references
     * to the original data.
     *
     * @param points    The vertices to extrude
     * @param closed    Whether the path is closed
     */
    PathExtruder(const std::vector<Vec2>& points, bool closed) : _calculated(false) {
        _input = points; _closed = closed;
    }
    
    /**
     * Creates an extruder with the given vertex data.
     *
     * The extrusion only uses the vertex data from the polygon.  It ignores any
     * existing indices.  The constructor assumes the polygon is closed if the
     * number of indices is twice the number of vertices.
     *
     * The vertex data is copied.  The extruder does not retain any references
     * to the original data.
     *
     * @param poly    The vertices to extrude
     */
    PathExtruder(const Poly2& poly) :  _calculated(false) { _input = poly._vertices; }
    
    /**
     * Deletes this extruder, releasing all resources.
     */
    ~PathExtruder() {}
    
#pragma mark -
#pragma mark Initialization
    /**
     * Sets the vertex data for this extruder.
     *
     * The extrusion only uses the vertex data from the polygon.  It ignores any
     * existing indices.
     *
     * The vertex data is copied.  The extruder does not retain any references
     * to the original data. The method assumes the polygon is closed if the
     * number of indices is twice the number of vertices.
     *
     * This method resets all interal data.  You will need to reperform the
     * calculation before accessing data.
     *
     * @param poly    The vertices to extrude
     */
    void set(const Poly2& poly);
    
    /**
     * Sets the vertex data for this extruder.
     *
     * The vertex data is copied.  The extruder does not retain any references
     * to the original data.
     *
     * This method resets all interal data.  You will need to reperform the
     * calculation before accessing data.
     *
     * @param points    The vertices to extruder
     * @param closed    Whether the path is closed
     */
    void set(const std::vector<Vec2>& points, bool closed) {
        reset();
        _input = points;
        _closed = closed;
    }
    
    /**
     * Clears all internal data, but still maintains the initial vertex data.
     */
    void reset() {
        _calculated = false;
        _outverts.clear(); _outindx.clear();
    }
    
    /**
     * Clears all internal data, the initial vertex data.
     *
     * When this method is called, you will need to set a new vertices before
     * calling calculate.
     */
    void clear() {
        _calculated = false;
        _input.clear(); _outverts.clear(); _outindx.clear();
    }
    
#pragma mark -
#pragma mark Calculation
    /**
     * Performs a extrusion of the current vertex data.
     *
     * An extrusion of a polygon is a second polygon that follows the path of
     * the first one, but gives it width.  Hence it takes a path and turns it
     * into a solid shape. This is more complicated than simply triangulating
     * the original polygon.  The new polygon has more vertices, depending on
     * the choice of joint (shape at the corners) and cap (shape at the end).
     *
     * CREDITS: This code is ported from the Kivy implementation of Line in 
     * package kivy.vertex_instructions.  We believe that this port is
     * acceptable within the scope of the Kivy license.  There are no specific 
     * credits in that file, so there is no one specific to credit.  However, 
     * thanks to the Kivy team for doing the heavy lifting on this method.
     *
     * Because they did all the hard work, we will plug their picture of how
     * joints and end caps work:
     *
     *      http://kivy.org/docs/_images/line-instruction.png
     *
     * @param stroke    The stroke width of the extrusion
     * @param joint     The extrusion joint type.
     * @param cap       The extrusion cap type.
     */
    void calculate(float stroke, PathJoint joint=PathJoint::ROUND, PathCap cap = PathCap::ROUND);
    
#pragma mark -
#pragma mark Materialization
    /**
     * Returns a polygon representing the path extrusion.
     *
     * The polygon contains the a completely new set of vertices together with 
     * the indices defining the extrusion path.  The extruder does not maintain
     * references to this polygon and it is safe to modify it.
     *
     * If the calculation is not yet performed, this method will return the
     * empty polygon.
     *
     * @return a polygon representing the path extrusion.
     */
    Poly2 getPolygon();
    
    /**
     * Stores the path extrusion in the given buffer.
     *
     * This method will add both the new vertices, and the corresponding
     * indices to the new buffer.  If the buffer is not empty, the indices
     * will be adjusted accordingly. You should clear the buffer first if
     * you do not want to preserve the original data.
     *
     * If the calculation is not yet performed, this method will do nothing.
     *
     * @param buffer    The buffer to store the extruded polygon
     *
     * @return a reference to the buffer for chaining.
     */
    Poly2* getPolygon(Poly2* buffer);
    
#pragma mark -
#pragma mark Internal Data Generation
private:
    /**
     * Computes the number of vertices and indices necessary for the extrusion.
     *
     * The number of vertices is put in vcount, while the number of indices
     * is put in icount.  The method returns the total number of points 
     * generating the extruded path, which may be more than the input size when 
     * the path is closed.
     *
     * @param joint     The extrusion joint type.
     * @param cap       The extrusion cap type.
     * @param vcount    Pointer to store the number of vertices needed.
     * @param icount    Pointer to store the number of vertices needed.
     *
     * @return the total number of points generating the extruded path
     */
    unsigned int computeSize(PathJoint joint, PathCap cap, unsigned int* vcount, unsigned int* icount);
    
    /**
     * Creates the extruded line segment from a to b.
     *
     * The new vertices are appended to _outvert, while the new _indices are 
     * appended to _outindx.
     *
     * @param a     The start of the line segment
     * @param b     The end of the line segment.
     * @param data  The data necessary to run the Kivy algorithm.
     */
    void makeSegment(const Vec2& a, const Vec2& b, KivyData* data);

    /**
     * Creates a joint immediately before point a.
     *
     * The new vertices are appended to _outvert, while the new _indices are
     * appended to _outindx.
     *
     * @param a         The generating point after the joint.
     * @param data      The data necessary to run the Kivy algorithm.
     *
     * @return true if a joint was successfully created.
     */
    bool makeJoint(const Vec2& a, KivyData* data);

    /**
     * Creates a mitre joint immediately before point a.
     *
     * The new vertices are appended to _outvert, while the new _indices are
     * appended to _outindx.
     *
     * @param a         The generating point after the joint.
     * @param jangle    The joint angle
     * @param data      The data necessary to run the Kivy algorithm.
     *
     * @return true if a joint was successfully created.
     */
    bool makeMitreJoint(const Vec2& a, float jangle, KivyData* data);

    /**
     * Creates a bevel joint immediately before point a.
     *
     * The new vertices are appended to _outvert, while the new _indices are
     * appended to _outindx.
     *
     * @param a         The generating point after the joint.
     * @param jangle    The joint angle
     * @param data      The data necessary to run the Kivy algorithm.
     *
     * @return true if a joint was successfully created.
     */
    bool makeBevelJoint(const Vec2& a, float jangle, KivyData* data);

    /**
     * Creates a round joint immediately before point a.
     *
     * The new vertices are appended to _outvert, while the new _indices are
     * appended to _outindx.
     *
     * @param a         The generating point after the joint.
     * @param jangle    The joint angle
     * @param data      The data necessary to run the Kivy algorithm.
     *
     * @return true if a joint was successfully created.
     */
    bool makeRoundJoint(const Vec2& a, float jangle, KivyData* data);
    
    /**
     * Creates the caps on the two ends of the open path.
     *
     * The new vertices are appended to _outvert, while the new _indices are
     * appended to _outindx.
     *
     * @param count     The number of generating points in the path.
     * @param data      The data necessary to run the Kivy algorithm.
     */
    void makeCaps(int count, KivyData* data);
    
    /**
     * Creates square caps on the two ends of the open path.
     *
     * The new vertices are appended to _outvert, while the new _indices are
     * appended to _outindx.
     *
     * @param count     The number of generating points in the path.
     * @param data      The data necessary to run the Kivy algorithm.
     */
    void makeSquareCaps(int count, KivyData* data);

    /**
     * Creates round caps on the two ends of the open path.
     *
     * The new vertices are appended to _outvert, while the new _indices are
     * appended to _outindx.
     *
     * @param count     The number of generating points in the path.
     * @param data      The data necessary to run the Kivy algorithm.
     */
    void makeRoundCaps(int count, KivyData* data);
    
    /**
     * Creates the final joint at the end of a closed path.
     *
     * The new vertices are appended to _outvert, while the new _indices are
     * appended to _outindx.
     *
     * @param data      The data necessary to run the Kivy algorithm.
     *
     * @return true if a joint was successfully created.
     */
    bool makeLastJoint(KivyData* data);
};
}

#endif /* __CU_PATH_EXTRUDER_H__ */

//
//  CUCubicSpline.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a class that represents a spline of cubic beziers. A
//  bezier spline is just a sequence of beziers joined together, so that the end
//  of one is the beginning of the other. Cubic beziers have four control points,
//  two for the vertex anchors and two for their tangents.
//
//  This class has been purposefully kept lightweight.  If you want to draw a
//  CubicSpline, you will need to allocate a Poly2 value for the spline using
//  the factory CubicBezierApproximator.  We have to turn shapes into polygons
//  to draw them anyway, and this allows us to do all of the cool things we can
//  already do with paths, like extrude them or create wireframes.
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
//  Version: 6/21/16

#include <cugl/math/CUCubicSpline.h>
#include <cugl/math/CUPolynomial.h>

using namespace std;
using namespace cugl;

/** Maximum recursion depth for de Castlejau's */
#define MAX_DEPTH   8

/** Tolerance to identify a point as "smooth" */
#define SMOOTH_TOLERANCE    0.0001f

#pragma mark -
#pragma mark Constructors

/**
 * Creates a spline of two points
 *
 * The minimum spline possible has 4 points: two anchors and two tangents.
 * This sets the start to be the first anchor point, and end to be the
 * second.  The tangents, are the same as the anchor points, which means
 * that the tangents are degenerate.  This has the effect of making the
 * bezier a straight line from start to end. The spline is open, unless
 * start and end are the same.
 *
 * @param  start    The first bezier anchor point
 * @param  end      The second bezier anchor point
 */
CubicSpline::CubicSpline(const Vec2& start, const Vec2& end) {
    _points.push_back(start);
    _points.push_back(start);
    _points.push_back(end);
    _points.push_back(end);
    
    _smooth.push_back(true);
    _smooth.push_back(true);
    
    _size = 1;
    _closed = (start.x == end.x && start.y == end.y);
}

/**
 * Creates a spline from the given control points.
 *
 * The control points must be specified in the form
 *
 *      anchor, tangent, tangent, anchor, tangent ... anchor
 *
 * That is, starts and ends with anchors, and every two anchors have two
 * tangents (right of the first, left of the second) in between. As each
 * point is two floats, the value size must be equal to 2 mod 6.
 *
 * The created spline is open.
 *
 * @param  points   The array of control points as floats
 * @param  offset   The starting offset in the control point array
 * @param  size     The number of floats to use in the array
 */
CubicSpline::CubicSpline(const float* points, int size, int offset) {
    CUAssertLog(size - 2 % 6 != 0, "Control point array is the wrong size");
    
    _size = (size - 2) / 6;
    _closed = false;
    
    for (int ii = 0; ii < size / 2; ii++) {
        _points.push_back(Vec2(points[2 * ii + offset], points[2 * ii + offset + 1]));
    }
    
    _smooth.resize(_size + 1, true);
    _smooth[0] = true;
    _smooth[_size] = true;
    
    for (int ii = 1; ii < _size; ii++) {
        Vec2 temp0 = _points[3 * ii - 1] - _points[3 * ii];
        Vec2 temp1 = _points[3 * ii + 1] - _points[3 * ii];
        temp0.normalize();
        temp1.normalize();
        temp0 -= temp1;
        _smooth[ii] = (temp0.lengthSquared() < SMOOTH_TOLERANCE);
    }
}

/**
 * Creates a spline from the given control points.
 *
 * The control points must be specified in the form
 *
 *      anchor, tangent, tangent, anchor, tangent ... anchor
 *
 * That is, starts and ends with anchors, and every two anchors have two
 * tangents (right of the first, left of the second) in between. As each
 * point is two floats, the size of the vector must be equal to 2 mod 6.
 *
 * The created spline is open.
 *
 * @param  points   The vector of control points as floats
 */
CubicSpline::CubicSpline(const vector<float>& points) {
    CUAssertLog(points.size() % 6 != 2, "Control point array is the wrong size");
    
    _size = ((int)points.size() - 2) / 6;
    _closed = false;
    
    for (int ii = 0; ii < _size / 2; ii++) {
        _points.push_back(Vec2(points[2 * ii], points[2 * ii + 1]));
    }
    
    _smooth.resize(_size + 1, true);
    _smooth[0] = true;
    _smooth[_size] = true;
    
    for (int ii = 1; ii < _size; ii++) {
        Vec2 temp0 = _points[3 * ii - 1] - _points[3 * ii];
        Vec2 temp1 = _points[3 * ii + 1] - _points[3 * ii];
        temp0.normalize();
        temp1.normalize();
        temp0 -= temp1;
        _smooth[ii] = (temp0.lengthSquared() < SMOOTH_TOLERANCE);
    }
}

/**
 * Creates a spline from the given control points.
 *
 * The control points must be specified in the form
 *
 *      anchor, tangent, tangent, anchor, tangent ... anchor
 *
 * That is, starts and ends with anchors, and every two anchors have two
 * tangents (right of the first, left of the second) in between. The
 * size of this vector must be equal to 1 mod 3.
 *
 * The created spline is open.
 *
 * @param  points   The vector of control points
 */
CubicSpline::CubicSpline(const vector<Vec2>& points) {
    CUAssertLog(points.size() % 3 != 1, "Control point array is the wrong size");
    
    _size = ((int)points.size() - 1) / 3;
    _closed = false;
    
    _points.assign(points.begin(), points.end());
    
    _smooth.resize(_size + 1, true);
    _smooth[0] = true;
    _smooth[_size] = true;
    
    for (int ii = 1; ii < _size; ii++) {
        Vec2 temp0 = _points[3 * ii - 1] - _points[3 * ii];
        Vec2 temp1 = _points[3 * ii + 1] - _points[3 * ii];
        temp0.normalize();
        temp1.normalize();
        temp0 -= temp1;
        _smooth[ii] = (temp0.lengthSquared() < SMOOTH_TOLERANCE);
    }
}

/**
 * Creates a copy of the given spline.
 *
 * @param  spline   The spline to copy
 */
CubicSpline::CubicSpline(const CubicSpline& spline) {
    _size = spline._size;
    _closed = spline._closed;
    _points.assign(spline._points.begin(), spline._points.end());
    _smooth.assign(spline._smooth.begin(), spline._smooth.end());
}


#pragma mark
#pragma mark Assignment Operators

/**
 * Sets this spline to be a line between two points
 *
 * The minimum spline possible has 4 points: two anchors and two tangents.
 * This sets the start to be the first anchor point, and end to be the
 * second.  The tangents, are the same as the anchor points, which means
 * that the tangents are degenerate.  This has the effect of making the
 * bezier a straight line from start to end. The spline is open, unless
 * start and end are the same.
 *
 * @param  start    The first bezier anchor point
 * @param  end      The second bezier anchor point
 *
 * @return This spline, returned for chaining
 */
CubicSpline& CubicSpline::set(const Vec2& start, const Vec2& end) {
    _points.push_back(start);
    _points.push_back(start);
    _points.push_back(end);
    _points.push_back(end);
    
    _smooth.push_back(true);
    _smooth.push_back(true);
    
    _size = 1;
    _closed = (start.x == end.x && start.y == end.y);
    return *this;
}

/**
 * Sets this spline to have the given control points.
 *
 * The control points must be specified in the form
 *
 *      anchor, tangent, tangent, anchor, tangent ... anchor
 *
 * That is, starts and ends with anchors, and every two anchors have two
 * tangents (right of the first, left of the second) in between. As each
 * point is two floats, the value size must be equal to 2 mod 6.
 *
 * This method makes the spline is open.
 *
 * @param  points   The array of control points as floats
 * @param  offset   The starting offset in the control point array
 * @param  size     The number of floats to use in the array
 *
 * @return This spline, returned for chaining
 */
CubicSpline& CubicSpline::set(const float* points, int size, int offset) {
    CUAssertLog(size - 2 % 6 != 0, "Constrol point array is the wrong size");
    
    _size = (size - 2) / 6;
    _closed = false;
    
    for (int ii = 0; ii < size / 2; ii++) {
        _points.push_back(Vec2(points[2 * ii + offset], points[2 * ii + offset + 1]));
    }
    
    _smooth.resize(_size + 1, true);
    _smooth[0] = true;
    _smooth[_size] = true;
    
    for (int ii = 1; ii < _size; ii++) {
        Vec2 temp0 = _points[3 * ii - 1] - _points[3 * ii];
        Vec2 temp1 = _points[3 * ii + 1] - _points[3 * ii];
        temp0.normalize();
        temp1.normalize();
        temp0 -= temp1;
        _smooth[ii] = (temp0.lengthSquared() < SMOOTH_TOLERANCE);
    }
    return *this;
}

/**
 * Sets this spline to have the given control points.
 *
 * The control points must be specified in the form
 *
 *      anchor, tangent, tangent, anchor, tangent ... anchor
 *
 * That is, starts and ends with anchors, and every two anchors have two
 * tangents (right of the first, left of the second) in between. As each
 * point is two floats, the size of the vector must be equal to 2 mod 6.
 *
 * This method makes the spline is open.
 *
 * @param  points   The vector of control points as floats
 *
 * @return This spline, returned for chaining
 */
CubicSpline& CubicSpline::set(const std::vector<float>& points) {
    CUAssertLog(points.size() % 6 != 2, "Control point array is the wrong size");
    
    _size = ((int)points.size() - 2) / 6;
    _closed = false;
    
    for (int ii = 0; ii < _size / 2; ii++) {
        _points.push_back(Vec2(points[2 * ii], points[2 * ii + 1]));
    }
    
    _smooth.resize(_size + 1, true);
    _smooth[0] = true;
    _smooth[_size] = true;
    
    for (int ii = 1; ii < _size; ii++) {
        Vec2 temp0 = _points[3 * ii - 1] - _points[3 * ii];
        Vec2 temp1 = _points[3 * ii + 1] - _points[3 * ii];
        temp0.normalize();
        temp1.normalize();
        temp0 -= temp1;
        _smooth[ii] = (temp0.lengthSquared() < SMOOTH_TOLERANCE);
    }
    return *this;
}

/**
 * Sets this spline to have the given control points.
 *
 * The control points must be specified in the form
 *
 *      anchor, tangent, tangent, anchor, tangent ... anchor
 *
 * That is, starts and ends with anchors, and every two anchors have two
 * tangents (right of the first, left of the second) in between. The
 * size of this vector must be equal to 1 mod 3.
 *
 * This method makes the spline is open.
 *
 * @param  points   The vector of control points
 *
 * @return This spline, returned for chaining
 */
CubicSpline& CubicSpline::set(const std::vector<Vec2>& points) {
    CUAssertLog(points.size() % 3 != 1, "Control point array is the wrong size");
    
    _size = ((int)points.size() - 1) / 3;
    _closed = false;
    
    _points.assign(points.begin(), points.end());
    
    _smooth.resize(_size + 1, true);
    _smooth[0] = true;
    _smooth[_size] = true;
    
    for (int ii = 1; ii < _size; ii++) {
        Vec2 temp0 = _points[3 * ii - 1] - _points[3 * ii];
        Vec2 temp1 = _points[3 * ii + 1] - _points[3 * ii];
        temp0.normalize();
        temp1.normalize();
        temp0 -= temp1;
        _smooth[ii] = (temp0.lengthSquared() < SMOOTH_TOLERANCE);
    }
    return *this;
}

/**
 * Sets this spline to be a copy of the given spline.
 *
 * @param  spline   The spline to copy
 *
 * @return This spline, returned for chaining
 */
CubicSpline& CubicSpline::set(const CubicSpline& spline) {
    _size = spline._size;
    _closed = spline._closed;
    _points.assign(spline._points.begin(), spline._points.end());
    _smooth.assign(spline._smooth.begin(), spline._smooth.end());
    return *this;
}


#pragma mark
#pragma mark Attribute Accessors

/**
 * Sets whether the spline is closed.
 *
 * A closed spline is one where the first and last anchor are the same.
 * Hence the first and last tangents are tangents (right, and left,
 * respectively) of the same point.  This is relevant for the setTangent()
 * method, particularly if the change is meant to be symmetric.
 *
 * A closed spline has no end. Therefore, anchors cannot be added to
 * a closed spline.  They may only be inserted between two other
 * anchors.
 *
 * @param flag whether the spline is closed
 */
void CubicSpline::setClosed(bool flag) {
    if (flag && (_points[0] != _points[3 * _size])) {
        addAnchor(_points[0]);
    }
    _closed = true;
}

/**
 * Returns the spline point for parameter tp.
 *
 * This method is like the public getPoint(), except that it is restricted
 * to a single bezier segment.  A bezier is parameterized with tp in 0..1,
 * with tp = 0 representing the first anchor and tp = 1 representing the
 * second. This method is used by the public getPoint() to compute its value.
 *
 * @param  segment  the bezier segment to select from
 * @param  tp       the parameterization value
 *
 * @return the spline point for parameter tp
 */
Vec2 CubicSpline::getPoint(int segment, float tp) const {
    CUAssertLog(segment >= 0 && segment < _size, "Illegal spline segment");
    CUAssertLog(tp >= 0.0f && tp <= 1.0f, "Illegal segment parameter");
    
    if (segment == _size) {
        return _points[3 * segment];
    }
    
    int index = 6 * segment;
    float sp = (1 - tp);
    float a = sp*sp;
    float d = tp*tp;
    float b = 3 * tp*a;
    float c = 3 * sp*d;
    a = a*sp;
    d = d*tp;
    return a*_points[index] + b*_points[index + 1] + c*_points[index + 2] + d*_points[index + 3];
}

/**
 * Sets the spline point at parameter tp.
 *
 * A bezier spline is a parameterized curve. For a single bezier, it is
 * parameterized with tp in 0..1, with tp = 0 representing the first
 * anchor and tp = 1 representing the second. In the spline, we generalize
 * this idea, where tp is an anchor if it is an int, and is inbetween
 * the anchors floor(tp) and ceil(tp) otherwise.
 *
 * In this method, if tp is an int, it will just reassign the associated
 * anchor value.  Otherwise, this will insert a new anchor point at
 * that parameter.  This has a side-effect of changing the parameterization
 * values for the curve, as the number of beziers has increased.
 *
 * @param  tp       the parameterization value
 * @param  point    the new value to assign
 */
void CubicSpline::setPoint(float tp, const Vec2& point) {
    CUAssertLog(tp >= 0 && tp <= _size, "Parameter out of bounds");
    CUAssertLog(!_closed || tp < _size, "Parameter out of bounds for closed spline");
    
    int seg = (int)tp;
    if (seg == tp) {
        setAnchor(seg, point);
    }
    else {
        tp = tp - seg;
        insertAnchor(seg, tp);
        setAnchor(seg + 1, point);
    }
}

/**
 * Returns the anchor point at the given index.
 *
 * If an open spline has n segments, then it has n+1 anchors. Similiarly,
 * a closed spline had n anchors.  The value index should be in the
 * appropriate range.
 *
 * @param  index    the anchor index (0..n+1 or 0..n)
 *
 * @return the anchor point at the given index.
 */
Vec2 CubicSpline::getAnchor(int index) const {
    CUAssertLog(index >= 0 && index < _size, "Index out of bounds");
    CUAssertLog(!_closed || index < _size - 1, "Index out of bounds for closed spline");
    return _points[3 * index];
}

/**
 * Sets the anchor point at the given index.
 *
 * This method will change both the anchor and its associated tangets.
 * The new tangents will have the same relative change in position.
 * As a result, the bezier will still have the same shape locally.
 * This is the natural behavior for changing an anchor, as seen in
 * Adobe Illustrator.
 *
 * If an open spline has n segments, then it has n+1 anchors. Similiarly,
 * a closed spline had n anchors.  The value index should be in the
 * appropriate range.
 *
 * @param  index    the anchor index (0..n+1 or 0..n)
 * @param  point    the new value to assign
 */
void CubicSpline::setAnchor(int index, const Vec2& point) {
    CUAssertLog(index >= 0 && index < _size, "Index out of bounds");
    CUAssertLog(!_closed || index < _size - 1, "Index out of bounds for closed spline");
    
    Vec2 diff = point - _points[3 * index];
    
    // Adjust left tangents
    if (index > 0) {
        _points[3 * index - 1] = _points[3 * index - 1] + diff;
    }
    else if (_closed) {
        _points[3 * _size - 1] = _points[3 * _size - 1] + diff;
    }
    
    // Adjust right tangents
    if (index < _size) {
        _points[3 * index + 1] = _points[3 * index + 1] + diff;
    }
    else if (_closed) {
        _points[1] = _points[1] + diff;
    }
    
    _points[3 * index] = point;
}

/**
 * Returns the smoothness for the anchor point at the given index.
 *
 * A smooth anchor is one in which the derivative of the curve at the
 * anchor is continuous.  Practically, this means that the left and
 * right tangents are always parallel.  Only a non-smooth anchor may
 * form a "hinge".
 *
 * If an open spline has n segments, then it has n+1 anchors. Similiarly,
 * a closed spline had n anchors.  The value index should be in the
 * appropriate range.
 *
 * @param  index    the anchor index (0..n+1 or 0..n)
 *
 * @return the smoothness for the anchor point at the given index.
 */
bool CubicSpline::getSmooth(int index) const {
    CUAssertLog(index >= 0 && index < _size, "Index out of bounds");
    CUAssertLog(!_closed || index < _size - 1, "Index out of bounds for closed spline");
    return _smooth[index];
}

/**
 * Sets the smoothness for the anchor point at the given index.
 *
 * A smooth anchor is one in which the derivative of the curve at the
 * anchor is continuous.  Practically, this means that the left and
 * right tangents are always parallel.  Only a non-smooth anchor may
 * form a "hinge".
 *
 * If you set a non-smooth anchor to smooth, it will adjust the
 * tangents accordingly.  In particular, it will average the two
 * tangents, making them parallel. This is the natural behavior for
 * changing an smoothness, as seen in Adobe Illustrator.
 *
 * If an open spline has n segments, then it has n+1 anchors. Similiarly,
 * a closed spline had n anchors.  The value index should be in the
 * appropriate range.
 *
 * @param  index    the anchor index (0..n+1 or 0..n)
 * @param  flag     the anchor smoothness
 *
 * @return the smoothness for the anchor point at the given index.
 */
void CubicSpline::setSmooth(int index, bool flag) {
    CUAssertLog(index >= 0 && index < _size, "Index out of bounds");
    CUAssertLog(!_closed || index < _size - 1, "Index out of bounds for closed spline");
    
    _smooth[index] = flag;
    if (flag && index > 0 && index < _size) {
        Vec2 temp0 = _points[3 * index - 1] - _points[3 * index];
        Vec2 temp1 = _points[3 * index] - _points[3 * index + 1];
        float scale0 = temp0.length();
        float scale1 = temp1.length();
        
        // Average the vectors
        temp0.normalize();
        temp1.normalize();
        Vec2 temp2 = temp0.getMidpoint(temp1);
        temp2.normalize();
        
        // Scale them appropriately
        temp0.set(temp2);
        temp0.scale(scale0);
        temp1.set(temp2);
        temp1.scale(scale1);
        
        _points[3 * index - 1] = _points[3 * index] + temp0;
        _points[3 * index + 1] = _points[3 * index] - temp1;
    }
}

/**
 * Returns the tangent at the given index.
 *
 * Tangents are specified as points, not vectors.  To get the tangent
 * vector for an anchor, you must subtract the anchor from its tangent
 * point.  Hence a curve is degenerate when the tangent and the
 * anchor are the same.
 *
 * If a spline has n segments, then it has 2n tangents. This is true
 * regardless of whether it is open or closed. The value index should
 * be in the appropriate range. An even index is a right tangent,
 * while an odd index is a left tangent. If the spline is closed, then
 * 2n-1 is the left tangent of the first point.
 *
 * @param  index    the tangent index (0..2n)
 *
 * @return the tangent at the given index.
 */
Vec2 CubicSpline::getTangent(int index) const {
    CUAssertLog(index >= 0 && index < 2 * _size, "Index out of bounds");
    int spline = (index + 1) / 2;
    int anchor = 3 * spline;
    int tangt = (index % 2 == 1 ? anchor - 1 : anchor + 1);
    return _points[tangt];
}

/**
 * Sets the tangent at the given index.
 *
 * Tangents are specified as points, not vectors.  To get the tangent
 * vector for an anchor, you must subtract the anchor from its tangent
 * point.  Hence a curve is degenerate when the tangent and the
 * anchor are the same.
 *
 * If the associated anchor point is smooth, changing the direction
 * of the tangent vector will also change the direction of the other
 * tangent vector (so that they remain parallel).  However, changing
 * only the magnitude will have no effect, unless symmetric is true.
 * In that case, it will modify the other tangent so that it has the
 * same magnitude and parallel direction. This is the natural behavior
 * for changing a tangent, as seen in Adobe Illustrator.
 *
 * If a spline has n segments, then it has 2n tangents. This is true
 * regardless of whether it is open or closed. The value index should
 * be in the appropriate range. An even index is a right tangent,
 * while an odd index is a left tangent. If the spline is closed, then
 * 2n-1 is the left tangent of the first point.
 *
 * @param  index     the tangent index (0..2n)
 * @param  tang      the new value to assign
 * @param  symmetric whether to make the other tangent symmetric
 */
void CubicSpline::setTangent(int index, const Vec2& tang, bool symmetric) {
    CUAssertLog(index >= 0 && index < 2 * _size, "Index out of bounds");
    
    int spline = (index + 1) / 2;
    int anchor = 3 * spline;
    int tangt1 = (index % 2 == 1 ? anchor - 1 : anchor + 1);
    int tangt2 = (index % 2 == 1 ? anchor + 1 : anchor - 1);
    
    if (spline == 0) {
        tangt2 = (_closed ? 3 * _size - 1 : -1);
    }
    else if (spline == _size) {
        tangt2 = (_closed ? 1 : -1);
    }
    
    if (symmetric && tangt2 != -1) {
        Vec2 temp0 = _points[anchor] - tang;
        _points[tangt2] = _points[anchor] + temp0;
    }
    else if (_smooth[spline] && tangt2 != -1) {
        Vec2 temp0 = _points[anchor] - _points[tangt2];
        float d = temp0.length();
        
        temp0 = _points[anchor] - tang;
        temp0.normalize();
        temp0.scale(d);
        
        _points[tangt2] = _points[anchor] + temp0;
    }
    
    _points[tangt1] = tang;
}

/**
 * Returns the x-axis bezier polynomial for the given segment.
 *
 * Bezier polynomials define the curve parameterization. They are
 * two dimension polynomials that give a point.  Rather than
 * extend polynomial to support multidimensional data, we extract
 * each axis separately.
 *
 * We also cannot define a single polynomial for the entire spline,
 * but we can do it for each segment.  The result is a cubic poly,
 * hence the name CubicSpline.
 *
 * @return the x-axis bezier polynomial for the given segment.
 */
Polynomial CubicSpline::getPolynomialX(int segment) const {
    CUAssertLog(segment >= 0 && segment < _size, "Segment out of bounds");
    
    //(1-t)3 p0 + 3(1-t)2 t p1 + 3 (1-t) t2 p2 + t3 p3
    float p3 = _points[3 * segment + 3].x;
    float p2 = _points[3 * segment + 2].x;
    float p1 = _points[3 * segment + 1].x;
    float p0 = _points[3 * segment].x;
    
    Polynomial poly;
    poly.push_back(p3 + 3 * p1 - p0 - 3 * p2);
    poly.push_back(p0 + 6 * p1 + 3 * p2);
    poly.push_back(3 * p1 - 3 * p0);
    poly.push_back(p0);
    return poly;
}

/**
 * Returns the y-axis bezier polynomial for the given segment.
 *
 * Bezier polynomials define the curve parameterization. They are
 * two dimension polynomials that give a point.  Rather than
 * extend polynomial to support multidimensional data, we extract
 * each axis separately.
 *
 * We also cannot define a single polynomial for the entire spline,
 * but we can do it for each segment.  The result is a cubic poly,
 * hence the name CubicSpline.
 *
 * @return the y-axis bezier polynomial for the given segment.
 */
Polynomial CubicSpline::getPolynomialY(int segment) const {
    CUAssertLog(segment >= 0 && segment < _size, "Segment out of bounds");
    
    //(1-t)3 p0 + 3(1-t)2 t p1 + 3 (1-t) t2 p2 + t3 p3
    float p3 = _points[3 * segment + 3].y;
    float p2 = _points[3 * segment + 2].y;
    float p1 = _points[3 * segment + 1].y;
    float p0 = _points[3 * segment].y;
    
    Polynomial poly;
    poly.push_back(p3 + 3 * p1 - p0 - 3 * p2);
    poly.push_back(p0 + 6 * p1 + 3 * p2);
    poly.push_back(3 * p1 - 3 * p0);
    poly.push_back(p0);
    return poly;
}


#pragma mark Anchor Editting Methods
/**
 * Adds the given point to the end of the spline, creating a new segment.
 *
 * The new segment will start at the previous end of the last spline and
 * extend to the given point. As closed splines have no end, this method
 * will fail on closed beziers. You should use insertAnchor instead for
 * closed beziers.
 *
 * This value tang is the left tangent of the new anchor point.
 *
 * @param  point    the new anchor point to add to the end
 * @param  tang     the left tangent of the new anchor point
 *
 * @return the new number of segments in this spline
 */
int CubicSpline::addAnchor(const Vec2& point, const Vec2& tang) {
    CUAssertLog(!_closed, "Cannot append to closed curve");
    
    _points.resize(_points.size() + 3, Vec2::ZERO);
    _smooth.resize(_smooth.size() + 1, true);
    
    int pos = 3 * _size + 1;
    if (_smooth[_size]) {
        _points[pos] = 2 * _points[pos - 1] - _points[pos - 2];
    }
    else {
        _points[pos] = _points[pos - 1];
    }
    
    _points[pos + 1] = tang;
    _points[pos + 2] = point;
    _size++;
    _smooth[_size] = true;
    return _size;
}

/**
 * Deletes the anchor point at the given index.
 *
 * The point is deleted as well as both of its tangents (left and right).
 * All remaining anchors after the deleted one will shift their indices
 * down by one. Deletion is allowed on closed splines; the spline will
 * remain closed after deletion.
 *
 * If an open spline has n segments, then it has n+1 anchors. Similiarly,
 * a closed spline had n anchors.  The value index should be in the
 * appropriate range.
 *
 * @param  index    the anchor index to delete
 */
void CubicSpline::deleteAnchor(int index) {
    CUAssertLog(index >= 0 && index < _size, "Index out of bounds");
    CUAssertLog(!_closed || index < _size - 1, "Index out of bounds for closed spline");
    
    // Shift everything left.
    _points.erase(_points.begin() + (3 * index), _points.begin() + (3 * (index + 1)));
    _smooth.erase(_smooth.begin() + index, _smooth.begin() + (index + 1));
    _size--;
}

/**
 * Inserts a new anchor point at parameter tp.
 *
 * Inserting an anchor point does not change the curve.  It just makes
 * an existing point that was not an anchor, now an anchor. This is the
 * natural behavior for inserting an index, as seen in Adobe Illustrator.
 *
 * This version of insertAnchor() specifies the segment for insertion,
 * simplifying the parameterization. For a single bezier, it is
 * parameterized with tp in 0..1, with tp = 0 representing the first
 * anchor and tp = 1 representing the second.
 *
 * The tangents of the new anchor point will be determined by de Castlejau's.
 * This is the natural behavior for inserting an anchor mid bezier, as seen
 * in Adobe Illustrator.
 *
 * @param  segment  the bezier segment to insert into
 * @param  tp       the parameterization value
 */
void CubicSpline::insertAnchor(int segment, float param) {
    CUAssertLog(segment >= 0 && segment < _size, "Illegal spline segment");
    CUAssertLog(param > 0.0f && param < 1.0f, "Illegal insertion parameter");
    
    // Split the bezier.
    vector<Vec2> left;
    vector<Vec2> right;
    subdivide(segment, param, left, right);
    
    // Replace first segment with left
    copy(left.begin(), left.end(), _points.begin() + (3 * segment));
    
    // Now insert the right
    _points.insert(_points.begin() + (3 * segment + 1), right.begin(), right.end());
    _smooth.insert(_smooth.begin() + (segment + 1), true);
    _size++;
}


#pragma mark Nearest Point Methods
/**
 * Returns the parameterization of the nearest point on the spline.
 *
 * The value is effectively the projection of the point onto the parametrized
 * curve. See getPoint() for an explanation of how the parameterization work. We
 * compute this value using the projection polynomial, described at
 *
 * http://jazzros.blogspot.com/2011/03/projecting-point-on-bezier-curve.html
 *
 * @param  point    the point to project
 *
 * @return the parameterization of the nearest point on the spline.
 */
float CubicSpline::nearestParameter(const Vec2& point) const {
    float tmin = -1;
    float dmin = -1;
    int smin = -1;
    
    for (int ii = 0; ii < _size; ii++) {
        Vec2 pair = getProjectionFast(point, ii);
        if (pair.x == -1) {
            pair = getProjectionSlow(point, ii);
        }
        if (smin == -1 || pair.y < dmin) {
            tmin = pair.x; dmin = pair.y; smin = ii;
        }
    }
    
    return smin + tmin;
}

/**
 * Returns the index of the anchor nearest the given point.
 *
 * If there is no anchor whose distance to point is less than the square root
 * of threshold (we use lengthSquared for speed), then this method returns -1.
 *
 * @param  point        the point to compare
 * @param  threshold    the distance threshold for picking an anchor
 *
 * @return the index of the anchor nearest the given point.
 */
int CubicSpline::nearestAnchor(const Vec2& point, float threshold) const {
    float best = std::numeric_limits<float>::infinity();
    int	 index = -1;
    
    for (int ii = 0; ii <= _size; ii++) {
        Vec2 temp0 = _points[3 * ii] - point;
        float d = temp0.lengthSquared();
        if (d < threshold && d < best) {
            best = d;
            index = ii;
        }
    }
    return index;
}

/**
 * Returns the index of the tangent nearest the given point.
 *
 * If there is no tangent whose distance to point is less than the square root
 * of threshold (we use lengthSquared for speed), then this method returns -1.
 *
 * @param  point        the point to compare
 * @param  threshold    the distance threshold for picking a tangent
 *
 * @return the index of the tangent nearest the given point.
 */
int CubicSpline::nearestTangent(const Vec2& point, float threshold) const {
    float best = std::numeric_limits<float>::infinity();
    int	 index = -1;
    
    for (int ii = 0; ii < _size; ii++) {
        Vec2 temp0 = _points[3 * ii + 1] - point;
        float d = temp0.lengthSquared();
        if (d < threshold && d < best) {
            best = d;
            index = 2 * ii + 1; // Right side of index ii.
        }
        temp0 = _points[3 * ii + 2] - point;
        d = temp0.lengthSquared();
        if (d < threshold && d < best) {
            best = d;
            index = 2 * ii + 2; // Left side of index ii+1
        }
    }
    return index;
}

#pragma mark Internal Helpers
/**
 * Applies de Castlejau's to a bezier, putting the result in left & right
 *
 * de Castlejau's takes a parameter tp in (0,1) and splits the bezier into two,
 * preserving the geometric information, but not the parameterization.  The control
 * points for the resulting two beziers are stored in left and right.
 *
 * This static method is not restricted to the current spline.  It can work
 * from any list of control points (and offset into those control points).
 * This is useful for recursive subdivision.
 *
 * @param  src      the control point list for the bezier
 * @param  soff     the offset into the control point list
 * @param  tp       the parameter to split at
 * @param  left     vector to store the left bezier
 * @param  right    vector to store the right bezier
 */
void CubicSpline::subdivide(const vector<Vec2>& src, int soff, float tp,
                            vector<Vec2>& left, vector<Vec2>& rght) {
    // Cross bar
    Vec2 h = (1 - tp)*src[soff + 1] + tp*src[soff + 2];
    
    // FIRST HALF
    left.resize(4, Vec2::ZERO);
    left[0] = src[soff];
    left[1] = (1 - tp)*src[soff] + tp*src[soff + 1];
    left[2] = (1 - tp)*left[1] + tp*h;
    
    // SECOND HALF
    rght.resize(4, Vec2::ZERO);
    rght[3] = src[soff + 3];
    rght[2] = (1 - tp)*src[soff + 2] + tp*src[soff + 3];
    rght[1] = (1 - tp)*h + tp*rght[2];
    rght[0] = (1 - tp)*left[2] + tp*rght[1];
    
    left[3] = rght[0];
}

/**
 * Returns the projection polynomial for the given point.
 *
 * The projection polynomial is used to find the nearest value to point
 * on the spline, as described at
 *
 * http://jazzros.blogspot.com/2011/03/projecting-point-on-bezier-curve.html
 *
 * There is no one projection polynomial for the entire spline. Each
 * segment bezier has its own polynomial.
 *
 * @param  point    the point to project
 * @param  segment  the bezier segment to project upon
 */
Polynomial CubicSpline::getProjectionPolynomial(const Vec2& point, int segment) const {
    CUAssertLog(segment >= 0 && segment < _size, "Illegal spline segment");
    
    Vec2 a = _points[3 * segment + 3] - 3 * _points[3 * segment + 2] + 3 * _points[3 * segment + 1] - _points[3 * segment];
    Vec2 b = 3 * _points[3 * segment + 2] - 6 * _points[3 * segment + 1] + 3 * _points[3 * segment];
    Vec2 c = 3 * (_points[3 * segment + 1] - _points[3 * segment]);
    Vec2 p = _points[3 * segment] - point;
    
    Polynomial result(5);
    result[0] = 3.0f*a.dot(a);                  // Q5
    result[1] = 5.0f*a.dot(b);					// Q4
    result[2] = 4.0f*a.dot(c) + 2.0f*b.dot(b);	// Q3
    result[3] = 3.0f*b.dot(c) + 3.0f*a.dot(p);	// Q2
    result[4] = c.dot(c) + 2.0f*b.dot(p);       // Q1
    result[5] = c.dot(p);                       // Q0
    return result;
}

/**
 * Returns the parameterization of the nearest point on the bezier segment.
 *
 * The value is effectively the projection of the point onto the parametrized
 * curve. See getPoint() for an explanation of how the parameterization work.
 *
 * This version does not use the projection polynomial.  Instead, it picks
 * a parameter resolution and walks the entire length of the curve.  The
 * result is both slow and inexact (as the actual point may be in-between
 * chose parameters). This version is only picked when getProjectionFast
 * fails because of an error with root finding.
 *
 * The value returned is a pair of the parameter, and its distance value.
 * This allows us to compare this result to other segments, picking the
 * best value for the entire spline.
 *
 * @param  point    the point to project
 * @param  segment  the bezier segment to project upon
 *
 * @return the parameterization of the nearest point on the spline.
 */
Vec2 CubicSpline::getProjectionSlow(const Vec2& point, int segment) const {
    Vec2 result(-1, -1);
    
    int RESOLUTION = (1 << MAX_DEPTH);
    for (int jj = 0; jj < RESOLUTION; jj++) {
        float t = ((float)jj) / RESOLUTION;
        Vec2 temp0 = getPoint(segment,t);
        temp0 -= point;
        float d = temp0.lengthSquared();
        if (result.x == -1 || d < result.y) {
            result.x = t; result.y = d;
        }
    }
    
    // Compare the last point.
    Vec2 temp0 = _points[3 * _size] - point;
    float d = temp0.lengthSquared();
    if (d < result.y) {
        result.x = 1.0f; result.y = d;
    }
    return result;
}

/**
 * Returns the parameterization of the nearest point on the bezier segment.
 *
 * The value is effectively the projection of the point onto the parametrized
 * curve. See getPoint() for an explanation of how the parameterization work.
 *
 * The value is effectively the projection of the point onto the parametrized
 * curve. See getPoint() for an explanation of how the parameterization work. We
 * compute this value using the projection polynomial, described at
 *
 * http://jazzros.blogspot.com/2011/03/projecting-point-on-bezier-curve.html
 *
 * The value returned is a pair of the parameter, and its distance value.
 * This allows us to compare this result to other segments, picking the
 * best value for the entire spline.
 *
 * This algorithm uses the projection polynomial, and searches for roots to
 * find the best (max of 5) candidates.  However, root finding may fail,
 * do to singularities in Bairstow's Method.  If the root finder fails, then
 * the first element of the pair will be -1 (an invalid parameter).
 *
 * @param  point    the point to project
 * @param  segment  the bezier segment to project upon
 *
 * @return the parameterization of the nearest point on the spline.
 */
Vec2 CubicSpline::getProjectionFast(const Vec2& point, int segment) const {
    vector<float> roots;
    
    Polynomial poly = getProjectionPolynomial(point, segment);
    
    float epsilon = 1.0f / (1 << (MAX_DEPTH + 1));
    bool success = poly.roots(roots, epsilon);
    if (!success) {
        // This will kick us to the slow method
        return Vec2(-1, 0);
    }
    
    Vec2 result;
    
    // Now compare the candidates
    result.x = 0.0f;
    Vec2 compare = getPoint(segment,1.0f) - point;
    result.y = compare.lengthSquared();
    
    float t = 1.0f;
    compare = getPoint(segment,1.0f) - point;
    float d = compare.lengthSquared();
    if (d < result.y) {
        result.set(t, d);
    }
    
    // Check the roots
    int RESOLUTION = (1 << MAX_DEPTH);
    for (auto it = roots.begin(); it != roots.end(); ++it) {
        float r = *it;
        if (r != nanf("") && r > 0 && r < 1) {
            // Convert to nearest step.
            t = r*RESOLUTION;
            t = roundf(t);
            t /= RESOLUTION;
            compare = getPoint(segment,t) - point;
            d = compare.lengthSquared();
            if (d < result.y) {
                result.set(t, d);
            }
        }
    }
    
    return result;
}

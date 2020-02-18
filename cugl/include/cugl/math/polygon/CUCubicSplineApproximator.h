//
//  CUCubicSplineApproximator.h
//  Cornell University Game Library (CUGL)
//
//  This module is a factory for producing Poly2 objects from CubicSpline.
//  In previous interations, this functionality was embedded in the CubicSpline
//  class.  That made that class much more heavyweight than we wanted for a
//  a simple math class.  By separating this out as a factory, we allow ourselves
//  the option of moving these calculations to a worker thread if necessary.
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

#ifndef __CU_CUBIC_SPLINE_APPROXIMATOR_H__
#define __CU_CUBIC_SPLINE_APPROXIMATOR_H__

#include <cugl/math/CUPoly2.h>
#include <cugl/math/CUCubicSpline.h>
#include <cugl/math/CUVec2.h>
#include <vector>

/** The default tolerance for the polygon approximation functions */
#define DEFAULT_TOLERANCE   0.25

namespace cugl {

/**
 * This class is a factory for producing Poly2 objects from a CubicSpline.
 *
 * In order to draw a cubic spline, we must first convert it to a Poly2 
 * object.  All of our rendering tools are designed around the basic Poly2
 * class.  In addition to generating a Poly2 for the spline path, this class 
 * can also generate Poly2 objects for UI elements such as handles and anchors.
 *
 * As with all factories, the methods are broken up into three phases:
 * initialization, calculation, and materialization.  To use the factory, you
 * first set the data (in this case a pointer to a CubicSpline) with the 
 * initialization methods.  You then call the calculation method.  Finally,
 * you use the materialization methods to access the data in several different
 * ways.
 *
 * This division allows us to support multithreaded calculation if the data
 * generation takes too long.  However, not that this factory keeps a pointer
 * to the spline, and it is unsafe to modify the spline while the calculation
 * is ongoing.  If you do multithread the calculation, you should force the
 * user to copy the spline first.
 */
class CubicSplineApproximator {
#pragma mark Values
private:
    /** A pointer to the spline data */
    const CubicSpline* _spline;
    /** The control data created by the approximation */
    std::vector<Vec2>  _pointbuff;
    /** The parameter data created by the approximation */
    std::vector<float> _parambuff;
    /** Whether the approximation curve is closed */
    bool _closed;
    /** Whether or not the calculation has been run */
    bool _calculated;
    
public:
    /**
     * Termination criteria for de Castlejau's recursive subdivision
     *
     * This is used by the polygon approximation functions.  To convert a bezier
     * into a polygon, we recursively subdivide the bezier until we reach the
     * terminal condition.  We then use the anchor points of the subdivided
     * bezier to define our polygon.
     */
    enum Criterion {
        /**
         * The FLAT termination criterion.
         *
         * It guarantees a limit on the flatness (which in this context means
         * the distance from the curve to the polygon divided by point spacing).
         */
        FLAT,
        
        /**
         * The DISTANCE termination criterion.
         *
         * It guarantees that the curve lies within a certain distance from the
         * polygon defined by the points.
         */
        DISTANCE,
        
        /**
         * The SPACING termination criterion .
         *
         * It guarantees that the points will be less than a certain distance 
         * apart.
         */
        SPACING
    };

#pragma mark -
#pragma mark Constructors
    /**
     * Creates a spline approximator with no spline data.
     */
    CubicSplineApproximator() : _spline(nullptr), _calculated(false) { }

    /**
     * Creates a spline approximator with the given spline as its initial data.
     *
     * @param spline    The spline to approximate
     */
    CubicSplineApproximator(const CubicSpline* spline) : _spline(spline), _calculated(false) { }

    /**
     * Deletes this spline approximator, releasing all resources.
     */
    ~CubicSplineApproximator();
    

#pragma mark -
#pragma mark Initialization
    /**
     * Sets the given spline as the data for this spline approximator.
     *
     * This method resets all interal data.  You will need to reperform the
     * calculation before accessing data.
     *
     * @param spline    The spline to approximate
     */
    void set(const CubicSpline* spline) {
        reset();
        _spline = spline;
    }

    /**
     * Clears all internal data, but still maintains a reference to the spline.
     *
     * Use this method when you want to reperform the approximation at a 
     * different resolution.  
     */
    void reset() {
        _calculated = false;
        _pointbuff.clear(); _parambuff.clear();
    }

    /**
     * Clears all internal data, including the spline data.
     *
     * When this method is called, you will need to set a new spline before
     * calling calculate.
     */
    void clear() {
        _calculated = false;
        _spline = nullptr;
        _pointbuff.clear(); _parambuff.clear();
    }


#pragma mark -
#pragma mark Calculation
    /**
     * Performs an approximation of the current spline
     *
     * A polygon approximation is creating by recursively calling de Castlejau's
     * until we reach a stopping condition. The stopping condition is determined
     * by the {@link Criterion}.  See that enum for a description of how the
     * various stopping conditions work.  The tolerance is the value associated
     * with the condition.  For example, for condition DISTANCE, tolerance is how
     * far the point can be away from the true curve.
     *
     * The calculation uses a reference to the spline; it does not copy it. 
     * Hence this method is not thread-safe.  If you are using this method in
     * a task thread, you should copy the spline first before starting the
     * calculation.
     *
     * @param  criterion    the stopping condition criterion
     * @param  tolerance    the error tolerance of the stopping condition
     */
    void calculate(Criterion criterion=Criterion::DISTANCE, float tolerance=DEFAULT_TOLERANCE);
    

#pragma mark -
#pragma mark Materialization
    /**
     * Returns a new polygon approximating this spline.
     *
     * The Poly2 indices will define a path traversing the vertices of the 
     * polygon.  Hence this Poly2 may be drawn as a wireframe.  The indices
     * will define a closed path if the spline is itself closed, and an open
     * path otherwise.
     *
     * The resolution of the polygon is determined by the {@link calculate()}
     * method.  See the description of that method for the various options.
     * If calculate has not been called, this method will create a polygon
     * from the control points on the original spline.
     *
     * @return a new polygon approximating this spline.
     */
    Poly2 getPath() const;

    /**
     * Stores vertex information approximating this spline in the buffer.
     *
     * The Poly2 indices will define a path traversing the vertices of the
     * polygon.  Hence this Poly2 may be drawn as a wireframe.  The indices
     * will define a closed path if the spline is itself closed, and an open
     * path otherwise.
     *
     * The resolution of the polygon is determined by the {@link calculate()}
     * method.  See the description of that method for the various options.
     * If calculate has not been called, this method will create a polygon
     * from the control points on the original spline.
     *
     * The vertices (and indices) will be appended to the the Poly2 if it is
     * not empty. You should clear the Poly2 first if you do not want to 
     * preserve the original data.
     *
     * @param buffer    The buffer to store the vertex data
     *
     * @return a reference to the buffer for chaining.
     */
    Poly2* getPath(Poly2* buffer) const;

    /**
     * Returns a list of parameters for a polygon approximation
     *
     * The parameters correspond to the generating values in the spline 
     * polynomial.  That is, if you evaluate the polynomial on the parameters,
     * {via {@link CubicSpline#getPoint()}, you will get the points in the
     * approximating polygon.
     *
     * The resolution of the polygon is determined by the {@link calculate()}
     * method.  See the description of that method for the various options.
     * If calculate has not been called, this method will choose parameters
     * for the control points on the original spline.
     *
     * @return a list of parameters for a polygon approximation
     */
    std::vector<float> getParameters() const;
    
    /**
     * Stores a list of parameters for the approximation in the buffer.
     *
     * The parameters correspond to the generating values in the spline
     * polynomial.  That is, if you evaluate the polynomial on the parameters,
     * {via {@link CubicSpline#getPoint()}, you will get the points in the
     * approximating polygon.
     *
     * The resolution of the polygon is determined by the {@link calculate()}
     * method.  See the description of that method for the various options.
     * If calculate has not been called, this method will choose parameters
     * for the control points on the original spline.
     *
     * The parameters will be appended to the buffer vector.  You should clear 
     * the buffer first if you do not want to preserve the original data.
     *
     * @param buffer    The buffer to store the parameter data
     *
     * @return the number of elements added to the buffer
     */
    size_t getParameters(std::vector<float> buffer);
    
    /**
     * Returns a list of tangents for a polygon approximation
     *
     * These tangent vectors are presented in control point order.  First, we 
     * have the right tangent of the first point, then the left tangent of the
     * second point, then the right, and so on.  Hence if the polygon contains
     * n points, this method will return 2(n-1) tangents.
     *
     * The resolution of the polygon is determined by the {@link calculate()}
     * method.  See the description of that method for the various options.
     * If calculate has not been called, this method will choose tangents
     * for the control points on the original spline.  This latter option
     * is useful when you want to draw a UI for the control point tangents.
     *
     * @return a list of tangents for a polygon approximation
     */
    std::vector<Vec2> getTangents() const;
    
    /**
     * Stores a list of tangents for the approximation in the buffer.
     *
     * These tangent vectors are presented in control point order.  First, we
     * have the right tangent of the first point, then the left tangent of the
     * second point, then the right, and so on.  Hence if the polygon contains
     * n points, this method will return 2(n-1) tangents.
     *
     * The resolution of the polygon is determined by the {@link calculate()}
     * method.  See the description of that method for the various options.
     * If calculate has not been called, this method will choose tangents
     * for the control points on the original spline.  This latter option
     * is useful when you want to draw a UI for the control point tangents.
     *
     * The tangents will be appended to the buffer vector.  You should clear
     * the buffer first if you do not want to preserve the original data.
     *
     * @return the number of elements added to the buffer
     */
    size_t getTangents(std::vector<Vec2> buffer);

    /**
     * Stores tangent data for the approximation in the buffer.
     *
     * When complete, the Poly2 will contain data for 2(n-1) lines, where the
     * polygon contains n points.  Hence this Poly2 may be drawn as a wireframe. 
     * Each line is a tangent vector anchored at its associated control point. 
     * The length of the line is determined by the length of the tangent vector.  
     *
     * The resolution of the polygon is determined by the {@link calculate()}
     * method.  See the description of that method for the various options.
     * If calculate has not been called, this method will choose tangents
     * for the control points on the original spline.  This latter option
     * is useful when you want to draw a UI for the control point tangents.
     *
     * The tangent data will be appended to the buffer.  You should clear
     * the buffer first if you do not want to preserve the original data.
     *
     * @return the number of elements added to the buffer
     */
    Poly2* getTangents(Poly2* buffer);

    /**
     * Returns a list of normals for a polygon approximation
     *
     * There is one normal per control point. If polygon contains n points, 
     * this method will also return n normals. The normals are determined by the
     * right tangents.  If the spline is open, then the normal of the last point
     * is determined by its left tangent.
     *
     * The resolution of the polygon is determined by the {@link calculate()}
     * method.  See the description of that method for the various options.
     * If calculate has not been called, this method will choose normals
     * for the control points on the original spline.  This latter option
     * is useful when you want to draw a UI for the control point normals.
     *
     * @return a list of normals for a polygon approximation
     */
    std::vector<Vec2> getNormals() const;

    /**
     * Stores a list of normals for the approximation in the buffer.
     *
     * There is one normal per control point. If polygon contains n points,
     * this method will also return n normals. The normals are determined by the
     * right tangents.  If the spline is open, then the normal of the last point
     * is determined by its left tangent.
     *
     * The resolution of the polygon is determined by the {@link calculate()}
     * method.  See the description of that method for the various options.
     * If calculate has not been called, this method will choose normals
     * for the control points on the original spline.  This latter option
     * is useful when you want to draw a UI for the control point normals.
     *
     * The normals will be appended to the buffer vector.  You should clear
     * the buffer first if you do not want to preserve the original data.
     *
     * @return the number of elements added to the buffer
     */
    size_t getNormals(std::vector<Vec2> buffer);

    /**
     * Stores normal data for the approximation in the buffer.
     *
     * When complete, the Poly2 will contain data for n lines, where the
     * polygon contains n points.  Hence this Poly2 may be drawn as a wireframe.
     * Each line is a normal vector anchored at its associated control point.
     * The length of the line is determined by the length of the associated
     * tangent vector (right tangent, except for the last control point).
     *
     * The resolution of the polygon is determined by the {@link calculate()}
     * method.  See the description of that method for the various options.
     * If calculate has not been called, this method will choose normals
     * for the control points on the original spline.  This latter option
     * is useful when you want to draw a UI for the control point normals.
     *
     * The normal data will be appended to the buffer.  You should clear
     * the buffer first if you do not want to preserve the original data.
     *
     * @return the number of elements added to the buffer
     */
    Poly2* getNormals(Poly2* buffer);
    
    /**
     * Returns a Poly2 representing handles for the anchor points
     *
     * This method returns a collection of vertex information for handles at 
     * the anchor points.  Handles are circular shapes of a given radius. This 
     * information may be drawn to provide a visual representation of the
     * anchor points (as seen in Adobe Illustrator).
     *
     * The resolution of the polygon is determined by the {@link calculate()}
     * method.  See the description of that method for the various options.
     * If calculate has not been called, this method will choose anchors
     * for the control points on the original spline.  This latter option
     * is useful when you want to draw a UI for the original control points.
     *
     * @param  radius   the radius of each handle
     * @param  segments the number of segments in the handle "circle"
     *
     * @return a Poly2 representing handles for the anchor points
     */
    Poly2 getAnchors(float radius, int segments=4) const;
    
    /**
     * Stores vertex information representing the anchor points in the buffer.
     *
     * This method creates a collection of vertex information for handles at
     * the anchor points.  Handles are circular shapes of a given radius. This
     * information may be drawn to provide a visual representation of the
     * anchor points (as seen in Adobe Illustrator).
     *
     * The resolution of the polygon is determined by the {@link calculate()}
     * method.  See the description of that method for the various options.
     * If calculate has not been called, this method will choose anchors
     * for the control points on the original spline.  This latter option
     * is useful when you want to draw a UI for the original control points.
     *
     * The vertices (and indices) will be appended to the the Poly2 if it is
     * not empty. You should clear the Poly2 first if you do not want to
     * preserve the original data.
     *
     * @param  buffer   The buffer to store the vertex data
     * @param  radius   The radius of each handle
     * @param  segments The number of segments in the handle "circle"
     *
     * @return a reference to the buffer for chaining.
     */
    Poly2* getAnchors(Poly2* buffer, float radius, int segments=4) const;
    
    /**
     * Returns a Poly2 representing handles for the tangent points
     *
     * This method returns vertex information for handles at the tangent
     * points.  Handles are circular shapes of a given radius. This information
     * may be passed to a PolygonNode to provide a visual representation of the
     * tangent points (as seen in Adobe Illustrator).
     *
     * The resolution of the polygon is determined by the {@link calculate()}
     * method.  See the description of that method for the various options.
     * If calculate has not been called, this method will choose the tangents
     * from the control points on the original spline.  This latter option
     * is useful when you want to draw a UI for the original tangent points.
     *
     * @param  radius   the radius of each handle
     * @param  segments the number of segments in the handle "circle"
     *
     * @return a Poly2 representing handles for the tangent points
     */
    Poly2 getHandles(float radius, int segments=4) const;

    /**
     * Stores vertex information representing tangent point handles in the buffer.
     *
     * This method creates vertex information for handles at the tangent
     * points.  Handles are circular shapes of a given radius. This information
     * may be passed to a PolygonNode to provide a visual representation of the
     * tangent points (as seen in Adobe Illustrator).
     *
     * The resolution of the polygon is determined by the {@link calculate()}
     * method.  See the description of that method for the various options.
     * If calculate has not been called, this method will choose the tangents
     * from the control points on the original spline.  This latter option
     * is useful when you want to draw a UI for the original tangent points.
     *
     * The vertices (and indices) will be appended to the the Poly2 if it is
     * not empty. You should clear the Poly2 first if you do not want to
     * preserve the original data.
     *
     * @param  buffer   The buffer to store the vertex data
     * @param  radius   the radius of each handle
     * @param  segments the number of segments in the handle "circle"
     *
     * @return a reference to the buffer for chaining.
     */
    Poly2* getHandles(Poly2* buffer, float radius, int segments=4) const;

    /**
     * Returns an expanded version of this spline
     *
     * When we use de Castlejau's to approximate the spline, it produces a list
     * of control points that are geometrically equal to this spline (e.g. ignoring
     * parameterization). Instead of flattening this information to a polygon,
     * this method presents this data as a new spline.
     *
     * The resolution of the polygon is determined by the {@link calculate()}
     * method.  See the description of that method for the various options.
     * If calculate has not been called, this method will copy the original spline.
     *
     * @return an expanded version of this spline
     */
    CubicSpline getRefinement() const;
    
    /**
     * Stores an expanded version of this spline in the given buffer.
     *
     * When we use de Castlejau's to approximate the spline, it produces a list
     * of control points that are geometrically equal to this spline (e.g. ignoring
     * parameterization). Instead of flattening this information to a polygon,
     * this method presents this data as a new spline.
     *
     * The resolution of the polygon is determined by the {@link calculate()}
     * method.  See the description of that method for the various options.
     * If calculate has not been called, this method will copy the original spline.
     *
     * The control points will be appended to the the spline if it is not
     * empty. You should clear the spline first if you do not want to preserve 
     * the original data.
     *
     * @param  buffer   The buffer to store the vertex data
     *
     * @return a reference to the buffer for chaining.
     */
    CubicSpline* getRefinement(CubicSpline* buffer) const;

#pragma mark -
#pragma mark Internal Data Generation
private:
    /**
     * Generates data via recursive use of de Castlejau's
     *
     * This method is the recursive helper for calculate(). It performs
     * de Castlejau's algorithm and stores the data in the buffer.  You 
     * will never call this method directly.
     *
     * @param  src          the control point list for the bezier
     * @param  soff         the offset into the control point list
     * @param  tp           the parameter to split at
     * @param  tolerance    the error tolerance of the stopping condition
     * @param  criterion    the stopping condition criterion
     * @param  depth        the current depth of the recursive call
     *
     * @return The number of (anchor) points generated by this recursive call.
     */
    int generate(const std::vector<Vec2>& src, int soff, float tp,
                 float tolerance, Criterion criterion, int depth);

    /**
     * Returns the currently "active" control points.
     *
     * If the calculation has been run, this is the data for the calculation.
     * Otherwise, it is the control points of the original spline
     */
    const std::vector<Vec2>* getActivePoints() const {
        return (_calculated ? &_pointbuff : (_spline ? &(_spline->_points) : nullptr));
    }
    
    /** 
     * Returns true if the current approximation is closed.
     *
     * @return true if the current approximation is closed.
     */
    bool isClosed() const {
        return (_calculated ? _closed : (_spline ? _spline->_closed : false));
    }
};

}
#endif /* __CU_CUBIC_SPLINE_APPROXIMATOR_H__ */

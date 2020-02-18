//
//  CUPathExtruder.cpp
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

#include <cugl/math/polygon/CUPathExtruder.h>
#include <cugl/util/CUDebug.h>
#include <iterator>

/** The number of segments to use in a rounded joint */
#define JOINT_PRECISION 10
/** The number of segments to use in a rounded cap */
#define CAP_PRECISION   10

namespace cugl {

/**
 * Data necessary for a single iteration of the Kivy algorithm
 *
 * The Kivy algorithm is great, but it is a tangled mess. There is a lot of
 * information that needs to be carried across iterations.  At each step, we
 * need to know the index position from the previous iteration, as well as from
 * two iterations ago.
 *
 * All of the data means that it is very hard to break the algorithm up
 * into multiple functions. The component functions would take larger numbers
 * of arguments, all passed by reference.  However, a function that long is a 
 * sin.  So we use this class to encapsulate the data we need.
 *
 * We are not even going to try to explain why all these are necessary.
 */
class KivyData {
public:
    /** The current algorithm iteraction */
    unsigned int index;
    /** The path stroke */
    float stroke;
    /** The path joint */
    PathJoint joint;
    /** The path cap */
    PathCap cap;
    /** The vector for the current segement */
    Vec2 c;
    /** The vector for the previous segement */
    Vec2 pc;
    /** The first side of the segment rectangle */
    Vec2 v1;
    /** The second side of the segment rectangle */
    Vec2 v2;
    /** The third side of the segment rectangle */
    Vec2 v3;
    /** The fourth side of the segment rectangle */
    Vec2 v4;
    /** The first side of the previous segment rectangle */
    Vec2 p1;
    /** The second side of the previous segment rectangle */
    Vec2 p2;
    /** The third side of the previous segment rectangle */
    Vec2 p3;
    /** The fourth side of the previous segment rectangle */
    Vec2 p4;
    /** The top edge of the current segement */
     Vec2 s1;
    /** The bottom edge of the current segement */
     Vec2 s4;
    /** The current angle */
    float angle;
    /** The current segment angle */
    float sangle;
    /** The previous angle */
    float pangle;
    /** The previous previous angle */
    float pangle2;
    /** The current vertex index */
    unsigned int pos;
    /** The previous vertex index */
    unsigned int ppos;
    /** The previous previous vertex index */
    unsigned int p2pos;
};
}

using namespace cugl;

#pragma mark -
#pragma mark Initialization
/**
 * Sets the vertex data for this extruder.
 *
 * The extrusion only uses the vertex data from the polygon.  It ignores any
 * existing indices. The method assumes the polygon is closed if the
 * number of indices is twice the number of vertices.
 *
 * The vertex data is copied.  The extruder does not retain any references
 * to the original data.
 *
 * This method resets all interal data.  You will need to reperform the
 * calculation before accessing data.
 *
 * @param poly    The vertices to extrude
 */
void PathExtruder::set(const Poly2& poly) {
    CUAssertLog(poly.getType() == Poly2::Type::PATH,
                "The polygon is not a path");
    reset();
    _input = poly._vertices;
    _closed = poly._indices.size() == poly._vertices.size()*2;
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
 * package kivy.vertex_instructions.  My belief is that this port is
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
void PathExtruder::calculate(float stroke, PathJoint joint, PathCap cap) {
    if (_input.size() == 0) {
        _calculated = true;
        return;
    }
        _outverts.clear();
    _outindx.clear();
    
    // Closed paths have no cap;
    if (_closed && _input.size() > 2) {
        cap = PathCap::NONE;
    }

    // Determine how large the new polygon is
    unsigned int vcount, icount;
    unsigned int count = computeSize(joint, cap, &vcount, &icount);
    _outverts.reserve(vcount*2);
    _outindx.reserve(icount);
    
    // Thanks Kivy guys for all the hard work.
    KivyData data;
    
    // Initialize the data
    data.stroke = stroke;
    data.joint = joint;
    data.cap = cap;
    
    // Iterate through the path
    data.angle = data.sangle = 0;
    data.pangle = data.pangle2 = 0;
    data.pos = data.ppos = data.p2pos = 0;
    int mod = (int)_input.size();
    for(int ii = 0; ii < count-1; ii++) {
        Vec2 a = _input[  ii   % mod];
        Vec2 b = _input[(ii+1) % mod];
        data.index = ii;
        
        makeSegment(a, b, &data);
        makeJoint(a, &data);
    }
    
    // Process the caps
    makeCaps(count, &data);
    
    // If closed, make one last joint
    if (_closed && mod > 2) {
        makeLastJoint(&data);
    }
    
    _calculated = true;
}

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
unsigned int PathExtruder::computeSize(PathJoint joint, PathCap cap,
                                       unsigned int* vcount, unsigned int* icount) {
    unsigned int count = (int)_input.size();
    if (_closed && _input.size() > 2) {
        count += 1;
    }
    
    // Determine the number of vertices and indices we need.
    *vcount = (count - 1) * 4;
    *icount = (count - 1) * 6;
    
    switch (joint) {
        case PathJoint::BEVEL:
            *icount += (count - 2) * 3;
            *vcount += (count - 2);
            break;
        case PathJoint::ROUND:
            *icount += (JOINT_PRECISION * 3) * (count - 2);
            *vcount += (JOINT_PRECISION) * (count - 2);
            break;
        case PathJoint::MITRE:
            *icount += (count - 2) * 6;
            *vcount += (count - 2) * 2;
            break;
        case PathJoint::NONE:
            // Nothing to do.
            break;
    }
    
    switch (cap) {
        case PathCap::SQUARE:
            *icount += 12;
            *vcount += 4;
            break;
        case PathCap::ROUND:
            *icount += (CAP_PRECISION * 3) * 2;
            *vcount += (CAP_PRECISION) * 2;
            break;
        case PathCap::NONE:
            // Nothing to do.
            break;
    }
    return count;
}

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
void PathExtruder::makeSegment(const Vec2& a, const Vec2& b, KivyData* data) {
    if (data->index > 0 && data->joint != PathJoint::NONE) {
        data->pc = data->c;
        data->p1 = data->v1; data->p2 = data->v2;
        data->p3 = data->v3; data->p4 = data->v4;
    }
    
    data->p2pos = data->ppos; data->ppos = data->pos;
    data->pangle2 = data->pangle; data->pangle = data->angle;
    
    // Calculate the orientation of the segment, between pi and -pi
    data->c = b - a;
    data->angle = atan2(data->c.y, data->c.x);
    float a1 = data->angle - M_PI_2;
    float a2 = data->angle + M_PI_2;
    
    // Calculate the position of the segment
    Vec2 temp1 = Vec2(cos(a1) * data->stroke, sin(a1) * data->stroke);
    Vec2 temp2 = Vec2(cos(a2) * data->stroke, sin(a2) * data->stroke);
    
    data->v1 = a+temp1;
    data->v4 = a+temp2;
    data->v2 = b+temp1;
    data->v3 = b+temp2;
    
    if (data->index == 0) {
        data->s1 = data->v1; data->s4 = data->v4;
        data->sangle = data->angle;
    }
    
    // Add the indices
    _outindx.push_back(data->pos  );
    _outindx.push_back(data->pos+1);
    _outindx.push_back(data->pos+2);
    _outindx.push_back(data->pos  );
    _outindx.push_back(data->pos+2);
    _outindx.push_back(data->pos+3);
    
    // Add the vertices
    _outverts.push_back(data->v1);
    _outverts.push_back(data->v2);
    _outverts.push_back(data->v3);
    _outverts.push_back(data->v4);
    data->pos += 4;
}

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
bool PathExtruder::makeJoint(const Vec2& a, KivyData* data) {
    if (data->index == 0 || data->joint == PathJoint::NONE) {
        return false;
    }
    
    // calculate the angle of the previous and current segment
    float jangle = atan2(data->c.x * data->pc.y - data->c.y * data->pc.x,
                         data->c.x * data->pc.x + data->c.y * data->pc.y);
    
    // in case of the angle is NULL, avoid the generation
    if (jangle == 0) {
        return false;
    }
    
    // Send to the specific joints
    switch (data->joint) {
        case PathJoint::BEVEL:
            return makeBevelJoint(a, jangle, data);
        case PathJoint::MITRE:
            return makeMitreJoint(a,jangle,data);
        case PathJoint::ROUND:
            return makeRoundJoint(a,jangle,data);
        case PathJoint::NONE:
            // Nothing to do
            break;
    }
    return true;
}

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
bool PathExtruder::makeMitreJoint(const Vec2& a, float jangle, KivyData* data) {
    _outverts.push_back(a);
    
    // Indices depend on angle
    if (jangle < 0) {
        float s, t;
        if (Vec2::doesLineIntersect(data->p1, data->p2, data->v1, data->v2, &s,&t)) {
            Vec2 temp = data->p1 + s*(data->p2-data->p1);
            _outverts.push_back(temp);
            _outindx.push_back(data->pos    );
            _outindx.push_back(data->pos+1  );
            _outindx.push_back(data->p2pos+1);
            _outindx.push_back(data->pos    );
            _outindx.push_back(data->ppos   );
            _outindx.push_back(data->pos+1  );
            data->pos += 2;
            return true;
        }
    } else {
        float s, t;
        if (Vec2::doesLineIntersect(data->p3, data->p4, data->v3, data->v4, &s, &t)) {
            Vec2 temp = data->p3 + s*(data->p4-data->p3);
            _outverts.push_back(temp);
            _outindx.push_back(data->pos    );
            _outindx.push_back(data->pos+1  );
            _outindx.push_back(data->p2pos+2);
            _outindx.push_back(data->pos    );
            _outindx.push_back(data->ppos+3 );
            _outindx.push_back(data->pos+1  );
            data->pos += 2;
            return true;
        }
    }
    return false;
}

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
bool PathExtruder::makeBevelJoint(const Vec2& a, float jangle, KivyData* data) {
    _outverts.push_back(a);

    // Indices depend on angle
    if (jangle < 0) {
        _outindx.push_back(data->p2pos+1);
        _outindx.push_back(data->ppos   );
        _outindx.push_back(data->pos    );
    } else {
        _outindx.push_back(data->p2pos+2);
        _outindx.push_back(data->ppos +3);
        _outindx.push_back(data->pos    );
    }
    data->pos += 1;
    return true;
}

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
bool PathExtruder::makeRoundJoint(const Vec2& a, float jangle, KivyData* data) {
    float a0, step;
    unsigned int s_pos, e_pos;
    
    if (jangle < 0) {
        a0 = data->angle + M_PI_2;
        step = (fabsf(jangle)) / (float)JOINT_PRECISION;
        s_pos = data->ppos + 3;
        e_pos = data->p2pos + 1;
    } else {
        a0 = data->angle - M_PI_2;
        step = -(fabsf(jangle)) / (float)JOINT_PRECISION;
        s_pos = data->ppos;
        e_pos = data->p2pos + 2;
    }
    
    unsigned int opos = data->pos;
    _outverts.push_back(a);
    data->pos += 1;
    for(int j = 0; j <  JOINT_PRECISION - 1; j++) {
        _outverts.push_back(a-Vec2(cos(a0 - step * j) * data->stroke,
                                   sin(a0 - step * j) * data->stroke));
        if (j == 0) {
            _outindx.push_back(opos );
            _outindx.push_back(s_pos);
            _outindx.push_back(data->pos);
        } else {
            _outindx.push_back(opos );
            _outindx.push_back(data->pos-1);
            _outindx.push_back(data->pos);
        }
        data->pos += 1;
    }
    
    _outindx.push_back(opos );
    _outindx.push_back(data->pos-1);
    _outindx.push_back(e_pos);
    return true;
}

/**
 * Creates the caps on the two ends of the open path.
 *
 * The new vertices are appended to _outvert, while the new _indices are
 * appended to _outindx.
 *
 * @param count     The number of generating points in the path.
 * @param data      The data necessary to run the Kivy algorithm.
 *
 * @return true if a joint was successfully created.
 */
void PathExtruder::makeCaps(int count, KivyData* data) {
    switch (data->cap) {
        case PathCap::SQUARE:
            makeSquareCaps(count, data);
            break;
        case PathCap::ROUND:
            makeRoundCaps(count, data);
            break;
        case PathCap::NONE:
            // Nothing to do.
            break;
    }
}

/**
 * Creates square caps on the two ends of the open path.
 *
 * The new vertices are appended to _outvert, while the new _indices are
 * appended to _outindx.
 *
 * @param count     The number of generating points in the path.
 * @param data      The data necessary to run the Kivy algorithm.
 *
 * @return true if a joint was successfully created.
 */
void PathExtruder::makeSquareCaps(int count, KivyData* data) {
    // cap end
    Vec2 temp = Vec2(cos(data->angle) * data->stroke,
                     sin(data->angle) * data->stroke);
    _outverts.push_back(data->v2+temp);
    _outverts.push_back(data->v3+temp);
    _outindx.push_back(data->ppos + 1);
    _outindx.push_back(data->ppos + 2);
    _outindx.push_back(data->pos  + 1);
    _outindx.push_back(data->ppos + 1);
    _outindx.push_back(data->pos);
    _outindx.push_back(data->pos + 1);
    data->pos += 2;
    
    // cap start
    temp = Vec2(cos(data->sangle) * data->stroke,
                sin(data->sangle) * data->stroke);
    _outverts.push_back(data->s1-temp);
    _outverts.push_back(data->s4-temp);
    _outindx.push_back(0);
    _outindx.push_back(3);
    _outindx.push_back(data->pos + 1 );
    _outindx.push_back(0);
    _outindx.push_back(data->pos);
    _outindx.push_back(data->pos + 1 );
    data->pos += 2;
}

/**
 * Creates round caps on the two ends of the open path.
 *
 * The new vertices are appended to _outvert, while the new _indices are
 * appended to _outindx.
 *
 * @param count     The number of generating points in the path.
 * @param data      The data necessary to run the Kivy algorithm.
 *
 * @return true if a joint was successfully created.
 */
void PathExtruder::makeRoundCaps(int count, KivyData* data) {
    // cap start
    float a1 = data->sangle - M_PI_2;
    float a2 = data->sangle + M_PI_2;
    float step = (a1 - a2) / (float)CAP_PRECISION;
    unsigned int opos = data->pos;
    data->c = _input[0];
    _outverts.push_back(data->c);
    data->pos += 1;
    for(int i = 0; i < CAP_PRECISION - 1; i++) {
        Vec2 temp = Vec2(cos(a1 + step * i) * data->stroke,
                         sin(a1 + step * i) * data->stroke);
        _outverts.push_back(data->c+temp);
        if (i == 0) {
            _outindx.push_back(opos);
            _outindx.push_back(0);
            _outindx.push_back(data->pos);
        } else {
            _outindx.push_back(opos);
            _outindx.push_back(data->pos-1);
            _outindx.push_back(data->pos);
        }
        data->pos += 1;
    }
    
    _outindx.push_back(opos );
    _outindx.push_back(data->pos-1);
    _outindx.push_back(3);
    
    // cap end
    a1 = data->angle - M_PI_2;
    a2 = data->angle + M_PI_2;
    step = (a2 - a1) / (float)CAP_PRECISION;
    opos = data->pos;
    data->c = _input[count-1];
    _outverts.push_back(data->c);
    data->pos += 1;
    for(int i = 0; i < CAP_PRECISION - 1; i++) {
        Vec2 temp =  Vec2(cos(a1 + step * i) * data->stroke,
                          sin(a1 + step * i) * data->stroke);
        _outverts.push_back(data->c+temp);
        if (i == 0) {
            _outindx.push_back(opos  );
            _outindx.push_back(data->ppos+1);
            _outindx.push_back(data->pos);
        } else {
            _outindx.push_back(opos );
            _outindx.push_back(data->pos-1);
            _outindx.push_back(data->pos);
        }
        data->pos += 1;
    }
    _outindx.push_back(opos);
    _outindx.push_back(data->pos-1);
    _outindx.push_back(data->ppos+2);
}

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
bool PathExtruder::makeLastJoint(KivyData* data) {
    Vec2 a = _input[0];
    Vec2 b = _input[1];
    
    data->pc = data->c; data->c = b-a;
    data->angle = atan2(data->c.y, data->c.x);
    float a1 = data->angle - M_PI_2;
    float a2 = data->angle + M_PI_2;
    Vec2 temp1, temp2;

    data->ppos = 0;
    float jangle = atan2(data->c.x * data->pc.y - data->c.y * data->pc.x,
                         data->c.x * data->pc.x + data->c.y * data->pc.y);
    
    switch (data->joint) {
        case PathJoint::BEVEL:
            data->p2pos = data->pos-5;
            return makeBevelJoint(a, jangle, data);
        case PathJoint::MITRE:
            data->p1 = data->v1; data->p2 = data->v2;
            data->p3 = data->v3; data->p4 = data->v4;
            
            // Calculate the position of the segment
            temp1 = Vec2(cos(a1) * data->stroke, sin(a1) * data->stroke);
            temp2 = Vec2(cos(a2) * data->stroke, sin(a2) * data->stroke);
            
            data->v1 = a+temp1; data->v4 = a+temp2;
            data->v2 = b+temp1; data->v3 = b+temp2;
            data->p2pos = data->pos-6;
            return makeMitreJoint(a, jangle, data);
        case PathJoint::ROUND:
            data->p2pos = data->pos-JOINT_PRECISION-4;
            return makeRoundJoint(a, jangle, data);
        case PathJoint::NONE:
            // Nothing to do
            break;
    }
    
    return true;
}


#pragma mark -
#pragma mark Materialization
/**
 * Returns a polygon representing the path extrusion.
 *
 * The polygon contains the original vertices together with the new
 * indices defining the wireframe path.  The extruder does not maintain
 * references to this polygon and it is safe to modify it.
 *
 * If the calculation is not yet performed, this method will return the
 * empty polygon.
 *
 * @return a polygon representing the path extrusion.
 */
Poly2 PathExtruder::getPolygon() {
    Poly2 poly;
    if (_calculated) {
        poly._vertices = _outverts;
        poly._indices  = _outindx;
        poly._type = Poly2::Type::SOLID;
        poly.computeBounds();
    }
    return poly;
}

/**
 * Stores the path extrusion in the given buffer.
 *
 * This method will add both the original vertices, and the corresponding
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
Poly2* PathExtruder::getPolygon(Poly2* buffer) {
    CUAssertLog(buffer, "Destination buffer is null");
    if (_calculated) {
        if (buffer->_vertices.size() == 0) {
            buffer->_vertices = _outverts;
            buffer->_indices  = _outindx;
        } else {
            int offset = (int)buffer->_vertices.size();
            buffer->_vertices.reserve(offset+_outverts.size());
            std::copy(_outverts.begin(),_outverts.end(),std::back_inserter(buffer->_vertices));
            
            buffer->_indices.reserve(buffer->_indices.size()+_outindx.size());
            for(auto it = _outindx.begin(); it != _outindx.end(); ++it) {
                buffer->_indices.push_back(offset+*it);
            }
        }
        buffer->_type = Poly2::Type::SOLID;
        buffer->computeBounds();
    }
    return buffer;
}

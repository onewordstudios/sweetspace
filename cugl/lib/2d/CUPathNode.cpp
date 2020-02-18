//
//  CUPathNode.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides a scene graph node that supports extruded paths. When
//  extruding paths, this node is better than PolygonNode, because it will align
//  the extruded path to the original wireframe.
//
//  This class is loosely coupled with PathExtruder.  You can use PathExtruder
//  independent of the PathNode, but all functionality is present in this class.
//
//  This class uses our standard shared-pointer architecture.
//
//  1. The constructor does not perform any initialization; it just sets all
//     attributes to their defaults.
//
//  2. All initialization takes place via init methods, which can fail if an
//     object is initialized more than once.
//
//  3. All allocation takes place via static constructors which return a shared
//     pointer.
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
//  Version: 6/27/16
#include <cugl/2d/CUPathNode.h>
#include <cugl/util/CUDebug.h>

using namespace cugl;

/** For handling JSON issues */
#define UNKNOWN_STR "<unknown>"

#pragma mark Constructors
/**
 * Creates an empty path node.
 *
 * You must initialize this PathNode before use.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
 * the heap, use one of the static constructors instead.
 */
PathNode::PathNode() : TexturedNode(),
_stroke(1.0f),
_closed(true),
_joint(PathJoint::NONE),
_endcap(PathCap::NONE) {
    _classname = "PathNode";
}

/**
 * Intializes a path with the given vertices and stroke width.
 *
 * You do not need to set the texture; rendering this into a SpriteBatch
 * will simply use the blank texture. Hence the wireframe will have a solid
 * color.
 *
 * The polygon will be extruded using the given sequence of vertices.
 * First it will traverse the vertices using either a closed or open
 * traveral.  Then it will extrude that polygon with the given joint
 * and cap. PathNode objects share a single extruder, so this initializer
 * is not thread safe.
 *
 * @param vertices  The vertices to texture (expressed in image space)
 * @param stroke    The stroke width of the extruded path.
 * @param joint     The joint between extrusion line segments.
 * @param cap       The end caps of the extruded paths.
 * @param closed    The whether the vertex path is open or closed.
 *
 * @return  true if the path node is initialized properly, false otherwise.
 */
bool PathNode::initWithVertices(const std::vector<Vec2>& vertices, float stroke,
                                PathJoint joint, PathCap cap, bool closed) {
    _joint  = joint;
    _endcap = cap;
    _closed = true;
    _stroke = stroke;
    return init(vertices);
}

/**
 * Intializes a path node with the given polygon and stroke width.
 *
 * You do not need to set the texture; rendering this into a SpriteBatch
 * will simply use the blank texture. Hence the wireframe will have a solid
 * color.
 *
 * The polygon will be extruded using the given polygon, assuming that it
 * is a (connected) path. It will extrude that polygon with the given joint
 * and cap.  It will assume the polygon is closed if the number of indices
 * is twice the number of vertices. PathNode objects share a single extruder,
 * so this initializer is not thread safe.
 *
 * @param poly      The polygon to texture (expressed in image space)
 * @param stroke    The stroke width of the extruded path.
 * @param joint     The joint between extrusion line segments.
 * @param cap       The end caps of the extruded paths.
 *
 * @return  true if the path node is initialized properly, false otherwise.
 */
bool PathNode::initWithPoly(const Poly2& poly, float stroke, PathJoint joint, PathCap cap) {
    _joint  = joint;
    _endcap = cap;
    _closed = (poly.getVertices().size()*2 == poly.getIndices().size());
    _stroke = stroke;
    return init(poly);
}

/**
 * Initializes a node with the given JSON specificaton.
 *
 * This initializer is designed to receive the "data" object from the
 * JSON passed to {@link SceneLoader}.  This JSON format supports all
 * of the attribute values of its parent class.  In addition, it supports
 * the following additional attributes:
 *
 *      "texture":  The name of a previously loaded texture asset
 *      "polygon":  An even array of polygon vertices (numbers)
 *      "indices":  An array of unsigned ints defining triangles from the
 *                  the vertices. The array size should be a multiple of 3.
 *      'stroke':   A number specifying the stroke width.
 *      'joint':    One of 'mitre', 'bevel', or 'round'.
 *      'cap':      One of 'square' or 'round'.
 *      'closed':   A boolean specifying if the path is closed.
 *
 * All attributes are optional.  However, it is generally a good idea to
 * specify EITHER the texture or the polygon.
 *
 * @param loader    The scene loader passing this JSON file
 * @param data      The JSON object specifying the node
 *
 * @return true if initialization was successful.
 */
bool PathNode::initWithData(const SceneLoader* loader, const std::shared_ptr<JsonValue>& data) {
    if (!data) {
        return init();
    } else if (!TexturedNode::initWithData(loader, data)) {
        return false;
    }
    
    // All of the code that follows can corrupt the position.
    Vec2 coord = getPosition();

    
    _stroke = data->getFloat("stroke", 1.0f);

    std::string joint = data->getString("joint",UNKNOWN_STR);
    if (joint == "mitre") {
        _joint = PathJoint::MITRE;
    } else if (joint == "bevel") {
        _joint = PathJoint::BEVEL;
    } else if (joint == "interior") {
        _joint = PathJoint::ROUND;
    } else {
        _joint = PathJoint::NONE;
    }

    std::string cap = data->getString("cap",UNKNOWN_STR);
    if (cap == "square") {
        _endcap = PathCap::SQUARE;
    } else if (cap == "round") {
        _endcap = PathCap::ROUND;
    } else {
        _endcap = PathCap::NONE;
    }
    
    if (data->has("closed")) {
        _closed = data->getBool("closed",false);
    } else {
        _closed = (_polygon.getVertices().size()*2 == _polygon.getIndices().size());
    }
    
    _extrusion.clear();
    updateExtrusion();
    setPosition(coord);
    return true;
}


#pragma mark -
#pragma mark Attributes
/**
 * Updates the extrusion polygon, based on the current settings.
 */
void PathNode::updateExtrusion() {
    clearRenderData();
    if (_stroke > 0) {
        _extruder.set(_polygon.getVertices(),_closed);
        _extruder.calculate(_stroke,_joint,_endcap);
        _extruder.getPolygon(&_extrusion);
        _extrbounds = _extrusion.getBounds();
        _extrbounds.origin -= _polygon.getBounds().origin;
    } else {
        _extrbounds.set(Vec2::ZERO,getContentSize());
    }
}


/**
 * Sets the stroke width of the path.
 *
 * This method affects the extruded polygon, but not the original path
 * polygon.
 *
 * @param stroke    The stroke width of the path
 */
void PathNode::setStroke(float stroke) {
    CUAssertLog(stroke >= 0, "Stroke width is invalid");
    bool changed = (stroke != _stroke);
    _stroke = stroke;

    if (changed) {
        clearRenderData();
        _extrusion.clear();
        updateExtrusion();
    }
}

/**
 * Sets whether the path is closed.
 *
 * This method affects both the extruded polygon and the original path
 * polygon.
 *
 * @param closed    Whether the path is closed.
 */
void PathNode::setClosed(bool closed) {
    bool changed = (closed != _closed);
    _closed = closed;
    
    if (changed) {
        clearRenderData();
        _extrusion.clear();
        
        _outliner.set(_polygon.getVertices());
        _outliner.calculate(_closed ? PathTraversal::CLOSED : PathTraversal::OPEN);
        _outliner.getPath(_polygon.getIndices());
        
        updateExtrusion();
    }
}

/**
 * Sets the joint type between path segments.
 *
 * This method affects the extruded polygon, but not the original path
 * polygon.
 *
 * @param joint The joint type between path segments
 */
void PathNode::setJoint(PathJoint joint) {
    bool changed = (joint != _joint);
    _joint = joint;
    
    if (changed && _stroke > 0) {
        clearRenderData();
        _extrusion.clear();
        updateExtrusion();
    }
}

/**
 * Sets the cap shape at the ends of the path.
 *
 * This method affects the extruded polygon, but not the original path
 * polygon.
 *
 * @param cap   The cap shape at the ends of the path.
 */
void PathNode::setCap(PathCap cap) {
    bool changed = (cap != _endcap);
    _endcap = cap;
    
    if (changed && _stroke > 0) {
        clearRenderData();
        _extrusion.clear();
        updateExtrusion();
    }
}


#pragma mark -
#pragma mark Polygons
/**
 * Sets the polgon to the vertices expressed in texture space.
 *
 * The polygon will be extruded using the given sequence of vertices.
 * First it will traverse the vertices using the current traversal. Then
 * it will extrude that polygon with the current joint and cap. PathNode
 * objects share a single extruder, so this method is not thread safe.
 *
 * @param vertices  The vertices to texture
 */
void PathNode::setPolygon(const std::vector<Vec2>& vertices) {
    _polygon.set(vertices);
    _outliner.set(_polygon.getVertices());
    _outliner.calculate(_closed ? PathTraversal::CLOSED : PathTraversal::OPEN);
    _outliner.getPath(_polygon.getIndices());
    setPolygon(_polygon);
}

/**
 * Sets the polygon to the given one in texture space.
 *
 * This method will extrude that polygon with the current joint and cap.
 * The polygon is assumed to be closed if the number of indices is twice
 * the number of vertices. PathNode objects share a single extruder, so
 * this method is not thread safe.
 *
 * @param poly  The polygon to texture
 */
void PathNode::setPolygon(const Poly2& poly) {
    _closed = poly.getVertices().size()*2 == poly.getIndices().size();
    TexturedNode::setPolygon(poly);
    updateExtrusion();
}

/**
 * Sets the texture polygon to one equivalent to the given rect.
 *
 * The rectangle will be converted into a Poly2, using the standard outline.
 * This is the same as passing Poly2(rect,false). It will then be extruded
 * with the current joint and cap. PathNode objects share a single extruder,
 * so this method is not thread safe.
 *
 * @param rect  The rectangle to texture
 */
void PathNode::setPolygon(const Rect& rect) {
    setPolygon(Poly2(rect,false));
}


#pragma mark -
#pragma mark Rendering
#pragma mark Rendering
/**
 * Draws this Node via the given SpriteBatch.
 *
 * This method only worries about drawing the current node.  It does not
 * attempt to render the children.
 *
 * This is the method that you should override to implement your custom
 * drawing code.  You are welcome to use any OpenGL commands that you wish.
 * You can even skip use of the SpriteBatch.  However, if you do so, you
 * must flush the SpriteBatch by calling end() at the start of the method.
 * in addition, you should remember to call begin() at the start of the
 * method.
 *
 * This method provides the correct transformation matrix and tint color.
 * You do not need to worry about whether the node uses relative color.
 * This method is called by render() and these values are guaranteed to be
 * correct.  In addition, this method does not need to check for visibility,
 * as it is guaranteed to only be called when the node is visible.
 *
 * @param batch     The SpriteBatch to draw with.
 * @param matrix    The global transformation matrix.
 * @param tint      The tint to blend with the Node color.
 */
void PathNode::draw(const std::shared_ptr<SpriteBatch>& batch, const Mat4& transform, Color4 tint) {
    if (!_rendered) {
        generateRenderData();
    }
    
    batch->setColor(tint);
    batch->setTexture(_texture);
    batch->setBlendEquation(_blendEquation);
    batch->setBlendFunc(_srcFactor, _dstFactor);
    if (_stroke > 0) {
        batch->fill(_vertices.data(),(unsigned int)_vertices.size(),0,
                    _extrusion.getIndices().data(),(unsigned int)_extrusion.getIndices().size(),0,
                    transform);
    } else {
        batch->outline(_vertices.data(),(unsigned int)_vertices.size(),0,
                       _polygon.getIndices().data(),(unsigned int)_polygon.getIndices().size(),0,
                      transform);
    }
}

/**
 * Allocate the render data necessary to render this node.
 */
void PathNode::generateRenderData() {
    CUAssertLog(!_rendered, "Render data is already present");
    if (_texture == nullptr) {
        return;
    }
    
    Poly2* source = (_stroke > 0 ? &_extrusion : &_polygon);
    
    Vertex2 temp;
    temp.color = Color4::WHITE;
    
    float w = (float)_texture->getWidth();
    float h = (float)_texture->getHeight();
    Vec2 offset = _polygon.getBounds().origin;
    for(auto it = source->getVertices().begin(); it != source->getVertices().end(); ++it) {
        temp.position = *it-offset;
        temp.texcoord.x = (it->x)/w;
        temp.texcoord.y = (it->y)/h;
        if (_flipHorizontal) {
            temp.texcoord.x = 1-temp.texcoord.x;
        }
        if (!_flipVertical) {
            temp.texcoord.y = 1-temp.texcoord.y;
        }
        _vertices.push_back(temp);
    }
    
    _rendered = true;
}

/** An extruder for those incomplete polygons */
PathExtruder PathNode::_extruder;
/** An outliner for those incomplete polygons */
PathOutliner PathNode::_outliner;


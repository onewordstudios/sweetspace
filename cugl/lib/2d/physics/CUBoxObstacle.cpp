//
//  CUBoxObstacle.cpp
//  Cornell University Game Library (CUGL)
//
//  This class implements a rectangular physics object, and is the primary type
//  of physics object to use.  Hence the name, Box2D.
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
//  This file is based on the CS 3152 PhysicsDemo Lab by Don Holden, 2007
//
//  Author: Walker White
//  Version: 11/6/16
//
#include <cugl/2d/physics/CUBoxObstacle.h>
#include <cugl/2d/CUPolygonNode.h>

using namespace cugl;

#pragma mark -
#pragma mark Constructors
/**
 * Initializes a new box object of the given dimensions.
 *
 * The scene graph is completely decoupled from the physics system.
 * The node does not have to be the same size as the physics body. We
 * only guarantee that the scene graph node is positioned correctly
 * according to the drawing scale.
 *
 * @param  pos  Initial position in world coordinates
 * @param  size The box size (width and height)
 */
bool BoxObstacle::init(const Vec2& pos, const Size& size) {
    Obstacle::init(pos);
    _geometry = nullptr;
    resize(size);
    return true;
}

/**
 * Resets the polygon vertices in the shape to match the dimension.
 *
 * @param  size The new dimension (width and height)
 */
void BoxObstacle::resize(const Size& size) {
    // Make the box with the center in the center
    _dimension = size;
    b2Vec2 corners[4];
    corners[0].x = -size.width/2.0f;
    corners[0].y = -size.height/2.0f;
    corners[1].x = -size.width/2.0f;
    corners[1].y =  size.height/2.0f;
    corners[2].x =  size.width/2.0f;
    corners[2].y =  size.height/2.0f;
    corners[3].x =  size.width/2.0f;
    corners[3].y = -size.height/2.0f;
    _shape.Set(corners, 4);
    if (_debug != nullptr) {
        resetDebug();
    }
}


#pragma mark -
#pragma mark Scene Graph Methods
/**
 * Redraws the outline of the physics fixtures to the debug node
 *
 * The debug node is use to outline the fixtures attached to this object.
 * This is very useful when the fixtures have a very different shape than
 * the texture (e.g. a circular shape attached to a square texture).
 *
 * Unfortunately, the current implementation is very inefficient.  Cocos2d
 * does not batch drawnode commands like it does Sprites or PolygonSprites.
 * Therefore, every distinct DrawNode is a distinct OpenGL call.  This can
 * really hurt framerate when debugging mode is on.  Ideally, we would refactor
 * this so that we only draw to a single, master draw node.  However, this
 * means that we would have to handle our own vertex transformations, instead
 * of relying on the transforms in the scene graph.
 */
void BoxObstacle::resetDebug() {
    Poly2 poly(Rect(Vec2::ZERO,_dimension));

    unsigned short indx[8] = { 0, 1, 1, 2, 2, 3, 3, 0 };
    poly.setIndices(indx, 8);
    if (_debug == nullptr) {
        _debug = cugl::WireNode::allocWithPoly(poly);
        _debug->setColor(_dcolor);
        if (_scene != nullptr) {
            _scene->addChild(_debug);
        }
    } else {
        _debug->setPolygon(poly);
    }
    _debug->setAnchor(Vec2::ANCHOR_CENTER);
    _debug->setPosition(getPosition());

}

#pragma mark -
#pragma mark Physics Methods
/**
 * Create new fixtures for this body, defining the shape
 *
 * This is the primary method to override for custom physics objects
 */
void BoxObstacle::createFixtures() {
    if (_body == nullptr) {
        return;
    }
    
    releaseFixtures();
    
    // Create the fixture
    _fixture.shape = &_shape;
    _geometry = _body->CreateFixture(&_fixture);
    markDirty(false);
}

/**
 * Release the fixtures for this body, reseting the shape
 *
 * This is the primary method to override for custom physics objects
 */
void BoxObstacle::releaseFixtures() {
    if (_geometry != nullptr) {
        _body->DestroyFixture(_geometry);
        _geometry = nullptr;
    }
}

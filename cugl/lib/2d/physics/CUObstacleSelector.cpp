//
//  CUObstacleSelector.cpp
//  Cornell University Game Library (CUGL)
//
//  This class implements a selection tool for dragging physics objects with a
//  mouse. It is essentially an instance of b2MouseJoint, but with an API that
//  makes it a lot easier to use. As with all instances of b2MouseJoint, there
//  will be some lag in the drag (though this is true on touch devices in general).
//  You can adjust the degree of this lag by adjusting the force.  However,
//  larger forces can cause artifacts when dragging an obstacle through other
//  obstacles.
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
#include <cugl/2d/physics/CUObstacle.h>
#include <cugl/2d/physics/CUObstacleWorld.h>
#include <cugl/2d/physics/CUObstacleSelector.h>
#include <Box2D/Dynamics/b2World.h>
#include <Box2D/Collision/Shapes/b2CircleShape.h>

using namespace cugl;


#pragma mark -
#pragma mark Constructors

/**
 * Initializes a new selector for the given ObstacleWorld and mouse size.
 *
 * This controller can never change.  If you want a selector for a different
 * ObstacleWorld, make a new instance.  However, the mouse size can be
 * changed at any time.
 *
 * @param world     the physics controller
 * @param mouseSize the mouse size
 *
 * @return  true if the obstacle is initialized properly, false otherwise.
 */
bool ObstacleSelector::init(const std::shared_ptr<ObstacleWorld>& world, const Size& mouseSize) {
    _controller = world;
    
    // Define the size and create a polygon
    _size = mouseSize;
    
    _jointDef.frequencyHz = DEFAULT_FREQUENCY;
    _jointDef.dampingRatio = DEFAULT_DAMPING;
    _force = DEFAULT_FORCE;
    
    b2BodyDef groundDef;
    groundDef.type = b2_staticBody;
    b2CircleShape groundShape;
    groundShape.m_radius = _size.width;
    
    _ground = _controller->getWorld()->CreateBody(&groundDef);
    if (_ground != nullptr) {
        b2FixtureDef groundFixture;
        groundFixture.shape = &groundShape;
        return _ground->CreateFixture(&groundFixture) != nullptr;
    }
    
    return false;
}

/**
 * Disposes all of the resources used by this selector.
 *
 * A disposed selector can be safely reinitialized.
 */
void ObstacleSelector::dispose(void) {
    if (_mouseJoint != nullptr) {
        _controller->getWorld()->DestroyJoint(_mouseJoint);
    }
    
    setDebugScene(nullptr);
    _controller->getWorld()->DestroyBody(_ground);
    _controller = nullptr;
}


#pragma mark -
#pragma mark Selection Methods

/**
 * Sets the current position of this selector (in World space)
 *
 * @param x  the x-coordinate of the selector
 * @param y  the y-coordinate of the selector
 */
void ObstacleSelector::setPosition(float x, float y) {
    _position.set(x,y);
    if (_mouseJoint) {
        _mouseJoint->SetTarget(b2Vec2(x,y));
    }
    updateDebug();
}

/**
 * Returns true if a physics body was selected at the given position.
 *
 * This method contructs and AABB the size of the mouse pointer, centered at the
 * given position.  If any part of the AABB overlaps a fixture, it is selected.
 *
 * @param  pos  the position (in physics space) to select
 *
 * @return true if a physics body was selected at the given position.
 */
bool ObstacleSelector::select() {
    std::function<bool(b2Fixture* fixture)> callback = [this](b2Fixture* fixture) {
        return onQuery(fixture);
    };
    
    Rect pointer(_position.x-_size.width/2.0f, _position.y-_size.height/2.0f,
                 _size.width,_size.height);
    _controller->queryAABB(callback,pointer);
    if (_selection != nullptr) {
        b2Body* body = _selection->GetBody();
        _jointDef.bodyA = _ground;  // Generally ignored
        _jointDef.bodyB = body;
        _jointDef.maxForce = _force * body->GetMass();
        _jointDef.target.Set(_position.x,_position.y);
        _mouseJoint = (b2MouseJoint*)_controller->getWorld()->CreateJoint(&_jointDef);
        body->SetAwake(true);
        
        Obstacle* obs = getObstacle();
        if (obs) {
            obs->setListener([this](Obstacle* obs) { updateTarget(obs); } );
        }
    } else {
        updateTarget(nullptr);
    }
    
    return _selection != nullptr;
}

/**
 * Deselects the physics body, discontinuing any mouse movement.
 *
 * The body may still continue to move of its own accord.
 */
void ObstacleSelector::deselect() {
    if (_selection != nullptr) {
        Obstacle* obs = getObstacle();
        if (obs) {
            obs->setListener(nullptr);
        }
        updateTarget(nullptr);
        _controller->getWorld()->DestroyJoint(_mouseJoint);
        _selection = nullptr;
        _mouseJoint = nullptr;
    }
}

/**
 * Returns a (weak) reference to the Obstacle selected (if any)
 *
 * Just because a physics body was selected does not mean that an Obstacle
 * was selected.  The body could be a basic Box2d body generated by other
 * means. If the body is not an Obstacle, this method returns nullptr.
 *
 * @return a (weak) reference to the Obstacle selected (if any)
 */
Obstacle* ObstacleSelector::getObstacle() {
    if (_selection != nullptr) {
        void* data = _selection->GetBody()->GetUserData();
        if (data != nullptr) {
            // Cannot dynamic cast void pointer. Is there alternative?
            return (Obstacle*)data;
        }
    }
    return nullptr;
}

/**
 * Callback function for mouse selection.
 *
 * This is the callback function used by the method queryAABB to select a physics
 * body at the current mouse location.
 *
 * @param  fixture  the fixture selected
 *
 * @return false to terminate the query.
 */
bool ObstacleSelector::onQuery(b2Fixture* fixture) {
    bool cn = fixture->TestPoint(b2Vec2(_position.x,_position.y));
    bool tl = fixture->TestPoint(b2Vec2(_position.x-_size.width/2,_position.y+_size.height/2));
    bool bl = fixture->TestPoint(b2Vec2(_position.x-_size.width/2,_position.y-_size.height/2));
    bool tr = fixture->TestPoint(b2Vec2(_position.x+_size.width/2,_position.y+_size.height/2));
    bool br = fixture->TestPoint(b2Vec2(_position.x+_size.width/2,_position.y-_size.height/2));
    if (cn || tl || bl || tr || br) {
        _selection = fixture;
        return _selection == nullptr;
    }
    return true;
}


#pragma mark -
#pragma mark Scene Graph Methods
/**
 * Sets the parent scene graph node for the debug wireframe
 *
 * The given node is the parent coordinate space for drawing physics.
 * All debug nodes for physics objects are drawn within this coordinate
 * space.  Setting the visibility of this node to false will disable
 * any debugging.  Similarly, setting this value to nullptr will
 * disable any debugging.
 *
 * The wireframe will be drawn using physics coordinates, which is possibly
 * much smaller than your drawing coordinates (e.g. 1 Box2D unit = 1 pixel).
 * If you want the wireframes to be larger, you should scale the parent
 * parent coordinate space to match the rest of the application.
 *
 * This scene graph node is intended for debugging purposes only.  If
 * you want a physics body to update a proper texture image, you should
 * either use the method {@link update(float)} for subclasses or
 * {@link setListener} for decoupled classes.
 *
 * @param node  he parent scene graph node for the debug wireframe
 */
void ObstacleSelector::setDebugScene(const std::shared_ptr<Node>& node) {
    // Release the node if we have one previously
    if (_scene != nullptr) {
        if (_hatch != nullptr && _hatch->getParent() == _scene.get()) {
            _scene->removeChild(_hatch);
        }
        if (_connect != nullptr && _connect->getParent() == _scene.get()) {
            _scene->removeChild(_connect);
        }
        _scene = nullptr;
    }
    if (node != nullptr) {
        _scene = node;
        // Add the debug node if necessary
        if (_hatch != nullptr && _hatch->getParent() == _scene.get()) {
            _scene->addChild(_hatch);
        }
        if (_connect != nullptr && _connect->getParent() == _scene.get()) {
            _scene->addChild(_connect);
        }
    }
    resetDebug();
}

/**
 * Repositions the scene node so that it agrees with the physics object.
 *
 * By default, the position of a node should be the body position times
 * the draw scale.  However, for some obstacles (particularly complex
 * obstacles), it may be desirable to turn the default functionality
 * off.  Hence we have made this virtual.
 */
void ObstacleSelector::updateTarget(Obstacle* obstacle) {
    if (obstacle) {
        if (_hatch != nullptr) {
            _hatch->setVisible(true);
        }
        if (_connect != nullptr) {
            Poly2 poly;
            Vec2 pos1 = getPosition();
            Vec2 pos2 = obstacle ? obstacle->getPosition() : pos1;
            Poly2::createLine(pos1,pos2,&poly);
            _connect->setPolygon(poly);
            _connect->setVisible(true);
        }
    } else {
        _hatch->setVisible(false);
        _connect->setVisible(false);
    }
}

/**
 * Creates the outline of the physics fixtures in the debug wireframe
 *
 * The debug wireframe is use to outline the fixtures attached to this
 * selector.  It is useful when you want to visualize the relationship
 * between the mouse and the selected shape.
 */
void ObstacleSelector::resetDebug() {
    if (_hatch == nullptr) {
        _hatch = WireNode::allocWithPoly(hatchPoly());
        _hatch->setColor(_dcolor);

        Poly2 poly;
        Poly2::createLine(Vec2::ZERO,Vec2::ZERO,&poly);
        _connect = WireNode::allocWithPoly(poly);
        _connect->setColor(_dcolor);
        _connect->setAbsolute(true);
        
        if (_scene != nullptr) {
            _scene->addChild(_hatch);
            _scene->addChild(_connect);
        }
    } else {
        Poly2 poly;
        _hatch = WireNode::allocWithPoly(hatchPoly());
        Poly2::createLine(Vec2::ZERO,Vec2::ZERO,&poly);
        _connect = WireNode::allocWithPoly(poly);
    }
    
    _hatch->setAnchor(Vec2::ANCHOR_CENTER);
    _hatch->setPosition(getPosition());
}

/**
 * Repositions the debug wireframe so that it agrees with the physics object.
 *
 * The debug wireframe is use to outline the fixtures attached to this
 * selector.  It is useful when you want to visualize the relationship
 * between the mouse and the selected shape.
 */
void ObstacleSelector::updateDebug() {
    if (_hatch) {
        _hatch->setPosition(getPosition());
        Obstacle* obs = getObstacle();
        if (obs) {
            updateTarget(obs);
        }
    }
}

/**
 * Returns a new polygon for the mouse hatch
 *
 * @return a new polygon for the mouse hatch
 */
Poly2 ObstacleSelector::hatchPoly() const {
    Vec2 verts[4];
    verts[0].set(-_size.width/2,_size.height/2);
    verts[1].set(_size.width/2,-_size.height/2);
    verts[2].set(-_size.width/2,-_size.height/2);
    verts[3].set(_size.width/2,_size.height/2);

    unsigned short indx[12] = { 0,3,3,1,1,2,2,0,0,1,2,3 };
    
    Poly2 poly(verts,4,indx,12);
    poly.setType(Poly2::Type::PATH);
    return poly;
}

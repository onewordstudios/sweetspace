//
//  CURotateAction.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides support for the rotation actions.  Rotation can specified
//  as either the end angle or the rotation amount.
//
//  These classes use our standard shared-pointer architecture.
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
//  Author: Sophie Huang and Walker White
//  Version: 3/12/17
//
#include <cugl/2d/actions/CURotateAction.h>
#include <string>
#include <sstream>

using namespace cugl;

#pragma mark -
#pragma mark RotateBy

/**
 * Initializes a rotation action of the given angle.
 *
 * When animated, this action will rotate its target by the given delta.
 * The angle is measured in radians, counter-clockwise from the x-axis.
 * The animation will take place over the given number of seconds.
 *
 * @param delta The amount to rotate the target node
 * @param time  The animation duration
 *
 * @return true if initialization was successful.
 */
bool RotateBy::init(float delta, float time) {
    _delta = delta;
    _duration = time;
    return true;
}

/**
 * Returns a newly allocated copy of this Action.
 *
 * @return a newly allocated copy of this Action.
 */
std::shared_ptr<Action> RotateBy::clone() {
    auto action = RotateBy::alloc();
    action->setDelta(_delta);
    return action;
}

/**
 * Executes an action on the given target node.
 *
 * The important state of the target is stored in the given state parameter.
 * The semantics of this state is action-dependent.
 *
 * @param target    The node to act on
 * @param state     The relevant node state
 * @param dt        The elapsed time to animate.
 */
void RotateBy::update(const std::shared_ptr<Node>& target, Uint64* state, float dt) {
    float angle = target->getAngle();
    target->setAngle(angle+_delta*dt);
}

/**
 * Returns a string representation of the action for debugging purposes.
 *
 * If verbose is true, the string will include class information.  This
 * allows us to unambiguously identify the class.
 *
 * @param verbose Whether to include class information
 *
 * @return a string representation of this action for debuggging purposes.
 */
std::string RotateBy::toString(bool verbose) const {
    std::stringstream ss;
    ss << "RotateBy{";
    ss << _delta;
    ss << "}'";
    return ss.str();
}

#pragma mark -
#pragma mark RotateTo

/**
 * Initializes a rotation action towards the given angle
 *
 * This angle is measured in radians, counter-clockwise from the x-axis.
 * The animation will be counter-clockwise if the target angle is larger
 * than the current one.  Otherwise it will be clockwise. The animation
 * will take place over the given number of seconds.
 *
 * @param angle The target rotation angle
 * @param time  The animation duration
 *
 * @return true if initialization was successful.
 */
bool RotateTo::init(float angle, float time) {
    _angle = angle;
    _duration = time;
    return true;
}

/**
 * Returns a newly allocated copy of this Action.
 *
 * @return a newly allocated copy of this Action.
 */
std::shared_ptr<Action> RotateTo::clone() {
    auto action = RotateTo::alloc();
    action->setAngle(_angle);
    return action;
}

/**
 * Prepares a target for action
 *
 * The important state of the target is stored in the given state parameter.
 * The semantics of this state is action-dependent.
 *
 * @param target    The node to act on
 * @param state     The relevant node state
 */
void RotateTo::load(const std::shared_ptr<Node>& target, Uint64* state) {
    float* angle = (float*)state;
    *angle = target->getAngle();
}

/**
 * Executes an action on the given target node.
 *
 * The important state of the target is stored in the given state parameter.
 * The semantics of this state is action-dependent.
 *
 * @param target    The node to act on
 * @param state     The relevant node state
 * @param dt        The elapsed time to animate.
 */
void RotateTo::update(const std::shared_ptr<Node>& target, Uint64* state, float dt) {
    float angle = target->getAngle();
    float* orig = (float*)state;
    float diff = _angle-(*orig);
    target->setAngle(angle+diff*dt);
}

/**
 * Returns a string representation of the action for debugging purposes.
 *
 * If verbose is true, the string will include class information.  This
 * allows us to unambiguously identify the class.
 *
 * @param verbose Whether to include class information
 *
 * @return a string representation of this action for debuggging purposes.
 */
std::string RotateTo::toString(bool verbose) const {
    std::stringstream ss;
    ss << "RotateTo{";
    ss << _angle;
    ss << "}'";
    return ss.str();
}

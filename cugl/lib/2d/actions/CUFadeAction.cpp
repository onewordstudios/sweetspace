//
//  CUFadeAction.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides support for the fading actions.  This includes fading
//  in and out.  Because they are absolute notions, these actions have no
//  addition state to them (as is the case with other actions)
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
#include <cugl/2d/actions/CUFadeAction.h>
#include <string>

using namespace cugl;

#pragma mark -
#pragma mark FadeOut

/**
 * Initializes an fade-out towards transparency
 *
 * When applied to a node, this action will adjust the alpha value of the node
 * color until it is eventually 0.  Unless the node is set for its children
 * to inherit is color, this will have no affect on the children of the node.
 * The animation will take place over the given number of seconds.
 *
 * @param time  The animation duration
 *
 * @return true if initialization was successful.
 */
bool FadeOut::init(float time) {
    _duration = time;
    return true;
}

/**
 * Returns a newly allocated copy of this Action.
 *
 * @return a newly allocated copy of this Action.
 */
std::shared_ptr<Action> FadeOut::clone() {
    auto action = FadeOut::alloc();
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
void FadeOut::load(const std::shared_ptr<Node>& target, Uint64* state) {
    Color4f color = target->getColor();
    // Because of clamping, we need to carry along two values this time.
    float* data = (float*)state;
    data[0] = color.a;
    data[1] = color.a;
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
void FadeOut::update(const std::shared_ptr<Node>& target, Uint64* state, float dt) {
    Color4f color = target->getColor();
    float* data = (float*)state;
    float orig = data[0];
    //float curr = data[1];
    
    // Internal state necessary because of clamping
    data[1] -= orig*dt;
    color.a = (data[1] < 0 ? 0.0f : data[1] > 1 ? 1.0f : data[1]);
    target->setColor(color);
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
std::string FadeOut::toString(bool verbose) const {
    return std::string("FadeOut");
}


#pragma mark -
#pragma mark FadeIn
/**
 * Initializes an fade-in towards opacity
 *
 * When applied to a node, this action will adjust the alpha value of the node
 * color until it is eventually 1.  Unless the node is set for its children
 * to inherit is color, this will have no affect on the children of the node.
 * The animation will take place over the given number of seconds.
 *
 * @param time  The animation duration
 *
 * @return true if initialization was successful.
 */
bool FadeIn::init(float time) {
    _duration = time;
    return true;
}

/**
 * Returns a newly allocated copy of this Action.
 *
 * @return a newly allocated copy of this Action.
 */
std::shared_ptr<Action> FadeIn::clone() {
    auto action = FadeIn::alloc();
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
void FadeIn::load(const std::shared_ptr<Node>& target, Uint64* state) {
    Color4f color = target->getColor();
    // Because of clamping, we need to carry along two values this time.
    float* data = (float*)state;
    data[0] = color.a;
    data[1] = color.a;
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
void FadeIn::update(const std::shared_ptr<Node>& target, Uint64* state, float dt) {
    Color4f color = target->getColor();
    float* data = (float*)state;
    float orig = data[0];
    //float curr = data[1];
    
    // Internal state necessary because of clamping
    data[1] += (1-orig)*dt;
    color.a = (data[1] < 0 ? 0.0f : data[1] > 1 ? 1.0f : data[1]);
    target->setColor(color);
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
std::string FadeIn::toString(bool verbose) const {
    return std::string("FadeIn");
}

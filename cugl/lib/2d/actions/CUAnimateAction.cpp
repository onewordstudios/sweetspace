//
//  CUAnimateAction.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides support for filmstrip animation.  The animation is
//  is represented as a sequence of frames.  There is no tweening support
//  between animation frames.
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
//  Author: Sophie Huang
//  Version: 11/1/16

#include <cugl/2d/actions/CUAnimateAction.h>
#include <cugl/2d/CUAnimationNode.h>
#include <string>
#include <sstream>

using namespace cugl;

/**
 * Initializes an animation sequence of frames start to end (inclusive).
 *
 * The animation sequence has start as its first frame and end as its last.
 * Animation will be in frame order, with an equal amount of time spent
 * on each frame. The value start must be less than (or equal to) end, as
 * this action does not know the filmstrip length.
 *
 * The repeat argument is optional.  It specifies the number of time to
 * repeat the animation sequence.  The total animation time will include
 * all repeats.
 *
 * @param start     The initial frame to animate
 * @param end       The final frame to animate
 * @param time      The animation duration
 * @param repeat    The number of times to repeat the sequence
 *
 * @return true if initialization was successful.
 */
bool Animate::init(int start, int end, float time, int repeat) {
    _frameset = std::vector<int>();
    _timestep = std::vector<float>();
    for(int jj = 0; jj < repeat; jj++) {
        for(int ii = start; ii <= end; ii++) {
            _frameset.push_back(ii);
        }
    }
    _uniform = true;
    _duration = time;
    return true;
    
}

/**
 * Initializes an animation sequence of uniform speed
 *
 * The animation sequence is given by the specified vector.  The animation
 * will spend an equal amount of time on each frame, so that the total time
 * spent animating is the one specified.
 *
 * @param frames    The animation sequence
 * @param time      The animation duration
 *
 * @return true if initialization was successful.
 */
bool Animate::init(const std::vector<int>& frames, float time) {
    _uniform = true;
    _frameset = frames;
    _timestep = std::vector<float>();
    _duration = time;
    return true;
}

/**
 * Initializes an animation sequence of variable speed
 *
 * The animation sequence is given by the first specified vector.  The
 * second vector specifies the number of seconds to spend on each frame.
 * the overall animation duration is the sum of this vector.
 *
 * Both vectors must be the same length.  They can be empty.
 *
 * @param frames    The animation sequence
 * @param time      The duration of each frame in the sequences
 *
 * @return true if initialization was successful.
 */
bool Animate::init(const std::vector<int>& frames, const std::vector<float>& time) {
    CUAssertLog(_frameset.size() == _timestep.size(), "The time steps do not agree with the frame sequence");
    _frameset = frames;
    _timestep = time;
    _duration = 0;
    for(int ii = 0; ii < _timestep.size(); ii++) {
        _duration += _timestep[ii];
    }
    _uniform = false;
    return true;
}

/**
 * Disposes all of the resources used by this action.
 *
 * A disposed action can be safely reinitialized.
 */
void Animate::dispose() {
    _frameset.clear();
    _timestep.clear();
    _uniform = true;
}


#pragma mark -
#pragma mark Atributes
/**
 * Returns the frame in the filmstrip to be animated at time index t in [0,1]
 *
 * @return the frame in the filmstrip to be animated at time index t in [0,1]
 */
int Animate::getFrame(float t) const {
    int pos = 0;
    if (_uniform && _duration > 0) {
        pos = (int)((int)_frameset.size()*t);
    } else if (_duration > 0) {
        float total = 0;
        for(int ii = 0; total < t && ii < _frameset.size(); ii++) {
            pos = ii;
            total += _timestep[ii]/_duration;
        }
    }
    if (pos >= _frameset.size()) {
        pos = (int)_frameset.size()-1;
    } else if (pos <= 0) {
        pos = 0;
    }
    
    return _frameset[pos];
}

/**
 * Sets the sequence of frames used in this animation
 *
 * If this set has a different size than the one initial set, this setter
 * will keep the overall animation duration, but will revert to a uniform
 * time step.
 *
 * Changing this value for an actively animating action can have
 * undefined effects.
 *
 * @param frames    the sequence of frames used in this animation
 */
void Animate::setSequence(const std::vector<int>& frames) {
    _frameset = frames;
    if (!_uniform && _frameset.size() != _timestep.size()) {
        _uniform = true;
        _timestep.clear();
    }
}

/**
 * Sets the sequence of frames used in this animation
 *
 * Both vectors must be the same length.  They can be empty.
 *
 * Changing this value for an actively animating action can have
 * undefined effects.
 *
 * @param frames    the sequence of frames used in this animation
 * @param time      the time to devote animating each frame
 */
void Animate::setSequence(const std::vector<int>& frames, const std::vector<float>& time) {
    CUAssertLog(_frameset.size() == _timestep.size(), "The time steps do not agree with the frame sequence");
    _frameset = frames;
    _timestep = time;
    _duration = 0;
    for(int ii = 0; ii < _timestep.size(); ii++) {
        _duration += _timestep[ii];
    }
    _uniform = false;
}

/**
 * Forces this animation to use a uniform time step for all frames
 *
 * Changing this value for an actively animating action can have
 * undefined effects.
 */
void Animate::setUniform() {
    _uniform = true;
    _timestep.clear();
}

#pragma mark -
#pragma mark Animation Methods
/**
 * Returns a newly allocated copy of this Action.
 *
 * @return a newly allocated copy of this Action.
 */
std::shared_ptr<Action> Animate::clone() {
    if (_uniform) {
        return Animate::alloc(_frameset,_duration);
    }
    return Animate::alloc(_frameset,_timestep);
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
void Animate::load(const std::shared_ptr<Node>& target, Uint64* state) {
    AnimationNode* strip = dynamic_cast<AnimationNode*>(target.get());
    CUAssertLog(strip,"Attempt to animate a node other than an AnimationNode");

    // Need internal state to track the animation time.
    float* time = (float*)state;
    *time = 0;
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
void Animate::update(const std::shared_ptr<Node>& target, Uint64* state, float dt) {
    AnimationNode* strip = dynamic_cast<AnimationNode*>(target.get());
    float* time = (float*)state;
    float curr = *time+dt;
    *time = curr;
    int frame = getFrame(curr);
    if (strip->getFrame() != frame) {
        strip->setFrame(frame);
    }
}

#pragma mark -
#pragma mark Debugging Methods
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
std::string Animate::toString(bool verbose) const {
    std::stringstream ss;
    ss << "Animate{";
    for(int ii = 0; ii < _frameset.size(); ii++) {
        if (ii > 0) {
            ss << ", ";
        }
        ss << "(" << _frameset[ii] << ",";
        if (_uniform) {
            ss << _duration/_frameset.size();
        } else {
            ss << _timestep[ii];
        }
        ss << ")";
    }
    ss << "}";
    return ss.str();
}

//
//  CUAnimateAction.h
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

#ifndef __CU_ANIMATE_ACTION_H__
#define __CU_ANIMATE_ACTION_H__

#include "CUAction.h"

namespace cugl {

/**
 * This action represents a sequence of film-strip frames for animation
 *
 * Each frame in the sequence is given a set amount of time to display. The 
 * animation will not tween between frames.  To do so would require a refactoring
 * of the scene graph nodes.
 *
 * An action contains only the definition of the animation. This can include
 * information about the transform to use or the duration of the animation.
 * However, it does not contain any attribute of the target. Hence, an action
 * can be reapplied to different targets.
 *
 * By itself, an action does nothing.  It only specifies an action that may
 * take place. To use an action, it must be passed to the ActionManager.  The
 * manager will create an action instance and animate that instance.  While an
 * action may be reused many times, an action instance corresponds to a single
 * animation.
 */
class Animate : public Action {
protected:
    /** The list of frames to animate */
    std::vector<int> _frameset;
    /** The amount of time for each frame */
    std::vector<float> _timestep;
    /** Whether or not the timestep is uniform */
    bool _uniform;

public:
#pragma mark Constructors
    /**
     * Creates an uninitialized animation action.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    Animate() : _uniform(true) {}

    /**
     * Deletes this action instance, disposing all resources
     */
    ~Animate() { dispose(); }
    
    /**
     * Disposes all of the resources used by this action.
     *
     * A disposed action can be safely reinitialized.
     */
    void dispose();
    
    /**
     * Initializes a degenerate animation action.
     *
     * The animation sequence is empty, meaning no animation takes place.
     *
     * @return true if initialization was successful.
     */
    bool init() { return true; }
    
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
    bool init(int start, int end, float time, int repeat = 1);

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
    bool init(const std::vector<int>& frames, float time);

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
    bool init(const std::vector<int>& frames, const std::vector<float>& time);


#pragma mark Static Constructors
    /**
     * Returns a newly allocated, degenerate animation action.
     *
     * The animation sequence is empty, meaning no animation takes place.
     *
     * @return a newly allocated, degenerate animation action.
     */
    static std::shared_ptr<Animate> alloc() {
        std::shared_ptr<Animate> result = std::make_shared<Animate>();
        return (result->init() ? result : nullptr);
    }

    /**
     * Returns a newly allocated animation sequence of frames start to end (inclusive).
     *
     * The animation sequence has start as its first frame and end as its last.
     * Animation will be in frame order, with an equal amount of time spent
     * on each frame. The value start must be less than (or equal to) end, as
     * this action does not know the filmstrip length.\
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
     * @return a newly allocated animation sequence of frames start to end (inclusive).
     */
    static std::shared_ptr<Animate> alloc(int start, int end, float time, int repeat = 1) {
        std::shared_ptr<Animate> result = std::make_shared<Animate>();
        return (result->init(start,end,time,repeat) ? result : nullptr);
    }

    /**
     * Returns a newly allocated animation sequence of uniform speed
     *
     * The animation sequence is given by the specified vector.  The animation
     * will spend an equal amount of time on each frame, so that the total time
     * spent animating is the one specified.
     *
     * @param frames    The animation sequence
     * @param time      The animation duration
     *
     * @return a newly allocated animation sequence of uniform speed
     */
    static std::shared_ptr<Animate> alloc(const std::vector<int>& frames, float time) {
        std::shared_ptr<Animate> result = std::make_shared<Animate>();
        return (result->init(frames,time) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated animation sequence of variable speed
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
     * @return a newly allocated animation sequence of variable speed
     */
    static std::shared_ptr<Animate> alloc(const std::vector<int>& frames, const std::vector<float>& time) {
        std::shared_ptr<Animate> result = std::make_shared<Animate>();
        return (result->init(frames,time) ? result : nullptr);
    }
    
#pragma mark Atributes
    /**
     * Returns the frame in the filmstrip to be animated at time index t in [0,1]
     *
     * @return the frame in the filmstrip to be animated at time index t in [0,1]
     */
    int getFrame(float t) const;
    
    /**
     * Returns the sequence of frames used in this animation
     *
     * Changing this value for an actively animating action can have
     * undefined effects.
     *
     * @return the sequence of frames used in this animation
     */
    const std::vector<int>& getSequence() const { return _frameset; }
    
    /**
     * Returns individual time steps for each frame
     *
     * If this animation uses a uniform time step for each frame, this set
     * will be empty.
     *
     * Changing this value for an actively animating action can have
     * undefined effects.
     *
     * @return the sequence of frames used in this animation
     */
    const std::vector<float>& getTimeSteps() const { return _timestep; }

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
    void setSequence(const std::vector<int>& frames);
    
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
    void setSequence(const std::vector<int>& frames, const std::vector<float>& time);
    
    /** 
     * Returns true if this animation uses a uniform time step for all frames
     *
     * Changing this value for an actively animating action can have
     * undefined effects.
     *
     * @return true if this animation uses a uniform time step for all frames
     */
    int isUniform() const { return _uniform; }

    /**
     * Forces this animation to use a uniform time step for all frames
     *
     * Changing this value for an actively animating action can have
     * undefined effects.
     */
    void setUniform();

#pragma mark Animation Methods
    /**
     * Returns a newly allocated copy of this Action.
     *
     * @return a newly allocated copy of this Action.
     */
    virtual std::shared_ptr<Action> clone() override;
    
    /**
     * Prepares a target for action
     *
     * The important state of the target is stored in the given state parameter.
     * The semantics of this state is action-dependent.
     *
     * @param target    The node to act on
     * @param state     The relevant node state
     */
    virtual void load(const std::shared_ptr<Node>& target, Uint64* state) override;
    
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
    virtual void update(const std::shared_ptr<Node>& target, Uint64* state, float dt) override;
    
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
    virtual std::string toString(bool verbose = false) const override;

};

}

#endif /* __CU_ANIMATE_ACTION_H__ */

//
//  CUEasingBezier.h
//  Cornell University Game Library (CUGL)
//
//  This module provides easing function support for sophisticated tweening.
//  This module provides a single class that can represent any bezier easing
//  function. This provides the user with more flexibility than the
//  EasingFunction factory.
//
//  These classe uses our standard shared-pointer architecture.
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
#ifndef __CU_EASING_BEZIER_H__
#define __CU_EASING_BEZIER_H__

#include <cugl/math/CUVec2.h>
#include "CUEasingFunction.h"
#include <vector>
#include <memory>

namespace cugl {

/**
 * This class represents a bexier curve that implements an easing function.
 *
 * The object itself stores the bezier curve, which cannot be manipulated
 * after creation (for thread safety). The bezier curve is defined as a
 * cubic polynomial that maps a paramter t onto the xy plane.
 *
 * The method {@link getEvaluator()} returns a function pointer that can be
 * used in {@link ActionManager}. The function retains a shared pointer to the
 * object, so the object reference can be safely discarded after getting the
 * function pointer.
 */
class EasingBezier :  public std::enable_shared_from_this<EasingBezier> {
#pragma mark -
#pragma mark Internal Helpers
protected:
    /** The C1 coefficient */
    Vec2 _c1;
    /** The C2 coefficient */
    Vec2 _c2;
    /** The C3 coefficient */
    Vec2 _c3;
    
    /** The rootset of the polynomial x-component (cached for efficiency). */
    std::vector<float> _rootset;
     
    /**
     * Stores the roots of a x^2 + b x + c into the rootset.
     *
     * This is a helper function for computing the root set from the
     * bezier polynomial.
     *
     * @param a The 2nd degree coefficient
     * @param b The linear coefficient
     * @param c The constant factor
     */
    void solveQuadraticEquation(float a, float b, float c);
    
    /**
     * Stores the roots of a x^3 + b x^2 + c x + d into the rootset.
     *
     * This is a helper function for computing the root set from the
     * bezier polynomial.
     *
     * @param a The 3rd degree coefficient
     * @param b The 2nd degree coefficient
     * @param c The linear coefficient
     * @param d The constant factor
     */
    void solveCubicEquation(float a, float b, float c, float d);
   
    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates an uninitialized easing function.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    EasingBezier();
    
    /**
     * Deletes this easing function, disposing all resources
     */
    ~EasingBezier() { dispose();  }
    
    /**
     * Disposes all of the resources used by this easing function.
     *
     * A disposed function can be safely reinitialized.
     */
    void dispose();
    
    /**
     * Initializes a linear easing function
     *
     * @return true if initialization was successful.
     */
    bool init() {
        return init(EasingFunction::Type::LINEAR);
    }
    
    /**
     * Initializes an easing function of the given type.
     *
     * Bezier easing functions can duplicate every easing function in
     * {@link EasingFunction} except for the bounce and elastic functions.
     *
     * @param type  The easing function type
     *
     * @return true if initialization was successful.
     */
    bool init(EasingFunction::Type type);
    
    /**
     * Initializes an easing function eith the given control points.
     *
     * Any cubic bezier can be defined by a two control points in the plane,
     * which define the tanget lines of the two endpoints.  These are often
     * manifested as handles in programs like Adobe Illustrator.
     *
     * @param x1    The x-coordinate of the first handle.
     * @param y1    The y-coordinate of the first handle.
     * @param x2    The x-coordinate of the second handle.
     * @param y2    The y-coordinate of the second handle.
     *
     * @return true if initialization was successful.
     */
    bool init(float x1, float y1, float x2, float y2);

    /**
     * Initializes an easing function eith the given control points.
     *
     * Any cubic bezier can be defined by a two control points in the plane,
     * which define the tanget lines of the two endpoints.  These are often
     * manifested as handles in programs like Adobe Illustrator.
     *
     * @param p1    The first handle.
     * @param p2    The second handle.
     *
     * @return true if initialization was successful.
     */
    bool init(const Vec2& p1, const Vec2& p2) {
        return init(p1.x,p1.y,p2.x,p2.y);
    }

#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a newly allocated linear easing function
     *
     * @return a newly allocated linear easing function
     */
    static std::shared_ptr<EasingBezier> alloc() {
        std::shared_ptr<EasingBezier> result = std::make_shared<EasingBezier>();
        return (result->init() ? result : nullptr);
    }

    /**
     * Returns a newly allocated easing function of the given type.
     *
     * Bezier easing functions can duplicate every easing function in
     * {@link EasingFunction} except for the bounce and elastic functions.
     *
     * @param type  The easing function type
     *
     * @return a newly allocated easing function of the given type.
     */
    static std::shared_ptr<EasingBezier> alloc(EasingFunction::Type type) {
        std::shared_ptr<EasingBezier> result = std::make_shared<EasingBezier>();
        return (result->init(type) ? result : nullptr);
    }

    /**
     * Returns a newly allocated easing function eith the given control points.
     *
     * Any cubic bezier can be defined by a two control points in the plane,
     * which define the tanget lines of the two endpoints.  These are often
     * manifested as handles in programs like Adobe Illustrator.
     *
     * @param x1    The x-coordinate of the first handle.
     * @param y1    The y-coordinate of the first handle.
     * @param x2    The x-coordinate of the second handle.
     * @param y2    The y-coordinate of the second handle.
     *
     * @return a newly allocated easing function eith the given control points.
     */
    static std::shared_ptr<EasingBezier> alloc(float x1, float y1, float x2, float y2) {
        std::shared_ptr<EasingBezier> result = std::make_shared<EasingBezier>();
        return (result->init(x1,y1,x2,y2) ? result : nullptr);
    }

    /**
     * Returns a newly allocated easing function eith the given control points.
     *
     * Any cubic bezier can be defined by a two control points in the plane,
     * which define the tanget lines of the two endpoints.  These are often
     * manifested as handles in programs like Adobe Illustrator.
     *
     * @param p1    The first handle.
     * @param p2    The second handle.
     *
     * @return a newly allocated easing function eith the given control points.
     */
    static std::shared_ptr<EasingBezier> alloc(const Vec2& p1, const Vec2& p2) {
        std::shared_ptr<EasingBezier> result = std::make_shared<EasingBezier>();
        return (result->init(p1,p2) ? result : nullptr);
    }
    
#pragma mark -
#pragma mark Easing Support

    /**
     * Returns the value of the easing function at t.
     *
     * The easing function is only well-defined when 0 <= t <= 1.
     *
     * @return the value of the easing function at t.
     */
    float evaluate(float t);

    /**
     * Returns a pointer to the function represented by this object.
     *
     * The function retains a shared pointer to the object, so the object
     * reference can be safely discarded after getting the function pointer.
     *
     * @return a pointer to the function represented by this object.
     */
    std::function<float(float)> getEvaluator();
    
};
    
}

#endif /* __CU_EASING_BEZIER_H__ */

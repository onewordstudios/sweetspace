//
//  CUEasingBezier.cpp
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
#include <cugl/cugl.h>
#include <cugl/math/CUEasingBezier.h>

using namespace cugl;

#pragma mark Constructors
/**
 * Creates an uninitialized easing function.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
 * the heap, use one of the static constructors instead.
 */
EasingBezier::EasingBezier() {
    _c1 = Vec2::ZERO;
    _c2 = Vec2::ZERO;
    _c3 = Vec2::ZERO;
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
bool EasingBezier::init(EasingFunction::Type type) {
    switch(type) {
    case EasingFunction::Type::LINEAR:
        return init(0.25f,0.25f,0.75f,0.75f);
    case EasingFunction::Type::SINE_IN:
        return init(0.47f, 0, 0.745f, 0.715f);
    case EasingFunction::Type::SINE_OUT:
        return init(0.39f, 0.575f, 0.565f, 1);
    case EasingFunction::Type::SINE_IN_OUT:
        return init(0.445f, 0.05f, 0.55f, 0.95f);
    case EasingFunction::Type::QUAD_IN:
        return init(0.55f, 0.085f, 0.68f, 0.53f);
    case EasingFunction::Type::QUAD_OUT:
        return init(0.25f, 0.46f, 0.45f, 0.94f);
    case EasingFunction::Type::QUAD_IN_OUT:
        return init(0.455f, 0.03f, 0.515f, 0.955f);
    case EasingFunction::Type::CUBIC_IN:
        return init(0.55f, 0.055f, 0.675f, 0.19f);
    case EasingFunction::Type::CUBIC_OUT:
        return init(0.215f, 0.61f, 0.355f, 1);
    case EasingFunction::Type::CUBIC_IN_OUT:
        return init(0.645f, 0.045f, 0.355f, 1);
    case EasingFunction::Type::QUART_IN:
        return init(0.95f, 0.05f, 0.795f, 0.035f);
    case EasingFunction::Type::QUART_OUT:
        return init(0.165f, 0.84f, 0.44f, 1);
    case EasingFunction::Type::QUART_IN_OUT:
        return init(0.77f, 0, 0.175f, 1);
    case EasingFunction::Type::QUINT_IN:
        return init(0.755f, 0.05f, 0.855f, 0.06f);
    case EasingFunction::Type::QUINT_OUT:
        return init(0.23f, 1, 0.32f, 1);
    case EasingFunction::Type::QUINT_IN_OUT:
        return init(0.86f, 0, 0.07f, 1);
    case EasingFunction::Type::EXPO_IN:
        return init(0.95f, 0.05f, 0.795f, 0.035f);
    case EasingFunction::Type::EXPO_OUT:
        return init(0.19f, 1, 0.22f, 1);
    case EasingFunction::Type::EXPO_IN_OUT:
        return init(1, 0, 0, 1);
    case EasingFunction::Type::CIRC_IN:
        return init(0.6f, 0.04f, 0.98f, 0.335f);
    case EasingFunction::Type::CIRC_OUT:
        return init(0.075f, 0.82f, 0.165f, 1);
    case EasingFunction::Type::CIRC_IN_OUT:
        return init(0.785f, 0.135f, 0.15f, 0.86f);
    case EasingFunction::Type::BACK_IN:
        return init(0.6f, -0.28f, 0.735f, 0.045f);
    case EasingFunction::Type::BACK_OUT:
        return init(0.175f, 0.885f, 0.32f, 1.275f);
    case EasingFunction::Type::BACK_IN_OUT:
        return init(0.68f, -0.55f, 0.265f, 1.55f);
    default:
        CUAssertLog(false,"There is no bezier easing function for this type: %d",type);
    }
    return false;
}

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
bool EasingBezier::init(float x1, float y1, float x2, float y2) {
    _c1.set(3*x1,3*y1);
    _c2.set(3*x2-6*x1,3*y2-6*y1);
    _c3.set(1-3*x2+3*x1,1-3*y2+3*y1);
    return true;
}

/**
 * Disposes all of the resources used by this easing function.
 *
 * A disposed function can be safely reinitialized.
 */
void EasingBezier::dispose() {
    _c1 = Vec2::ZERO;
    _c2 = Vec2::ZERO;
    _c3 = Vec2::ZERO;
    _rootset.clear();
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
float EasingBezier::evaluate(float t) {
    _rootset.clear();
    solveCubicEquation(_c3.x, _c2.x, _c1.x, -t);
    float choice = 0;
    if (!_rootset.empty()) {
        choice = _rootset[0];
    }
    
    return choice*choice*choice*_c3.y+choice*choice*_c2.y+choice*_c1.y;
}

/**
 * Returns a pointer to the function represented by this object.
 *
 * The function retains a shared pointer to the object, so the object
 * reference can be safely discarded after getting the function pointer.
 *
 * @return a pointer to the function represented by this object.
 */
std::function<float(float)> EasingBezier::getEvaluator() {
    std::shared_ptr<EasingBezier> context = shared_from_this();
    return [=] (float t){ return context->evaluate(t); };
}


#pragma mark -
#pragma mark Internal Helpers
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
void EasingBezier::solveQuadraticEquation(float a, float b, float c) {
    float  discriminant = b * b - 4 * a * c;
    if (discriminant >= 0) {
        _rootset.push_back((-b + sqrtf(discriminant)) / (2.0f * a));
        _rootset.push_back((-b - sqrtf(discriminant)) / (2.0f * a));
    }

}

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
void EasingBezier::solveCubicEquation(float a, float b, float c, float d) {
    if (a == 0) {
        solveQuadraticEquation(b, c, d);
        return;
    }
    
    b /= a;
    c /= a;
    d /= a;
    
    float p = (3 * c - b * b)/3.0f;
    float q = (2 * b * b * b - 9 * b * c + 27 * d)/27.0f;
    
    if (p == 0) {
        _rootset.push_back(powf(-q, 1.0f/3.0f));
    } else if (q == 0) {
        _rootset.push_back(sqrtf(-p));
        _rootset.push_back(-sqrtf(-p));
    } else {
        float discriminant = q*q/4.0f + p*p*p/27.0f;
        if (discriminant == 0) {
            _rootset.push_back(powf(q/2.0f, 1.0f/3.0f) - b/3.0f);
        } else if (discriminant > 0) {
            _rootset.push_back(powf(-(q/2.0f) + sqrtf(discriminant), 1.0f/3.0f) -
                               powf((q/2.0f)  + sqrtf(discriminant), 1.0f/3.0f) - b/3.0f);
        } else {
            float r = sqrtf( powf(-(p/3.0f), 3.0f) );
            float phi = acosf(-(q / (2 * sqrtf(-p*p*p/27.0f))));
            float s = 2 * powf(r, 1.0f/3.0f);
            _rootset.push_back(s * cosf(phi / 3.0f) - b / 3.0f);
            _rootset.push_back(s * cosf((phi + 2 * (float)M_PI) / 3.0f) - b / 3.0f);
            _rootset.push_back(s * cosf((phi + 4 * (float)M_PI) / 3.0f) - b / 3.0f);
        }
    }
}



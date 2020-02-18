//
//  CUEasingFunction.h
//  Cornell University Game Library (CUGL)
//
//  This module provides easing function support for sophisticated tweening.
//  All of the easing functions are implemented directly, using the definitions
//  provided by http:://easings.net.
//
//  This class is simply a factory for returning function pointers to the
//  appropriate static method.
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
#ifndef __CU_EASING_FUNCTION_H__
#define __CU_EASING_FUNCTION_H__

#include <cugl/math/CUVec2.h>
#include <vector>
#include <memory>

/** The period for the elastic easing functions */
#define ELASTIC_PERIOD 0.3f

namespace cugl {
    
/**
 * This class is a factory for returning easing functions.
 *
 * An easing function is an interpolation function that (usually) maps [0,1]
 * to [0,1] with f(0) = 0 and f(1) = 1.  It is used to control the speed of
 * a tweening operation. If f(t) = t, the result is standard linear
 * interpolation, providing a smooth animation.  Nonlinear curves allow the
 * tweening to adjust its speed over time.  An easing function can map outside
 * of the range so long as the end points are still fixed.  This allows for
 * overshoot and correction in the tweening.
 *
 * The supported easing functions are all implemented as static methods.
 * The {@link alloc()} method is used to return a function value that can
 * can be passed to the {@link ActionManager}.
 */
class EasingFunction {
public:
    /**
     * This enum lists the easing functions supported by this factory.
     */
    enum class Type : int {
        /** A linear easing function (the default) */
        LINEAR,
        /** A 1-cosine function with an asymptotic start at t=0 */
        SINE_IN,
        /** A sine function with an asymptotic finish at t=1 */
        SINE_OUT,
        /** A concatenation of the SINE_IN and SINE_OUT functions */
        SINE_IN_OUT,
        /** A quadratic polynomial with an asymptotic start at t=0 */
        QUAD_IN,
        /** A quadratic polynomial with an asymptotic finish at t=1 */
        QUAD_OUT,
        /** A concatenation of the QUAD_IN and QUAD_OUT functions */
        QUAD_IN_OUT,
        /** A cubic polynomial with an asymptotic start at t=0 */
        CUBIC_IN,
        /** A cubic polynomial with an asymptotic finish at t=1 */
        CUBIC_OUT,
        /** A concatenation of the CUBIC_IN and CUBIC_OUT functions */
        CUBIC_IN_OUT,
        /** A fourth-degree polynomial with an asymptotic start at t=0 */
        QUART_IN,
        /** A fourth-degree polynomial with an asymptotic finish at t=1 */
        QUART_OUT,
        /** A concatenation of the QUART_IN and QUART_OUT functions */
        QUART_IN_OUT,
        /** A fifth-degree polynomial with an asymptotic start at t=0 */
        QUINT_IN,
        /** A fifth-degree polynomial with an asymptotic finish at t=1 */
        QUINT_OUT,
        /** A concatenation of the QUINT_IN and QUINT_OUT functions */
        QUINT_IN_OUT,
        /** An exponential function with an asymptotic start at t=0 */
        EXPO_IN,
        /** An exponential function with an asymptotic finish at t=1 */
        EXPO_OUT,
        /** A concatenation of the EXPO_IN and EXPO_OUT functions */
        EXPO_IN_OUT,
        /** A quarter circle with an asymptotic start at t=0 */
        CIRC_IN,
        /** A quarter circle with an asymptotic finish at t=1 */
        CIRC_OUT,
        /** A concatenation of the CIRC_IN and CIRC_OUT functions */
        CIRC_IN_OUT,
        /** An easing function that briefly dips below t=0 after the start */
        BACK_IN,
        /** An easing function that briefly rises above t=1 before the finish */
        BACK_OUT,
        /** A concatenation of the BACK_IN and BACK_OUT functions */
        BACK_IN_OUT,
        /** An easing function that bounces down to t=0 several times after the start. */
        BOUNCE_IN,
        /** An easing function that bounces up to t=1 several times before this finish. */
        BOUNCE_OUT,
        /** A concatenation of the BOUNCE_IN and BOUNCE_OUT functions */
        BOUNCE_IN_OUT,
        /** An easing function that bounces back-and-forth across t=0 several times after the start. */
        ELASTIC_IN,
        /** An easing function that bounces back-and-forth across t=1 several times before the finish. */
        ELASTIC_OUT,
        /** A concatenation of the ELASTIC_IN and ELASTIC_OUT functions */
        ELASTIC_IN_OUT
    };
    
    /**
     * Returns a linear easing function
     *
     * @return A linear easing function
     */
    static std::function<float(float)> alloc() {
        return alloc(Type::LINEAR);
    }

    /**
     * Returns an easing function of the given type.
     *
     * The optional value period only applies to elastic easing functions, as
     * their bounce factor is adjustable.
     *
     * @param type      The easing function type
     * @param period    The period of an elastic easing function
     *
     * @return An easing function of the given type.
     */
    static std::function<float(float)> alloc(Type type, float period = ELASTIC_PERIOD);

    /**
     * Returns an adjustment of the tweening time
     *
     * This is a linear easing function (the default)
     *
     * @param time  The time in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float linear(float time);
    
    /**
     * Returns an adjustment of the tweening time
     *
     * This is a linear easing function (the default)
     *
     * @param time  The time in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float sineIn(float time);
    
    /**
     * Returns an adjustment of the tweening time
     *
     * This is a sine function with an asymptotic finish at t=1.
     *
     * @param time  The time in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float sineOut(float time);
    
    /**
     * Returns an adjustment of the tweening time
     *
     * This is a concatenation of the {@link sineIn(float)} and {@link sineOut(float)}
     * easing functions.
     *
     * @param time  The time in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float sineInOut(float time);
    
    /**
     * Returns an adjustment of the tweening time
     *
     * This is a quadratic polynomial with an asymptotic start at t=0.
     *
     * @param time  The time in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float quadIn(float time);
    
    /**
     * Returns an adjustment of the tweening time
     *
     * This is a quadratic polynomial with an asymptotic finish at t=1.
     *
     * @param time  The time in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float quadOut(float time);
    
    /**
     * Returns an adjustment of the tweening time
     *
     * This is a concatenation of the {@link quadIn(float)} and {@link quadOut(float)}
     * easing functions.
     *
     * @param time  The time in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float quadInOut(float time);
    
    /**
     * Returns an adjustment of the tweening time
     *
     * This is a cubic polynomial with an asymptotic start at t=0.
     *
     * @param time  The time in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float cubicIn(float time);
    
    /**
     * Returns an adjustment of the tweening time
     *
     * This is a cubic polynomial with an asymptotic finish at t=1.
     *
     * @param time  The time in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float cubicOut(float time);
    
    /**
     * Returns an adjustment of the tweening time
     *
     * This is a concatenation of the {@link cubicIn(float)} and {@link cubicOut(float)}
     * easing functions.
     *
     * @param time  The time in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float cubicInOut(float time);

    /**
     * Returns an adjustment of the tweening time
     *
     * This is a fourth-degree polynomial with an asymptotic start at t=0.
     *
     * @param time  The time in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float quartIn(float time);

    /**
     * Returns an adjustment of the tweening time
     *
     * This is a fourth-degree polynomial with an asymptotic finish at t=1.
     *
     * @param time  The time in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float quartOut(float time);

    /**
     * Returns an adjustment of the tweening time
     *
     * This is a concatenation of the {@link quartIn(float)} and {@link quartOut(float)}
     * easing functions.
     *
     * @param time  The time in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float quartInOut(float time);
    
    /**
     * Returns an adjustment of the tweening time
     *
     * This is a fifth-degree polynomial with an asymptotic start at t=0.
     *
     * @param time  The time in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float quintIn(float time);
    
    /**
     * Returns an adjustment of the tweening time
     *
     * This is a fifth-degree polynomial with an asymptotic finish at t=1.
     *
     * @param time  The time in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float quintOut(float time);
    
    /**
     * Returns an adjustment of the tweening time
     *
     * This is a concatenation of the {@link quintIn(float)} and {@link quintOut(float)}
     * easing functions.
     *
     * @param time  The time in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float quintInOut(float time);
    
    /**
     * Returns an adjustment of the tweening time
     *
     * This is an exponential function with an asymptotic start at t=0.
     *
     * @param time  The time in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float expoIn(float time);
    
    /**
     * Returns an adjustment of the tweening time
     *
     * This is an exponential function with an asymptotic finish at t=1.
     *
     * @param time  The time in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float expoOut(float time);
    
    /**
     * Returns an adjustment of the tweening time
     *
     * This is a concatenation of the {@link expoIn(float)} and {@link expoOut(float)}
     * easing functions.
     *
     * @param time  The time in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float expoInOut(float time);
    
    /**
     * Returns an adjustment of the tweening time
     *
     * This is a quarter circle with an asymptotic start at t=0.
     *
     * @param time  The time in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float circIn(float time);
    
    /**
     * Returns an adjustment of the tweening time
     *
     * This is a quarter circle with an asymptotic finish at t=1.
     *
     * @param time  The time in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float circOut(float time);
    
    /**
     * Returns an adjustment of the tweening time
     *
     * This is a concatenation of the {@link circIn(float)} and {@link circOut(float)}
     * easing functions.
     *
     * @param time  The time in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float circInOut(float time);
    
    /**
     * Returns an adjustment of the tweening time
     *
     * This is an easing function that briefly dips below t=0 after the start.
     *
     * @param time  The time in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float backIn(float time);
    
    /**
     * Returns an adjustment of the tweening time
     *
     * This is an easing function that briefly rises above t=1 before the finish.
     *
     * @param time  The time in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float backOut(float time);
    
    /**
     * Returns an adjustment of the tweening time
     *
     * This is a concatenation of the {@link backIn(float)} and {@link backOut(float)}
     * easing functions.
     *
     * @param time  The time in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float backInOut(float time);
    
    /**
     * Returns an adjustment of the tweening time
     *
     * This is an easing function that bounces down to t=0 several times after the
     * start.
     *
     * @param time  The time in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float bounceIn(float time);
    
    /**
     * Returns an adjustment of the tweening time
     *
     * This is an easing function that bounces up to t=1 several times before the
     * finish.
     *
     * @param time  The time in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float bounceOut(float time);
    
    /**
     * Returns an adjustment of the tweening time
     *
     * This is a concatenation of the {@link bounceIn(float)} and {@link bounceOut(float)}
     * easing functions.
     *
     * @param time  The time in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float bounceInOut(float time);
    
    /**
     * Returns an adjustment of the tweening time
     *
     * This is an easing function that bounces back-and-forth across t=0 several
     * times after the start.
     *
     * @param time      The time in seconds.
     * @param period    The period in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float elasticIn(float time, float period);
    
    /**
     * Returns an adjustment of the tweening time
     *
     * This is an easing function that bounces back-and-forth across t=1 several
     * times before the finish.
     *
     * @param time      The time in seconds.
     * @param period    The period in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float elasticOut(float time, float period);
    
    /**
     * Returns an adjustment of the tweening time
     *
     * This is a concatenation of the {@link elasticIn(float,float)} and
     * {@link elasticOut(float,float)} easing functions.
     *
     * @param time      The time in seconds.
     * @param period    The period in seconds.
     *
     * @return An adjustment of the tweening time
     */
    static float elasticInOut(float time, float period);
    
};

}


#endif /* __CU_EASING_ELASTIC_H__ */

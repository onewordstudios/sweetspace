//
//  CUEasingFunction.cpp
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
#include <cugl/math/CUEasingFunction.h>

using namespace cugl;

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
std::function<float(float)> EasingFunction::alloc(Type type, float period) {
    switch(type) {
    case Type::LINEAR:
        return EasingFunction::linear;
    case Type::SINE_IN:
        return EasingFunction::sineIn;
    case Type::SINE_OUT:
        return EasingFunction::sineOut;
    case Type::SINE_IN_OUT:
        return EasingFunction::sineInOut;
    case Type::QUAD_IN:
        return EasingFunction::quadIn;
    case Type::QUAD_OUT:
        return EasingFunction::quadOut;
    case Type::QUAD_IN_OUT:
        return EasingFunction::quadInOut;
    case Type::CUBIC_IN:
        return EasingFunction::cubicIn;
    case Type::CUBIC_OUT:
        return EasingFunction::cubicOut;
    case Type::CUBIC_IN_OUT:
        return EasingFunction::cubicInOut;
    case Type::QUART_IN:
        return EasingFunction::quartIn;
    case Type::QUART_OUT:
        return EasingFunction::quartOut;
    case Type::QUART_IN_OUT:
        return EasingFunction::quartInOut;
    case Type::QUINT_IN:
        return EasingFunction::quintIn;
    case Type::QUINT_OUT:
        return EasingFunction::quintOut;
    case Type::QUINT_IN_OUT:
        return EasingFunction::quintInOut;
    case Type::EXPO_IN:
        return EasingFunction::expoIn;
    case Type::EXPO_OUT:
        return EasingFunction::expoOut;
    case Type::EXPO_IN_OUT:
        return EasingFunction::expoInOut;
    case Type::CIRC_IN:
        return EasingFunction::circIn;
    case Type::CIRC_OUT:
        return EasingFunction::circOut;
    case Type::CIRC_IN_OUT:
        return EasingFunction::circInOut;
    case Type::BACK_IN:
        return EasingFunction::backIn;
    case Type::BACK_OUT:
        return EasingFunction::backOut;
    case Type::BACK_IN_OUT:
        return EasingFunction::backInOut;
    case Type::BOUNCE_IN:
        return EasingFunction::bounceIn;
    case Type::BOUNCE_OUT:
        return EasingFunction::bounceOut;
    case Type::BOUNCE_IN_OUT:
        return EasingFunction::bounceInOut;
    case Type::ELASTIC_IN:
        return [=] (float time) { return EasingFunction::elasticIn(time, period); };
    case Type::ELASTIC_OUT:
        return [=] (float time) { return EasingFunction::elasticOut(time, period); };
    case Type::ELASTIC_IN_OUT:
        return [=] (float time) { return EasingFunction::elasticInOut(time, period); };
    }
    
    return nullptr;
}

/**
 * Returns an adjustment of the tweening time
 *
 * This is a linear easing function (the default)
 *
 * @param time  The time in seconds.
 *
 * @return An adjustment of the tweening time
 */
float EasingFunction::linear(float time) {
    return time;
}

/**
 * Returns an adjustment of the tweening time
 *
 * This is a 1-cosine function with an asymptotic start at t=0.
 *
 * @param time  The time in seconds.
 *
 * @return An adjustment of the tweening time
 */
float EasingFunction::sineIn(float time) {
    return -1 * cosf(time * (float)M_PI_2) + 1;
}

/**
 * Returns an adjustment of the tweening time
 *
 * This is a sine function with an asymptotic finish at t=1.
 *
 * @param time  The time in seconds.
 *
 * @return An adjustment of the tweening time
 */
float EasingFunction::sineOut(float time) {
    return sinf(time * (float)M_PI_2);
}

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
float EasingFunction::sineInOut(float time) {
    return -0.5f * (cosf((float)M_PI * time) - 1);
}

/**
 * Returns an adjustment of the tweening time
 *
 * This is a quadratic polynomial with an asymptotic start at t=0.
 *
 * @param time  The time in seconds.
 *
 * @return An adjustment of the tweening time
 */
float EasingFunction::quadIn(float time) {
    return time * time;
}

/**
 * Returns an adjustment of the tweening time
 *
 * This is a quadratic polynomial with an asymptotic finish at t=1.
 *
 * @param time  The time in seconds.
 *
 * @return An adjustment of the tweening time
 */
float EasingFunction::quadOut(float time) {
    return -1 * time * (time - 2);
}

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
float EasingFunction::quadInOut(float time) {
    time = time*2;
    if (time < 1) {
        return 0.5f * time * time;
    }
    --time;
    return -0.5f * (time * (time - 2) - 1);
}

/**
 * Returns an adjustment of the tweening time
 *
 * This is a cubic polynomial with an asymptotic start at t=0.
 *
 * @param time  The time in seconds.
 *
 * @return An adjustment of the tweening time
 */
float EasingFunction::cubicIn(float time) {
    return time * time * time;
}

/**
 * Returns an adjustment of the tweening time
 *
 * This is a cubic polynomial with an asymptotic finish at t=1.
 *
 * @param time  The time in seconds.
 *
 * @return An adjustment of the tweening time
 */
float EasingFunction::cubicOut(float time) {
    time -= 1;
    return (time * time * time + 1);
}

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
float EasingFunction::cubicInOut(float time) {
    time = time*2;
    if (time < 1) {
        return 0.5f * time * time * time;
    }
    time -= 2;
    return 0.5f * (time * time * time + 2);
}

/**
 * Returns an adjustment of the tweening time
 *
 * This is a fourth-degree polynomial with an asymptotic start at t=0.
 *
 * @param time  The time in seconds.
 *
 * @return An adjustment of the tweening time
 */
float EasingFunction::quartIn(float time) {
    return time * time * time * time;
}

/**
 * Returns an adjustment of the tweening time
 *
 * This is a fourth-degree polynomial with an asymptotic finish at t=1.
 *
 * @param time  The time in seconds.
 *
 * @return An adjustment of the tweening time
 */
float EasingFunction::quartOut(float time) {
    time -= 1;
    return -(time * time * time * time - 1);
}

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
float EasingFunction::quartInOut(float time) {
    time = time*2;
    if (time < 1) {
        return 0.5f * time * time * time * time;
    }
    time -= 2;
    return -0.5f * (time * time * time * time - 2);
}

/**
 * Returns an adjustment of the tweening time
 *
 * This is a fifth-degree polynomial with an asymptotic start at t=0.
 *
 * @param time  The time in seconds.
 *
 * @return An adjustment of the tweening time
 */
float EasingFunction::quintIn(float time) {
    return time * time * time * time * time;
}

/**
 * Returns an adjustment of the tweening time
 *
 * This is a fifth-degree polynomial with an asymptotic finish at t=1.
 *
 * @param time  The time in seconds.
 *
 * @return An adjustment of the tweening time
 */
float EasingFunction::quintOut(float time) {
    time -=1;
    return (time * time * time * time * time + 1);
}

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
float EasingFunction::quintInOut(float time) {
    time = time*2;
    if (time < 1) {
        return 0.5f * time * time * time * time * time;
    }
    time -= 2;
    return 0.5f * (time * time * time * time * time + 2);
}

/**
 * Returns an adjustment of the tweening time
 *
 * This is an exponential function with an asymptotic start at t=0.
 *
 * @param time  The time in seconds.
 *
 * @return An adjustment of the tweening time
 */
float EasingFunction::expoIn(float time) {
    return time == 0 ? 0 : powf(2, 10 * (time/1 - 1)) - 1 * 0.001f;
}

/**
 * Returns an adjustment of the tweening time
 *
 * This is an exponential function with an asymptotic finish at t=1.
 *
 * @param time  The time in seconds.
 *
 * @return An adjustment of the tweening time
 */
float EasingFunction::expoOut(float time) {
    return time == 1 ? 1 : (-powf(2, -10 * time / 1) + 1);
}

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
float EasingFunction::expoInOut(float time) {
    time /= 0.5f;
    if (time < 1) {
        time = 0.5f * powf(2, 10 * (time - 1));
    } else {
        time = 0.5f * (-powf(2, -10 * (time - 1)) + 2);
    }
    
    return time;
}

/**
 * Returns an adjustment of the tweening time
 *
 * This is a quarter circle with an asymptotic start at t=0.
 *
 * @param time  The time in seconds.
 *
 * @return An adjustment of the tweening time
 */
float EasingFunction::circIn(float time) {
    return -1 * (sqrt(1 - time * time) - 1);
}

/**
 * Returns an adjustment of the tweening time
 *
 * This is a quarter circle with an asymptotic finish at t=1.
 *
 * @param time  The time in seconds.
 *
 * @return An adjustment of the tweening time
 */
float EasingFunction::circOut(float time) {
    time = time - 1;
    return sqrt(1 - time * time);
}

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
float EasingFunction::circInOut(float time) {
    time = time * 2;
    if (time < 1) {
        return -0.5f * (sqrt(1 - time * time) - 1);
    }
    time -= 2;
    return 0.5f * (sqrt(1 - time * time) + 1);
}

/**
 * Returns an adjustment of the tweening time
 *
 * This is an easing function that briefly dips below t=0 after the start.
 *
 * @param time  The time in seconds.
 *
 * @return An adjustment of the tweening time
 */
float EasingFunction::backIn(float time) {
    float overshoot = 1.70158f;
    return time * time * ((overshoot + 1) * time - overshoot);
}

/**
 * Returns an adjustment of the tweening time
 *
 * This is an easing function that briefly rises above t=1 before the finish.
 *
 * @param time  The time in seconds.
 *
 * @return An adjustment of the tweening time
 */
float EasingFunction::backOut(float time) {
    float overshoot = 1.70158f;
    time = time - 1;
    return time * time * ((overshoot + 1) * time + overshoot) + 1;
}

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
float EasingFunction::backInOut(float time) {
    float overshoot = 1.70158f * 1.525f;
    
    time = time * 2;
    if (time < 1) {
        return (time * time * ((overshoot + 1) * time - overshoot)) / 2;
    } else {
        time = time - 2;
        return (time * time * ((overshoot + 1) * time + overshoot)) / 2 + 1;
    }
}

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
float EasingFunction::bounceIn(float time) {
    return 1 - bounceOut(1 - time);
}

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
float EasingFunction::bounceOut(float time) {
    if (time < 1 / 2.75) {
        return 7.5625f * time * time;
    } else if (time < 2 / 2.75) {
        time -= 1.5f / 2.75f;
        return 7.5625f * time * time + 0.75f;
    } else if(time < 2.5 / 2.75) {
        time -= 2.25f / 2.75f;
        return 7.5625f * time * time + 0.9375f;
    }
    
    time -= 2.625f / 2.75f;
    return 7.5625f * time * time + 0.984375f;
}

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
float EasingFunction::bounceInOut(float time) {
    float result = 0;
    if (time < 0.5f) {
        time = time * 2;
        result = (1 - bounceOut(1 - time)) * 0.5f;
    } else {
        result = bounceOut(time * 2 - 1) * 0.5f + 0.5f;
    }
    
    return result;
}

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
float EasingFunction::elasticIn(float time, float period) {
    float result = 0;
    if (time == 0 || time == 1) {
        result = time;
    } else {
        float s = period / 4;
        time = time - 1;
        result = -powf(2, 10 * time) * sinf((time - s) * ((float)M_PI) * 2.0f / period);
    }
    
    return result;
}

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
float EasingFunction::elasticOut(float time, float period) {
    float result = 0;
    if (time == 0 || time == 1) {
        result = time;
    } else {
        float s = period / 4;
        result = powf(2, -10 * time) * sinf((time - s) * ((float)M_PI) * 2.0f / period) + 1;
    }
    
    return result;
}

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
float EasingFunction::elasticInOut(float time, float period) {
    float result = 0;
    if (time == 0 || time == 1) {
        result = time;
    } else {
        time = time * 2;
        if (! period) {
            period = 0.3f * 1.5f;
        }
        
        float s = period / 4;
        
        time = time - 1;
        if (time < 0) {
            result = -0.5f * powf(2, 10 * time) * sinf((time -s) * ((float)M_PI) * 2.0f / period);
        } else {
            result = powf(2, -10 * time) * sinf((time - s) * ((float)M_PI)* 2.0f / period) * 0.5f + 1;
        }
    }
    return result;
}

//
//  CUColor4.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a standard colors.  It provides both a
//  float based color solution and a byte-based color solution.  The former
//  is better for blending and calculations.  The later is better for storage.
//
//  Because math objects are intended to be on the stack, we do not provide
//  any shared pointer support in this class.
//
//  This module is based on an original file from GamePlay3D: http://gameplay3d.org.
//  It has been modified to support the CUGL framework.
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
//  Author: Walker White
//  Version: 5/30/16
#ifndef __CU_COLOR4_H__
#define __CU_COLOR4_H__

#include <math.h>
#include <functional>
#include <cugl/math/CUMathBase.h>
#include <cugl/base/CUEndian.h>

/** Conversion from byte to float in [0,1] */
#define COLOR_BYTE_TO_FLOAT(x)       ( static_cast<float>(x)/255.0f )
/** Conversion from float in [0,1] to byte */
#define COLOR_FLOAT_TO_BYTE(x)       ( static_cast<int>(roundf(x*255.0f)) )

namespace cugl {
    
// Forward references to the classes we convert to
class Color4;
class Vec3;
class Vec4;
    
#pragma mark -
#pragma mark Color with Float Attributes
    
/**
 * This class is a RGBA color composed of 4 floats.
 *
 * This is the preferred transparent color class for when you need to do
 * a lot of calculations.  However, it is not a compact storage format and is
 * not ideal for shipping to the GPU.  Used {@link Color4} for that instead.
 *
 * This class is in standard layout with fields of uniform type.  This means
 * that it is safe to reinterpret_cast objects to float arrays and Vec4.  In
 * addition, it has cast support for the latter.  However, this class contains
 * arithmetic methods to keep these casts from being necessary. In addition,
 * all of the arithmetic operations implicitly clamp to maintain the invariants
 * on all of the color values.
 */
class Color4f {
        
#pragma mark Values
public:
    /** The red value */
    GLfloat r;
    /** The green value */
    GLfloat g;
    /** The blue value */
    GLfloat b;
    /** The alpha value */
    GLfloat a;
    
    /** The Clear color (0,0,0,0) */
    static const Color4f CLEAR;
    /** The White color (1,1,1,1) */
    static const Color4f WHITE;
    /** The Black color (0,0,0,1) */
    static const Color4f BLACK;
    /** The Yellow color (1,1,0,1) */
    static const Color4f YELLOW;
    /** The Blue color (0,0,1,1) */
    static const Color4f BLUE;
    /** The Green color (0,1,0,1) */
    static const Color4f GREEN;
    /** The Red color (1,0,0,1) */
    static const Color4f RED;
    /** The Magenta color (1,0,1,1) */
    static const Color4f MAGENTA;
    /** The Cyan color (0,1,1,1) */
    static const Color4f CYAN;
    /** The Orange color (1,0.5,0,1) */
    static const Color4f ORANGE;
    /** The Gray color (0.65,0.65,0.65,1) */
    static const Color4f GRAY;
    /** The classic XNA color (0.392,0.584,0.93,1) */
    static const Color4f CORNFLOWER;
    /** The Playing Fields color (0.933f,0.99f,0.65f,1.0f) */
    static const Color4f PAPYRUS;
    
#pragma mark Constructors
public:
    /**
     * Constructs a new clear color (all zeros).
     */
    Color4f() : r(0), g(0), b(0), a(0) {}
        
    /**
     * Constructs a new color initialized to the specified values.
     *
     * The color values must all be in the range 0..1.
     *
     * @param r The red color.
     * @param g The green color.
     * @param b The blue color.
     * @param a The alpha value (optional).
     */
    Color4f(float r, float g, float b, float a = 1);
        
    /**
     * Creates a new color from an integer interpreted as an RGBA value.
     *
     * This constructor processes the integer in RGBA order. Hence, 0xff0000ff
     * represents red or the color (1, 0, 0, 1).
     *
     * @param color The integer to interpret as an RGBA value.
     */
    Color4f(GLuint color);
        
    /**
     * Constructs a new color from the values in the specified array.
     *
     * The color values must all be in the range 0..1.
     *
     * @param array An array containing the color values in the order r, g, b, a.
     */
    Color4f(const float* array);
        
    /**
     *  Destroys this color, releasing all resources
     */
    ~Color4f() {}
        
        
#pragma mark Setters
    /**
     * Sets this color to an integer interpreted as an RGBA value.
     *
     * This setter processes the integer in RGBA order. Hence, 0xff0000ff
     * represents red or the color (1, 0, 0, 1).
     *
     * @param color The integer to interpret as an RGBA value.        
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4f& operator=(unsigned int color) {
        return set(color);
    }
        
    /**
     * Sets the elements of this color from the values in the specified array.
     *
     * The color values must all be in the range 0..1.
     *
     * @param array An array containing the color values in the order r, g, b, a.
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4f& operator=(const float* array) {
        return set(array);
    }
        
    /**
     * Sets the elements of this color to the specified values.
     *
     * The color values must all be in the range 0..1.
     *
     * @param r The red color.
     * @param g The green color.
     * @param b The blue color.
     * @param a The alpha value (optional).
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4f& set(float r, float g, float b, float a = 1);
        
    /**
     * Sets the elements of this color from the values in the specified array.
     *
     * The color values must all be in the range 0..1.
     *
     * @param array An array containing the color values in the order r, g, b, a.
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4f& set(const float* array);
        
    /**
     * Sets this color to an integer interpreted as an RGBA value.
     *
     * This setter processes the integer in RGBA order. Hence, 0xff0000ff
     * represents red or the color (1, 0, 0, 1).
     *
     * @param color The integer to interpret as an RGBA value.
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4f& set(GLuint color);
    
    /**
     * Sets the values of this color to those in the specified color.
     *
     * @param c The color to copy.
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4f& set(const Color4f& c) {
        r = c.r; g = c.g; b = c.b; a = c.a;
        return *this;
    }
        
        
#pragma mark Arithmetic
    /**
     * Clamps this color within the given range.
     *
     * @param min The minimum value.
     * @param max The maximum value.
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4f& clamp(const Color4f& min, const Color4f& max);
        
    /**
     * Returns a copy of this color clamped within the given range.
     *
     * Note: this does not modify this color.
     *
     * @param min The minimum value.
     * @param max The maximum value.
     *
     * @return A copy of this color clamped within the given range.
     */
    Color4f getClamp(const Color4f& min, const Color4f& max) const {
        return Color4f(clampf(r,min.r,max.r), clampf(g,min.g,max.g),
                       clampf(b,min.b,max.b), clampf(a,min.a,max.a));
    }
        
    /**
     * Adds the given color to this one in place.
     *
     * This operation is functionally identical to additive blending.
     * The addition is clamped so that this remains a valid color.
     *
     * @param c     The color to add
     * @param alpha Whether to add the alpha values (optional)
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4f& add(const Color4f& c, bool alpha = false) {
        r = clampf(r+c.r,0.0,1.0);
        g = clampf(g+c.g,0.0,1.0);
        b = clampf(b+c.b,0.0,1.0);
        a = (alpha ? clampf(a+c.a,0.0,1.0) : a);
        return *this;
    }
        
    /**
     * Adds the given values to this color.
     *
     * The addition is clamped so that this remains a valid color.
     *
     * @param r The red color to add.
     * @param g The green color to add.
     * @param b The blue color to add.
     * @param a The alpha value (optional).
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4f& add(float r, float g, float b, float a = 0) {
        this->r = clampf(this->r+r,0.0,1.0);
        this->g = clampf(this->g+g,0.0,1.0);
        this->b = clampf(this->b+b,0.0,1.0);
        this->a = clampf(this->a+a,0.0,1.0);
        return *this;
    }
        
    /**
     * Subtracts the given color from this one in place.
     *
     * This operation is functionally identical to subtractive blending.
     * The subtraction is clamped so that this remains a valid color.
     *
     * @param c     The color to subtract
     * @param alpha Whether to subtract the alpha value (optional)
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4f& subtract(const Color4f& c, bool alpha = false) {
        r = clampf(r-c.r,0.0,1.0);
        g = clampf(g-c.g,0.0,1.0);
        b = clampf(b-c.b,0.0,1.0);
        a = (alpha ? clampf(a-c.a,0.0,1.0) : a);
        return *this;
    }
        
    /**
     * Subtracts the given values from this color.
     *
     * The subtraction is clamped so that this remains a valid color.
     *
     * @param r The red color to subtract.
     * @param g The green color to subtract.
     * @param b The blue color to subtract.
     * @param a The alpha value (optional).
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4f& subtract(float r, float g, float b, float a = 0) {
        this->r = clampf(this->r-r,0.0,1.0);
        this->g = clampf(this->g-g,0.0,1.0);
        this->b = clampf(this->b-b,0.0,1.0);
        this->a = clampf(this->a-a,0.0,1.0);
        return *this;
    }
        
    /**
     * Scales this color in place by the given factor.
     *
     * The scaling is clamped so that this remains a valid color.
     *
     * @param s     The scalar to multiply by
     * @param alpha Whether to scale the alpha value (optional)
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4f& scale(float s, bool alpha = false) {
        r = clampf(r*s,0.0,1.0);
        g = clampf(g*s,0.0,1.0);
        b = clampf(b*s,0.0,1.0);
        a = (alpha ? clampf(a*s,0.0,1.0) : a);
        return *this;
    }
        
    /**
     * Scales this color nonuniformly by the given factors.
     *
     * The scaling is clamped so that this remains a valid color.
     *
     * @param sr The scalar to multiply the red value
     * @param sg The scalar to multiply the green value
     * @param sb The scalar to multiply the blue value
     * @param sa The scalar to multiply alpha value (optional).
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4f& scale(float sr, float sg, float sb, float sa = 1) {
            r = clampf(r*sr,0.0,1.0);
            g = clampf(g*sg,0.0,1.0);
            b = clampf(b*sb,0.0,1.0);
            a = clampf(a*sa,0.0,1.0);
            return *this;
        }
        
    /**
     * Scales this color nonuniformly by the given color.
     *
     * This operation is functionally identical to multiplicative blending.
     *
     * @param c     The color to scale by
     * @param alpha Whether to scale the alpha value (optional)
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4f& scale(const Color4f& c, bool alpha = false) {
        r *= c.r; g *= c.g; b *= c.b;
        a = (alpha ? a*c.a : a);
        return *this;
    }
        
    /**
     * Maps the given function to the color values in place.
     *
     * This method supports any function that has the signature float func(float);
     * This includes many of the functions in math.h. However, the values are
     * clamped to ensure that this remains a valid color.
     *
     * @param func  The function to map on the color values.
     * @param alpha Whether to modify the alpha value (optional)
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4f& map(std::function<float(float)> func, bool alpha = false) {
        r = clampf(func(r),0,1); g = clampf(func(g),0,1); b = clampf(func(b),0,1);
        a = (alpha ? clampf(func(a),0,1) : a);
        return *this;
    }
        
    /**
     * Returns a copy of this color with func applied to each component.
     *
     * This method supports any function that has the signature float func(float);
     * This includes many of the functions in math.h.
     *
     * @param func  The function to map on the color values.
     * @param alpha Whether to modify the alpha value (optional)
     *
     * @return A copy of this color with func applied to each component.
     */
    Color4f getMap(std::function<float(float)> func, bool alpha = false) const {
        return Color4f(clampf(func(r),0,1), clampf(func(g),0,1),
                       clampf(func(b),0,1), (alpha ? clampf(func(a),0,1) : a));
    }

    
#pragma mark Color Operations
    /**
     * Complements this color.
     *
     * The complement of the color is the (1-v) for each value v.
     *
     * @param alpha Whether to complement the alpha value (optional)
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4f& complement(bool alpha = false) {
        r = 1-r; g = 1-g; b = 1-b; a = (alpha ? 1-a : a);
        return *this;
    }
    
    /**
     * Returns the complement of this color.
     *
     * The complement of the color is the (1-v) for each value v.
     *
     * @param alpha Whether to complement the alpha value (optional)
     *
     * Note: this does not modify this color.
     *
     * @return The complement of this color
     */
    Color4f getComplement(bool alpha = false) const {
        return Color4f(1-r, 1-g, 1-b, (alpha ? 1-a : a));
    }
    
    /**
     * Modifies this color to be the linear interpolation with other.
     *
     * If alpha is 0, the color is unchanged.  If alpha is 1, the color is
     * other.  Otherwise it is a value in between.  If alpha is outside of the
     * range 0 to 1, it is clamped to the nearest value.
     *
     * This method just implements standard linear interpolation.  It does
     * not attempt to give any blending semantics to it.
     *
     * @param other The color to interpolate with.
     * @param alpha The interpolation value in 0..1
     *
     * @return This color, after the interpolation.
     */
    Color4f& lerp(const Color4f& other, float alpha) {
        float x = clampf(alpha,0,1);
        *this *= (1.f - x);
        return *this += other * x;
    }
    
    /**
     * Blends this color with the other one, storing the new value in place.
     *
     * The blending is the standard over operation with this color as the
     * destination. It assumes that the color values are not premultiplied.
     *
     * @param other The color to interpolate with.
     *
     * @return This color, after the blending.
     */
    Color4f& blend(const Color4f& other) {
        float a1 = a*(1-other.a);
        float a2 = other.a+a1;
        r = (other.r*other.a+r*a1)/a2;
        g = (other.g*other.a+g*a1)/a2;
        b = (other.b*other.a+b*a1)/a2;
        a = a2;
        return *this;
    }
    
    /**
     * Blends this color with the other one, storing the new value in place.
     *
     * The blending is the standard over operation with this color as the
     * destination. It assumes that this color is premultiplied.
     *
     * @param other The color to interpolate with.
     *
     * @return This color, after the blending.
     */
    Color4f& blendPre(const Color4f& other) {
        a = other.a+a*(1-other.a);
        r = other.r+r*(1-other.a);
        g = other.g+g*(1-other.a);
        b = other.b+b*(1-other.a);
        return *this;
    }
    
    /**
     * Premultiplies this color with its current alpha.
     *
     * This class does not store whether the color is already premultiplied.
     * Hence premultiplying an already premultiplied color will have a 
     * compounding effect.
     *
     * @return This color, after premultiplication.
     */
    Color4f& premultiply() {
        r *= a; g *= a; b *= a;
        return *this;
    }

    /**
     * Undoes premultiplication this color with its current alpha.
     *
     * This class does not store whether the color is already premultiplied.
     * Hence unpremultiplying an non-premultiplied color will have a
     * compounding effect.
     *
     * If the alpha value is 0, the color is unchanged.
     *
     * @return This color, after undoing premultiplication.
     */
    Color4f& unpremultiply() {
        if (a > 0) {
            r /= a; g /= a; b /= a;
        }
        return *this;
    }
    
    /**
     * Returns the linear interpolation of this color with other.
     *
     * If alpha is 0, the color is unchanged.  If alpha is 1, the color is
     * other.  Otherwise it is a value in between.  If alpha is outside of the
     * range 0 to 1, it is clamped to the nearest value.
     *
     * This method just implements standard linear interpolation.  It does
     * not attempt to give any blending semantics to it.
     *
     * Note: this does not modify this color.
     *
     * @param other The color to interpolate with.
     * @param alpha The interpolation value in 0..1
     *
     * @return The linear interpolation of this color with other.
     */
    Color4f getLerp(const Color4f& other, float alpha) const {
        float x = clampf(alpha,0,1);
        return *this * (1.f - x) + other * x;
    }

    /**
     * Returns a blend of this color with the other one.
     *
     * The blending is the standard over operation with this color as the
     * destination. It assumes that the color values are not premultiplied.
     *
     * Note: this does not modify this color.
     *
     * @param other The color to interpolate with.
     *
     * @return The newly blended color
     */
    Color4f getBlend(const Color4f& other) const {
        float a1 = a*(1-other.a);
        float a2 = other.a+a1;
        return Color4f((other.r*other.a+r*a1)/a2,
                       (other.g*other.a+g*a1)/a2,
                       (other.b*other.a+b*a1)/a2,
                       a2);
    }
    
    /**
     * Returns a blend of this color with the other one.
     *
     * The blending is the standard over operation with this color as the
     * destination. It assumes that this color is premultiplied.
     *
     * Note: this does not modify this color.
     *
     * @param other The color to interpolate with.
     *
     * @return The newly blended color
     */
    Color4f getBlendPre(const Color4f& other) const {
        float oa = other.a+a*(1-other.a);
        return Color4f((other.r+r*(1-other.a)), (other.g+g*(1-other.a)),
                       (other.b+b*(1-other.a)), oa);
    }
    
    /**
     * Returns the premultiplied version of this color, using its current alpha.
     *
     * This class does not store whether the color is already premultiplied.
     * Hence premultiplying an already premultiplied color will have a
     * compounding effect.
     *
     * Note: this does not modify this color.
     *
     * @return The newly premultiplied color
     */
    Color4f getPremultiplied() const {
        return Color4f(r*a,g*a,b*a,a);
    }
    
    /**
     * Returns the unpremultiplied version of this color, using its current alpha.
     *
     * This class does not store whether the color is already premultiplied.
     * Hence unpremultiplying an non-premultiplied color will have a
     * compounding effect.
     *
     * If the alpha value is 0, the copy is equal to the original color.
     *
     * Note: this does not modify this color.
     *
     * @return The newly premultiplied color
     */
    Color4f getUnpremultiplied() const {
        if (a > 0) {
            return Color4f(r/a,g/a,b/a,a);
        }
        return *this;
    }
    
    /**
     * Interpolates the two colors c1 and c2, and stores the result in dst.
     *
     * If alpha is 0, the result is c1.  If alpha is 1, the color is c2.
     * Otherwise it is a value in c1..c2.  If alpha is outside of the
     * range 0 to 1, it is clamped to the nearest value.
     *
     * This method just implements standard linear interpolation.  It does
     * not attempt to give any blending semantics to it.
     *
     * @param c1    The first color.
     * @param c2    The second color.
     * @param alpha The interpolation value in 0..1
     * @param dst   The color to store the result in
     *
     * @return A reference to dst for chaining
     */
    static Color4f* lerp(const Color4f& c1, const Color4f& c2, float alpha, Color4f* dst);
    
    /**
     * Blends the two colors c1 and c2, assuming they are not premultiplied.
     *
     * The blending is the standard over operation with color c1 as the source
     * and c2 as the destination. It assumes that the color values are not 
     * premultiplied.
     *
     * @param c1    The source color.
     * @param c2    The destination color.
     * @param dst   The color to store the result in     
     *
     * @return A reference to dst for chaining
     */
    static Color4f* blend(const Color4f& c1, const Color4f& c2, Color4f* dst);
    
    /**
     * Blends the two colors c1 and c2, assuming they are premultiplied.
     *
     * The blending is the standard over operation with color c1 as the source
     * and c2 as the destination. It assumes that the color values are
     * premultiplied.
     *
     * @param c1    The source color.
     * @param c2    The destination color.
     * @param dst   The color to store the result in
     *
     * @return A reference to dst for chaining
     */
    static Color4f* blendPre(const Color4f& c1, const Color4f& c2, Color4f* dst);
    
    /**
     * Returns the packed integer representation of this color
     *
     * This method converts the color to a Color4 and returns the packed color
     * of that result. This representation returned is native to the platform.
     *
     * In this representation, red will always be the highest order byte and
     * alpha will always be the lowest order byte.
     *
     * @return the packed integer representation of this color
     */
    GLuint getRGBA() const;
    
#pragma mark Operators
    /**
     * Adds the given color to this one in place.
     *
     * This operation is functionally identical to additive blending.
     * The addition is clamped so that this remains a valid color.
     *
     * This version of addition always adds alpha values.
     *
     * @param c The color to add
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4f& operator+=(const Color4f& c) {
        r = clampf(r+c.r,0.0,1.0);
        g = clampf(g+c.g,0.0,1.0);
        b = clampf(b+c.b,0.0,1.0);
        a = clampf(a+c.a,0.0,1.0);
        return *this;
    }
        
    /**
     * Subtracts the given color from this one in place.
     *
     * This operation is functionally identical to subtractive blending.
     * The subtraction is clamped so that this remains a valid color.
     *
     * This version of subtraction always subtracts alpha values.
     *
     * @param c The color to subtract
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4f& operator-=(const Color4f& c) {
        r = clampf(r-c.r,0.0,1.0);
        g = clampf(g-c.g,0.0,1.0);
        b = clampf(b-c.b,0.0,1.0);
        a = clampf(a-c.a,0.0,1.0);
        return *this;
    }
        
    /**
     * Scales this color in place by the given factor.
     *
     * The scaling is clamped so that this remains a valid color.
     *
     * This version of scaling always multiplies the alpha values.
     *
     * @param s The value to scale by
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4f& operator*=(float s) {
        r = clampf(r*s,0.0,1.0);
        g = clampf(g*s,0.0,1.0);
        b = clampf(b*s,0.0,1.0);
        a = clampf(a*s,0.0,1.0);
        return *this;
    }
        
    /**
     * Scales this color nonuniformly by the given color.
     *
     * This operation is functionally identical to multiplicative blending.
     *
     * @param c The color to scale by
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4f& operator*=(const Color4f& c) {
        r *= c.r; g *= c.g; b *= c.b; a *= c.a;
        return *this;
    }
        
    /**
     * Returns the sum of this color with the given color.
     *
     * This operation is functionally identical to additive blending.
     * The addition is clamped so that this remains a valid color.
     *
     * This version of addition always adds alpha values.
     *
     * Note: this does not modify this color.
     *
     * @param c The color to add.
     *
     * @return The sum of this color with the given color.
     */
    const Color4f operator+(const Color4f& c) const {
        Color4f result(*this);
        return result += c;
    }
        
    /**
     * Returns the difference of this color with the given color.
     *
     * This operation is functionally identical to subtractive blending.
     * The subtraction is clamped so that this remains a valid color.
     *
     * This version of subtraction always adds subtracts values.
     *
     * Note: this does not modify this color.
     *
     * @param c The color to subtract.
     *
     * @return The difference of this color with the given color.
     */
    const Color4f operator-(const Color4f& c) const  {
        Color4f result(*this);
        return result -= c;
    }
    
    /**
     * Returns the scalar product of this color with the given color.
     *
     * The scaling is clamped so that this remains a valid color.
     *
     * Note: this does not modify this color.
     *
     * @param s The value to scale by.
     *
     * @return The scalar product of this color with the given value.
     */
    const Color4f operator*(float s) const {
        Color4f result(*this);
        return result *= s;
    }
    
    /**
     * Returns the scalar product of this color with the given color.
     *
     * This operation is functionally identical to multiplicative blending.
     *
     * Note: this does not modify this color.
     *
     * @param c The color to scale by.
     *
     * @return The scalar product of this color with the given color.
     */
    const Color4f operator*(const Color4f& c) const {
        Color4f result(*this);
        return result *= c;
    }
        
        
#pragma mark Comparisons
    /**
     * Returns true if this color is less than the given color.
     *
     * This comparison uses lexicographical order of rgba.  To test if all
     * components in this color are less than those of c, use the method
     * darkerThan().
     *
     * @param c The color to compare against.
     *
     * @return True if this color is less than the given color.
     */
    bool operator<(const Color4f& c) const;
        
    /**
     * Returns true if this color is less than or equal the given color.
     *
     * This comparison uses lexicographical order of rgba.  To test if all
     * components in this color are less than those of c, use the method
     * darkerThan().
     *
     * @param c The color to compare against.
     *
     * @return True if this color is less than or equal the given color.
     */
    bool operator<=(const Color4f& c) const {
        return *this < c || *this == c;
    }
        
    /**
     * Returns true if this color is greater than the given color.
     *
     * This comparison uses lexicographical order of rgba.  To test if all
     * components in this color are greater than those of c, use the method
     * lighterThan().
     *
     * @param c The color to compare against.
     *
     * @return True if this color is greater than the given color.
     */
    bool operator>(const Color4f& c) const;
        
    /**
     * Returns true if this color is greater than or equal the given color.
     *
     * This comparison uses lexicographical order of rgba.  To test if all
     * components in this color are greater than those of c, use the method
     * lighterThan().
     *
     * @param c The color to compare against.
     *
     * @return True if this color is greater than or equal the given color.
     */
    bool operator>=(const Color4f& c) const {
        return *this > c || *this == c;
    }
        
    /**
     * Returns true if this color is equal to the given color.
     *
     * Comparison is exact, which may be unreliable given that the attributes
     * are floats.
     *
     * @param c The color to compare against.
     *
     * @return True if this color is equal to the given color.
     */
    bool operator==(const Color4f& c) const {
        return r == c.r && g == c.g && b == c.b && a == c.a;
    }
        
    /**
     * Returns true if this color is not equal to the given color.
     *
     * Comparison is exact, which may be unreliable given that the attributes
     * are floats.
     *
     * @param c The color to compare against.
     *
     * @return True if this color is not equal to the given color.
     */
    bool operator!=(const Color4f& c) const {
        return r != c.r || g != c.g || b != c.b || a != c.a;
    }
        
    /**
     * Returns true if this color is dominated by the given color.
     *
     * Domination means that all components of the given color are greater than
     * or equal to the components of this one.  However, alpha is reversed for
     * this computation, as a greater value means more opaque (and hence darker).
     *
     * @param c The color to compare against.
     *
     * @return True if this color is dominated by the given color.
     */
    bool darkerThan(const Color4f& c) const {
        return r <= c.r &&  g <= c.g && b <= c.b && c.a <= a;
    }
        
    /**
     * Returns true if this color dominates the given color.
     *
     * Domination means that all components of this color are greater than
     * or equal to the components of the given color. However, alpha is reversed 
     * for this computation, as a lesser value means more transparent (and hence 
     * lighter).
     *
     * @param c The color to compare against.
     *
     * @return True if this color is dominated by the given color.
     */
    bool lighterThan(const Color4f& c) const {
        return r >= c.r && g >= c.g && b >= c.b && c.a >= a;
    }
        
    /**
     * Returns true if the color are within tolerance of each other.
     *
     * The tolerance bounds each color attribute separately.
     *
     * @param color     The color to compare against.
     * @param variance  The comparison tolerance.
     *
     * @return true if the color are within tolerance of each other.
     */
    bool equals(const Color4f& color, float variance=CU_MATH_EPSILON) const {
        return (fabsf(r-color.r) < variance && fabsf(g-color.g) < variance &&
                fabsf(b-color.b) < variance && fabsf(a-color.a) < variance);
    }
        
        
#pragma mark Conversion Methods
public:
    /**
     * Returns a string representation of this vector for debuggging purposes.
     *
     * If verbose is true, the string will include class information.  This
     * allows us to unambiguously identify the class.
     *
     * @param verbose Whether to include class information
     *
     * @return a string representation of this vector for debuggging purposes.
     */
    std::string toString(bool verbose = false) const;
        
    /** Cast from Color4f to a string. */
    operator std::string() const { return toString(); }
        
    /** Cast from Color4f to a vector. */
    operator Vec4() const;
        
    /**
     * Creates a color from the given vector.
     *
     * The attributes are read in the order x,y,z,w.
     *
     * @param vector    The vector to convert
     */
    explicit Color4f(const Vec4& vector);
        
    /**
     * Sets the coordinates of this color to those of the given vector.
     *
     * The attributes are read in the order x,y,z,w.
     *
     * @param vector    The vector to convert
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4f& operator= (const Vec4& vector);
    
    /** Cast from Color4f to a vector. */
    operator Vec3() const;
    
    /**
     * Creates a color from the given vector.
     *
     * The attributes are read in the order x,y,z. The alpha value is 1.
     *
     * @param vector    The vector to convert
     */
    explicit Color4f(const Vec3& vector);
    
    /**
     * Sets the coordinates of this color to those of the given vector.
     *
     * The attributes are read in the order x,y,z. The alpha value is 1.
     *
     * @param vector    The vector to convert
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4f& operator= (const Vec3& vector);
    
    /** Cast from Color4f to a byte-based Color4. */
    operator Color4() const;
        
    /**
     * Creates a float-based color from the given byte-based color.
     *
     * The attributes are read in the order r,g,b,a. They are all divided by
     * 255.0f before assignment.
     *
     * @param color The color to convert
     */
    explicit Color4f(Color4 color);
        
    /**
     * Sets the attributes of this color from the given byte-based color.
     *
     * The attributes are read in the order r,g,b,a. They are all divided by
     * 255.0f before assignment.
     *
     * @param color The color to convert.
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4f& operator= (Color4 color);
};
  

#pragma mark -
#pragma mark Color with Byte Attributes
    
/**
 * This class is a RGBA color composed of 4 unsigned bytes.
 *
 * This is the preferred (non-transparent) color class for storage.  It
 * is slightly slower on calculations, because it often has to shift to
 * 0..1 for all its values.  It is also slower to cast to Vec3.
 */
class Color4 {
        
#pragma mark Values
public:
union {
    struct {
        /** The red value */
        GLubyte r;
        /** The green value */
        GLubyte g;
        /** The blue value */
        GLubyte b;
        /** The alpha value */
        GLubyte a;
    };
    /** The color as a packed integer (WARNING: Endian specific) */
    Uint32 rgba;
};
    
    /** The Clear color (0,0,0,0) */
    static const Color4 CLEAR;
    /** The White color (1,1,1,1) */
    static const Color4 WHITE;
    /** The Black color (0,0,0,1) */
    static const Color4 BLACK;
    /** The Yellow color (1,1,0,1) */
    static const Color4 YELLOW;
    /** The Blue color (0,0,1,1) */
    static const Color4 BLUE;
    /** The Green color (0,1,0,1) */
    static const Color4 GREEN;
    /** The Red color (1,0,0,1) */
    static const Color4 RED;
    /** The Magenta color (1,0,1,1) */
    static const Color4 MAGENTA;
    /** The Cyan color (0,1,1,1) */
    static const Color4 CYAN;
    /** The Orange color (1,0.5,0,1) */
    static const Color4 ORANGE;
    /** The Gray color (0.65,0.65,0.65,1) */
    static const Color4 GRAY;
    /** The classic XNA color (0.392,0.584,0.93,1) */
    static const Color4 CORNFLOWER;
    /** The Playing Fields color (0.933f,0.99f,0.65f,1.0f) */
    static const Color4 PAPYRUS;
    
        
#pragma mark Constructors
public:
    /**
     * Constructs a new clear color (all zeros).
    */
    Color4() : rgba(0) {}
        
    /**
     * Constructs a new color initialized to the specified values.
     *
     * @param r The red color.
     * @param g The green color.
     * @param b The blue color.
     * @param a The alpha value (optional).
     */
    Color4(GLubyte r, GLubyte g, GLubyte b, GLubyte a = 255) {
        this->r = r; this->g = g; this->b = b; this->a = a;
    }
        
    /**
     * Creates a new color from an integer interpreted as an RGBA value.
     *
     * This representation assigned is native to the platform, but it will be
     * normalized to network order. In this representation, red will always be 
     * the highest order byte and alpha will always be the lowest order byte.     
     *
     * @param color The integer to interpret as an RGBA value.
     */
    Color4(GLuint color) { set(color); }
        
    /**
     * Constructs a new color from the values in the specified array.
     *
     * The color values must all be in the range 0..1. They are multiplied 
     * by 255.0 and rounded up.
     *
     * @param array An array containing the color values in the order r, g, b, a.
     */
    Color4(const float* array);
    
        
#pragma mark Setters        
    /**
     * Sets this color to the integer interpreted as an RGBA value.
     *
     * This representation assigned is native to the platform, but it will be
     * normalized to network order. In this representation, red will always be
     * the highest order byte and alpha will always be the lowest order byte.
     *
     * @param color The integer to interpret as an RGBA value.
     *
     * @return A reference to this (modified) Color4 for chaining.
     */
    Color4& operator=(GLuint color) {
        return set(color);
    }
        
    /**
     * Sets the elements of this color from the values in the specified array.
     *
     * The color values must all be in the range 0..1. They are multiplied
     * by 255.0 and rounded up
     *
     * @param array An array containing the color values in the order r, g, b, a
     *
     * @return A reference to this (modified) Color4 for chaining.
     */
    Color4& operator=(const float* array) {
        return set(array);
    }
        
    /**
     * Sets the elements of this color to the specified values.
     *
     * @param r The red color.
     * @param g The green color.
     * @param b The blue color.
     * @param a The alpha value (optional).
     *
     * @return A reference to this (modified) Color43 for chaining.
     */
    Color4& set(GLubyte r, GLubyte g, GLubyte b, GLubyte a = 255) {
        this->r = r; this->g = g; this->b = b; this->a = a;
        return *this;
    }
        
    /**
     * Sets the elements of this color from the values in the specified array.
     *
     * The color values must all be in the range 0..1. They are multiplied
     * by 255.0 and rounded up
     *
     * @param array An array containing the color values in the order r, g, b, a
     *
     * @return A reference to this (modified) Color4 for chaining.
     */
    Color4& set(const float* array);
        
    /**
     * Sets this color to the integer interpreted as an RGBA value.
     *
     * This setter processes the integer in RGBA order. Hence, 0xff0000ff
     * represents red or the color (1, 0, 0, 1).
     *
     * @param color The integer to interpret as an RGBA value.
     *
     * @return A reference to this (modified) Color4 for chaining.
     */
    Color4& set(GLuint color) {
        rgba = marshall(color);
        return *this;
    }
        
    /**
     * Sets the values of this color to those in the specified color.
     *
     * @param c The color to copy.
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4& set(const Color4& c) {
        rgba = c.rgba;
        return *this;
    }
        
        
#pragma mark Arithmetic
    /**
     * Clamps this color within the given range.
     *
     * @param min The minimum value.
     * @param max The maximum value.
     *
     * @return A reference to this (modified) Color4 for chaining.
     */
    Color4& clamp(Color4 min, Color4 max);
        
    /**
     * Returns a copy of this color clamped within the given range.
     *
     * Note: this does not modify this color.
     *
     * @param min The minimum value.
     * @param max The maximum value.
     *
     * @return A copy of this color clamped within the given range.
     */
    Color4 getClamp(Color4 min, Color4 max) const {
        return Color4(clampb(r,min.r,max.r), clampb(g,min.g,max.g),
                     clampb(b,min.b,max.b), clampb(a,min.a,max.a));
    }
        
    
    /**
     * Adds the given color to this one in place.
     *
     * This operation is functionally identical to additive blending.
     * The addition is clamped so that this remains a valid color.
     *
     * @param c     The color to add
     * @param alpha Whether to add the alpha values (optional)
     *
     * @return A reference to this (modified) Color4 for chaining.
     */
    Color4& add(Color4 c, bool alpha = false) {
        r = clampb(r+c.r,0,255);
        g = clampb(g+c.g,0,255);
        b = clampb(b+c.b,0,255);
        a = (alpha ? clampb(a+c.a,0,255) : a);
        return *this;
    }
        
    /**
     * Adds the given values to this color.
     *
     * The addition is clamped so that this remains a valid color.
     *
     * @param r The red color to add.
     * @param g The green color to add.
     * @param b The blue color to add.
     * @param a The alpha value (optional).
     *
     * @return A reference to this (modified) Color4 for chaining.
     */
    Color4& add(GLubyte r, GLubyte g, GLubyte b, GLubyte a = 0) {
        this->r = clampb(this->r+r,0,255);
        this->g = clampb(this->g+g,0,255);
        this->b = clampb(this->b+b,0,255);
        this->a = clampb(this->a+a,0,255);
        return *this;
    }
        
    /**
     * Subtracts the given color from this one in place.
     *
     * This operation is functionally identical to subtractive blending.
     * The subtraction is clamped so that this remains a valid color.
     *
     * @param c     The color to subtract
     * @param alpha Whether to subtract the alpha value (optional)
     *
     * @return A reference to this (modified) Color4 for chaining.
     */
    Color4& subtract(Color4 c, bool alpha = false) {
        r = clampb(r-c.r,0,255);
        g = clampb(g-c.g,0,255);
        b = clampb(b-c.b,0,255);
        a = (alpha ? clampb(a-c.a,0,255) : a);
        return *this;
    }
        
    /**
     * Subtracts the given values from this color.
     *
     * The subtraction is clamped so that this remains a valid color.
     *
     * @param r The red color to subtract.
     * @param g The green color to subtract.
     * @param b The blue color to subtract.
     * @param a The alpha value (optional).
     *
     * @return A reference to this (modified) Color4 for chaining.
     */
    Color4& subtract(GLubyte r, GLubyte g, GLubyte b, GLubyte a = 0) {
        this->r = clampb(this->r-r,0,255);
        this->g = clampb(this->g-g,0,255);
        this->b = clampb(this->b-b,0,255);
        this->a = clampb(this->a-a,0,255);
        return *this;
    }
    
    /**
     * Scales this color in place by the given factor.
     *
     * The scaling is clamped so that this remains a valid color.
     *
     * @param s     The scalar to multiply by
     * @param alpha Whether to scale the alpha value (optional)
     *
     * @return A reference to this (modified) Color4 for chaining.
     */
    Color4& scale(float s, bool alpha = false) {
        r = clampb(static_cast<GLubyte>(r*s),0,255);
        g = clampb(static_cast<GLubyte>(g*s),0,255);
        b = clampb(static_cast<GLubyte>(b*s),0,255);
        a = (alpha ? clampb(static_cast<GLubyte>(a*s),0,255) : a);
        return *this;
    }
        
    /**
     * Scales this color nonuniformly by the given factors.
     *
     * The scaling is clamped so that this remains a valid color.
     *
     * @param sr The scalar to multiply the red value
     * @param sg The scalar to multiply the green value
     * @param sb The scalar to multiply the blue value
     * @param sa The scalar to multiply the alpha value (optional)
     *
     * @return A reference to this (modified) Color4 for chaining.
     */
    Color4& scale(float sr, float sg, float sb, float sa = 1) {
        r = clampb(static_cast<GLuint>(r*sr),0,255);
        g = clampb(static_cast<GLuint>(g*sg),0,255);
        b = clampb(static_cast<GLuint>(b*sb),0,255);
        a = clampb(static_cast<GLuint>(a*sa),0,255);
        return *this;
    }
    
    /**
     * Scales this color nonuniformly by the given color.
     *
     * This operation is functionally identical to multiplicative blending.
     *
     * @param c     The color to scale by
     * @param alpha Whether to scale the alpha value (optional)
     *
     * @return A reference to this (modified) Color4 for chaining.
     */
    Color4& scale(Color4 c, bool alpha = false) {
         r = clampb(static_cast<GLuint>(COLOR_BYTE_TO_FLOAT(c.r)*r),0,255);
         g = clampb(static_cast<GLuint>(COLOR_BYTE_TO_FLOAT(c.g)*g),0,255);
         b = clampb(static_cast<GLuint>(COLOR_BYTE_TO_FLOAT(c.b)*b),0,255);
         a = (alpha ?  clampb(static_cast<GLuint>(COLOR_BYTE_TO_FLOAT(c.a)*a),0,255) : a);
         return *this;
    }

    /**
     * Maps the given function to the color values in place.
     *
     * This method supports any function that has the signature
     * Glubyte func(Glubyte).  If you wish to use float-oriented function,
     * use Color4f instead.
     *
     * @param func  The function to map on the color values.
     * @param alpha Whether to modify the alpha value (optional)
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4& map(std::function<GLubyte(GLubyte)> func, bool alpha = false) {
        r = func(r); g = func(g); b = func(b);
        a = (alpha ? func(a) : a);
        return *this;
    }
    
    /**
     * Returns a copy of this color with func applied to each component.
     *
     * This method supports any function that has the signature 
     * Glubyte func(Glubyte).  If you wish to use float-oriented function, 
     * use Color4f instead.
     *
     * @param func  The function to map on the color values.
     * @param alpha Whether to modify the alpha value (optional)
     *
     * @return A copy of this color with func applied to each component.
     */
    Color4 getMap(std::function<GLubyte(GLubyte)> func, bool alpha = false) const {
        return Color4(func(r),func(g),func(b),(alpha ? func(a) : a));
    }

    
#pragma mark Color4 Operations
    /**
     * Complements this color.
     *
     * The complement of the color is the (255-v) for each value v.
     *
     * @param alpha Whether to complement the alpha value (optional)
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4& complement(bool alpha = false) {
        r = 255-r; g = 255-g; b = 255-b; a = (alpha ? 255-a : a);
        return *this;
    }
    
    /**
     * Returns the complement of this color.
     *
     * The complement of the color is the (255-v) for each value v.
     *
     * @param alpha Whether to complement the alpha value (optional)
     *
     * Note: this does not modify this color.
     *
     * @return The complement of this color
     */
    Color4 getComplement(bool alpha = false) const {
        return Color4(255-r, 255-g, 255-b, (alpha ? 255-a : a));
    }
    
    /**
     * Modifies this color to be the linear interpolation with other.
     *
     * If alpha is 0, the color is unchanged.  If alpha is 1, the color is
     * other.  Otherwise it is a value in between.  If alpha is outside of the
     * range 0 to 1, it is clamped to the nearest value.
     *
     * This method just implements standard linear interpolation.  It does
     * not attempt to give any blending semantics to it.
     *
     * @param other The color to interpolate with.
     * @param alpha The interpolation value in 0..1
     *
     * @return This color, after the interpolation.
     */
    Color4& lerp(const Color4f& other, float alpha) {
        float x = clampf(alpha,0,1);
        *this *= (1.f - x);
        return *this += other * x;
    }
    
    /**
     * Blends this color with the other one, storing the new value in place.
     *
     * The blending is the standard over operation with this color as the
     * destination. It assumes that the color values are not premultiplied.
     *
     * @param other The color to interpolate with.
     *
     * @return This color, after the blending.
     */
    Color4& blend(Color4 other) {
        float srca = COLOR_BYTE_TO_FLOAT(other.a);
        float a1 = COLOR_BYTE_TO_FLOAT(a)*(1-srca);
        float a2 = srca+a1;
        r = clampb((GLubyte)((other.r*srca+r*a1)/a2),0,255);
        g = clampb((GLubyte)((other.g*srca+g*a1)/a2),0,255);
        b = clampb((GLubyte)((other.b*srca+b*a1)/a2),0,255);
        a = COLOR_FLOAT_TO_BYTE(a2);
        return *this;
    }
    
    /**
     * Blends this color with the other one, storing the new value in place.
     *
     * The blending is the standard over operation with this color as the
     * destination. It assumes that this color is premultiplied.
     *
     * @param other The color to interpolate with.
     *
     * @return This color, after the blending.
     */
    Color4& blendPre(Color4 other) {
        float srca = COLOR_BYTE_TO_FLOAT(other.a);
        float a1 = srca+COLOR_BYTE_TO_FLOAT(a)*(1-srca);
        r = clampb((GLubyte)(other.r+r*(1-srca)),0,255);
        g = clampb((GLubyte)(other.g+g*(1-srca)),0,255);
        b = clampb((GLubyte)(other.b+b*(1-srca)),0,255);
        a = COLOR_FLOAT_TO_BYTE(a1);
        return *this;
    }
    
    /**
     * Premultiplies this color with its current alpha.
     *
     * This class does not store whether the color is already premultiplied.
     * Hence premultiplying an already premultiplied color will have a
     * compounding effect.
     *
     * @return This color, after premultiplication.
     */
    Color4& premultiply() {
        scale(COLOR_BYTE_TO_FLOAT(a),false);
        return *this;
    }
    
    /**
     * Undoes premultiplication this color with its current alpha.
     *
     * This class does not store whether the color is already premultiplied.
     * Hence unpremultiplying an non-premultiplied color will have a
     * compounding effect.
     *
     * If the alpha value is 0, the color is unchanged.
     *
     * @return This color, after undoing premultiplication.
     */
    Color4& unpremultiply() {
        if (a > 0) {
            float a1 = 1/COLOR_BYTE_TO_FLOAT(a);
            scale(a1,false);
        }
        return *this;
    }
    
    /**
     * Returns the linear interpolation of this color with other.
     *
     * If alpha is 0, the color is unchanged.  If alpha is 1, the color is
     * other.  Otherwise it is a value in between.  If alpha is outside of the
     * range 0 to 1, it is clamped to the nearest value.
     *
     * This method just implements standard linear interpolation.  It does
     * not attempt to give any blending semantics to it.
     *
     * Note: this does not modify this color.
     *
     * @param other The color to interpolate with.
     * @param alpha The interpolation value in 0..1
     *
     * @return The linear interpolation of this color with other.
     */
    Color4 getLerp(Color4 other, float alpha) const {
        float x = clampf(alpha,0,1);
        return *this * (1.f - x) + other * x;
    }
    
    /**
     * Returns a blend of this color with the other one.
     *
     * The blending is the standard over operation with this color as the
     * destination. It assumes that the color values are not premultiplied.
     *
     * Note: this does not modify this color.
     *
     * @param other The color to interpolate with.
     *
     * @return The newly blended color
     */
    Color4 getBlend(Color4 other) const {
        float srca = COLOR_BYTE_TO_FLOAT(other.a);
        float a1 = COLOR_BYTE_TO_FLOAT(a)*(1-srca);
        float a2 = srca+a1;
        return Color4(clampb((GLubyte)((other.r*srca+r*a1)/a2), 0, 255),
                      clampb((GLubyte)((other.g*srca+g*a1)/a2), 0, 255),
                      clampb((GLubyte)((other.b*srca+b*a1)/a2), 0, 255),
                      COLOR_FLOAT_TO_BYTE(a2));
    }
    
    /**
     * Returns a blend of this color with the other one.
     *
     * The blending is the standard over operation with this color as the
     * destination. It assumes that this color is premultiplied.
     *
     * Note: this does not modify this color.
     *
     * @param other The color to interpolate with.
     *
     * @return The newly blended color
     */
    Color4f getBlendPre(Color4 other) const {
        float srca = COLOR_BYTE_TO_FLOAT(other.a);
        float dsta = COLOR_BYTE_TO_FLOAT(a);
        float a1 = srca+dsta*(1-srca);
        return Color4(clampb((GLubyte)(other.r+r*(1-srca)),0,255),
                      clampb((GLubyte)(other.g+g*(1-srca)),0,255),
                      clampb((GLubyte)(other.b+b*(1-srca)),0,255),
                      COLOR_FLOAT_TO_BYTE(a1));
    }
    
    /**
     * Returns the premultiplied version of this color, using its current alpha.
     *
     * This class does not store whether the color is already premultiplied.
     * Hence premultiplying an already premultiplied color will have a
     * compounding effect.
     *
     * Note: this does not modify this color.
     *
     * @return The newly premultiplied color
     */
    Color4 getPremultiplied() const {
        Color4 result(r,g,b,255);
        result *= COLOR_BYTE_TO_FLOAT(a);
        return result;
    }
    
    /**
     * Returns the unpremultiplied version of this color, using its current alpha.
     *
     * This class does not store whether the color is already premultiplied.
     * Hence unpremultiplying an non-premultiplied color will have a
     * compounding effect.
     *
     * If the alpha value is 0, the copy is equal to the original color.
     *
     * Note: this does not modify this color.
     *
     * @return The newly premultiplied color
     */
    Color4 getUnpremultiplied() const {
        if (a > 0) {
            float a1 = 1/COLOR_BYTE_TO_FLOAT(a);
            Color4 result(*this);
            return result.scale(a1,false);
        }
        return *this;
    }
    
    /**
     * Interpolates the two colors c1 and c2, and stores the result in dst.
     *
     * If alpha is 0, the result is c1.  If alpha is 1, the color is c2.
     * Otherwise it is a value in c1..c2.  If alpha is outside of the
     * range 0 to 1, it is clamped to the nearest value.
     *
     * This method just implements standard linear interpolation.  It does
     * not attempt to give any blending semantics to it.
     *
     * @param c1    The first color.
     * @param c2    The second color.
     * @param alpha The interpolation value in 0..1
     * @param dst   The color to store the result in
     *
     * @return A reference to dst for chaining
     */
    static Color4* lerp(Color4 c1, Color4 c2, float alpha, Color4* dst);
    
    /**
     * Blends the two colors c1 and c2, assuming they are not premultiplied.
     *
     * The blending is the standard over operation with color c1 as the source
     * and c2 as the destination. It assumes that the color values are not
     * premultiplied.
     *
     * @param c1    The source color.
     * @param c2    The destination color.
     * @param dst   The color to store the result in
     *
     * @return A reference to dst for chaining
     */
    static Color4* blend(Color4 c1, Color4 c2, Color4* dst);
    
    /**
     * Blends the two colors c1 and c2, assuming they are premultiplied.
     *
     * The blending is the standard over operation with color c1 as the source
     * and c2 as the destination. It assumes that the color values are
     * premultiplied.
     *
     * @param c1    The source color.
     * @param c2    The destination color.
     * @param dst   The color to store the result in
     *
     * @return A reference to dst for chaining
     */
    static Color4* blendPre(Color4 c1, Color4 c2, Color4* dst);
    
    /**
     * Returns the packed integer representation of this color
     *
     * This representation returned is native to the platform.  In other words,
     * it is guaranteed to be the same value set in either the constructor or
     * a setter. 
     *
     * In this representation, red will always be the highest order byte and
     * alpha will always be the lowest order byte.
     *
     * @return the packed integer representation of this color
     */
    GLuint getRGBA() const {
        return marshall(rgba);
    }
    
#pragma mark Operators
    /**
     * Adds the given color to this one in place.
     *
     * This operation is functionally identical to additive blending.
     * The addition is clamped so that this remains a valid color.
     *
     * This version of addition always adds alpha values.
     *
     * @param c The color to add
     *
     * @return A reference to this (modified) Color4 for chaining.
     */
    Color4& operator+=(Color4 c) {
        r = clampb(r+c.r,0,255);
        g = clampb(g+c.g,0,255);
        b = clampb(b+c.b,0,255);
        a = clampb(a+c.a,0,255);
        return *this;
    }
        
    /**
     * Subtracts the given color from this one in place.
     *
     * This operation is functionally identical to subtractive blending.
     * The subtraction is clamped so that this remains a valid color.
     *
     * This version of subraction always subtracts alpha values.
     *
     * @param c The color to subtract
     *
     * @return A reference to this (modified) Color4 for chaining.
     */
    Color4& operator-=(Color4 c) {
        r = clampb(r-c.r,0,255);
        g = clampb(g-c.g,0,255);
        b = clampb(b-c.b,0,255);
        a = clampb(a-c.a,0,255);
        return *this;
    }
        
    /**
     * Scales this color in place by the given factor.
     *
     * The scaling is clamped so that this remains a valid color.
     *
     * This version of scaling always multiplies alpha values.
     *
     * @param s The value to scale by
     *
     * @return A reference to this (modified) Color4 for chaining.
     */
    Color4& operator*=(float s) {
        r = clampb(static_cast<GLubyte>(r*s),0,255);
        g = clampb(static_cast<GLubyte>(g*s),0,255);
        b = clampb(static_cast<GLubyte>(b*s),0,255);
        a = clampb(static_cast<GLubyte>(a*s),0,255);
        return *this;
    }
        
    /**
     * Scales this color nonuniformly by the given color.
     *
     * This operation is functionally identical to multiplicative blending.
     *
     * @param c The color to scale by
     *
     * @return A reference to this (modified) Color4 for chaining.
     */
    Color4& operator*=(Color4 c) {
        r = clampb(static_cast<GLuint>(COLOR_BYTE_TO_FLOAT(c.r)*r),0,255);
        g = clampb(static_cast<GLuint>(COLOR_BYTE_TO_FLOAT(c.g)*g),0,255);
        b = clampb(static_cast<GLuint>(COLOR_BYTE_TO_FLOAT(c.b)*b),0,255);
        a = clampb(static_cast<GLuint>(COLOR_BYTE_TO_FLOAT(c.a)*a),0,255);
        return *this;
    }
        
    /**
     * Returns the sum of this color with the given color.
     *
     * This operation is functionally identical to additive blending.
     * The addition is clamped so that this remains a valid color.
     *
     * This version of addition always adds alpha values.
     *
     * Note: this does not modify this color.
     *
     * @param c The color to add.
     *
     * @return The sum of this color with the given color.
     */
    const Color4 operator+(Color4 c) const {
        Color4 result(*this);
        return result += c;
    }
        
    /**
     * Returns the difference of this color with the given color.
     *
     * This operation is functionally identical to subtractive blending.
     * The subtraction is clamped so that this remains a valid color.
     *
     * This version of subraction always subtracts alpha values.
     *
     * Note: this does not modify this color.
     *
     * @param c The color to subtract.
     *
     * @return The difference of this color with the given color.
     */
    const Color4 operator-(Color4 c) const  {
        Color4 result(*this);
        return result -= c;
    }
        
    /**
     * Returns the scalar product of this color with the given color.
     *
     * The scaling is clamped so that this remains a valid color.
     *
     * This version of scaling always multiplies alpha values.
     *
     * Note: this does not modify this color.
     *
     * @param s The value to scale by.
     *
     * @return The scalar product of this color with the given value.
     */
    const Color4 operator*(float s) const {
        Color4 result(*this);
        return result *= s;
    }
        
    /**
     * Returns the scalar product of this color with the given color.
     *
     * This operation is functionally identical to multiplicative blending.
     *
     * Note: this does not modify this color.
     *
     * @param c The color to scale by.
     *
     * @return The scalar product of this color with the given color.
     */
    const Color4 operator*(Color4 c) const {
        Color4 result(*this);
        return result *= c;
    }
        
        
#pragma mark Comparisons
    /**
     * Returns true if this color is less than the given color.
     *
     * This comparison uses lexicographical order of rgba.  To test if all
     * components in this color are less than those of c, use the method
     * darkerThan().
     *
     * @param c The color to compare against.
     *
     * @return True if this color is less than the given color.
     */
    bool operator<(Color4 c) const;
    
    /**
     * Returns true if this color is less than or equal the given color.
     *
     * This comparison uses lexicographical order of rgba.  To test if all
     * components in this color are less than those of c, use the method
     * darkerThan().
     *
     * @param c The color to compare against.
     *
     * @return True if this color is less than or equal the given color.
     */
    bool operator<=(Color4 c) const {
        return *this < c || rgba == c.rgba;
    }
        
    /**
     * Returns true if this color is greater than the given color.
     *
     * This comparison uses lexicographical order of rgba.  To test if all
     * components in this color are greater than those of c, use the method
     * lighterThan().
     *
     * @param c The color to compare against.
     *
     * @return True if this color is greater than the given color.
     */
    bool operator>(Color4 c) const;
    
    /**
     * Returns true if this color is greater than or equal the given color.
     *
     * This comparison uses lexicographical order of rgba.  To test if all
     * components in this color are greater than those of c, use the method
     * lighterThan().
     *
     * @param c The color to compare against.
     *
     * @return True if this color is greater than or equal the given color.
     */
    bool operator>=(Color4 c) const {
        return *this > c || rgba == c.rgba;
    }
        
    /**
     * Returns true if this color is equal to the given color.
     *
     * @param c The color to compare against.
     *
     * @return True if this color is equal to the given color.
     */
    bool operator==(Color4 c) const {
        return rgba == c.rgba;
    }
        
    /**
     * Returns true if this color is not equal to the given color.
     *
     * Comparison is exact, which may be unreliable given that the attributes
     * are floats.
     *
     * @param c The color to compare against.
     *
     * @return True if this color is not equal to the given color.
     */
    bool operator!=(Color4 c) const {
        return rgba != c.rgba;
    }
        
    /**
     * Returns true if this color is dominated by the given color.
     *
     * Domination means that all components of the given color are greater than
     * or equal to the components of this one.  However, alpha is reversed for
     * this computation, as a greater value means more opaque (and hence darker).
     *
     * @param c The color to compare against.
     *
     * @return True if this color is dominated by the given color.
     */
    bool darkerThan(Color4 c) const {
        return r <= c.r &&  g <= c.g && b <= c.b && c.a <= a;
    }
    
    /**
     * Returns true if this color dominates the given color.
     *
     * Domination means that all components of this color are greater than
     * or equal to the components of the given color. However, alpha is reversed
     * for this computation, as a lesser value means more transparent (and hence
     * lighter).
     *
     * @param c The color to compare against.
     *
     * @return True if this color is dominated by the given color.
     */
    bool lighterThan(Color4 c) const {
        return r >= c.r && g >= c.g && b >= c.b && c.a >= a;
    }

#pragma mark Conversion Methods
public:
    /**
     * Returns a string representation of this vector for debuggging purposes.
     *
     * If verbose is true, the string will include class information.  This
     * allows us to unambiguously identify the class.
     *
     * @param verbose Whether to include class information
     *
     * @return a string representation of this vector for debuggging purposes.
     */
    std::string toString(bool verbose = false) const;
    
    /** Cast from Color4f to a string. */
    operator std::string() const { return toString(); }
    
    /** 
     * Cast from Color4 to a vector.
     *
     * The attributes are all divided by 255.0.
     */
    operator Vec4() const;
    
    /**
     * Creates a color from the given vector.
     *
     * The attributes are read in the order x,y,z,w.  They are all multiplied
     * by 255.0f and rounded up before assignment.
     *
     * @param vector    The vector to convert
     */
    explicit Color4(const Vec4& vector);
    
    /**
     * Sets the coordinates of this color to those of the given vector.
     *
     * The attributes are read in the order x,y,z,w.  They are all multiplied
     * by 255.0f and rounded up before assignment.
     *
     * @param vector    The vector to convert
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4& operator= (const Vec4& vector);
    
    /** 
     * Cast from Color4f to a vector. 
     *
     * The attributes are all divided by 255.0.  The alpha value is dropped.
     */
    operator Vec3() const;
    
    /**
     * Creates a color from the given vector.
     *
     * The attributes are read in the order x,y,z. They are all multiplied
     * by 255.0f and rounded up before assignment. The alpha value is 255.
     *
     * @param vector    The vector to convert
     */
    explicit Color4(const Vec3& vector);
    
    /**
     * Sets the coordinates of this color to those of the given vector.
     *
     * The attributes are read in the order x,y,z. They are all multiplied
     * by 255.0f and rounded up before assignment. The alpha value is 255.
     *
     * @param vector    The vector to convert
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4& operator= (const Vec3& vector);
    
    /** Cast from Color4f to a float-based Color4. */
    operator Color4f() const;
    
    /**
     * Creates a byte-based color from the given float-based color.
     *
     * The attributes are read in the order r,g,b,a. They are all multiplied by
     * 255.0f and rounded up before assignment.
     *
     * @param color The color to convert
     */
    explicit Color4(const Color4f& color);
    
    /**
     * Sets the attributes of this color from the given float-based color.
     *
     * The attributes are read in the order r,g,b,a. They are all multiplied by
     * 255.0f and rounded up before assignment.
     *
     * @param color The color to convert.
     *
     * @return A reference to this (modified) Color4f for chaining.
     */
    Color4& operator= (const Color4f& color);
};

#pragma mark -
#pragma mark Friend Operations
    
/**
 * Returns the scalar product of the given color with the given value.
 *
 * The scaling is clamped so that this remains a valid color.
 *
 * This version of scaling always multiplies the alpha values.
 *
 * @param s The value to scale by.
 * @param c The color to scale.
 *
 * @return The scaled color.
 */
inline const Color4f operator*(float s, const Color4f& c) {
    Color4f result(c);
    return result *= s;
}
    
/**
 * Returns the scalar product of the given color with the given value.
 *
 * The scaling is clamped so that this remains a valid color.
 *
 * This version of scaling always multiplies the alpha values.
 *
 * @param s The value to scale by.
 * @param c The color to scale.
 *
 * @return The scaled color.
 */
inline const Color4 operator*(float s, Color4 c) {
    Color4 result(c);
    return result *= s;
}
    
}

#endif /* __CU_COLOR4_H__ */

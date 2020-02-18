//
//  CUPolynomial.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a class that represents a polynomial. It has basic
//  methods for evaluation and root finding.  The primary purpose of this class
//  is to support CubicSpline and other beziers. However, we provide it publicly
//  in case it is useful for other applications.
//
//  Because math objects are intended to be on the stack, we do not provide
//  any shared pointer support in this class.
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
//  Version: 6/20/16

#ifndef __CU_POLYNOMIAL_H__
#define __CU_POLYNOMIAL_H__

#include <cugl/math/CUMathBase.h>
#include <cugl/util/CUDebug.h>
#include <iostream>
#include <vector>

namespace cugl {

/**
 * This class represents a polynomial.
 *
 * A polynomial is a vector of floats.  This vector represents the polynomial 
 * from highest degree to constant.  For example, the vector [1, -1, 2, 0, -3] 
 * is equivalent to
 *
 *    1*x^4  - 1*x^3  + 2*x^2 + 0*x - 3
 *
 * Hence the degree of the polynomial is one less than the length of the list.
 *
 * We make all of the vector methods still available. However, note that there 
 * is some danger in using the vector methods carelessly.  In order to be 
 * well-formed, a polynomial vector must have at least one element.  Furthermore, 
 * if it has more than one element, the first element must be non-zero.  If this 
 * is not the case, there is no guarantee that any of the polynomial methods 
 * (such as root finding) will work properly.
 *
 * To avoid this problem, we have provided the isValid() and validate() methods. 
 * If you believe there is some possibility of the polynomial being corrupted, 
 * you should use these.
 */
class Polynomial : public std::vector<float>{
public:
#pragma mark Constants
    /** The zero polynomial */
    static const Polynomial ZERO;
    /** The unit polynomial */
    static const Polynomial ONE;

#pragma mark -
#pragma mark Constructors
    /**
     * Creates a zero polynomial
     */
    Polynomial() : std::vector<float>(1,0) { }
    
    /**
     * Creates the polynomial x^d where is the degree.
     *
     * The first coefficient is 1.  All other coefficients are 0.
     *
     * @param  degree   the degree of the polynomial
     */
    Polynomial(long degree) : std::vector<float>(degree+1,0)	{
        (*this)[0] = 1;
    }
    
    /**
     * Creates the polynomial x^d where is the degree.
     *
     * The value is the coefficient of all terms.  This has a chance of making 
     * an invalid polynomial (e.g. if value is 0).  However, this constructor
     * does not enforce validity. Hence it is a way to create a 0 polynomial
     * with multiple terms.
     *
     * @param  degree   the degree of the polynomial
     * @param  value    the coefficient of each term
     */
    
    Polynomial(long degree, float value) : std::vector<float>(degree+1,value) {
    }
    
    /**
     * Creates a copy of the given polynomial.
     *
     * @param  poly   the polynomial to copy
     */
    Polynomial(const Polynomial& poly) : std::vector<float>(poly) {
    }

    /**
     * Takes then resources from the given polynomial.
     *
     * @param  poly   the polynomial to take from
     */
    Polynomial(Polynomial&& poly) : std::vector<float>(poly) {
    }

    /**
     * Creates a polynomial from the given iterators.
     *
     * The elements are copied in order.  A valid iterator must have at least 
     * one element, and the first element cannot be 0 if there is more than one
     * element.
     *
     * This constructor is provided for fast copying from other vectors.
     *
     * @param  first   the beginning iterator
     * @param  last    the terminating iterator
     */
    Polynomial(const_iterator first, const_iterator last) : std::vector<float>(first,last) {
        CUAssertLog(isValid(), "The array data is invalid");
    }
    
    /**
     * Creates a polynomial from the given array.
     *
     * The elements are copied in order.  A valid array must have at least one 
     * element, and the first element cannot be 0 if there is more than one element.
     *
     * @param  array   the array of polynomial coefficients
     * @param  size    the number of elements to copy from the array
     * @param  offset  the offset position to start in the array
     */
    Polynomial(float* array, unsigned int size, unsigned int offset=0) : std::vector<float>() {
        assign(array+offset,array+size+offset);
        CUAssertLog(isValid(), "The array data is invalid");
    }
    
    /**
     * Deletes this polynomial
     */
    virtual ~Polynomial() { }
    
#pragma mark -
#pragma mark Attributes
    /**
     * Returns the degree of this polynomial.
     *
     * The degree is 1 less than the size.  We make the degree a long instead 
     * of size-type so that it is safe to use in math formulas where the degree 
     * may go negative.
     *
     * @return the degree of this polynomial.
     */
    long degree() const  { return (long)size()-1; }
    
    /**
     * Returns true if this polynomial is a constant.
     *
     * @return true if this polynomial is a constant.
     */
    bool isConstant() const  { return size() == 1; }
    
    /**
     * Returns true if the polynomial is valid.
     *
     * A valid polynomial is a vector of at least one element, and the first 
     * element cannot be 0 if there is more than one element.
     *
     * @return true if the polynomial is valid.
     */
    bool isValid() const { return size() == 1 || (size() > 1 && at(0) != 0); }
    
    /**
     * Returns true if the polynomial is the zero polynomial.
     *
     * The zero polynomial has exactly one element and the value is 0.  An 
     * invalid polynomial will return false if this method is called.
     *
     * @return true if the polynomial is the zero polynomial.
     */
    bool isZero() const  { return size() == 1 && at(0) == 0; }
    
    
#pragma mark -
#pragma mark Calculation Methods
    /**
     * Returns the derivative of this polynomial
     *
     * The derivative has degree one less than original, unless it the original 
     * has degree 1.  In that case, the derivative is 0.
     *
     * @return the derivative of this polynomial
     */
    Polynomial derivative() const;
    
    /**
     * Returns the evaluation of the polynomial on the given value.
     *
     * Evaluation plugs the value in for the polynomial variable.
     *
     * @return the evaluation of the polynomial on the given value.
     */
    float evaluate(float value) const;
    
    /**
     * Converts this polynomial into an equivalent valid polynomial.
     *
     * This method trims the zero values from the front of the vector until 
     * reaching a non-zero value, or there is only one value left.
     */
    void validate();
    
    /**
     * Converts this polynomial into the associated mononomial.
     *
     * This method divides the polynomial by the coefficient of the first term. 
     * If the polynomial is invalid, this method will fail.
     *
     * @return the coefficient divider of the original polynomial
     */
    float normalize();
    
    /**
     * Computes the roots of this polynomial using Bairstow's method
     *
     * Bairstow's method is an approximate root finding technique.  The value
     * epsilon is the error value for all of the roots.  A good description
     * of Bairstow's method can be found here:
     *
     *    http://nptel.ac.in/courses/122104019/numerical-analysis/Rathish-kumar/ratish-1/f3node9.html
     *
     * The roots are stored in the provided vector.  When complete, the vector
     * will have degree many elements.  If any root is complex, this method will
     * have added NaN in its place.
     *
     * It is possible for Bairstow's method to fail, which is why this method
     * has a return value.
     *
     * @param  roots    the vector to store the root values
     * @param  epsilon  the error tolerance for the root values
     *
     * @return true if Bairstow's method completes successfully
     */
    bool roots(vector<float>& roots, float epsilon=CU_MATH_EPSILON) const;
    
#pragma mark -
#pragma mark Setters
    /**
     * Sets this polynomial to be a copy of the one specified
     *
     * @param poly  The polynomial to copy.
     *
     * @return A reference to this (modified) Polynomial for chaining.
     */
    Polynomial& operator=(const Polynomial& poly) {
        std::vector<float>::operator=(poly);
        return *this;
    }

    /**
     * Sets this polynomial to have the resources of the one specified
     *
     * @param poly  The polynomial to take from.
     *
     * @return A reference to this (modified) Polynomial for chaining.
     */
    Polynomial& operator=(Polynomial&& poly) {
        std::vector<float>::operator=(poly);
        return *this;
    }
    
    /**
     * Sets this polynomial to be the given constant value.
     *
     * @param value The float to copy.
     *
     * @return A reference to this (modified) Polynomial for chaining.
     */
    Polynomial& operator=(float value) {
        return set(value);
    }

    /**
     * Sets the elements of this polynomial to those in the specified array.
     *
     * @param array The array to copy.
     * @param size  The number of elements in the array.
     *
     * @return A reference to this (modified) Polynomial for chaining.
     */
    Polynomial& set(float* array, int size);

    /**
     * Sets this polynomial to be the given constant value.
     *
     * @param value The float to copy.
     *
     * @return A reference to this (modified) Polynomial for chaining.
     */
    Polynomial& set(float value) {
        resize(1); at(0) = value;
        return *this;
    }

#pragma mark -
#pragma mark Comparisons
    /**
     * Returns true if this polynomial is less than the given polynomial.
     *
     * Polynomials are compared by degree, so that larger polynomials 
     * are always greater.  Then they are compared coefficient by coefficient,
     * starting at the coefficient of highest degree.
     *
     * @param p The polynomial to compare against.
     *
     * @return True if this polynomial is less than the given polynomial.
     */
    bool operator<(const Polynomial& p) const;

    /**
     * Returns true if this polynomial is less than the given float.
     *
     * Polynomials are compared by degree, so that larger polynomials
     * are always greater.  Then they are compared coefficient by coefficient,
     * starting at the coefficient of highest degree.
     *
     * @param value The float to compare against.
     *
     * @return True if this polynomial is less than the given float.
     */
    bool operator<(float value) const {
        return size() == 1 && at(0) < value;
    }
    
    /**
     * Returns true if this polynomial is less than or equal the given polynomial.
     *
     * This comparison uses the lexicographical order.  To test if all
     * components in this vector are less than those of v, use the method
     * under().
     *
     * @param p The polynomial to compare against.
     *
     * @return True if polynomial vector is less than or equal the given polynomial.
     */
    bool operator<=(const Polynomial& p) const;
    
    /**
     * Returns true if this polynomial is less than or equal the given float.
     *
     * Polynomials are compared by degree, so that larger polynomials
     * are always greater.  Then they are compared coefficient by coefficient,
     * starting at the coefficient of highest degree.
     *
     * @param value The float to compare against.
     *
     * @return True if this polynomial is less than or equal the given float.
     */
    bool operator<=(float value) const {
        return size() == 1 && at(0) <= value;
    }
    
    /**
     * Determines if this polynomial is greater than the given polynomial.
     *
     * This comparison uses the lexicographical order.  To test if all
     * components in this vector are greater than those of v, use the method
     * over().
     *
     * @param p The polynomial to compare against.
     *
     * @return True if this polynomial is greater than the given polynomial.
     */
    bool operator>(const Polynomial& p) const;
    
    /**
     * Returns true if this polynomial is greater than the given float.
     *
     * Polynomials are compared by degree, so that larger polynomials
     * are always greater.  Then they are compared coefficient by coefficient,
     * starting at the coefficient of highest degree.
     *
     * @param value The float to compare against.
     *
     * @return True if this polynomial is greater than the given float.
     */
    bool operator>(float value) const {
        return size() > 1 || at(0) > value;
    }
    
    /**
     * Determines if this polynomial is greater than or equal the given polynomial.
     *
     * This comparison uses the lexicographical order.  To test if all
     * components in this vector are greater than those of v, use the method
     * over().
     *
     * @param p The polynomial to compare against.
     *
     * @return True if this polynomial is greater than or equal the given polynomial.
     */
    bool operator>=(const Polynomial& p) const;
    
    /**
     * Returns true if this polynomial is greater than or equal the given float.
     *
     * Polynomials are compared by degree, so that larger polynomials
     * are always greater.  Then they are compared coefficient by coefficient,
     * starting at the coefficient of highest degree.
     *
     * @param value The float to compare against.
     *
     * @return True if this polynomial is greater than or equal the given float.
     */
    bool operator>=(float value) const {
        return size() > 1 || at(0) >= value;
    }
    
    /**
     * Returns true if this polynomial is a constant equal to this float.
     *
     * @param value The float to compare
     *
     * @return true if this polynomial is a constant equal to this float.
     */
    bool operator==(float value) const {
        return size() == 1 && at(0) == value;
    }
    
    /**
     * Returns true if this polynomial is nonconstant or not equal to this float.
     *
     * @param value The float to compare
     *
     * @return true if this polynomial is nonconstant or not equal to this float.
     */
    bool operator!=(float value) const {
        return size() > 1 || at(0) != value;
    }
    
#pragma mark -
#pragma mark Operators
    /**
     * Adds the given polynomial in place.
     *
     * @param  other    The polynomial to add
     *
     * @return This polynomial, modified.
     */
    Polynomial& operator+=(const Polynomial& other);
    
    /**
     * Subtracts this polynomial by the given polynomial in place.
     *
     * @param  other    The polynomial to subtract
     *
     * @return This polynomial, modified.
     */
    Polynomial& operator-=(const Polynomial& other);
    
    /**
     * Multiplies the given polynomial in place.
     *
     * @param  other    The polynomial to multiply
     *
     * @return This polynomial, modified.
     */
    Polynomial& operator*=(const Polynomial& other) {
        return *this = (*this)*other;
    }
    
    /**
     * Divides this polynomial by the given polynomial in place.
     *
     * If other is not valid, then this method will fail.
     *
     * @param  other    The polynomial to divide by
     *
     * @return This polynomial, modified.
     */
    Polynomial& operator/=(const Polynomial& other);
    
    /**
     * Assigns this polynomial the division remainder of the other polynomial.
     *
     * If other is not valid, then this method will fail.
     *
     * @param  other    The polynomial to divide by
     *
     * @return This polynomial, modified.
     */
    Polynomial& operator%=(const Polynomial& other);
    
    /**
     * Returns the sum of this polynomial and other.
     *
     * @param  other    The polynomial to add
     *
     * @return the sum of this polynomial and other.
     */
    Polynomial operator+(const Polynomial& other) const {
        return Polynomial(*this) += other;
    }
    
    /**
     * Returns the result of subtracting other from this.
     *
     * @param  other    The polynomial to subtract
     *
     * @return the result of subtracting other from this.
     */
    Polynomial operator-(const Polynomial& other) const {
        return Polynomial(*this) -= other;
    }
    
    /**
     * Returns the product of this polynomial and other.
     *
     * @param  other    The polynomial to multiply
     *
     * @return the product of this polynomial and other.
     */
    Polynomial operator*(const Polynomial& other) const;
    
    /**
     * Returns the result of dividing this polynomial by other.
     *
     * @param  other    The polynomial to divide by
     *
     * @return the result of dividing this polynomial by other.
     */
    Polynomial operator/(const Polynomial& other) const {
        return Polynomial(*this) /= other;
    }
    
    /**
     * Returns the remainder when dividing this polynomial by other.
     *
     * @param  other    The polynomial to divide by
     *
     * @return the remainder when dividing this polynomial by other.
     */
    Polynomial operator%(const Polynomial& other) const {
        return Polynomial(*this) %= other;
    }
    
    /**
     * Adds the given constant in place.
     *
     * @param  value    The value to add
     *
     * @return This polynomial, modified.
     */
    Polynomial& operator+=(float value);
    
    /**
     * Subtracts this polynomial by the given value in place.
     *
     * @param  value    The value to subtract
     *
     * @return This polynomial, modified.
     */
    Polynomial& operator-=(float value);
    
    /**
     * Multiplies the given value in place.
     *
     * @param  value    The value to multiply
     *
     * @return This polynomial, modified.
     */
    Polynomial& operator*=(float value);
    
    /**
     * Divides this polynomial by the given value in place.
     *
     * If value is zero, then this method will fail.
     *
     * @param  value    The value to divide by
     *
     * @return This polynomial, modified.
     */
    Polynomial& operator/=(float value);
    
    /**
     * Assigns this polynomial the division remainder of value.
     *
     * If value is zero, then this method will fail.
     *
     * @param  value    The value to divide by
     *
     * @return This polynomial, modified.
     */
    Polynomial& operator%=(float value);
    
    /**
     * Returns the sum of this polynomial and value.
     *
     * @param  value    The value to add
     *
     * @return the sum of this polynomial and value.
     */
    Polynomial operator+(float value) const {
        return Polynomial(*this) += value;
    }
    
    /**
     * Returns the result of subtracting value from this.
     *
     * @param  value    The value to subtract
     *
     * @return the result of subtracting value from this.
     */
    Polynomial operator-(float value) const {
        return Polynomial(*this) -= value;
    }
    
    /**
     * Returns the product of this polynomial and value.
     *
     * @param  value    The value to multiply
     *
     * @return the product of this polynomial and value.
     */
    Polynomial operator*(float value) const {
        return Polynomial(*this) *= value;
    }
    
    /**
     * Returns the result of dividing this polynomial by value.
     *
     * @param  value    The value to divide by
     *
     * @return the result of dividing this polynomial by value.
     */
    Polynomial operator/(float value) const {
        return Polynomial(*this) /= value;
    }
    
    /**
     * Returns the remainder when dividing this polynomial by value.
     *
     * @param  value    The value to divide by
     *
     * @return the remainder when dividing this polynomial by value.
     */
    Polynomial operator%(float value) const {
        return Polynomial(*this) %= value;
    }

    /**
     * Returns the negation of this polynomial.
     *
     * @return  the negation of this polynomial.
     */
    Polynomial operator-() const;

#pragma mark -
#pragma mark Friend Functions
    /**
     * Returns the sum of the polynomial and value.
     *
     * @param  left    The value to add
     * @param  right   The polynomial
     *
     * @return the sum of the polynomial and value.
     */
    friend Polynomial operator+(float left, const Polynomial& right);
    
    /**
     * Returns the result of subtracting the polynomial from value.
     *
     * @param  left    The initial value
     * @param  right   The polynomial to subtract
     *
     * @return the result of subtracting the polynomial from value.
     */
    friend Polynomial operator-(float left, const Polynomial& right);
    
    /**
     * Returns the product of the polynomial and value.
     *
     * @param  left    The value to multiply
     * @param  right   The polynomial
     *
     * @return the product of the polynomial and value.
     */
    friend Polynomial operator*(float left, const Polynomial& right);
    
    /**
     * Returns the result of dividing value by the polynomial.
     *
     * The result will always be 0, unless the polynomial is a
     * constant.
     *
     * @param  left    The initial value
     * @param  right   The polynomial to divide by
     *
     * @return the result of dividing value by the polynomial.
     */
    friend Polynomial operator/(float left, const Polynomial& right);
    
    /**
     * Returns the remainder when dividing value by the polynomial.
     *
     * The value will be the polynomial unless the polynomial is constant.
     *
     * @param  left    The initial value
     * @param  right   The polynomial to divide by
     *
     * @return the remainder when dividing value by the polynomial.
     */
    friend Polynomial operator%(float left, const Polynomial& right);
    
    /**
     * Returns true if the float is less than the given polynomial.
     *
     * Polynomials are compared by degree, so that larger polynomials
     * are always greater.  Then they are compared coefficient by coefficient,
     * starting at the coefficient of highest degree.
     *
     * @param left    The float to compare against
     * @param right   The polynomial to compare
     *
     * @return True if the float is less than the given polynomial.
     */
    friend bool operator<(float left, const Polynomial& right) {
        return right.size() > 1 || right[0] > left;
    }

    /**
     * Returns true if the float is less than or equal the given polynomial.
     *
     * Polynomials are compared by degree, so that larger polynomials
     * are always greater.  Then they are compared coefficient by coefficient,
     * starting at the coefficient of highest degree.
     *
     * @param left    The float to compare against
     * @param right   The polynomial to compare
     *
     * @return True if the float is less than or equal the given polynomial.
     */
    friend bool operator<=(float left, const Polynomial& right) {
        return right.size() > 1 || right[0] >= left;
    }
    
    /**
     * Returns true if the float is greater than the given polynomial.
     *
     * Polynomials are compared by degree, so that larger polynomials
     * are always greater.  Then they are compared coefficient by coefficient,
     * starting at the coefficient of highest degree.
     *
     * @param left    The float to compare against
     * @param right   The polynomial to compare
     *
     * @return True if the float is greater than the given polynomial.
     */
    friend bool operator>(float left, const Polynomial& right) {
        return right.size() == 1 && right[0] < left;
    }
    
    /**
     * Returns true if the float is greater than or equal the given polynomial.
     *
     * Polynomials are compared by degree, so that larger polynomials
     * are always greater.  Then they are compared coefficient by coefficient,
     * starting at the coefficient of highest degree.
     *
     * @param left    The float to compare against
     * @param right   The polynomial to compare
     *
     * @return True if the float is greater than or equal the given polynomial.
     */
    friend bool operator>=(float left, const Polynomial& right) {
        return right.size() == 1 && right[0] <= left;
    }

    
#pragma mark -
#pragma mark Conversion Methods
    /**
     * Returns a string representation of this polynomial.
     *
     * There are two ways to represent a polynomial. One is in polynomial form, like
     *
     *    x^4 - x^3 + 2x^2 - 3
     *
     * Alternatively, we could represent the same polynomial as its vector contents
     * [1, -1, 2, 0, -3].  This is the purpose of the optional parameter.
     *
     * @param  format   whether to format as a polynomial
     *
     * @return a string representation of this polynomial
     */
    std::string toString(bool format=true) const;
    
    /** Cast from a Polynomial to a string. */
    operator std::string() const { return toString(); }

    
#pragma mark -
#pragma mark Internal Helpers
protected:
    /**
     * Returns the product of polynomials a and b.
     *
     * This method multiplies the two polynomials with a nested for-loop. It
     * is O(nm) where n is the degree of a and m the degree of b.  It is,
     * however, faster on small polynomials.
     *
     * @param  a    The first polynomial to muliply
     * @param  b    The second polynomial to muliply
     *
     * @return the product of polynomials a and b
     */
    static Polynomial iterative_multiply(const Polynomial& a, const Polynomial& b);
    
    /**
     * Returns the product of polynomials a and b.
     *
     * This method multiplies the two polynomials with recursively using a
     * divide-and-conquer algorithm. The algorithm is described here:
     *
     *  http://algorithm.cs.nthu.edu.tw/~course/Extra_Info/Divide%20and%20Conquer_supplement.pdf
     *
     * This algorithm is θ(n) where n is the maximum degree of a and b.  It is, 
     * however, slower on small polynomials.
     *
     * @param  a    The first polynomial to muliply
     * @param  b    The second polynomial to muliply
     *
     * @return the product of polynomials a and b
     */
    static Polynomial recursive_multiply(const Polynomial& a, const Polynomial& b);
    
    /**
     * Returns the synthetic division of this polynomial by other
     *
     * This method is adopted from the python code provided at
     *
     *   https://en.wikipedia.org/wiki/Synthetic_division
     *
     * Synthetic division preserves the length of the vector.  The beginning is
     * the result, and the tail is the remainder.  This value must be broken up 
     * to implement the / and % operators.  However, some algorithms (like 
     * Bairstow's method) prefer this method just the way it is.
     *
     * @param  other    the polynomial to divide by
     *
     * @return the synthetic division of this polynomial by other
     */
    Polynomial& synthetic_divide(const Polynomial& other);
    
    /**
     * Uses Bairstow's method to find a quadratic polynomial dividing this one.
     *
     * Bairstow's method iteratively divides this polynomial by quadratic factors,
     * until it finds one that divides it within epsilon.  This method can fail
     * (takes to many iterations; the Jacobian is singular), hence the return value.
     * For more information, see
     *
     *    http://nptel.ac.in/courses/122104019/numerical-analysis/Rathish-kumar/ratish-1/f3node9.html
     *
     * When calling this method, quad must be provided as an initial guess, while
     * result can be empty.  This method will modify both quad and result. quad
     * is the final quadratic divider. result is the result of the division.
     *
     * @param  quad     the final quadratic divisor chosen
     * @param  result   the result of the final division
     * @param  epsilon  the error tolerance for quad
     *
     * @return true if Bairstow's method completes successfully
     */
    bool bairstow_factor(Polynomial& quad, Polynomial& result, float epsilon) const;
    
    /**
     * Solve for the roots of this polynomial with the quadratic formula.
     *
     * Obviously, this method will fail if the polynomial is not quadratic. The 
     * roots are added to the provided vector (the original contents are not 
     * erased). If any root is complex, this method will have added NaN in its 
     * place.
     *
     * @param  roots    the vector to store the root values
     */
    void solve_quadratic(vector<float>& roots) const;
};

}

#endif /* __CU_POLYNOMIAL_H__ */

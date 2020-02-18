//
//  CUPolynomial.cpp
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

#include <cugl/math/CUPolynomial.h>
#include <algorithm>
#include <functional>
#include <sstream>

/** Whether or not a value is in the range epsilon */
#define INRANGE(x,epsilon)	(x < epsilon && -x < epsilon)

/** Maximum number of iterations for Bairstow's method */
#define MAX_ITERATIONS	50

/** Maximum number of attempts to find a root */
#define MAX_ATTEMPTS    10

/** Minimum polynomial degree to switch to recursive multiplication */
#define MULT_THRESHOLD  5

using namespace std;
using namespace cugl;


#pragma mark -
#pragma mark Calculation Methods

/**
 * Returns the derivative of this polynomial
 *
 * The derivative has degree one less than original, unless it the original has
 * degree 1.  In that case, the derivative is 0.
 *
 * @return the derivative of this polynomial
 */
Polynomial Polynomial::derivative() const {
    Polynomial result(*this);
    for(int ii = 0; ii < size(); ii++) {
        result[ii] *= size()-ii-1;
    }
    result.pop_back();
    if (result.size() == 0) {
        result.push_back(0);
    }
    return result;
}

/**
 * Returns the evaluation of the polynomial on the given value.
 *
 * Evaluation plugs the value in for the polynomial variable.
 *
 * @return the evaluation of the polynomial on the given value.
 */
float Polynomial::evaluate(float value) const {
    float accum = at(0);
    for (auto it = begin()+1; it != end(); ++it) {
        accum = accum*value+(*it);
    }
    return accum;
}

/**
 * Converts this polynomial into an equivalent valid polynomial.
 *
 * This method trims the zero values from the front of the vector until reaching
 * a non-zero value, or there is only one value left.
 */
void Polynomial::validate() {
    if (size() == 0) {
        push_back(0);
        return;
    } else if (at(0) != 0) {
        return;
    }
    
    int offset = 0;
    while (at(offset) == 0 && offset < degree()) {
        offset++;
    }
    copy(begin()+offset, end(), begin());
    resize(size()-offset);
}

/**
 * Converts this polynomial into the associated mononomial.
 *
 * This method divides the polynomial by the coefficient of the first term. If
 * the polynomial is invalid, this method will fail.
 *
 * @return the coefficient divider of the original polynomial
 */
float Polynomial::normalize() {
    CUAssertLog(at(0), "Cannot normalize with leading zero");
    float coeff = at(0);
    *this /= coeff;
    return coeff;
}

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
bool Polynomial::roots(vector<float>& roots, float epsilon) const {
    Polynomial result1(*this);
    Polynomial quad(2);
    Polynomial result2;
    
    // Remove the x's
    while (result1.back() == 0) {
        roots.push_back(0.0f);
        result1.pop_back();
    }
    
    long degree = result1.degree();
    
    int attempts = 0;
    while (degree > 2 && attempts <= MAX_ATTEMPTS) {
        float a = (float)rand()/RAND_MAX;
        float b = (float)rand()/RAND_MAX;
        quad[1] = -a-b;
        quad[2] = a*b;
        if (result1.bairstow_factor(quad,result2,epsilon)) {
            quad.solve_quadratic(roots);
            degree -= 2; attempts = 0;
            result1 = result2;
        } else {
            attempts++;
        }
    }
    
    if (attempts > MAX_ATTEMPTS) {
        return false;
    }
    
    if (degree == 2) {
        result1.solve_quadratic(roots);
    } else if (degree == 1) {
        roots.push_back(-result1[1]);
    }
    return true;
}

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
string Polynomial::toString(bool format) const {
    std::stringstream ss;
    if (format) {
        int leng = (int)size();
        for(int ii = 0; ii < leng; ii++) {
            if ((*this)[ii] != 0 || ii == leng-1) {
                if ((*this)[ii] > 0 && ii > 0) {
                    ss << "+";
                }
                if (((*this)[ii] != 1 && ii < leng-1) ||
                    (ii == leng-1 && (*this)[ii] != 0) || leng == 1) {
                    ss << (*this)[ii];
                }
                if (ii < leng-2) {
                    ss << "x^" << (leng-ii-1);
                } else if (ii < leng-1) {
                    ss << "x";
                }
            }
        }
    } else {
        ss << "[";
        for(int ii = 0; ii < size(); ii++) {
            ss << (*this)[ii];
            if (ii != size()-1) {
                ss << ",";
            }
        }
        ss << "]";
    }
    return ss.str();
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
 * @param v The polynomial to compare against.
 *
 * @return True if this polynomial is less than the given polynomial.
 */
bool Polynomial::operator<(const Polynomial& p) const {
    if (p.size() != size()) {
        return size() < p.size();
    }
    for(int ii = 0; ii < size(); ii++) {
        if (at(ii) != p[ii]) {
            return (at(ii) < p[ii]);
        }
    }
    return false;
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
bool Polynomial::operator<=(const Polynomial& p) const {
    if (p.size() != size()) {
        return size() < p.size();
    }
    for(int ii = 0; ii < size(); ii++) {
        if (at(ii) != p[ii]) {
            return (at(ii) < p[ii]);
        }
    }
    return true;
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
bool Polynomial::operator>(const Polynomial& p) const {
    if (p.size() != size()) {
        return size() > p.size();
    }
    for(int ii = 0; ii < size(); ii++) {
        if (at(ii) != p[ii]) {
            return (at(ii) > p[ii]);
        }
    }
    return false;
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
bool Polynomial::operator>=(const Polynomial& p) const {
    if (p.size() != size()) {
        return size() > p.size();
    }
    for(int ii = 0; ii < size(); ii++) {
        if (at(ii) != p[ii]) {
            return (at(ii) > p[ii]);
        }
    }
    return true;
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
Polynomial& Polynomial::operator+=(const Polynomial& other) {
    int origsz = (int)size();
    int diffsz = (int)size()-(int)other.size();
    for(int ii=origsz-1; ii-diffsz >= 0 && ii >= 0; ii--) {
        (*this)[ii] += other[ii-diffsz];
    }
    if (diffsz < 0) {
        int end = (int)other.size()-(int)size();
        insert(begin(),other.begin(),other.begin()+end);
    }
    if (!isValid()) {
        validate();
    }
    return *this;
}

/**
 * Subtracts this polynomial by the given polynomial in place.
 *
 * @param  other    The polynomial to subtract
 *
 * @return This polynomial, modified.
 */
Polynomial& Polynomial::operator-=(const Polynomial& other) {
    int origsz = (int)size();
    int diffsz = (int)size()-(int)other.size();
    for(int ii=origsz-1; ii-diffsz >= 0 && ii >= 0; ii--) {
        (*this)[ii] -= other[ii-diffsz];
    }
    if (diffsz < 0) {
        int end = (int)other.size()-(int)size();
        insert(begin(),other.begin(),other.begin()+end);
        for(int ii = 0; ii < end; ii++) {
            at(ii) = -at(ii);
        }
    }
    if (!isValid()) {
        validate();
    }
    return *this;
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
Polynomial& Polynomial::operator/=(const Polynomial& other) {
    synthetic_divide(other);
    long osize = other.degree();
    resize((osize > size() ? 1 : size()-osize));
    return *this;
}

/**
 * Assigns this polynomial the division remainder of the other polynomial.
 *
 * If other is not valid, then this method will fail.
 *
 * @param  other    The polynomial to divide by
 *
 * @return This polynomial, modified.
 */
Polynomial& Polynomial::operator%=(const Polynomial& other) {
    synthetic_divide(other);
    long osize  = other.degree();
    long offset = (long)(osize > size() ? 1 : size()-osize);
    while (at(offset) == 0 && offset < degree()) {
        offset++;
    }
    copy(this->begin()+offset, this->end(), this->begin());
    resize(size()-offset);
    return *this;
}

/**
 * Returns the product of this polynomial and other.
 *
 * @param  other    The polynomial to multiply
 *
 * @return the product of this polynomial and other.
 */
Polynomial Polynomial::operator*(const Polynomial& other) const {
    if (degree() > MULT_THRESHOLD && other.degree() > MULT_THRESHOLD) {
        return recursive_multiply(*this,other);
    }
    return iterative_multiply(*this,other);
}

/**
 * Adds the given constant in place.
 *
 * @param  value    The value to add
 *
 * @return This polynomial, modified.
 */
Polynomial& Polynomial::operator+=(float value) {
    this->back() += value;
    return *this;
}

/**
 * Subtracts this polynomial by the given value in place.
 *
 * @param  value    The value to subtract
 *
 * @return This polynomial, modified.
 */
Polynomial& Polynomial::operator-=(float value) {
    this->back() -= value;
    return *this;
}

/**
 * Multiplies the given value in place.
 *
 * @param  value    The value to multiply
 *
 * @return This polynomial, modified.
 */
Polynomial& Polynomial::operator*=(float value) {
    if (value == 0) {
        clear();
        push_back(0);
    } else {
        std::transform(begin(), end(), begin(),
                       std::bind(std::multiplies<float>(),value,std::placeholders::_1));
    }
    return *this;
}

/**
 * Divides this polynomial by the given value in place.
 *
 * If value is zero, then this method will fail.
 *
 * @param  value    The value to divide by
 *
 * @return This polynomial, modified.
 */
Polynomial& Polynomial::operator/=(float value) {
    CUAssertLog(value != 0, "Zero division error");
    std::transform(begin(), end(), begin(),
                   std::bind(std::divides<float>(),value,std::placeholders::_1));
    return *this;
}

/**
 * Assigns this polynomial the division remainder of value.
 *
 * If value is zero, then this method will fail.
 *
 * @param  value    The value to divide by
 *
 * @return This polynomial, modified.
 */
Polynomial& Polynomial::operator%=(float value) {
    CUAssertLog(value != 0, "Zero division error");
    clear();
    push_back(0);
    return *this;
}

/**
 * Returns the negation of this polynomial.
 *
 * @return  the negation of this polynomial.
 */
Polynomial Polynomial::operator-() const {
    Polynomial result;
    result[0] = -at(0);
    for(int ii = 1; ii < size(); ii++) {
        result.push_back(-at(ii));
    }
    return result;
}

#pragma mark -
#pragma mark Operator Helpers
/**
 * Sets the elements of this vector to those in the specified array.
 *
 * @param array The array to copy.
 * @param size  The number of elements in the array.
 *
 * @return A reference to this (modified) Polynomial for chaining.
 */
Polynomial& Polynomial::set(float* array, int size) {
    resize(size,0.0f);
    for(int ii = 0; ii < size; ii++) {
        at(ii) = array[ii];
    }
    return *this;
}

/**
 * Returns the product of polynomials a and b.
 *
 * This method multiplies the two polynomials with a nested for-loop. It
 * is O(nm) where n is the degree of a and m the degree of b.  It is,
 * however, faster on small polynomials.
 *
 * @param  s    The first polynomial to muliply
 * @param  b    The second polynomial to muliply
 *
 * @return the product of polynomials a and b
 */
Polynomial Polynomial::iterative_multiply(const Polynomial& a, const Polynomial& b) {
    Polynomial result(a.degree()+b.degree(),0);
    for(int ii = 0; ii < b.size(); ii++) {
        for(int jj = 0; jj < a.size(); jj++) {
            result[ii+jj] += a[jj]*b[ii];
        }
    }
    return result;
}

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
 * @param  s    The first polynomial to muliply
 * @param  b    The second polynomial to muliply
 *
 * @return the product of polynomials a and b
 */
Polynomial Polynomial::recursive_multiply(const Polynomial& a, const Polynomial& b) {
    if (a.size() == 1) {
        Polynomial result(b.degree());
        std::transform(b.begin(), b.end(), result.begin(),
                       std::bind(std::multiplies<float>(),a[0],std::placeholders::_1));
        return result;
    } else if (b.size() == 1) {
        Polynomial result(a.degree());
        std::transform(a.begin(), a.end(), result.begin(),
                       std::bind(std::multiplies<float>(),b[0],std::placeholders::_1));
        return result;
    }
    
    long an = (long)a.size()/2;
    long bn = (long)b.size()/2;
    Polynomial a0(a.begin(),a.begin()+an);
    Polynomial a1(a.begin()+an,a.end());
    Polynomial b0(b.begin(),b.begin()+bn);
    Polynomial b1(b.begin()+bn,b.end());
    
    Polynomial p1 = recursive_multiply(a0,b0);
    p1.resize(a1.size()+b1.size()+p1.size(),0);
    
    Polynomial p2 = recursive_multiply(a1,b0);
    for(int ii = 1; ii <= p2.size(); ii++) {
        p1[p1.size()-ii-b1.size()] += p2[p2.size()-ii];
    }
    
    p2 = recursive_multiply(a0,b1);
    for(int ii = 1; ii <= p2.size(); ii++) {
        p1[p1.size()-ii-a1.size()] += p2[p2.size()-ii];
    }
    
    p2 = recursive_multiply(a1,b1);
    for(int ii = 1; ii <= p2.size(); ii++) {
        p1[p1.size()-ii] += p2[p2.size()-ii];
    }
    
    return p1;
}

/**
 * Returns the synthetic division of this polynomial by other
 *
 * This method is adopted from the python code provided at
 *
 *   https://en.wikipedia.org/wiki/Synthetic_division
 *
 * Synthetic division preserves the length of the vector.  The beginning is the
 * result, and the tail is the remainder.  This value must be broken up to
 * implement the / and % operators.  However, some algorithms (like Bairstow's 
 * method) prefer this method just the way it is.
 *
 * @param  other    the polynomial to divide by
 *
 * @return the synthetic division of this polynomial by other
 */
Polynomial& Polynomial::synthetic_divide(const Polynomial& other) {
    CUAssertLog(other.isValid(), "Division by invalid polynomial");
    CUAssertLog(!other.isZero(), "Division by zero polynomial");
    // Error leng 0 or first element 0.
    if (other.size() > size()) {
        this->insert(this->begin(),0);
        return *this;
    }
    
    float normalizer = other[0];
    int cols = (int)size()-(int)other.size()+1;
    for(int ii = 0; ii < cols; ii++) {
        // Normalize the divisor for synthetic division
        at(ii) /= normalizer;
        float coef = at(ii);
        if (coef != 0) {	// useless to multiply if coef is 0
            for(int jj = 1; jj < other.size(); jj++) {
                (*this)[ii + jj] += -other[jj] * coef;
            }
        }
    }
    return *this;
}


#pragma mark -
#pragma mark Root Helpers

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
 * result can be empty.  This method will modify both quad and result. quad is
 * the final quadratic divider. result is the result of the division.
 *
 * @param  quad     the final quadratic divisor chosen
 * @param  result   the result of the final division
 * @param  epsilon  the error tolerance for quad
 *
 * @return true if Bairstow's method completes successfully
 */
bool Polynomial::bairstow_factor(Polynomial& quad, Polynomial& result, float epsilon) const {
    // Assert plength > 2
    Polynomial temp;
    
    float dr = 2*epsilon;
    float ds = 2*epsilon;
    for(int ii = 0; ii < MAX_ITERATIONS; ii++) {
        result = *this;
        result.synthetic_divide(quad);  // Leave on remainder for algorithm
        
        temp = result;
        temp.synthetic_divide(quad);
        
        float b1 = result[result.size()-2];
        float b0 = result[result.size()-1]-quad[1]*b1;
        
        float c1 = temp[temp.size()-2];
        float c2 = temp[temp.size()-3];
        float c3 = (temp.size() > 3 ? temp[temp.size()-4] : 0.0f);
        
        float det = c3*c1-c2*c2;
        if (b0 == 0 && b1 == 0) {
            dr = 0; ds = 0;
        } else if (det != 0) {
            dr = (b1*c2-b0*c3)/det;
            ds = (b0*c2-b1*c1)/det;
        }
        
        float rerr = 100*dr/quad[1];
        float serr = 100*ds/quad[2];
        
        if ((INRANGE(rerr,epsilon) && INRANGE(serr,epsilon)) || det == 0) {
            break;
        }
        quad[1] -= dr;
        quad[2] -= ds;
    }
    
    // Cut off the remainder now.
    result.resize(result.size()-2);
    if (INRANGE(dr,epsilon) && INRANGE(ds,epsilon)) {
        return true;
    }
    return false;
}

/**
 * Solve for the roots of this polynomial with the quadratic formula.
 *
 * Obviously, this method will fail if the polynomial is not quadratic. The 
 * roots are added to the provided vector (the original contents are not erased). 
 * If any root is complex, this method will have added NaN in its place.
 *
 * @param  roots    the vector to store the root values
 */
void Polynomial::solve_quadratic(vector<float>& roots) const {
    CUAssertLog(degree() == 2, "Polynomial is not quadratic");
    // Assert degree 2
    float first = at(0);
    float secnd = at(1);
    float det = secnd*secnd-4*first*back();
    if (det < 0) {
        roots.push_back(nanf(""));
        roots.push_back(nanf(""));
    } else {
        det = (float)sqrt(det);
        roots.push_back((-secnd+det)/(2.0f*first));
        roots.push_back((-secnd-det)/(2.0f*first));
    }
}


#pragma mark -
#pragma mark Friend Functions
namespace cugl {
/**
 * Returns the sum of the polynomial and value.
 *
 * @param  left    The value to add
 * @param  right   The polynomial
 *
 * @return the sum of the polynomial and value.
 */
Polynomial operator+(float left, const Polynomial& right) {
    return right+left;
}

/**
 * Returns the result of subtracting the polynomial from value.
 *
 * @param  left    The initial value
 * @param  right   The polynomial to subtract
 *
 * @return the result of subtracting the polynomial from value.
 */
Polynomial operator-(float left, const Polynomial& right) {
    Polynomial result(right.degree());
    std::transform(right.begin(), right.end(), result.begin(),
                   std::bind(std::multiplies<float>(),-1,std::placeholders::_1));
    result.back() += left;
    return result;
}

/**
 * Returns the product of the polynomial and value.
 *
 * @param  left    The value to multiply
 * @param  right   The polynomial
 *
 * @return the product of the polynomial and value.
 */
Polynomial operator*(float left, const Polynomial& right) {
    return right*left;
}

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
Polynomial operator/(float left, const Polynomial& right) {
    CUAssertLog(right.isValid(), "Division by invalid polynomial");
    CUAssertLog(!right.isZero(), "Division by zero polynomial");
    Polynomial poly;
    if (right.size() == 1) {
        poly[0] = left/right.back();
    }
    return poly;
}

/**
 * Returns the remainder when dividing value by the polynomial.
 *
 * The value will be the float unless the polynomial is constant.
 *
 * @param  left    The initial value
 * @param  right   The polynomial to divide by
 *
 * @return the remainder when dividing value by the polynomial.
 */
Polynomial operator%(float left, const Polynomial& right) {
    CUAssertLog(right.isValid(), "Division by invalid polynomial");
    CUAssertLog(!right.isZero(), "Division by zero polynomial");
    Polynomial poly;
    if (right.size() > 1) {
        poly[0] = left;
    }
    return poly;
}

}

#pragma mark -
#pragma mark Constants
/** The zero polynomial */
const Polynomial Polynomial::ZERO;
/** The unit polynomial */
const Polynomial Polynomial::ONE(0,1.0f);

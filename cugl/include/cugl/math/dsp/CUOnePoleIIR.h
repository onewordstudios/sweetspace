//
//  CUOnePoleIIR.h
//  Cornell University Game Library (CUGL)
//
//  This class is represents a one-pole IIR filter. This is the standard class
//  for implementing first order lowpass filters.  For first-order filters,
//  it is significantly more performant than IIRFilter.
//
//  This class supports vector optimizations for SSE and Neon 64.  In timed
//  simulations, these optimizations provide at least a 3-4x performance
//  increase (and in isolated cases, much higher). These optimizations make
//  use of the matrix precomputation outlined in "Implementation of Recursive
//  Digital Filters into Vector SIMD DSP Architectures"
//
//    https://pdfs.semanticscholar.org/d150/a3f75dc033916f14029cd9101a8ea1d050bb.pdf
//
//  The algorithm in this paper performs extremely well in our tests, and even
//  out-performs Apple's Acceleration library.  However, our implementation is
//  limited to 128-bit words as 256-bit (e.g. AVX) and higher show no significant
//  increase in performance.
//
//  For performance reasons, this class does not have a (virtualized) subclass
//  relationship with other IIR or FIR filters.  However, the signature of the
//  the calculation and coefficient methods has been standardized so that it
//  can support templated polymorphism.
//
//  This class is NOT THREAD SAFE.  This is by design, for performance reasons.
//  External locking may be required when the filter is shared between multiple
//  threads (such as between an audio thread and the main thread).
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
//  Version: 6/11/18
//
#ifndef __CU_ONE_POLE_IIR_H__
#define __CU_ONE_POLE_IIR_H__

#include <cugl/math/dsp/CUIIRFilter.h>
#include <cugl/math/CUMathBase.h>
#include <cugl/util/CUAligned.h>
#include <cstring>
#include <vector>

namespace cugl {
    namespace dsp {

/**
 * This class implements a one-pole digital filter.
 *
 * This is the standard class for implementing first order lowpass filters.
 * It is a lowpass filter when the pole is positive (and close to 1).  Use
 * the method {@link setLowpass} for setting the lowpass frequency.
 *
 * Frequencies are specified in "normalized" format.  A normalized frequency
 * is frequency/sample rate. For example, a 7 kHz frequency with a 44100 Hz
 * sample rate has a normalized value 7000/44100 = 0.15873.
 *
 * This class supports vector optimizations for SSE and Neon 64. In timed 
 * simulations, these optimizations provide at least a 3-4x performance increase 
 * (and for 4 or 8 channel audio, much higher). These optimizations make use of 
 * the matrix precomputation outlined in "Implementation of Recursive Digital 
 * Filters into Vector SIMD DSP Architectures".
 *
 *   https://pdfs.semanticscholar.org/d150/a3f75dc033916f14029cd9101a8ea1d050bb.pdf
 *
 * The algorithm in this paper performs extremely well in our tests, and even
 * out-performs Apple's Acceleration library. However, our implementation is
 * limited to 128-bit words as 256-bit (e.g. AVX) and higher show no significant
 * increase in performance.
 *
 * For performance reasons, this class does not have a (virtualized) subclass
 * relationship with other IIR or FIR filters.  However, the signature of the
 * the calculation and coefficient methods has been standardized so that it
 * can support templated polymorphism.
 *
 * This class is not thread safe.  External locking may be required when
 * the filter is shared between multiple threads (such as between an audio
 * thread and the main thread).
 */
class OnePoleIIR {
private:
    /** THe number of channels to support */
    unsigned _channels;
    /** The cached gain factor */
    float _b0;
    /** The (lower) coefficient for the IIR filter */
    float _a1;
    
    /** The previously produced output matching the lower coefficients */
    cugl::Aligned<float> _outs;
    
    // Optimization data structures (single channel)
    float __attribute__((__aligned__(16))) _c1[4];
    float __attribute__((__aligned__(16))) _d1[16];
    
    // Optimization data structures (dual channel)
    float __attribute__((__aligned__(16))) _c2[8];
    float __attribute__((__aligned__(16))) _d2[16];
    
    /**
     * Resets the caching data structures for this filter
     *
     * This must be called if the number of channels or coefficients change.
     */
    void reset();

#pragma mark SPECIALIZED FILTERS
    /**
     * Performs a strided filter of interleaved input data.
     *
     * The output is written to the given output array, which should be the
     * same size as the input array.  The channel data is assumed to start
     * at position 0 for each array.  However, subsequent elements are
     * channel elements ahead in the array.  Hence the channel attribute
     * specifies a data stride.
     *
     * The size is the number of frames, not samples.  Hence the arrays must be
     * size times the number of channels (minus the specific channel) in size.
     * The value size must be a multiple of 4, otherwise the output is
     * undefined. If you need to process an array that is not a multiple of
     * 4, you should use the {@link step} method for the remaining elements.
     *
     * To provide real time processing, the output is delayed by the number
     * of a-coefficients.  Delayed results are buffered to be used the next
     * time the filter is used (though they may be extracted with the
     * {@link flush} method).  The gain parameter is applied at the filter
     * input, but does not affect the filter coefficients.
     *
     * This method uses the vectorized algorithm, if available.
     *
     * @param gain      The input in factor
     * @param input     The array of input samples
     * @param output    The array to write the sample output
     * @param size      The input size in frames
     * @param channel   The specific channel to process
     */
    void stride(float gain, float* input, float* output, size_t size, unsigned channel);
    
    /**
     * Performs a filter of single channel input data.
     *
     * The output is written to the given output array, which should be the
     * same size as the input array. The value size must be a multiple of 4;
     * otherwise the output is undefined. If you need to process an array that
     * is not a multiple of 4, you should use the {@link step} method for the
     * remaining elements.
     *
     * To provide real time processing, the output is delayed by the number
     * of a-coefficients.  Delayed results are buffered to be used the next
     * time the filter is used (though they may be extracted with the
     * {@link flush} method).  The gain parameter is applied at the filter
     * input, but does not affect the filter coefficients.
     *
     * This method uses the vectorized algorithm, if available.
     *
     * @param gain      The input gain factor
     * @param input     The array of input samples
     * @param output    The array to write the sample output
     * @param size      The input size in frames
     */
    void single(float gain, float* input, float* output, size_t size);
    
    /**
     * Performs a filter of interleaved, dual channel input data.
     *
     * The output is written to the given output array, which should be the
     * same size as the input array. The size is the number of frames, not
     * samples.  Hence the arrays must be two times size in length. The value
     * size must be a multiple of 4, otherwise the output is undefined. If you
     * need to process an array that is not a multiple of 4, you should use
     * the {@link step} method for the remaining elements.
     *
     * To provide real time processing, the output is delayed by the number
     * of a-coefficients.  Delayed results are buffered to be used the next
     * time the filter is used (though they may be extracted with the
     * {@link flush} method).  The gain parameter is applied at the filter
     * input, but does not affect the filter coefficients.
     *
     * This method uses the vectorized algorithm, if available.
     *
     * @param gain      The input gain factor
     * @param input     The array of input samples
     * @param output    The array to write the sample output
     * @param size      The input size in frames
     */
    void dual(float gain, float* input, float* output, size_t size);
    
    /**
     * Performs a filter of interleaved, triple channel input data.
     *
     * This method only provides significant optimization for NEON.  On SSE, it
     * defaults to a strided, single channel computation.
     *
     * The output is written to the given output array, which should be the
     * same size as the input array. The size is the number of frames, not
     * samples.  Hence the arrays must be two times size in length. The value
     * size must be a multiple of 4, otherwise the output is undefined. If you
     * need to process an array that is not a multiple of 4, you should use
     * the {@link step} method for the remaining elements.
     *
     * To provide real time processing, the output is delayed by the number
     * of a-coefficients.  Delayed results are buffered to be used the next
     * time the filter is used (though they may be extracted with the
     * {@link flush} method).  The gain parameter is applied at the filter
     * input, but does not affect the filter coefficients.
     *
     * This method uses the vectorized algorithm, if available.
     *
     * @param gain      The input gain factor
     * @param input     The array of input samples
     * @param output    The array to write the sample output
     * @param size      The input size in frames
     */
    void trio(float gain, float* input, float* output, size_t size);
    
    /**
     * Performs a filter of interleaved, 4 channel input data.
     *
     * The output is written to the given output array, which should be the
     * same size as the input array. The size is the number of frames, not
     * samples.  Hence the arrays must be four times size in length. The value
     * size must be a multiple of 4, otherwise the output is undefined. If you
     * need to process an array that is not a multiple of 4, you should use
     * the {@link step} method for the remaining elements.
     *
     * To provide real time processing, the output is delayed by the number
     * of a-coefficients.  Delayed results are buffered to be used the next
     * time the filter is used (though they may be extracted with the
     * {@link flush} method).  The gain parameter is applied at the filter
     * input, but does not affect the filter coefficients.
     *
     * This method uses the vectorized algorithm, if available.
     *
     * @param gain      The input gain factor
     * @param input     The array of input samples
     * @param output    The array to write the sample output
     * @param size      The input size in frames
     */
    void quad(float gain, float* input, float* output, size_t size);
    
    /**
     * Performs a filter of interleaved, 8 channel (e.g. 7.1) input data.
     *
     * The output is written to the given output array, which should be the
     * same size as the input array. The size is the number of frames, not
     * samples.  Hence the arrays must be eight times size in length. The value
     * size must be a multiple of 4, otherwise the output is undefined. If you
     * need to process an array that is not a multiple of 4, you should use
     * the {@link step} method for the remaining elements.
     *
     * To provide real time processing, the output is delayed by the number
     * of a-coefficients.  Delayed results are buffered to be used the next
     * time the filter is used (though they may be extracted with the
     * {@link flush} method).  The gain parameter is applied at the filter
     * input, but does not affect the filter coefficients.
     *
     * This method uses the vectorized algorithm, if available.
     *
     * @param gain      The input gain factor
     * @param input     The array of input samples
     * @param output    The array to write the sample output
     * @param size      The input size in frames
     */
    void quart(float gain, float* input, float* output, size_t size);
    
public:
    /** Whether to use a vectorization algorithm (Access not thread safe) */
    static bool VECTORIZE;

#pragma mark Constructors
    /**
     * Creates a first-order pass-through filter for a single channel.
     */
    OnePoleIIR();
    
    /**
     * Creates a first-order pass-through filter for the given number of channels.
     *
     * @param channels  The number of channels
     */
    OnePoleIIR(unsigned channels);

    /**
     * Creates an IIR filter with the given coefficients and number of channels.
     *
     * This filter implements the standard difference equation:
     *
     *      y[n] = b[0]*x[n]-a[1]*y[n-1]
     *
     * where y is the output and x in the input.
     *
     * @param channels  The number of channels
     * @param b0        The upper zero-order coefficient
     * @param a1        The lower first-order coefficient
     */
    OnePoleIIR(unsigned channels, float b0, float a1);
    
    /**
     * Creates a copy of the one-pole filter.
     *
     * @param copy	The filter to copy
     */
    OnePoleIIR(const OnePoleIIR& copy);
    
    /**
     * Creates a one-pole filter with the resources of the original.
     *
     * @param filter    The filter to acquire
     */
    OnePoleIIR(OnePoleIIR&& filter);
    
    /**
     * Destroys the filter, releasing all resources.
     */
    ~OnePoleIIR();

#pragma mark IIR Signature
    /**
     * Returns the number of channels for this filter
     *
     * The data buffers depend on the number of channels.  Changing this value
     * will reset the data buffers to 0.
     *
     * @return the number of channels for this filter
     */
    unsigned getChannels() const { return _channels; }
    
    /**
     * Sets the number of channels for this filter
     *
     * The data buffers depend on the number of channels.  Changing this value
     * will reset the data buffers to 0.
     *
     * @param channels  The number of channels for this filter
     */
    void setChannels(unsigned channels);
    
    /**
     * Sets the coefficients for this IIR filter.
     *
     * This filter implements the standard difference equation:
     *
     *   a[0]*y[n] = b[0]*x[n]+...+b[nb]*x[n-nb]-a[1]*y[n-1]-...-a[na]*y[n-na]
     *
     * where y is the output and x in the input. If a[0] is not equal to 1,
     * the filter coeffcients are normalized by a[0].
     *
     * All b-coefficients after the first, and all a-coefficients after the
     * second are ignored.  If any coefficients are missing, they are replaced
     * with 1 for b[0] and a[0], and 0 otherwise.
     *
     * @param bvals The upper coefficients
     * @param avals The lower coefficients
     */
    void setCoeff(const std::vector<float> &bvals, const std::vector<float> &avals);
    
    /**
     * Returns the upper coefficients for this IIR filter.
     *
     * This filter implements the standard difference equation:
     *
     *   a[0]*y[n] = b[0]*x[n]+...+b[nb]*x[n-nb]-a[1]*y[n-1]-...-a[na]*y[n-na]
     *
     * where y is the output and x in the input.
     *
     * @return The upper coefficients
     */
    const std::vector<float> getBCoeff() const;
    
    /**
     * Returns the lower coefficients for this IIR filter.
     *
     * This filter implements the standard difference equation:
     *
     *   a[0]*y[n] = b[0]*x[n]+...+b[nb]*x[n-nb]-a[1]*y[n-1]-...-a[na]*y[n-na]
     *
     * where y is the output and x in the input.
     *
     * @return The lower coefficients
     */
    const std::vector<float> getACoeff() const;
    
    /**
     * Sets the transfer function for this IIR filter.
     *
     * Every digital filter is defined by by a z-domain transfer function. This
     * function has the form
     *
     *    H(z) = p(z)/q(z)
     *
     * where p(z) and q(z) are polynomials of z^-1.  This function uniquely
     * determines the coefficients of the digital filter.  In particular, the
     * the coefficients of p are the b-coefficients and the coefficients of q
     * are the q-coefficients.
     *
     * We provide this setter method because filter chaining corresponds to
     * multiplication in the transfer domain.  Hence complex filter chains can
     * be collapsed into a single filter for optimization.
     *
     * @param p     The numerator polynomial
     * @param q     The denominator polynomial
     */
    void setTransfer(const Polynomial& p, const Polynomial& q);
    
    /**
     * Returns the numerator polynomail for the filter transfer function.
     *
     * Every digital filter is defined by by a z-domain transfer function. This
     * function has the form
     *
     *    H(z) = p(z)/q(z)
     *
     * where p(z) and q(z) are polynomials of z^-1.  This function uniquely
     * determines the coefficients of the digital filter.  In particular, the
     * the coefficients of p are the b-coefficients and the coefficients of q
     * are the q-coefficients.
     *
     * @return The numerator polynomail for the filter transfer function.
     */
    Polynomial getNumerator() const;
    
    /**
     * Returns the denominator polynomail for the filter transfer function.
     *
     * Every digital filter is defined by by a z-domain transfer function. This
     * function has the form
     *
     *    H(z) = p(z)/q(z)
     *
     * where p(z) and q(z) are polynomials of z^-1.  This function uniquely
     * determines the coefficients of the digital filter.  In particular, the
     * the coefficients of p are the b-coefficients and the coefficients of q
     * are the q-coefficients.
     *
     * @return The denominator polynomail for the filter transfer function.
     */
    Polynomial getDenominator() const;
    
#pragma mark Specialized Attributes
    /**
     * Sets the upper zero-order coefficient.
     *
     * @param b0    The upper zero-order coefficient
     */
    void setBCoeff(float b0);
    
    /**
     * Sets the lower first-order coefficient.
     *
     * @param a1    The lower first-order coefficient
     */
    void setACoeff(float a1);
    
    /**
     * Sets the (normalized) cutoff frequency for a lowpass filter
     *
     * A normalized frequency is frequency/sample rate. For example, a 7 kHz
     * frequency with a 44100 Hz sample rate has a normalized value of
     * 7000/44100 = 0.15873.
     *
     * Filters are not intended to be model classes, and so it does not save
     * the defining frequency.
     *
     * @param frequency The (normalized) cutoff frequency for a lowpass filter
     */
    void setLowpass(float frequency);
    
    /**
     * Returns the pole position in the z-plane.
     *
     * A positive pole value produces a low-pass filter, while a negative
     * pole value produces a high-pass filter. The magnitude should be
     * less than one to maintain filter stability.
     *
     * @return the pole position in the z-plane.
     */
    float getPole() const { return _a1; }
    
    /**
     * Sets the pole position in the z-plane.
     *
     * This method sets the pole position along the real-axis of the z-plane
     * and normalizes the coefficients for a maximum gain of one. A positive
     * pole value produces a low-pass filter, while a negative pole value
     * produces a high-pass filter.  This method does not affect the filter
     * gain. The argument magnitude should be less than one to maintain filter
     * stability.
     *
     * @param pole  The filter pole
     */
    void setPole(float pole);
    
#pragma mark Filter Methods
    /**
     * Performs a filter of single frame of data.
     *
     * The output is written to the given output array, which should be the
     * same size as the input array. The size should be the number of channels.
     *
     * To provide real time processing, the output is delayed by the number
     * of a-coefficients.  Delayed results are buffered to be used the next
     * time the filter is used (though they may be extracted with the
     * {@link flush} method).  The gain parameter is applied at the filter
     * input, but does not affect the filter coefficients.
     *
     * @param gain      The input gain factor
     * @param input     The input frame
     * @param output    The frame to receive the output
     */
    void step(float gain, float* input, float* output);
    
    /**
     * Performs a filter of interleaved input data.
     *
     * The output is written to the given output array, which should be the
     * same size as the input array. The size is the number of frames, not
     * samples.  Hence the arrays must be size times the number of channels
     * in size.
     *
     * To provide real time processing, the output is delayed by the number
     * of a-coefficients.  Delayed results are buffered to be used the next
     * time the filter is used (though they may be extracted with the
     * {@link flush} method).  The gain parameter is applied at the filter
     * input, but does not affect the filter coefficients.
     *
     * @param gain      The input gain factor
     * @param input     The array of input samples
     * @param output    The array to write the sample output
     * @param size      The input size in frames
     */
    void calculate(float gain, float* input, float* output, size_t size);
    
    /**
     * Clears the filter buffer of any delayed outputs or cached inputs
     */
    void clear();
    
    /**
     * Flushes any delayed outputs to the provided array
     *
     * The array size should be the number of channels. This method will
     * also clear the buffer.
     *
     * @return The number of frames (not samples) written
     */
    size_t flush(float* output);
};
    }
}

#endif /* __CU_ONE_POLE_IIR_H__ */

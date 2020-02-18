//
//  CUIIRFilter.h
//  Cornell University Game Library (CUGL)
//
//  This class is represents a infinite impulse response filter. It is a
//  general purpose filter that allows an arbitrary number of coefficients.
//  It should only be used for 3-order or higher IIRs.  In all other cases,
//  you should use one of the more specific filters for performance reasons.
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
#ifndef __CU_IIR_FILTER_H__
#define __CU_IIR_FILTER_H__

#include <cugl/math/CUMathBase.h>
#include <cugl/math/CUPolynomial.h>
#include <cugl/util/CUAligned.h>
#include <cstring>
#include <vector>

namespace cugl {
    namespace dsp {

#pragma mark -
#pragma mark IIR Filter
/**
 * This class implements an infinite impulse response filter.
 *
 * In particular, this class implements the standard difference equation:
 *
 *   a[0]*y[n] = b[0]*x[n]+...+b[nb]*x[n-nb]-a[1]*y[n-1]-...-a[na]*y[n-na]
 *
 * If a[0] is not equal to 1, the filter coeffcients are normalized by a[0].
 *
 * For filters containing only feedforward terms, {@link FIRFilter} is slightly
 * more efficient.
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
class IIRFilter {
private:
    /** THe number of channels to support */
    unsigned _channels;
    /** The cached gain factor */
    float _b0;
    
    /** The upper coefficients for the FIR filter */
    cugl::Aligned<float> _bval;
    /** The lower coefficients for the IIR filter */
    cugl::Aligned<float> _aval;

    /** The previously recieved input matching the upper coefficients */
    cugl::Aligned<float> _inns;
    /** The previously produced output matching the lower coefficients */
    cugl::Aligned<float> _outs;
    
    // Optimization data structures (single channel)
    cugl::Aligned<float> _c1;
    float __attribute__((__aligned__(16))) _d1[16];
    
    // Optimization data structures (dual channel)
    cugl::Aligned<float> _c2;
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
     * Creates a zero-order pass-through filter for a single channel.
     */
    IIRFilter();
    
    /**
     * Creates a zero-order pass-through filter for the given number of channels.
     *
     * @param channels  The number of channels
     */
    IIRFilter(unsigned channels);

    /**
     * Creates an IIR filter with the given coefficients and number of channels.
     *
     * This filter implements the standard difference equation:
     *
     *   a[0]*y[n] = b[0]*x[n]+...+b[nb]*x[n-nb]-a[1]*y[n-1]-...-a[na]*y[n-na]
     *
     * where y is the output and x in the input. If a[0] is not equal to 1,
     * the filter coeffcients are normalized by a[0].
     *
     * @param channels  The number of channels
     * @param bvals     The upper coefficients
     * @param avals     The lower coefficients
     */
    IIRFilter(unsigned channels, const std::vector<float> &bvals, const std::vector<float> &avals);

    /**
     * Creates a copy of the IIR filter.
     *
     * @param copy	The filter to copy
     */
    IIRFilter(const IIRFilter& copy);
    
    /**
     * Creates an IIR filter with the resources of the original.
     *
     * @param filter    The filter to acquire
     */
    IIRFilter(IIRFilter&& filter);
    
    /**
     * Destroys the filter, releasing all resources.
     */
    ~IIRFilter();

    
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
    std::vector<float> getBCoeff() const;

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
    std::vector<float> getACoeff() const;

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
     * The array size should be the number of channels times one less the
     * number of a-coefficients.
     *
     * This method will also clear the buffer.
     *
     * @return The number of frames (not samples) written
     */
    size_t flush(float* output);
};
    }
}

#endif /* __CU_IIR_FILTER_H__ */

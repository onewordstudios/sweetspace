//
//  CUOneZeroFIR.h
//  Cornell University Game Library (CUGL)
//
//  This class is represents a one-zero FIR filter. For first-degree filters,
//  it is significantly more performant than FIRFilter.
//
//  This class supports vector optimizations for SSE and Neon 64.  In timed
//  simulations, these optimizations provide at least a 3-4x performance
//  increase (and in isolated cases, much higher). Our implementation is
//  limited to 128-bit words as 256-bit (e.g. AVX) and higher show no
//  significant increase in performance.
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
#ifndef __CU_ONE_ZERO_FIR_H__
#define __CU_ONE_ZERO_FIR_H__

#include <cugl/math/dsp/CUIIRFilter.h>
#include <cugl/math/CUMathBase.h>
#include <cugl/util/CUAligned.h>
#include <cstring>
#include <vector>

namespace cugl {
    namespace dsp {
    
/**
 * This class implements a one-zero digital filter.
 *
 * There is a method to set the zero position along the real axis of the
 * z-plane while maintaining a constant filter gain.
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
class OneZeroFIR {
private:
    /** THe number of channels to support */
    unsigned _channels;
    /** The upper coefficients for the FIR filter */
    float _b0, _b1;
    /** The previously recieved input matching the upper coefficients */
    cugl::Aligned<float> _inns;
    
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
    OneZeroFIR();

    /**
     * Creates a zero-order pass-through filter for the given number of channels.
     *
     * @param channels  The number of channels
     */
    OneZeroFIR(unsigned channels);
    
    /**
     * Creates a FIR filter with the given coefficients and number of channels.
     *
     * This filter implements the standard difference equation:
     *
     *      y[n] = b[0]*x[n] + b[1]*x[n-1]
     *
     * where y is the output and x in the input.
     *
     * @param channels  The number of channels
     * @param b0        The upper zero-order coefficient
     * @param b1        The upper first-order coefficient
     */
    OneZeroFIR(unsigned channels, float b0, float b1);
    
    /**
     * Creates a copy of the FIR filter.
     *
     * @param copy      The filter to copy
     */
    OneZeroFIR(const OneZeroFIR& copy);
    
    /**
     * Creates a FIR filter with the resources of the original.
     *
     * @param filter    The filter to acquire
     */
    OneZeroFIR(OneZeroFIR&& filter);
    
    /**
     * Destroys the filter, releasing all resources.
     */
    ~OneZeroFIR();
    
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
     *    a[0]*y[n] = b[0]*x[n] + ... + b[nb]*x[n-nb]
     *
     * where y is the output and x in the input. If a[0] is not equal to 1,
     * the filter coeffcients are normalized by a[0].  All other a-coefficients
     * are ignored (they are only present for signature standardization).
     * Similarly, all b-coefficients after the second are ignored. If any
     * coefficients are missing, they are replaced with 1 for b[0] and a[0],
     * and 0 otherwise.
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
    
#pragma mark Specialized Attributes
    /**
     * Sets the coefficients for this IIR filter.
     *
     * This filter implements the standard difference equation:
     *
     *      y[n] = b[0]*x[n] + b[1]*x[n-1]
     *
     * where y is the output and x in the input.
     *
     * @param b0    The upper zero-order coefficient
     * @param b1    The upper first-order coefficient
     */
    void setBCoeff(float b0, float b1);

    /**
     * Returns the zero position in the z-plane.
     *
     * A positive zero value produces a high-pass filter, while a negative
     * pole value produces a low-pass filter.
     *
     * @return the zero position in the z-plane.
     */
    float getZero() const;
    
    /**
     * Sets the zero position in the z-plane.
     *
     * This method sets the zero position along the real-axis of the z-plane
     * and normalizes the coefficients for a maximum gain of one. A positive
     * pole value produces a high-pass filter, while a negative pole value
     * produces a low-pass filter.  This method does not affect the filter
     * gain.
     *
     * @param zero  The filter zero
     */
    void setZero(float zero);
    
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
     * As this filter has no delayed terms, this method will write nothing. It
     * is only here to standardize the filter signature.
     *
     * @return The number of frames (not samples) written
     */
    size_t flush(float* output);
};
    }
}
#endif /* __CU_ONE_ZERO_FIR_H__ */

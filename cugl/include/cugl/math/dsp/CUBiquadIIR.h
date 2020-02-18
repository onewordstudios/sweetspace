//
//  CUBiquadIIR.h
//  Cornell University Game Library (CUGL)
//
//  This class is represents a biquad filter that supports a wide variety of
//  of 2nd order filters (lowpass, highpass, bandpass, etc...)  It is adapted
//  from the code by Nigel Redmon at
//
//      http://www.earlevel.com/main/2012/11/26/biquad-c-source-code/
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
#ifndef __CU_BIQUAD_IIR_H__
#define __CU_BIQUAD_IIR_H__

#include <cugl/math/dsp/CUIIRFilter.h>
#include <cugl/math/CUMathBase.h>
#include <cugl/util/CUAligned.h>
#include <cstring>
#include <vector>

/** The default q-value */
#define INV_SQRT2   0.7071067812

namespace cugl {
    namespace dsp {
    
/**
 * This class implements a biquad digital filter.
 *
 * This is the most efficient filter acceptable for a parametric equalizer.
 * As such, this filter has several types for quick creation of parameteric
 * components.  However, in most settings Butterworth filters are preferred
 * because they have better roll off.
 *
 * Frequencies are specified in "normalized" format.  A normalized frequency
 * is frequency/sample rate. For example, a 7 kHz frequency with a 44100 Hz
 * sample rate has a normalized value 7000/44100 = 0.15873. However, filters
 * are not intended to be model classes, and so it does not save the defining
 * frequency.
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
class BiquadIIR {
public:
    /**
     * The underlying type of the biquad filter.
     *
     * Most biquad filters are intended for a parameteric equalizer, and so
     * will have one of the filters types below.  If the coefficients of
     * the biquad filter are set directly, it will have type UNDEFINED.
     */
    enum class Type : int {
        /** For direct coefficient manipulation */
        UNDEFINED = 0,
        /** A second-order lowpass filter */
        LOWPASS   = 1,
        /** A second-order lowpass filter */
        HIGHPASS  = 2,
        /** A second-order bandpass filter */
        BANDPASS  = 3,
        /** A second-order allpass filter */
        ALLPASS   = 4,
        /** Inverse of a bandpass (called a band-stop) */
        NOTCH     = 5,
        /** Parametric equalizer */
        PEAK      = 6,
        /** The inverse of a lowpass */
        LOWSHELF  = 7,
        /** The inverse of a highpass */
        HIGHSHELF = 8,
        /** A resonance filter with radius Q */
        RESONANCE = 9,
    };
    
    /** Whether to use a vectorization algorithm (Access not thread safe) */
    static bool VECTORIZE;
    
private:
    /** THe number of channels to support */
    unsigned _channels;
    /** The (upper) coefficients for the FIR filter */
    float _b0, _b1, _b2;
    /** The (lower) coefficients for the IIR filter */
    float _a1, _a2;

    /** The previously recieved input matching the upper coefficients */
    cugl::Aligned<float> _inns;
    /** The previously produced output matching the lower coefficients */
    cugl::Aligned<float> _outs;
    
    // Optimization data structures (single channel)
    float __attribute__((__aligned__(16))) _c1[8];
    float __attribute__((__aligned__(16))) _d1[16];
    
    // Optimization data structures (dual channel)
    float __attribute__((__aligned__(16))) _c2[16];
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
#pragma mark Constructors
    /**
     * Creates a second-order pass-through filter for a single channel.
     */
    BiquadIIR();
    
    /**
     * Creates a second-order pass-through filter for the given number of channels.
     *
     * @param channels  The number of channels
     */
    BiquadIIR(unsigned channels);
    
    /**
     * Creates a special purpose filter of the given type
     *
     * In addition to the type, the filter is defined by the target frequency
     * and the gain for that frequency (which may be negative). This gain
     * will be applied to the target frequency, but will roll-off or attenuate
     * for other frequencies according to the type.  The gain is specified in
     * decibels, not as a multiplicative factor.
     *
     * Frequencies are specified in "normalized" format. A normalized frequency
     * is frequency/sample rate. For example, a 7 kHz frequency with a 44100 Hz
     * sample rate has a normalized value 7000/44100 = 0.15873.
     *
     * The Q factor is the inverse of the bandwidth, and is generally only
     * relevant for the BANDPASS and BANDSTOP filter types.  For the other
     * types, the default value of 1/sqrt(2) is generally sufficient. For
     * BANDPASS and BANDSTOP, the {@link bandwidth2q()} value produces the
     * appropriate value for proper filter design.
     *
     * Filters are not intended to be model classes, and so it does not save
     * the defining frequency, type, gain, or other values.
     *
     * If the type is undefined, the frequency and peakGain will be ignored,
     * creating a pass-through filter.
     *
     * @param channels  The number of channels to process
     * @param type      The filter type
     * @param frequency The (normalized) target frequency
     * @param gainDB    The gain at the target frequency in decibels
     * @param qVal      The special Q factor
     */
    BiquadIIR(unsigned channels, Type type, float frequency, float gainDB, float qVal=INV_SQRT2);
    
    /**
     * Creates a copy of the biquad filter.
     *
     * @param copy	The filter to copy
     */
    BiquadIIR(const BiquadIIR& copy);
    
    /**
     * Creates a biquad filter with the resources of the original.
     *
     * @param filter	The filter to acquire
     */
    BiquadIIR(BiquadIIR&& filter);
    
    /**
     * Destroys the filter, releasing all resources.
     */
    ~BiquadIIR();
    
    
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
     * All b-coefficients and a-coefficients after the third are ignored.  If any
     * coefficients are missing, they are replaced with 1 for b[0] and a[0], and
     * 0 otherwise.
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
     * Sets the upper coefficients.
     *
     * Setting this leaves the lower coefficients unchanged.
     *
     * @param b0    The upper zero-order coefficient
     * @param b1    The upper first-order coefficient
     * @param b2    The upper second-order coefficient
     */
    void setBCoeff(float b0, float b1, float b2);

    /**
     * Sets the lower coefficients.
     *
     * Setting this leaves the upper coefficients unchanged.
     *
     * @param a1    The lower first-order coefficient
     * @param a2    The lower second-order coefficient
     */
    void setACoeff(float a1, float a2);

    /**
     * Sets this filter to the special purpose one of the given type
     *
     * In addition to the type, the filter is defined by the target frequency
     * and the gain for that frequency (which may be negative). This gain
     * will be applied to the target frequency, but will roll-off or attenuate
     * for other frequencies according to the type.  The gain is specified in
     * decibels, not as a multiplicative factor,
     *
     * Frequencies are specified in "normalized" format. A normalized frequency
     * is frequency/sample rate. For example, a 7 kHz frequency with a 44100 Hz
     * sample rate has a normalized value 7000/44100 = 0.15873.
     *
     * The Q factor is the inverse of the bandwidth, and is generally only
     * relevant for the BANDPASS and BANDSTOP filter types.  For the other
     * types, the default value of 1/sqrt(2) is generally sufficient. For
     * BANDPASS and BANDSTOP, the {@link bandwidth2q()} value produces the
     * appropriate value for proper filter design.
     *
     * Filters are not intended to be model classes, and so it does not save
     * the defining frequency, type, gain, or other values.
     *
     * If the type is undefined, the frequency and peakGain will be ignored,
     * creating a pass-through filter.
     *
     * @param type      The filter type
     * @param frequency The (normalized) target frequency
     * @param gainDB    The gain at the target frequency in decibels
     * @param qVal      The special Q factor
     */
    void setType(Type type, float frequency, float gainDB, float qVal=INV_SQRT2);
    
    /**
     * Returns the gain factor for given value in decibels.
     *
     * The factor is the amount to multiply the amplitude signal.  The provided
     * value is represented in decibels, so there may be some round-off error in
     * conversion.
     *
     * @param gainDB The audio gain in decibels
     *
     * @return The gain factor for given value in decibels.
     */
    static float db2gain(float gainDB);

    /**
     * Returns the decibel gain for given factor.
     *
     * The factor is the amount to multiply the amplitude signal. The returned
     * result is in decibels, so there may be some round-off error in conversion.
     *
     * @param gain  The audio gain factor
     *
     * @return The decibel gain for given factor.
     */
    static float gain2db(float gain);

    /**
     * Returns the q value for the given the filter bandwidth (in octaves)
     *
     * The filter bandwidth is generally only relevant for the BANDPASS and
     * BANDSTOP filter types.   For the other types, the q value of 1/sqrt(2)
     * is generally sufficient.
     *
     * @param width The filter bandwidth in octaves
     *
     * @return the q value for the given the filter bandwidth
     */
    static float bandwidth2q(float width);

    /**
     * Returns the filter bandwidth (in octaves) for the given q value
     *
     * The filter bandwidth is generally only relevant for the BANDPASS and
     * BANDSTOP filter types.   For the other types, the q value of 1/sqrt(2)
     * is generally sufficient.
     *
     * @param qVal  The special Q factor
     *
     * @return the filter bandwidth (in octaves) for the given q value
     */
    static float q2Bandwidth(float qVal);

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
     * The array size should be twice the number of channels. This method will
     * also clear the buffer.
     *
     * @return The number of frames (not samples) written
     */
    size_t flush(float* output);
};
    
    }
}

#endif /* __CU_BIQUAD_IIR_H__ */

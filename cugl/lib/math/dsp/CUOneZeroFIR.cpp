//
//  CUOneZeroFIR.cpp
//  Cornell University Game Library (CUGL)
//
//  This class is represents a one-zero FIR filter. For second-degree filters,
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
#include <cugl/math/dsp/CUOneZeroFIR.h>
#include <cugl/util/CUDebug.h>
#include "cuDSP128.inl"

using namespace cugl;
using namespace cugl::dsp;

/** Whether to use a vectorization algorithm */
bool OneZeroFIR::VECTORIZE = true;

#pragma mark Constructors

/**
 * Creates a zero-order pass-through filter for a single channel.
 */
OneZeroFIR::OneZeroFIR() :
_b0(1),
_b1(0),
_channels(1) {
    _inns.reset(0, 16);
    reset();
}

/**
 * Creates a zero-order pass-through filter for the given number of channels.
 *
 * @param channels  The number of channels
 */
OneZeroFIR::OneZeroFIR(unsigned channels) :
_b0(1),
_b1(0) {
    _inns.reset(0, 16);
    _channels = channels;
    reset();
}

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
OneZeroFIR::OneZeroFIR(unsigned channels, float b0, float b1) {
    _inns.reset(0, 16);
    _channels = channels;
    setBCoeff(b0, b1);
}

/**
 * Creates a copy of the FIR filter.
 *
 * @param copy  The filter to copy
 */
OneZeroFIR::OneZeroFIR(const OneZeroFIR& copy) {
    _b0 = copy._b0;
    _b1 = copy._b1;
    _channels = copy._channels;
    _inns = copy._inns;
}

/**
 * Creates an FIR filter with the resources of the original.
 *
 * @param filter    The filter to acquire
 */
OneZeroFIR::OneZeroFIR(OneZeroFIR&& filter) {
    _b0 = filter._b0;
    _b1 = filter._b1;
    _channels = filter._channels;
    _inns = std::move(filter._inns);
}

/**
 * Destroys the filter, releasing all resources.
 */
OneZeroFIR::~OneZeroFIR() {}

/**
 * Resets the caching data structures for this filter
 *
 * This must be called if the number of channels or coefficients change.
 */
void OneZeroFIR::reset() {
    _inns.reset(_channels,16);
    clear();
}


#pragma mark -
#pragma mark IIR Signature
/**
 * Sets the number of channels for this filter
 *
 * The data buffers depend on the number of channels.  Changing this value
 * will reset the data buffers to 0.
 *
 * @param channels  The number of channels for this filter
 */
void OneZeroFIR::setChannels(unsigned channels) {
    CUAssertLog(channels > 0, "Channels %d must be non-zero.",channels);
    _channels = channels;
    reset();
}

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
void OneZeroFIR::setCoeff(const std::vector<float> &bvals, const std::vector<float> &avals) {
    // Only look at first a-coefficient
    float a0 = avals.size() == 0 ? 1.0f : avals[0];
    
    // Upper coefficients are in forward order
    _b0 = (bvals.size() == 0 ? 1.0f : bvals[0])/a0;
    _b1 = (bvals.size() <= 1 ? 0.0f : bvals[1])/a0;
    reset();
}

/**
 * Returns the upper coefficients for this IIR filter.
 *
 * This filter implements the standard difference equation:
 *
 *   a[0]*y[n] = b[0]*x[n]+...+b[nb]*x[n-nb]-a[1]*y[n-1]-...-a[na]*y[n-na]
 *
 * where y is the output and x in the input.  The coefficients have been
 * normalizes so that a[0] is 1.
 *
 * @return The upper coefficients
 */
const std::vector<float> OneZeroFIR::getBCoeff() const {
    std::vector<float> result;
    result.push_back(_b0);
    result.push_back(_b1);
    return result;
}

/**
 * Returns the lower coefficients for this IIR filter.
 *
 * This filter implements the standard difference equation:
 *
 *   a[0]*y[n] = b[0]*x[n]+...+b[nb]*x[n-nb]-a[1]*y[n-1]-...-a[na]*y[n-na]
 *
 * where y is the output and x in the input.  The coefficients have been
 * normalizes so that a[0] is 1.
 
 *
 * @return The lower coefficients
 */
const std::vector<float> OneZeroFIR::getACoeff() const {
    std::vector<float> result;
    result.push_back(1.0f);  // Assume normalization
    return result;
}

#pragma mark -
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
void OneZeroFIR::setBCoeff(float b0, float b1) {
    _b0 = b0;
    _b1 = b1;
    reset();
}

/**
 * Returns the zero position in the z-plane.
 *
 * A positive zero value produces a high-pass filter, while a negative
 * pole value produces a low-pass filter.
 *
 * @return the zero position in the z-plane.
 */
float OneZeroFIR::getZero() const {
    return -_b1/_b0;
}

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
void OneZeroFIR::setZero(float zero) {
    _b0 = 1.0f / (1.0f + ((zero > 0) - (zero < 0))*zero);
    _b1 = -zero * _b0;
}


#pragma mark -
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
 * This specific algorithm used depends on the {@link getAlgorithm()}. By
 * default, a filter is vectorized at the frame level, as this is the
 * most optimized approach.
 *
 * @param gain      The input gain factor
 * @param input     The input frame
 * @param output    The frame to receive the output
 * @param size      The input size in frames
 */
void OneZeroFIR::step(float gain, float* input, float* output) {
    for(size_t ckk = 0; ckk < _channels; ckk++) {
        output[ckk] = gain * _b0 * input[ckk] + _b1 * _inns[ckk];
        _inns[ckk]  = gain * input[ckk];
    }
}

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
 * The specific algorithm used depends on the {@link getAlgorithm()}.  By
 * default, a filter is vectorized at the frame level, as this is the
 * most optimized approach.
 *
 * @param gain      The input gain factor
 * @param input     The array of input samples
 * @param output    The array to write the sample output
 * @param size      The input size in frames
 */
void OneZeroFIR::calculate(float gain,float* input, float* output, size_t size) {
    size_t valid = VECTORIZE ? size-(size % 4) : size;
    switch (_channels) {
        case 1:
            single(gain,input,output,valid);
            break;
        case 2:
            dual(gain,input,output,valid);
            break;
        case 3:
            trio(gain,input,output,valid);
            break;
        case 4:
            quad(gain,input,output,valid);
            break;
        case 8:
            quart(gain,input,output,valid);
            break;
        default:
            for(int ii = 0; ii < _channels; ii++) {
                stride(gain,input+ii,output+ii,valid,ii);
            }
            break;
    }
    if (valid < size) {
        for(size_t ii = size-valid; ii < size; ii++) {
            for(size_t ckk = 0; ckk < _channels; ckk++) {
                output[ii*_channels+ckk] = gain * _b0 * input[ii*_channels+ckk] + _b1 * _inns[ckk];
                _inns[ckk]  = gain * input[ii*_channels+ckk];
            }
        }
    }
}


/**
 * Clears the filter buffer of any delayed outputs or cached inputs
 */
void OneZeroFIR::clear() {
    for(size_t ii = 0; ii < _inns.size(); ii++) {
        _inns[ii] = 0.0f;
    }
}

/**
 * Flushes any delayed outputs to the provided array.
 *
 * As this filter has no delayed terms, this method will write nothing. It
 * is only here to standardize the filter signature.
 *
 * This method will also clear the buffer.
 *
 * @return The number of frames (not samples) written
 */
size_t OneZeroFIR::flush(float* output) {
    clear();
    return 0;
}


#pragma mark -
#pragma mark Specialized Filters
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
void OneZeroFIR::stride(float gain, float* input, float* output, size_t size, unsigned channel) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        unsigned stride = _channels;
        __m128 prev, shuf;
        __m128 temp;
        __m128 data = _mm_setzero_ps();
        
        __m128 factor0 = _mm_set1_ps(_b0);
        __m128 factor1 = _mm_set1_ps(_b1);

        prev = _mm_set_ps(_inns[channel],0,0,0);
        for(int ii = 0; ii < size; ii += 4) {
            // Pack to alignment
            data = _mm_mul_ps(_mm_set1_ps(gain), _mm_skipload_ps(input+ii*stride,stride));
            
            shuf = _mm_shuffle_ps(prev,data,_MM_SHUFFLE(1,0,2,3));
            shuf = _mm_shuffle_ps(shuf,data,_MM_SHUFFLE(2,1,2,0));

            prev = _mm_mul_ps(factor0,data);
            temp = _mm_add_ps(prev,_mm_mul_ps(factor1,shuf));

            // Unpack to store
            _mm_skipstore_ps(output+ii*stride, temp, stride);
            prev = data;
        }
        
        _inns[channel] = prev[3];
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        unsigned stride = _channels;
        float32x4_t prev = { 0.0f, 0.0f, 0.0f, _inns[channel]};
        float32x4_t temp, data, shuf;
        
        float32x4_t factor0 = vld1q_dup_f32(&_b0);
        float32x4_t factor1 = vld1q_dup_f32(&_b1);
        for(size_t ii = 0; ii < size; ii += 4) {
            data = vld1q_skip_f32(input+ii*stride,stride);
            data = vmulq_f32(vld1q_dup_f32(&gain),data);
            shuf = vextq_f32(prev,data,3);
            temp = vmlaq_f32(vmulq_f32(factor0,data),factor1,shuf);

            vst1q_skip_f32(output+ii*stride,temp,stride);
            prev = data;
        }
        
        _inns[channel] = vgetq_lane_f32(prev,3);
    } else {
#else
    {
#endif
        unsigned stride = _channels;
        
        output[0] = gain * _b0 * input[0] + _b1 * _inns[channel];
        for(size_t ii = 1; ii < size; ii++) {
            output[ii*stride] = gain * (_b0 * input[ii*stride] + _b1 * input[(ii-1)*stride]);
        }
        _inns[channel] = gain * input[(size-1)*stride];
    }
}

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
void OneZeroFIR::single(float gain, float* input, float* output, size_t size) {
 #if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        __m128 prev = _mm_set_ps(_inns[0],0,0,0);
        __m128 temp, data, shuf;
        
        __m128 factor0 = _mm_set1_ps(_b0);
        __m128 factor1 = _mm_set1_ps(_b1);
        for(size_t ii = 0; ii < size; ii += 4) {
            data = _mm_mul_ps(_mm_set1_ps(gain), _mm_loadu_ps(input+ii));
            shuf = _mm_shuffle_ps(prev,data,_MM_SHUFFLE(1,0,2,3));
            shuf = _mm_shuffle_ps(shuf,data,_MM_SHUFFLE(2,1,2,0));
            temp = _mm_add_ps(_mm_mul_ps(factor0,data),_mm_mul_ps(factor1,shuf));
            _mm_storeu_ps(output+ii,temp);
            prev = data;
        }
        
        _inns[0] = prev[3];
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float32x4_t prev = { 0.0f, 0.0f, 0.0f, _inns[0]};
        float32x4_t temp, data, shuf;
        
        float32x4_t factor0 = vld1q_dup_f32(&_b0);
        float32x4_t factor1 = vld1q_dup_f32(&_b1);
        for(size_t ii = 0; ii < size; ii += 4) {
            data = vld1q_f32(input+ii);
            data = vmulq_f32(vld1q_dup_f32(&gain),data);
            shuf = vextq_f32(prev,data,3);
            temp = vmlaq_f32(vmulq_f32(factor0,data),factor1,shuf);
            
            vst1q_f32(output+ii,temp);
            prev = data;
        }
        
        _inns[0] = vgetq_lane_f32(prev,3);
    } else {
#else
    {
#endif
        output[(size_t)0] = gain * _b0 * input[0] + _b1 * _inns[(size_t)0];
        for(size_t ii = 1; ii < size; ii++) {
            output[ii] = gain * (_b0 * input[ii] + _b1 * input[ii-1]);
        }
        _inns[(size_t)0] = gain * input[size-1];
    }
}

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
void OneZeroFIR::dual(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        __m128 prev = _mm_set_ps(_inns[1],_inns[0],0,0);
        __m128 temp, data, shuf;
        
        __m128 factor0 = _mm_set1_ps(_b0);
        __m128 factor1 = _mm_set1_ps(_b1);
        for(size_t ii = 0; ii < 2*size; ii += 4) {
            data = _mm_mul_ps(_mm_set1_ps(gain), _mm_loadu_ps(input+ii));
            shuf = _mm_shuffle_ps(prev,data,_MM_SHUFFLE(1,0,3,2));
            temp = _mm_add_ps(_mm_mul_ps(factor0,data),_mm_mul_ps(factor1,shuf));
            _mm_storeu_ps(output+ii,temp);
            prev = data;
        }
        _mm_store_ps(_inns,_mm_movehl_ps(_mm_setzero_ps(),prev));
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float32x4_t prev = { 0.0f, 0.0f, _inns[0], _inns[1]};
        float32x4_t temp, data, shuf;
        
        float32x4_t factor0 = vld1q_dup_f32(&_b0);
        float32x4_t factor1 = vld1q_dup_f32(&_b1);
        for(size_t ii = 0; ii < 2*size; ii += 4) {
            data = vld1q_f32(input+ii);
            data = vmulq_f32(vld1q_dup_f32(&gain),data);
            shuf = vextq_f32(prev,data,2);
            temp = vmlaq_f32(vmulq_f32(factor0,data),factor1,shuf);
            
            vst1q_f32(output+ii,temp);
            prev = data;
        }
        
        _inns[1] = vgetq_lane_f32(prev,3);
        _inns[0] = vgetq_lane_f32(prev,2);
    } else {
#else
    {
#endif
        output[(size_t)0] = gain * _b0 * input[0] + _b1 * _inns[(size_t)0];
        output[(size_t)1] = gain * _b0 * input[1] + _b1 * _inns[(size_t)1];
        for(size_t ii = 1; ii < size; ii++) {
            output[2*ii  ] = gain * (_b0 * input[2*ii  ] + _b1 * input[2*(ii-1)  ]);
            output[2*ii+1] = gain * (_b0 * input[2*ii+1] + _b1 * input[2*(ii-1)+1]);
        }
        _inns[(size_t)0] = gain * input[2*(size-1)  ];
        _inns[(size_t)1] = gain * input[2*(size-1)+1];
    }
}

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
void OneZeroFIR::trio(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float32x4x3_t prev;
        float32x4x3_t data, temp;
        float32x4_t shuf;
        
        prev.val[0] = { 0.0f, 0.0f, 0.0f, _inns[0] };
        prev.val[1] = { 0.0f, 0.0f, 0.0f, _inns[1] };
        prev.val[2] = { 0.0f, 0.0f, 0.0f, _inns[2] };
        
        float32x4_t factor0 = vld1q_dup_f32(&_b0);
        float32x4_t factor1 = vld1q_dup_f32(&_b1);
        for(size_t ii = 0; ii < 3*size; ii += 12) {
            data = vld3q_f32(input+ii);
            data.val[0] = vmulq_f32(vld1q_dup_f32(&gain),data.val[0]);
            data.val[1] = vmulq_f32(vld1q_dup_f32(&gain),data.val[1]);
            data.val[2] = vmulq_f32(vld1q_dup_f32(&gain),data.val[2]);
            
            shuf = vextq_f32(prev.val[0],data.val[0],3);
            temp.val[0] = vmlaq_f32(vmulq_f32(factor0,data.val[0]),factor1,shuf);
            
            shuf = vextq_f32(prev.val[1],data.val[1],3);
            temp.val[1] = vmlaq_f32(vmulq_f32(factor0,data.val[1]),factor1,shuf);
            
            shuf = vextq_f32(prev.val[2],data.val[2],3);
            temp.val[2] = vmlaq_f32(vmulq_f32(factor0,data.val[2]),factor1,shuf);
            
            vst3q_f32(output+ii,temp);
            prev = data;
        }
        
        _inns[2] = vgetq_lane_f32(prev.val[2],3);
        _inns[1] = vgetq_lane_f32(prev.val[1],3);
        _inns[0] = vgetq_lane_f32(prev.val[0],3);
    } else {
#else
    {
#endif
        stride(gain,input+0,output+0,size,0);
        stride(gain,input+1,output+1,size,1);
        stride(gain,input+2,output+2,size,2);
    }
}

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
void OneZeroFIR::quad(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        __m128 prev = _mm_load_ps(_inns);
        __m128 data, temp;
        
        __m128 factor0 = _mm_set1_ps(_b0);
        __m128 factor1 = _mm_set1_ps(_b1);
        for(size_t ii = 0; ii < 4*size; ii += 4) {
            data = _mm_mul_ps(_mm_set1_ps(gain), _mm_loadu_ps(input+ii));
            temp = _mm_add_ps(_mm_mul_ps(factor0,data),_mm_mul_ps(factor1,prev));
            _mm_storeu_ps(output+ii,temp);
            prev = data;
        }
        
        _mm_store_ps(_inns,prev);
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float32x4_t prev = vld1q_f32(_inns+0);
        float32x4_t data, temp;
        
        float32x4_t factor0 = vld1q_dup_f32(&_b0);
        float32x4_t factor1 = vld1q_dup_f32(&_b1);
        for(size_t ii = 0; ii < 4*size; ii += 4) {
            data = vld1q_f32(input+ii);
            data = vmulq_f32(vld1q_dup_f32(&gain),data);
            temp = vmlaq_f32(vmulq_f32(factor0,data),factor1,prev);
            vst1q_f32(output+ii,temp);
            prev = data;
        }
        
        vst1q_f32(_inns+0,prev);
    } else {
#else
    {
#endif
        for(size_t jj = 0; jj < 4; jj++) {
            output[jj] = gain * _b0 * input[jj] + _b1 * _inns[jj];
        }
        for(size_t ii = 1; ii < size; ii++) {
            for(int jj = 0; jj < 4; jj++) {
                output[4*ii+jj] = gain * (_b0 * input[4*ii+jj] + _b1 * input[4*(ii-1)+jj]);
            }
        }
        for(size_t jj = 0; jj < 4; jj++) {
            _inns[jj] = gain * input[4*(size-1)+jj];
        }
    }
}

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
void OneZeroFIR::quart(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        __m128 prev1 = _mm_load_ps(_inns);
        __m128 prev2 = _mm_load_ps(_inns+4);
        __m128 data, temp;
        
        __m128 factor0 = _mm_set1_ps(_b0);
        __m128 factor1 = _mm_set1_ps(_b1);
        for(size_t ii = 0; ii < 8*size; ii += 8) {
            data = _mm_mul_ps(_mm_set1_ps(gain), _mm_loadu_ps(input+ii));
            temp = _mm_add_ps(_mm_mul_ps(factor0,data),_mm_mul_ps(factor1,prev1));
            _mm_storeu_ps(output+ii,temp);
            prev1 = data;
            data = _mm_mul_ps(_mm_set1_ps(gain), _mm_loadu_ps(input+ii+4));
            temp = _mm_add_ps(_mm_mul_ps(factor0,data),_mm_mul_ps(factor1,prev2));
            _mm_storeu_ps(output+ii+4,temp);
            prev2 = data;
        }
        
        _mm_store_ps(_inns,prev1);
        _mm_store_ps(_inns+4,prev2);
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float32x4_t prev1 = vld1q_f32(_inns+0);
        float32x4_t prev2 = vld1q_f32(_inns+4);
        float32x4_t data, temp;
        
        float32x4_t factor0 = vld1q_dup_f32(&_b0);
        float32x4_t factor1 = vld1q_dup_f32(&_b1);
        for(size_t ii = 0; ii < 8*size; ii += 8) {
            data = vld1q_f32(input+ii);
            data = vmulq_f32(vld1q_dup_f32(&gain),data);
            temp = vmlaq_f32(vmulq_f32(factor0,data),factor1,prev1);
            vst1q_f32(output+ii,temp);
            prev1 = data;
            data = vld1q_f32(input+ii+4);
            data = vmulq_f32(vld1q_dup_f32(&gain),data);
            temp = vmlaq_f32(vmulq_f32(factor0,data),factor1,prev2);
            vst1q_f32(output+ii+4,temp);
            prev2 = data;
            
        }
        
        vst1q_f32(_inns+0,prev1);
        vst1q_f32(_inns+4,prev2);
    } else {
#else
    {
#endif
        for(size_t jj = 0; jj < 8; jj++) {
            output[jj] = gain * _b0 * input[jj] + _b1 * _inns[jj];
        }
        for(size_t ii = 1; ii < size; ii++) {
            for(int jj = 0; jj < 8; jj++) {
                output[8*ii+jj] = gain * (_b0 * input[8*ii+jj] + _b1 * input[8*(ii-1)+jj]);
            }
        }
        for(size_t jj = 0; jj < 8; jj++) {
            _inns[jj  ] = gain * input[8*(size-1)+jj];
        }
    }
}

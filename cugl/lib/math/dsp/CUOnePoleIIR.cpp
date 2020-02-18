//
//  CUOnePoleIIR.cpp
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
#include <cugl/math/dsp/CUOnePoleIIR.h>
#include <cugl/util/CUDebug.h>
#include "cuDSP128.inl"

using namespace cugl;
using namespace cugl::dsp;

/** Whether to use a vectorization algorithm */
bool OnePoleIIR::VECTORIZE = true;

#pragma mark Constructors
/**
 * Creates a first-order pass-through filter for a single channel.
 */
OnePoleIIR::OnePoleIIR() :
_b0(1),
_a1(0),
_channels(1) {
    reset();
}

/**
 * Creates a first-order pass-through filter for the given number of channels.
 *
 * @param channels  The number of channels
 */
OnePoleIIR::OnePoleIIR(unsigned channels) :
_b0(1),
_a1(0) {
    _channels = channels;
    reset();
}

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
OnePoleIIR::OnePoleIIR(unsigned channels, float b0, float a1) {
    _b0 = b0;
    _a1 = a1;
    _channels = channels;
    reset();
}

/**
 * Creates a copy of the one-pole filter.
 *
 * @param filter    The filter to copy
 */
OnePoleIIR::OnePoleIIR(const OnePoleIIR& copy) {
    _b0 = copy._b0;
    _a1 = copy._a1;
    _channels = copy._channels;
    std::memcpy(_c1,copy._c1,4*sizeof(float));
    std::memcpy(_c2,copy._c2,8*sizeof(float));
    std::memcpy(_d1,copy._d1,16*sizeof(float));
    std::memcpy(_d2,copy._d2,16*sizeof(float));
    _outs = copy._outs;
}

/**
 * Creates a one-pole filter with the resources of the original.
 *
 * @param filter    The filter to acquire
 */
OnePoleIIR::OnePoleIIR(OnePoleIIR&& filter) {
    _b0 = filter._b0;
    _a1 = filter._a1;
    _channels = filter._channels;
    std::memcpy(_c1,filter._c1,4*sizeof(float));
    std::memcpy(_c2,filter._c2,8*sizeof(float));
    std::memcpy(_d1,filter._d1,16*sizeof(float));
    std::memcpy(_d2,filter._d2,16*sizeof(float));
    _outs = std::move(filter._outs);
}

/**
 * Destroys the filter, releasing all resources.
 */
OnePoleIIR::~OnePoleIIR() {}

/**
 * Resets the caching data structures for this filter
 *
 * This must be called if the number of channels or coefficients change.
 */
void OnePoleIIR::reset() {
    // Single channel optimizations
    _c1[0] = -_a1;
    _c1[1] = -_a1*_c1[0];
    _c1[2] = -_a1*_c1[1];
    _c1[3] = -_a1*_c1[2];
    
#if defined (CU_MATH_VECTOR_SSE)
    _mm_store_ps(_d1,    _mm_setr_ps(1, _c1[0], _c1[1], _c1[2]));
    _mm_store_ps(_d1+4,  _mm_setr_ps(0,  1,     _c1[0], _c1[1]));
    _mm_store_ps(_d1+8,  _mm_setr_ps(0,  0,      1,     _c1[0]));
    _mm_store_ps(_d1+12, _mm_setr_ps(0,  0,      0,      1));
    
    // Dual channel optimizations
    _mm_store_ps(_c2,       _mm_setr_ps( -_a1,   0,     _a1*_a1,      0        ));
    _mm_store_ps(_c2+4,     _mm_setr_ps(   0,  -_a1,     0,          _a1*_a1   ));

    _mm_store_ps(_d2,    _mm_setr_ps(1.0f, 0.0f, _c2[0], 0.0f));
    _mm_store_ps(_d2+4,  _mm_setr_ps(0.0f, 1.0f,  0.0f, _c2[5]));
    _mm_store_ps(_d2+8,  _mm_setr_ps(0.0f, 0.0f,  1.0f,  0.0f));
    _mm_store_ps(_d2+12, _mm_setr_ps(0.0f, 0.0f,  0.0f,  1.0f));
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if ( android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    {
#endif
        float32x4_t temp;
        temp = {   1,   _c1[0], _c1[1],      _c1[2] };
        vst1q_f32(_d1   , temp);
        temp = {   0,    1,     _c1[0],      _c1[1] };
        vst1q_f32(_d1+4 , temp);
        temp = {   0,    0,      1,          _c1[0] };
        vst1q_f32(_d1+8 , temp);
        temp = {   0,    0,      0,           1     };
        vst1q_f32(_d1+12, temp);

        temp = { -_a1,    0,  _a1*_a1,       0      };
        vst1q_f32(_c2   , temp);
        temp = {    0, -_a1,        0,  _a1*_a1     };
        vst1q_f32(_c2+4 , temp);
     
        temp = {   1,    0,     _c2[0],       0      };
        vst1q_f32(_d2   , temp);
        temp = {   0,    1,      0,          _c2[5]  };
        vst1q_f32(_d2+4 , temp);
        temp = {   0,    0,      1,           0      };
        vst1q_f32(_d2+8 , temp);
        temp = {   0,    0,      0,           1      };
        vst1q_f32(_d2+12, temp);
    }
#endif
    _outs.reset(_channels,16);
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
void OnePoleIIR::setChannels(unsigned channels) {
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
 * All b-coefficients after the first, and all a-coefficients after the
 * second are ignored.  If any coefficients are missing, they are replaced
 * with 1 for b[0] and a[0], and 0 otherwise.
 *
 * @param bvals The upper coefficients
 * @param avals The lower coefficients
 */
void OnePoleIIR::setCoeff(const std::vector<float> &bvals, const std::vector<float> &avals) {
    float a0 = avals.size() == 0 ? 1.0f : avals[0];
    _b0 = (bvals.size() == 0 ? 1.0f : bvals[0])/a0;
    _a1 = (avals.size() < 2 ? 0.0f : avals[1])/a0;
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
const std::vector<float> OnePoleIIR::getBCoeff() const {
    std::vector<float> result;
    result.push_back(_b0);
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
const std::vector<float> OnePoleIIR::getACoeff() const {
    std::vector<float> result;
    result.push_back(1.0f);  // Assume normalization
    result.push_back(_a1);
    return result;
}

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
void OnePoleIIR::setTransfer(const Polynomial& p, const Polynomial& q) {
    size_t asize = q.degree();
    
    float a0 = q.back();
    _b0 = p.back()/a0;
    _a1 = (asize < 1 ? 0.0f : q[asize-1])/a0;
    reset();
}

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
Polynomial OnePoleIIR::getNumerator() const {
    Polynomial result(0);
    result[0] = _b0;
    return result;
}

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
Polynomial OnePoleIIR::getDenominator() const {
    Polynomial result(1);
    result[1] = _a1;
    result[2] = 1.0f; // Assume normalization
    return result;
}


#pragma mark -
#pragma mark Specialized Attributes
/**
 * Sets the upper zero-order coefficient.
 *
 * @param b0    The upper zero-order coefficient
 */
void OnePoleIIR::setBCoeff(float b0) {
    _b0 = b0;
    reset();
}

/**
 * Sets the lower first-order coefficient.
 *
 * @param a1    The lower first-order coefficient
 */
void OnePoleIIR::setACoeff(float a1) {
    _a1 = a1;
    reset();
}

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
void OnePoleIIR::setLowpass(float frequency) {
    double tmp = frequency*M_PI*2.0;
    _b0 = (float)(tmp/(tmp+1));
    _a1 = _b0-1.0f;
    reset();
}

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
void OnePoleIIR::setPole(float pole) {
    CUAssertLog(fabsf(pole) < 1.0f, "Pole %f is out of range.",pole);
    _a1 = -pole;
    _b0 = 1.0f + ((pole < 0) - (pole > 0))*pole;
    reset();
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
void OnePoleIIR::step(float gain, float* input, float* output) {
    for(size_t ckk = 0; ckk < _channels; ckk++) {
        output[ckk] = _outs[ckk];
        _outs[ckk]  = gain * _b0 * input[ckk] -_a1 * _outs[ckk];
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
void OnePoleIIR::calculate(float gain,float* input, float* output, size_t size) {
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
                output[ii*_channels+ckk] = _outs[ckk];
                _outs[ckk]  = gain * _b0 * input[ii*_channels+ckk] -_a1 * _outs[ckk];
            }
        }
    }
}

/**
 * Clears the filter buffer of any delayed outputs or cached inputs
 */
void OnePoleIIR::clear() {
    for(size_t ii = 0; _outs && ii < _outs.size(); ii++) {
        _outs[ii] = 0.0f;
    }
}

/**
 * Flushes any delayed outputs to the provided array
 *
 * The array size should be the number of channels. This method will
 * also clear the buffer.
 *
 * @return The number of frames (not samples) written
 */
size_t OnePoleIIR::flush(float* output) {
    for(size_t ii = 0; _outs && ii < _outs.size(); ii++) {
        output[ii] = _outs[ii];
        _outs[ii] = 0.0f;
    }
    return _outs.size()/_channels;
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
void OnePoleIIR::stride(float gain, float* input, float* output, size_t size, unsigned channel) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        __m128 prev;
        __m128 tmp1, tmp2, tmp3;
        __m128 data = _mm_setzero_ps();
        
        unsigned stride = _channels;
        prev = _mm_set1_ps(_outs[channel]);
        for(int ii = 0; ii < size; ii += 4) {
            // C[r] * y
            tmp1 = _mm_mul_ps(prev,_mm_load_ps(_c1));

            // FIR
            data = _mm_skipload_ps(input+ii*stride,stride);
            tmp2 = _mm_mul_ps(_mm_set1_ps(gain * _b0), data);

            // D[r] * x
            tmp3 = _mm_mul_ps(_mm_set1_ps(tmp2[0]),_mm_load_ps(_d1));
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[1]),_mm_load_ps(_d1+4),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[2]),_mm_load_ps(_d1+8),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[3]),_mm_load_ps(_d1+12),tmp3);

            // Shift to output and repeat
            tmp2 = _mm_add_ps(tmp1,tmp3);
            data = _mm_shuffle_ps(tmp2, tmp2, _MM_SHUFFLE(2,1,0,3));
            data[0] = prev[3];

            _mm_skipstore_ps(output+ii*stride,data,stride);
            prev = _mm_set1_ps(tmp2[3]);
        }
        
        _outs[channel] = prev[3];
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        unsigned stride = _channels;
        float32x4_t tmp1, tmp2, tmp3;
        float prev = _outs[channel];

        float coeff = gain * _b0;
        float32x4_t fact = vld1q_dup_f32(&coeff);
        for(size_t ii = 0; ii < size; ii += 4) {
            // C[r] * y
            tmp1 = vmulq_f32(vld1q_dup_f32(&prev),vld1q_f32(_c1));
            
            // FIR
            tmp2 = vmulq_f32(fact, vld1q_skip_f32(input+ii*stride,stride));

            // D[r] * x
            tmp3 = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2),0),vld1q_f32(_d1));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_low_f32(tmp2),1), vld1q_f32(_d1+4));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),0),vld1q_f32(_d1+8));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),1),vld1q_f32(_d1+12));
            
            // Shift to output and repeat
            tmp2 = vaddq_f32(tmp1,tmp3);
            tmp3 = vextq_f32(vld1q_dup_f32(&prev), tmp2, 3);
            vst1q_skip_f32(output+ii*stride, tmp3, stride);
            prev = tmp2[3];
        }
        _outs[channel] = prev;
    } else {
#else
    {
#endif
        unsigned stride = _channels;
        output[0] = _outs[channel];
        
        for(size_t ii = 0; ii < size-1; ii++) {
            output[(ii+1)*stride] = gain * _b0 * input[ii*stride] - _a1 * output[ii*stride];
        }
        
        _outs[channel] = gain * _b0 * input[stride*(size-1)] - _a1 * output[stride*(size-1)];
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
void OnePoleIIR::single(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        __m128 prev;
        __m128 tmp1, tmp2, tmp3;
        
        prev = _mm_set1_ps(_outs[0]);
        for(int ii = 0; ii < size; ii += 4) {
            // C[r] * y
            tmp1 = _mm_mul_ps(prev,_mm_load_ps(_c1));
            
            // FIR
            tmp2 = _mm_mul_ps(_mm_set1_ps(gain * _b0), _mm_load_ps(input+ii));
            
            // D[r] * x
            tmp3 = _mm_mul_ps(_mm_set1_ps(tmp2[0]),_mm_load_ps(_d1));
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[1]),_mm_load_ps(_d1+4),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[2]),_mm_load_ps(_d1+8),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[3]),_mm_load_ps(_d1+12),tmp3);
            
            // Shift to output and repeat
            tmp2 = _mm_add_ps(tmp1,tmp3);
            tmp3 = _mm_shuffle_ps(tmp2, tmp2, _MM_SHUFFLE(2,1,0,3));
            tmp3[0] = prev[3];
            _mm_store_ps(output+ii, tmp3);
            prev = _mm_set1_ps(tmp2[3]);
        }
        
        _outs[0] = prev[3];
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
         (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
         float32x4_t tmp1, tmp2, tmp3;
         float prev = _outs[0];
        
         float coeff = gain * _b0;
         float32x4_t fact = vld1q_dup_f32(&coeff);
         for(size_t ii = 0; ii < size; ii += 4) {
             // C[r] * y
             tmp1 = vmulq_f32(vld1q_dup_f32(&prev),vld1q_f32(_c1));
             
             // FIR
             tmp2 = vmulq_f32(fact, vld1q_f32(input+ii));
             
             // D[r] * x
             tmp3 = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2),0),vld1q_f32(_d1));
             tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_low_f32(tmp2),1), vld1q_f32(_d1+4));
             tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),0),vld1q_f32(_d1+8));
             tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),1),vld1q_f32(_d1+12));
             
             // Shift to output and repeat
             tmp2 = vaddq_f32(tmp1,tmp3);
             tmp3 = vextq_f32(vld1q_dup_f32(&prev), tmp2, 3);
             vst1q_f32(output+ii, tmp3);
             prev = tmp2[3];
         }
         _outs[0] = prev;
    } else {
#else
    {
#endif
        output[0] = _outs[(size_t)0];
        
        for(size_t ii = 0; ii < size-1; ii++) {
            output[ii+1] = gain * _b0 * input[ii] - _a1 * output[ii];
        }
        
        _outs[(size_t)0] = gain * _b0 * input[size-1] - _a1 * output[size-1];
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
void OnePoleIIR::dual(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        __m128 prev;
        __m128 tmp1, tmp2, tmp3;
        prev = _mm_set_ps(_outs[1],_outs[0],0.0f,0.0f);
        for(int ii = 0; ii < 2*size; ii += 4) {
            // C[r] * y
            tmp1 = _mm_mul_ps(_mm_set1_ps(prev[2]),_mm_loadu_ps(_c2));
            tmp1 = _mm_fmadd_ps(_mm_set1_ps(prev[3]),_mm_load_ps(_c2+4), tmp1);
            
            // FIR
            tmp2 = _mm_mul_ps(_mm_set1_ps(gain * _b0), _mm_load_ps(input+ii));
            
            // D[r] * x
            tmp3 = _mm_mul_ps(_mm_set1_ps(tmp2[0]),_mm_load_ps(_d2));
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[1]),_mm_load_ps(_d2+4),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[2]),_mm_load_ps(_d2+8),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[3]),_mm_load_ps(_d2+12),tmp3);

            // Shift to output and repeat
            tmp2 = _mm_add_ps(tmp1,tmp3);
            tmp3 = _mm_shuffle_ps(prev, tmp2, _MM_SHUFFLE(1,0,3,2));
            _mm_storeu_ps(output+ii, tmp3);
            prev = tmp2;
        }
        
        // Do I need to handle the not divisible by 2 case?
        _outs[0] = prev[2];
        _outs[1] = prev[3];
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float32x4_t tmp1, tmp2, tmp3;
        float32x4_t prev = { 0.0f, 0.0f, _outs[0], _outs[1] };

        float coeff = gain * _b0;
        float32x4_t fact = vld1q_dup_f32(&coeff);
        for(size_t ii = 0; ii < 2*size; ii += 4) {
            // C[r] * y
            tmp1 = vmulq_f32(vdupq_lane_f32(vget_high_f32(prev),0),vld1q_f32(_c2));
            tmp1 = vmlaq_f32(tmp1,vdupq_lane_f32(vget_high_f32(prev),1), vld1q_f32(_c2+4));

            // FIR
            tmp2 = vmulq_f32(fact, vld1q_f32(input+ii));
            
            // D[r] * x
            tmp3 = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2),0),vld1q_f32(_d2));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_low_f32(tmp2),1), vld1q_f32(_d2+4));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),0),vld1q_f32(_d2+8));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),1),vld1q_f32(_d2+12));
            
            // Shift to output and repeat
            tmp2 = vaddq_f32(tmp1,tmp3);
            tmp3 = vextq_f32(prev,tmp2, 2);
            vst1q_f32(output+ii, tmp3);
            prev = tmp2;
        }
        _outs[0] = vgetq_lane_f32(prev,2);
        _outs[1] = vgetq_lane_f32(prev,3);
    } else {
#else
    {
#endif
        output[0] = _outs[(size_t)0];
        output[1] = _outs[(size_t)1];
        
        for(size_t ii = 0; ii < size-1; ii++) {
            output[2*(ii+1)  ] = gain * _b0 * input[2*ii  ] - _a1 * output[2*ii  ];
            output[2*(ii+1)+1] = gain * _b0 * input[2*ii+1] - _a1 * output[2*ii+1];
        }
        
        _outs[(size_t)0] = gain * _b0 * input[2*(size-1)  ] - _a1 * output[2*(size-1)  ];
        _outs[(size_t)1] = gain * _b0 * input[2*(size-1)+1] - _a1 * output[2*(size-1)+1];
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
void OnePoleIIR::trio(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float32x4x3_t data, outr;
        float32x4_t tmp1, tmp2, tmp3;
        
        float prev0 = _outs[0];
        float prev1 = _outs[1];
        float prev2 = _outs[2];
        
        float coeff = gain * _b0;
        float32x4_t fact = vld1q_dup_f32(&coeff);
        for(size_t ii = 0; ii < 3*size; ii += 12) {
            data = vld3q_f32(input+ii);
            
            // Channel 0
            // C[r] * y
            tmp1 = vmulq_f32(vld1q_dup_f32(&prev0),vld1q_f32(_c1));
            
            // FIR
            tmp2 = vmulq_f32(fact, data.val[0]);
            
            // D[r] * x
            tmp3 = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2),0),vld1q_f32(_d1));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_low_f32(tmp2),1), vld1q_f32(_d1+4));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),0),vld1q_f32(_d1+8));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),1),vld1q_f32(_d1+12));
            
            // Shift to output and repeat
            tmp2 = vaddq_f32(tmp1,tmp3);
            tmp3 = vextq_f32(vld1q_dup_f32(&prev0), tmp2, 3);
            outr.val[0] = tmp3;
            prev0 = tmp2[3];
            
            // Channel 1
            // C[r] * y
            tmp1 = vmulq_f32(vld1q_dup_f32(&prev1),vld1q_f32(_c1));
            
            // FIR
            tmp2 = vmulq_f32(fact, data.val[1]);
            
            // D[r] * x
            tmp3 = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2),0),vld1q_f32(_d1));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_low_f32(tmp2),1), vld1q_f32(_d1+4));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),0),vld1q_f32(_d1+8));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),1),vld1q_f32(_d1+12));
            
            // Shift to output and repeat
            tmp2 = vaddq_f32(tmp1,tmp3);
            tmp3 = vextq_f32(vld1q_dup_f32(&prev1), tmp2, 3);
            outr.val[1] = tmp3;
            prev1 = tmp2[3];
            
            // Channel 2
            // C[r] * y
            tmp1 = vmulq_f32(vld1q_dup_f32(&prev2),vld1q_f32(_c1));
            
            // FIR
            tmp2 = vmulq_f32(fact, data.val[2]);
            
            // D[r] * x
            tmp3 = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2),0),vld1q_f32(_d1));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_low_f32(tmp2),1), vld1q_f32(_d1+4));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),0),vld1q_f32(_d1+8));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),1),vld1q_f32(_d1+12));
            
            // Shift to output and repeat
            tmp2 = vaddq_f32(tmp1,tmp3);
            tmp3 = vextq_f32(vld1q_dup_f32(&prev2), tmp2, 3);
            outr.val[2] = tmp3;
            prev2 = tmp2[3];
            
            vst3q_f32(output+ii, outr);
        }
        
        _outs[2] = prev2;
        _outs[1] = prev1;
        _outs[0] = prev0;
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
void OnePoleIIR::quad(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        __m128 prev;
        __m128 temp;
        
        prev = _mm_load_ps(_outs);
        for(int ii = 0; ii < 4*size; ii += 4) {
            temp = _mm_mul_ps(_mm_set1_ps(gain * _b0), _mm_load_ps(input+ii));
            temp = _mm_fmadd_ps(_mm_set1_ps(-_a1),prev,temp);
            _mm_store_ps(output+ii, prev);
            prev = temp;
        }
        _mm_store_ps(_outs,prev);
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float32x4_t prev;
        float32x4_t temp;
        
        prev = vld1q_f32(_outs);

        float coeff = gain * _b0;
        float32x4_t fact = vld1q_dup_f32(&coeff);
        for(int ii = 0; ii < 4*size; ii += 4) {
            temp = vmulq_f32(fact, vld1q_f32(input+ii));
            temp = vmlaq_f32(temp, vld1q_dup_f32(_c1) ,prev);
            vst1q_f32(output+ii, prev);
            prev = temp;
        }
        vst1q_f32(_outs,prev);
    } else {
#else
    {
#endif
        for(size_t ii = 0; ii < 4; ii++) {
            output[ii] = _outs[ii];
        }
        
        for(size_t ii = 0; ii < size-1; ii++) {
            output[4*(ii+1)  ] = gain * _b0 * input[4*ii  ] - _a1 * output[4*ii  ];
            output[4*(ii+1)+1] = gain * _b0 * input[4*ii+1] - _a1 * output[4*ii+1];
            output[4*(ii+1)+2] = gain * _b0 * input[4*ii+2] - _a1 * output[4*ii+2];
            output[4*(ii+1)+3] = gain * _b0 * input[4*ii+3] - _a1 * output[4*ii+3];
        }
        
        for(size_t ii = 0; ii < 4; ii++) {
            _outs[ii] = gain * _b0 * input[4*(size-1)+ii] - _a1 * output[4*(size-1)+ii];
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
void OnePoleIIR::quart(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        __m128 prva,prvb;
        __m128 temp;
        
        prva = _mm_load_ps(_outs);
        prvb = _mm_load_ps(_outs+4);
        for(int ii = 0; ii < 8*size; ii += 8) {
            temp = _mm_mul_ps(_mm_set1_ps(gain * _b0), _mm_load_ps(input+ii));
            temp = _mm_fmadd_ps(_mm_set1_ps(-_a1),prva,temp);
            _mm_store_ps(output+ii, prva);
            prva = temp;
            
            temp = _mm_mul_ps(_mm_set1_ps(gain * _b0), _mm_load_ps(input+ii+4));
            temp = _mm_fmadd_ps(_mm_set1_ps(-_a1),prvb,temp);
            _mm_store_ps(output+ii+4, prvb);
            prvb = temp;
        }
        _mm_store_ps(_outs,prva);
        _mm_store_ps(_outs+4,prvb);
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float32x4_t prva,prvb;
        float32x4_t temp;
        
        prva = vld1q_f32(_outs);
        prvb = vld1q_f32(_outs+4);
        
        float coeff = gain * _b0;
        float32x4_t fact = vld1q_dup_f32(&coeff);
        for(int ii = 0; ii < 8*size; ii += 8) {
            temp = vmulq_f32(fact, vld1q_f32(input+ii));
            temp = vmlaq_f32(temp, vld1q_dup_f32(_c1), prva);
            vst1q_f32(output+ii, prva);
            prva = temp;
            
            temp = vmulq_f32(fact, vld1q_f32(input+ii+4));
            temp = vmlaq_f32(temp, vld1q_dup_f32(_c1), prvb);
            vst1q_f32(output+ii+4, prvb);
            prvb = temp;
        }
        vst1q_f32(_outs,prva);
        vst1q_f32(_outs+4,prvb);
    } else {
#else
    {
#endif
        for(size_t ii = 0; ii < 8; ii++) {
            output[ii] = _outs[ii];
        }
        
        for(size_t ii = 0; ii < size-1; ii++) {
            output[8*(ii+1)  ] = gain * _b0 * input[8*ii  ] - _a1 * output[8*ii  ];
            output[8*(ii+1)+1] = gain * _b0 * input[8*ii+1] - _a1 * output[8*ii+1];
            output[8*(ii+1)+2] = gain * _b0 * input[8*ii+2] - _a1 * output[8*ii+2];
            output[8*(ii+1)+3] = gain * _b0 * input[8*ii+3] - _a1 * output[8*ii+3];
            output[8*(ii+1)+4] = gain * _b0 * input[8*ii+4] - _a1 * output[8*ii+4];
            output[8*(ii+1)+5] = gain * _b0 * input[8*ii+5] - _a1 * output[8*ii+5];
            output[8*(ii+1)+6] = gain * _b0 * input[8*ii+6] - _a1 * output[8*ii+6];
            output[8*(ii+1)+7] = gain * _b0 * input[8*ii+7] - _a1 * output[8*ii+7];
        }
        
        for(size_t ii = 0; ii < 8; ii++) {
            _outs[ii] = gain * _b0 * input[8*(size-1)+ii] - _a1 * output[8*(size-1)+ii];
        }
    }
}

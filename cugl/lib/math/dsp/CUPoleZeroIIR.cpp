//
//  CUPoleZeroFIR.h
//  Cornell University Game Library (CUGL)
//
//  This class is represents a one-pole, one-zero IIR filter. This is the
//  standard class for implementing first order highpass filters.  For
//  first-order filters, it is significantly more performant than IIRFilter.
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
#include <cugl/math/dsp/CUPoleZeroIIR.h>
#include <cugl/util/CUDebug.h>
#include "cuDSP128.inl"

using namespace cugl;
using namespace cugl::dsp;

/** Whether to use a vectorization algorithm */
bool PoleZeroFIR::VECTORIZE = true;

#pragma mark Constructors

/**
 * Creates a zero-order pass-through filter for a single channel.
 */
PoleZeroFIR::PoleZeroFIR() :
_b0(1),
_b1(0),
_a1(0),
_channels(1) {
    reset();
}

/**
 * Creates a zero-order pass-through filter for the given number of channels.
 *
 * @param channels  The number of channels
 */
PoleZeroFIR::PoleZeroFIR(unsigned channels) :
_b0(1),
_b1(0),
_a1(0) {
    _channels = channels;
    reset();
}

/**
 * Creates a FIR filter with the given coefficients and number of channels.
 *
 * This filter implements the standard difference equation:
 *
 *      y[n] = b[0]*x[n] + b[1]*x[n-1] - a[1]*y[n-1]
 *
 * where y is the output and x in the input.
 *
 * @param channels  The number of channels
 * @param b0        The upper zero-order coefficient
 * @param b1        The upper first-order coefficient
 * @param a1        The lower first-order coefficient
 */
PoleZeroFIR::PoleZeroFIR(unsigned channels, float b0, float b1, float a1) {
    _channels = channels;
    _b0 = b0;
    _b1 = b1;
    _a1 = a1;
    reset();
}

/**
 * Creates a copy of the FIR filter.
 *
 * @param copy  The filter to copy
 */
PoleZeroFIR::PoleZeroFIR(const PoleZeroFIR& copy) {
    _b0 = copy._b0;
    _b1 = copy._b1;
    _a1 = copy._a1;
    _channels = copy._channels;
    std::memcpy(_c1,copy._c1,4*sizeof(float));
    std::memcpy(_c2,copy._c2,8*sizeof(float));
    std::memcpy(_d1,copy._d1,16*sizeof(float));
    std::memcpy(_d2,copy._d2,16*sizeof(float));
    _inns = copy._inns;
    _outs = copy._outs;
}

/**
 * Creates an FIR filter with the resources of the original.
 *
 * @param filter    The filter to acquire
 */
PoleZeroFIR::PoleZeroFIR(PoleZeroFIR&& filter) {
    _b0 = filter._b0;
    _b1 = filter._b1;
    _a1 = filter._a1;
    _channels = filter._channels;
    std::memcpy(_c1,filter._c1,4*sizeof(float));
    std::memcpy(_c2,filter._c2,8*sizeof(float));
    std::memcpy(_d1,filter._d1,16*sizeof(float));
    std::memcpy(_d2,filter._d2,16*sizeof(float));
    _inns = std::move(filter._inns);
    _outs = std::move(filter._outs);
}

/**
 * Destroys the filter, releasing all resources.
 */
PoleZeroFIR::~PoleZeroFIR() {}

/**
 * Resets the caching data structures for this filter
 *
 * This must be called if the number of channels or coefficients change.
 */
void PoleZeroFIR::reset() {
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
void PoleZeroFIR::setChannels(unsigned channels) {
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
 * All b-coefficients and a-coefficients after the second are ignored.  If any
 * coefficients are missing, they are replaced with 1 for b[0] and a[0], and
 * 0 otherwise.
 *
 * @param bvals The upper coefficients
 * @param avals The lower coefficients
 */
void PoleZeroFIR::setCoeff(const std::vector<float> &bvals, const std::vector<float> &avals) {
    // Only look at first a-coefficient
    float a0 = avals.size() == 0 ? 1.0f : avals[0];
    
    // Upper coefficients are in forward order
    _b0 = (bvals.size() == 0 ? 1.0f : bvals[0])/a0;
    _b1 = (bvals.size() <= 1 ? 0.0f : bvals[1])/a0;
    _a1 = (avals.size() <= 1 ? 0.0f : avals[1])/a0;
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
const std::vector<float> PoleZeroFIR::getBCoeff() const {
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
const std::vector<float> PoleZeroFIR::getACoeff() const {
    std::vector<float> result;
    result.push_back(1.0f);  // Assume normalization
    result.push_back(_a1);
    return result;
}

#pragma mark -
#pragma mark Specialized Attributes
/**
 * Sets the upper coefficients.
 *
 * Setting this leaves the lower coefficient unchanged.
 *
 * @param b0    The upper zero-order coefficient
 * @param b1    The upper first-order coefficient
 */
void PoleZeroFIR::setBCoeff(float b0, float b1) {
    _b0 = b0;
    _b1 = b1;
    reset();
}

/**
 * Sets the lower coefficient.
 *
 * Setting this leaves the upper coefficients unchanged.
 *
 * @param a1    The lower first-order coefficient
 */
void PoleZeroFIR::setACoeff(float a1) {
    _a1 = a1;
    reset();
}
/**
 * Sets the filter to be a first-order highpass for the given frequency
 *
 * The resulting filter is typically considered th esimplest effective
 * highpass filter.  We do not remember the frequency as this class supports
 * other filters than highpass filters.
 *
 * The frequency is specified in "normalized" format. A normalized frequency
 * is frequency/sample rate. For example, a 7 kHz frequency with a 44100 Hz
 * sample rate has a normalized value 7000/44100 = 0.15873.
 *
 * @param frequency The normalized cutoff frequency
 */
void PoleZeroFIR::setHighpass(float frequency) {
    double tmp = 1.0/(frequency*M_PI*2+1.0);
    _b0 = (float)tmp;
    _b1 = -_b0;
    _a1 = -_b0;
}

/**
 * Sets the filter to be a first-order allpass with the given coefficient.
 *
 * The allpass filter has unity gain at all frequencies.  The coefficient
 * magnitude must be less than one to maintain filter stability.
 *
 * @param coefficient   The allpass coefficient
 */
void PoleZeroFIR::setAllpass( float coefficient )   {
    CUAssertLog(fabsf(coefficient) < 1.0f, "Coefficient %f is out of range.",coefficient);
    _b0 = coefficient;
    _b1 = 1.0;
    _a1 = coefficient;
}

/**
 * Sets the filter to be a DC blocking filter with the given pole position.
 *
 * This method sets the given pole position, together with a zero at z=1,
 * to create a DC blocking filter.  The argument magnitude should be close
 * to (but less than) one to minimize low-frequency attenuation.
 *
 * @param pole  The filter pole
 */
void PoleZeroFIR::setBlockZero(float pole) {
    CUAssertLog(fabsf(pole) < 1.0f, "Pole %f is out of range.", pole);
    _b0 = 1.0f;
    _b1 = -1.0f;
    _a1 = -pole;;
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
void PoleZeroFIR::step(float gain, float* input, float* output) {
    for(size_t ckk = 0; ckk < _channels; ckk++) {
        output[ckk] = _outs[ckk];
        _outs[ckk]  = gain * _b0 * input[ckk] + _b1 * _inns[ckk] - _a1 * _outs[ckk];
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
void PoleZeroFIR::calculate(float gain,float* input, float* output, size_t size) {
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
                _outs[ckk]  = gain * _b0 * input[ii*_channels+ckk] + _b1 * _inns[ckk] - _a1 * _outs[ckk];
                _inns[ckk]  = gain * input[ii*_channels+ckk];
            }
        }
    }
}


/**
 * Clears the filter buffer of any delayed outputs or cached inputs
 */
void PoleZeroFIR::clear() {
    for(size_t ii = 0; ii < _inns.size(); ii++) {
        _inns[ii] = 0.0f;
    }
    for(size_t ii = 0; ii < _outs.size(); ii++) {
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
size_t PoleZeroFIR::flush(float* output) {
    for(size_t ii = 0; ii < _inns.size(); ii++) {
        _inns[ii] = 0.0f;
    }
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
void PoleZeroFIR::stride(float gain, float* input, float* output, size_t size, unsigned channel) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        unsigned stride = _channels;
        __m128 pout = _mm_set1_ps(_outs[channel]);
        __m128 pinn = _mm_set_ps(_inns[channel],0,0,0);
        __m128 tmp1, tmp2, tmp3;
        __m128 data, shuf;
        
        __m128 factor0 = _mm_set1_ps(_b0);
        __m128 factor1 = _mm_set1_ps(_b1);
        for(size_t ii = 0; ii < size; ii += 4) {
            // C[r] * y
            tmp1 = _mm_mul_ps(pout,_mm_load_ps(_c1));
            
            // FIR
            data = _mm_mul_ps(_mm_set1_ps(gain), _mm_skipload_ps(input+ii*stride,stride));
            shuf = _mm_shuffle_ps(pinn,data,_MM_SHUFFLE(1,0,2,3));
            shuf = _mm_shuffle_ps(shuf,data,_MM_SHUFFLE(2,1,2,0));
            tmp2 = _mm_add_ps(_mm_mul_ps(factor0,data),_mm_mul_ps(factor1,shuf));
            pinn = data;
            
            // D[r] * x
            tmp3 = _mm_mul_ps(_mm_set1_ps(tmp2[0]),_mm_load_ps(_d1));
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[1]),_mm_load_ps(_d1+4),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[2]),_mm_load_ps(_d1+8),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[3]),_mm_load_ps(_d1+12),tmp3);
            
            // Shift to output and repeat
            tmp2 = _mm_add_ps(tmp1,tmp3);
            tmp3 = _mm_shuffle_ps(tmp2, tmp2, _MM_SHUFFLE(2,1,0,3));
            tmp3[0] = pout[3];
            
            _mm_skipstore_ps(output+ii*stride, tmp3,stride);
            pout = _mm_set1_ps(tmp2[3]);
        }
        
        _outs[channel] = pout[3];
        _inns[channel] = pinn[3];
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        unsigned stride = _channels;
        float32x4_t pinn = { 0, 0, 0, _inns[channel] };
        float pout = _outs[channel];
        
        float32x4_t tmp1, tmp2, tmp3;
        float32x4_t data, shuf;
        
        float32x4_t factor0 = vld1q_dup_f32(&_b0);
        float32x4_t factor1 = vld1q_dup_f32(&_b1);
        for(size_t ii = 0; ii < size; ii += 4) {
            // C[r] * y
            tmp1 = vmulq_f32(vld1q_dup_f32(&pout),vld1q_f32(_c1));
            
            // FIR
            data = vmulq_f32(vld1q_dup_f32(&gain), vld1q_skip_f32(input+ii*stride,stride));
            shuf = vextq_f32(pinn, data, 3);
            tmp2 = vmlaq_f32(vmulq_f32(factor0,data),factor1,shuf);
            pinn = data;
            
            // D[r] * x
            tmp3 = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2),0),vld1q_f32(_d1));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_low_f32(tmp2),1), vld1q_f32(_d1+4));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),0),vld1q_f32(_d1+8));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),1),vld1q_f32(_d1+12));
            
            // Shift to output and repeat
            tmp2 = vaddq_f32(tmp1,tmp3);
            tmp3 = vextq_f32(vld1q_dup_f32(&pout), tmp2, 3);
            
            vst1q_skip_f32(output+ii*stride, tmp3, stride);
            pout = tmp2[3];
        }
        
        _outs[channel] = pout;
        _inns[channel] = vgetq_lane_f32(pinn,3);
    } else {
#else
    {
#endif
        unsigned stride = _channels;
        output[0] = _outs[channel];
        
        output[stride] = gain * _b0 * input[0] + _b1 * _inns[channel]  - _a1 * output[0];
        for(size_t ii = 1; ii < size-1; ii++) {
            output[(ii+1)*stride] = gain * (_b0 * input[ii*stride] + _b1 * input[(ii-1)*stride]) - _a1 * output[ii*stride];
        }
        
        _outs[channel] = gain * (_b0 * input[(size-1)*stride] + _b1 * input[(size-2)*stride]) - _a1 * output[(size-1)*stride];
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
void PoleZeroFIR::single(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        __m128 pout = _mm_set1_ps(_outs[0]);
        __m128 pinn = _mm_set_ps(_inns[0],0,0,0);
        __m128 tmp1, tmp2, tmp3;
        __m128 data, shuf;
        
        __m128 factor0 = _mm_set1_ps(_b0);
        __m128 factor1 = _mm_set1_ps(_b1);
        for(size_t ii = 0; ii < size; ii += 4) {
            // C[r] * y
            tmp1 = _mm_mul_ps(pout,_mm_load_ps(_c1));
            
            // FIR
            data = _mm_mul_ps(_mm_set1_ps(gain), _mm_loadu_ps(input+ii));
            shuf = _mm_shuffle_ps(pinn,data,_MM_SHUFFLE(1,0,2,3));
            shuf = _mm_shuffle_ps(shuf,data,_MM_SHUFFLE(2,1,2,0));
            tmp2 = _mm_add_ps(_mm_mul_ps(factor0,data),_mm_mul_ps(factor1,shuf));
            pinn = data;
            
            // D[r] * x
            tmp3 = _mm_mul_ps(_mm_set1_ps(tmp2[0]),_mm_load_ps(_d1));
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[1]),_mm_load_ps(_d1+4),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[2]),_mm_load_ps(_d1+8),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[3]),_mm_load_ps(_d1+12),tmp3);
            
            // Shift to output and repeat
            tmp2 = _mm_add_ps(tmp1,tmp3);
            tmp3 = _mm_shuffle_ps(tmp2, tmp2, _MM_SHUFFLE(2,1,0,3));
            tmp3[0] = pout[3];
            
            _mm_storeu_ps(output+ii, tmp3);
            pout = _mm_set1_ps(tmp2[3]);
        }
        
        _outs[0] = pout[3];
        _inns[0] = pinn[3];
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float32x4_t pinn = { 0, 0, 0, _inns[0] };
        float pout = _outs[0];
        
        float32x4_t tmp1, tmp2, tmp3;
        float32x4_t data, shuf;
        
        float32x4_t factor0 = vld1q_dup_f32(&_b0);
        float32x4_t factor1 = vld1q_dup_f32(&_b1);
        for(size_t ii = 0; ii < size; ii += 4) {
            // C[r] * y
            tmp1 = vmulq_f32(vld1q_dup_f32(&pout),vld1q_f32(_c1));
            
            // FIR
            data = vmulq_f32(vld1q_dup_f32(&gain), vld1q_f32(input+ii));
            shuf = vextq_f32(pinn, data, 3);
            tmp2 = vmlaq_f32(vmulq_f32(factor0,data),factor1,shuf);
            pinn = data;
            
            // D[r] * x
            tmp3 = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2),0),vld1q_f32(_d1));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_low_f32(tmp2),1), vld1q_f32(_d1+4));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),0),vld1q_f32(_d1+8));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),1),vld1q_f32(_d1+12));
            
            // Shift to output and repeat
            tmp2 = vaddq_f32(tmp1,tmp3);
            tmp3 = vextq_f32(vld1q_dup_f32(&pout), tmp2, 3);
            
            vst1q_f32(output+ii, tmp3);
            pout = tmp2[3];
        }
        
        _outs[0] = pout;
        _inns[0] = vgetq_lane_f32(pinn,3);
    } else {
#else
    {
#endif
        output[0] = _outs[(size_t)0];
        output[1] = gain * _b0 * input[0] + _b1 * _inns[(size_t)0] - _a1 * output[0];
        for(size_t ii = 1; ii < size-1; ii++) {
            output[ii+1] = gain * (_b0 * input[ii] + _b1 * input[ii-1]) - _a1 * output[ii];
        }
        
        _outs[(size_t)0] = gain * (_b0 * input[size-1] + _b1 * input[size-2]) - _a1 * output[size-1];
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
void PoleZeroFIR::dual(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        __m128 pout = _mm_set_ps(_outs[1],_outs[0],0,0);
        __m128 pinn = _mm_set_ps(_inns[1],_inns[0],0,0);
        __m128 tmp1, tmp2, tmp3;
        __m128 data, shuf;
        
        __m128 factor0 = _mm_set1_ps(_b0);
        __m128 factor1 = _mm_set1_ps(_b1);
        for(size_t ii = 0; ii < 2*size; ii += 4) {
            // C[r] * y
            tmp1 = _mm_mul_ps(_mm_set1_ps(pout[2]),_mm_loadu_ps(_c2));
            tmp1 = _mm_fmadd_ps(_mm_set1_ps(pout[3]),_mm_load_ps(_c2+4), tmp1);
            
            // FIR
            data = _mm_mul_ps(_mm_set1_ps(gain), _mm_loadu_ps(input+ii));
            shuf = _mm_shuffle_ps(pinn,data,_MM_SHUFFLE(1,0,3,2));
            tmp2 = _mm_add_ps(_mm_mul_ps(factor0,data),_mm_mul_ps(factor1,shuf));
            pinn = data;
            
            // D[r] * x
            tmp3 = _mm_mul_ps(_mm_set1_ps(tmp2[0]),_mm_load_ps(_d2));
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[1]),_mm_load_ps(_d2+4),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[2]),_mm_load_ps(_d2+8),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[3]),_mm_load_ps(_d2+12),tmp3);
            
            // Shift to output and repeat
            tmp2 = _mm_add_ps(tmp1,tmp3);
            tmp3 = _mm_shuffle_ps(pout, tmp2, _MM_SHUFFLE(1,0,3,2));
            _mm_storeu_ps(output+ii, tmp3);
            pout = tmp2;
        }
        _mm_store_ps(_outs,_mm_movehl_ps(_mm_setzero_ps(),pout));
        _mm_store_ps(_inns,_mm_movehl_ps(_mm_setzero_ps(),pinn));
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float32x2_t pout = vld1_f32(_outs);
        float32x2_t pinn = vld1_f32(_inns);
        float32x4_t tmp1, tmp2, tmp3;
        float32x4_t data, shuf;
        
        float32x4_t factor0 = vld1q_dup_f32(&_b0);
        float32x4_t factor1 = vld1q_dup_f32(&_b1);
        for(size_t ii = 0; ii < 2*size; ii += 4) {
            // C[r] * y
            tmp1 = vmulq_f32(vdupq_lane_f32(pout,0), vld1q_f32(_c2));
            tmp1 = vmlaq_f32(tmp1, vdupq_lane_f32(pout,1), vld1q_f32(_c2+4));
            
            // FIR
            data = vmulq_f32(vld1q_dup_f32(&gain), vld1q_f32(input+ii));
            shuf = vcombine_f32(pinn, vget_low_f32(data));
            tmp2 = vmlaq_f32(vmulq_f32(factor0,data),factor1,shuf);
            pinn = vget_high_f32(data);
            
            // D[r] * x
            tmp3 = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2),0),vld1q_f32(_d2));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_low_f32(tmp2),1), vld1q_f32(_d2+4));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),0),vld1q_f32(_d2+8));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),1),vld1q_f32(_d2+12));
            
            // Shift to output and repeat
            tmp2 = vaddq_f32(tmp1,tmp3);
            tmp3 = vcombine_f32(pout, vget_low_f32(tmp2));
            vst1q_f32(output+ii, tmp3);
            pout = vget_high_f32(tmp2);
        }
        vst1_f32(_outs,pout);
        vst1_f32(_inns,pinn);
    } else {
#else
    {
#endif
        output[(size_t)0] = _outs[(size_t)0];
        output[(size_t)1] = _outs[(size_t)1];
        output[(size_t)2] = gain * _b0 * input[0] + _b1 * _inns[(size_t)0] - _a1 * output[0];
        output[(size_t)3] = gain * _b0 * input[1] + _b1 * _inns[(size_t)1] - _a1 * output[1];
        for(size_t ii = 1; ii < size-1; ii++) {
            output[2*ii+2] = gain * (_b0 * input[2*ii  ] + _b1 * input[2*(ii-1)  ]) - _a1 * output[2*ii  ];
            output[2*ii+3] = gain * (_b0 * input[2*ii+1] + _b1 * input[2*(ii-1)+1]) - _a1 * output[2*ii+1];
        }
        _outs[(size_t)0] = gain * (_b0 * input[2*(size-1)  ] + _b1 * input[2*(size-2)  ]) - _a1 * output[2*(size-1)  ];
        _outs[(size_t)1] = gain * (_b0 * input[2*(size-1)+1] + _b1 * input[2*(size-2)+1]) - _a1 * output[2*(size-1)+1];
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
void PoleZeroFIR::trio(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float32x4x3_t data, outr;
        float32x4_t tmp1, tmp2, tmp3, shuf;
        
        float32x4x3_t pinn;
        pinn.val[0] = { 0, 0, 0, _inns[0] };
        pinn.val[1] = { 0, 0, 0, _inns[1] };
        pinn.val[2] = { 0, 0, 0, _inns[2] };
        float pout0 = _outs[0];
        float pout1 = _outs[1];
        float pout2 = _outs[2];
        
        float32x4_t thegain = vld1q_dup_f32(&gain);
        float32x4_t factor0 = vld1q_dup_f32(&_b0);
        float32x4_t factor1 = vld1q_dup_f32(&_b1);
        for(size_t ii = 0; ii < 3*size; ii += 12) {
            data = vld3q_f32(input+ii);
            data.val[0] = vmulq_f32(thegain,data.val[0]);
            data.val[1] = vmulq_f32(thegain,data.val[1]);
            data.val[2] = vmulq_f32(thegain,data.val[2]);
            
            // Channel 0
            // C[r] * y
            tmp1 = vmulq_f32(vld1q_dup_f32(&pout0),vld1q_f32(_c1));
            
            // FIR
            shuf = vextq_f32(pinn.val[0], data.val[0], 3);
            tmp2 = vmlaq_f32(vmulq_f32(factor0,data.val[0]),factor1,shuf);
            
            // D[r] * x
            tmp3 = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2),0),vld1q_f32(_d1));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_low_f32(tmp2),1), vld1q_f32(_d1+4));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),0),vld1q_f32(_d1+8));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),1),vld1q_f32(_d1+12));
            
            // Shift to output and repeat
            tmp2 = vaddq_f32(tmp1,tmp3);
            tmp3 = vextq_f32(vld1q_dup_f32(&pout0), tmp2, 3);
            outr.val[0] = tmp3;
            pout0 = tmp2[3];
            
            // Channel 1
            // C[r] * y
            tmp1 = vmulq_f32(vld1q_dup_f32(&pout1),vld1q_f32(_c1));
            
            // FIR
            shuf = vextq_f32(pinn.val[1], data.val[1], 3);
            tmp2 = vmlaq_f32(vmulq_f32(factor0,data.val[1]),factor1,shuf);
            
            // D[r] * x
            tmp3 = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2),0),vld1q_f32(_d1));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_low_f32(tmp2),1), vld1q_f32(_d1+4));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),0),vld1q_f32(_d1+8));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),1),vld1q_f32(_d1+12));
            
            // Shift to output and repeat
            tmp2 = vaddq_f32(tmp1,tmp3);
            tmp3 = vextq_f32(vld1q_dup_f32(&pout1), tmp2, 3);
            outr.val[1] = tmp3;
            pout1 = tmp2[3];
            
            // Channel 2
            // C[r] * y
            tmp1 = vmulq_f32(vld1q_dup_f32(&pout2),vld1q_f32(_c1));
            
            // FIR
            shuf = vextq_f32(pinn.val[2], data.val[2], 3);
            tmp2 = vmlaq_f32(vmulq_f32(factor0,data.val[2]),factor1,shuf);
            
            // D[r] * x
            tmp3 = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2),0),vld1q_f32(_d1));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_low_f32(tmp2),1), vld1q_f32(_d1+4));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),0),vld1q_f32(_d1+8));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),1),vld1q_f32(_d1+12));
            
            // Shift to output and repeat
            tmp2 = vaddq_f32(tmp1,tmp3);
            tmp3 = vextq_f32(vld1q_dup_f32(&pout2), tmp2, 3);
            outr.val[2] = tmp3;
            pout2 = tmp2[3];
            
            pinn = data;
            vst3q_f32(output+ii, outr);
        }
        
        _outs[2] = pout2;
        _outs[1] = pout1;
        _outs[0] = pout0;
        _inns[2] = data.val[2][3];
        _inns[1] = data.val[1][3];
        _inns[0] = data.val[0][3];
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
void PoleZeroFIR::quad(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        __m128 pout = _mm_load_ps(_outs);
        __m128 pinn = _mm_load_ps(_inns);
        __m128 data, temp;
        
        __m128 factor0 = _mm_set1_ps(_b0);
        __m128 factor1 = _mm_set1_ps(_b1);
        for(size_t ii = 0; ii < 4*size; ii += 4) {
            data = _mm_mul_ps(_mm_set1_ps(gain), _mm_loadu_ps(input+ii));
            temp = _mm_fmadd_ps(factor0,data,_mm_mul_ps(factor1,pinn));
            temp = _mm_fmadd_ps(_mm_set1_ps(-_a1),pout,temp);
            _mm_storeu_ps(output+ii,pout);
            pinn = data;
            pout = temp;
        }
        
        _mm_store_ps(_inns,pinn);
        _mm_store_ps(_outs,pout);
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float32x4_t pout = vld1q_f32(_outs);
        float32x4_t pinn = vld1q_f32(_inns);
        float32x4_t data, temp;
        
        float32x4_t thegain = vld1q_dup_f32(&gain);
        float32x4_t factor0 = vld1q_dup_f32(&_b0);
        float32x4_t factor1 = vld1q_dup_f32(&_b1);
        for(size_t ii = 0; ii < 4*size; ii += 4) {
            data = vmulq_f32(thegain, vld1q_f32(input+ii));
            temp = vmlaq_f32(vmulq_f32(factor0,data),factor1,pinn);
            temp = vmlaq_f32(temp,vld1q_dup_f32(_c1),pout);
            vst1q_f32(output+ii,pout);
            pinn = data;
            pout = temp;
        }
        
        vst1q_f32(_inns,pinn);
        vst1q_f32(_outs,pout);
    } else {
#else
    {
#endif
        for(size_t ii = 0; ii < 4; ii++) {
            output[ii] = _outs[ii];
        }
        for(size_t ii = 0; ii < 4; ii++) {
            output[4+ii] = gain * _b0 * input[ii] + _b1 * _inns[ii] - _a1 * output[ii];
        }
        for(size_t ii = 1; ii < size-1; ii++) {
            for(int jj = 0; jj < 4; jj++) {
                output[4*(ii+1)+jj] = gain * (_b0 * input[4*ii+jj] + _b1 * input[4*(ii-1)+jj]) - _a1 * output[4*ii+jj];
            }
        }
        for(size_t ii = 0; ii < 4; ii++) {
            _outs[ii] = gain * (_b0 * input[4*(size-1)+ii] + _b1 * input[4*(size-2)+ii]) - _a1 * output[4*(size-1)+ii];
            _inns[ii] = gain * input[4*(size-1)+ii];
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
void PoleZeroFIR::quart(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        __m128 pout1 = _mm_load_ps(_outs);
        __m128 pout2 = _mm_load_ps(_outs+4);
        __m128 pinn1 = _mm_load_ps(_inns);
        __m128 pinn2 = _mm_load_ps(_inns+4);
        __m128 data, temp;
        
        __m128 factor0 = _mm_set1_ps(_b0);
        __m128 factor1 = _mm_set1_ps(_b1);
        for(size_t ii = 0; ii < 8*size; ii += 8) {
            data = _mm_mul_ps(_mm_set1_ps(gain), _mm_loadu_ps(input+ii));
            temp = _mm_fmadd_ps(factor0,data,_mm_mul_ps(factor1,pinn1));
            temp = _mm_fmadd_ps(_mm_set1_ps(-_a1),pout1,temp);
            _mm_storeu_ps(output+ii,pout1);
            pinn1 = data;
            pout1 = temp;
            
            data = _mm_mul_ps(_mm_set1_ps(gain), _mm_loadu_ps(input+ii+4));
            temp = _mm_fmadd_ps(factor0,data,_mm_mul_ps(factor1,pinn2));
            temp = _mm_fmadd_ps(_mm_set1_ps(-_a1),pout2,temp);
            _mm_storeu_ps(output+ii+4,pout2);
            pinn2 = data;
            pout2 = temp;
        }
        
        _mm_store_ps(_inns,pinn1);
        _mm_store_ps(_inns+4,pinn2);
        _mm_store_ps(_outs,pout1);
        _mm_store_ps(_outs+4,pout2);
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float32x4_t pout1 = vld1q_f32(_outs);
        float32x4_t pout2 = vld1q_f32(_outs+4);
        float32x4_t pinn1 = vld1q_f32(_inns);
        float32x4_t pinn2 = vld1q_f32(_inns+4);
        float32x4_t data, temp;
        
        float32x4_t thegain = vld1q_dup_f32(&gain);
        float32x4_t factor0 = vld1q_dup_f32(&_b0);
        float32x4_t factor1 = vld1q_dup_f32(&_b1);
        for(size_t ii = 0; ii < 8*size; ii += 8) {
            data = vmulq_f32(thegain, vld1q_f32(input+ii));
            temp = vmlaq_f32(vmulq_f32(factor0,data),factor1,pinn1);
            temp = vmlaq_f32(temp,vld1q_dup_f32(_c1),pout1);
            vst1q_f32(output+ii,pout1);
            pinn1 = data;
            pout1 = temp;
            
            data = vmulq_f32(thegain, vld1q_f32(input+ii+4));
            temp = vmlaq_f32(vmulq_f32(factor0,data),factor1,pinn2);
            temp = vmlaq_f32(temp,vld1q_dup_f32(_c1),pout2);
            vst1q_f32(output+ii+4,pout2);
            pinn2 = data;
            pout2 = temp;
        }
        
        vst1q_f32(_inns,pinn1);
        vst1q_f32(_inns+4,pinn2);
        vst1q_f32(_outs,pout1);
        vst1q_f32(_outs+4,pout2);
    } else {
#else
    {
#endif
        for(size_t ii = 0; ii < 8; ii++) {
            output[ii] = _outs[ii];
        }
        for(size_t ii = 0; ii < 8; ii++) {
            output[8+ii] = gain * _b0 * input[ii] + _b1 * _inns[ii] - _a1 * output[ii];
        }
        for(size_t ii = 1; ii < size-1; ii++) {
            for(int jj = 0; jj < 8; jj++) {
                output[8*(ii+1)+jj] = gain * (_b0 * input[8*ii+jj] + _b1 * input[8*(ii-1)+jj]) - _a1 * output[8*ii+jj];
            }
        }
        for(size_t ii = 0; ii < 8; ii++) {
            _outs[ii] = gain * (_b0 * input[8*(size-1)+ii] + _b1 * input[8*(size-2)+ii]) - _a1 * output[8*(size-1)+ii];
            _inns[ii] = gain * input[8*(size-1)+ii];
        }
    }
}

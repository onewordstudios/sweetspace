//
//  CUFIRFilter.cpp
//  Cornell University Game Library (CUGL)
//
//  This class is represents a finite impulse response filter. It is a
//  general purpose filter that allows an arbitrary number of coefficients.
//  It is significantly faster than the general purpose IIR filter when no
//  feedback terms are required.  With that said, for less than 3-order FIR
//  filters, you should use one of the more specific filters for performance
//  reasons.
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
///Users/wmwhite/Developer/CUGL/dsp
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
#include <cugl/math/dsp/CUFIRFilter.h>
#include <cugl/util/CUDebug.h>
#include "cuDSP128.inl"

using namespace cugl;
using namespace cugl::dsp;

/** Whether to use a vectorization algorithm */
bool FIRFilter::VECTORIZE = true;

#pragma mark Constructors

/**
 * Creates a zero-order pass-through filter for a single channel.
 */
FIRFilter::FIRFilter() :
_b0(1),
_channels(1) {
    _bval.reset(0, 16);
    _inns.reset(0, 16);
    reset();
}

/**
 * Creates a zero-order pass-through filter for the given number of channels.
 *
 * @param channels  The number of channels
 */
FIRFilter::FIRFilter(unsigned channels) :
_b0(1) {
    _channels = channels;
    _bval.reset(0, 16);
    _inns.reset(0, 16);
    reset();
}

/**
 * Creates an IIR filter with the given coefficients and number of channels.
 *
 * This filter implements the standard difference equation:
 *
 *      y[n] = b[0]*x[n] + ... + b[nb]*x[n-nb]
 *
 * where y is the output and x in the input.
 *
 * @param channels  The number of channels
 * @param bvals     The upper coefficients
 */
FIRFilter::FIRFilter(unsigned channels, const std::vector<float> &bvals) :
_b0(1) {
    _channels = channels;
    setBCoeff(bvals);
}

/**
 * Creates a copy of the IIR filter.
 *
 * @param filter    The filter to copy
 */
FIRFilter::FIRFilter(const FIRFilter& copy) {
    _b0 = copy._b0;
    _channels = copy._channels;
    _bval = copy._bval;
    _inns = copy._inns;
}

/**
 * Creates and IIR filter with  the resources of the original.
 *
 * @param filter    The filter to acquire
 */
FIRFilter::FIRFilter(FIRFilter&& filter) {
    _b0 = filter._b0;
    _channels = filter._channels;
    _bval = std::move(filter._bval);
    _inns = std::move(filter._inns);
}

/**
 * Destroys the filter, releasing all resources.
 */
FIRFilter::~FIRFilter() {}

/**
 * Resets the caching data structures for this filter
 *
 * This must be called if the number of channels or coefficients change.
 */
void FIRFilter::reset() {
    _inns.reset(_bval.size()*_channels,16);
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
void FIRFilter::setChannels(unsigned channels) {
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
void FIRFilter::setCoeff(const std::vector<float> &bvals, const std::vector<float> &avals) {
    size_t bsize = bvals.size() > 0 ? bvals.size()-1 : 0;
    _bval.reset(bsize,16);

    // Only look at first a-coefficient
    float a0 = avals.size() == 0 ? 1.0f : avals[0];

    // Upper coefficients are in reverse order
    _b0 = bvals.size() == 0 ? 0.0f : bvals[0]/a0;
    for(int ii = 0; ii < bsize; ii++) {
        _bval[bsize-ii-1] = bvals[ii+1]/a0;
    }
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
const std::vector<float> FIRFilter::getBCoeff() const {
    std::vector<float> result;
    result.push_back(_b0);
    for(size_t ii = 0; ii < _bval.size(); ii++) {
        result.push_back(_bval[ii]);
    }
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
const std::vector<float> FIRFilter::getACoeff() const {
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
 *    y[n] = b[0]*x[n] + ... + b[nb]*x[n-nb]
 *
 * where y is the output and x in the input.
 *
 * @param bvals The upper coefficients
 */
void FIRFilter::setBCoeff(const std::vector<float> &bvals) {
    size_t bsize = bvals.size() > 0 ? bvals.size()-1 : 0;
    _bval.reset(bsize,16);
    
    // Upper coefficients are in forward order
    _b0 = bvals.size() == 0 ? 0.0f : bvals[0];
    for(size_t ii = 0; ii < bsize; ii++) {
        _bval[ii] = bvals[ii+1];
    }
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
void FIRFilter::step(float gain, float* input, float* output) {
    size_t bsize = _bval.size();

    for(size_t ckk = 0; ckk < _channels; ckk++) {
        float temp = gain * _b0 * input[ckk];
        for(size_t bjj = 0; bjj < bsize; bjj++) {
            temp += _bval[bjj]*_inns[bjj*_channels+ckk];
        }
        output[ckk] = temp;
    }
    
    for(size_t bjj = 0; bjj < _channels*(bsize-1); bjj++) {
        _inns[bjj] = _inns[bjj+_channels];
    }
    for(size_t ckk = 0; ckk < _channels; ckk++) {
        _inns[ckk+_channels*(bsize-1)] = gain * input[ckk];
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
void FIRFilter::calculate(float gain,float* input, float* output, size_t size) {
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
            step(gain,input+ii*_channels,output+ii*_channels);
        }
    }
}

/**
 * Clears the filter buffer of any delayed outputs or cached inputs
 */
void FIRFilter::clear() {
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
size_t FIRFilter::flush(float* output) {
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
void FIRFilter::stride(float gain, float* input, float* output, size_t size, unsigned channel) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        unsigned stride = _channels;
        size_t bsize = _bval.size();

        __m128 temp, base;
        base = _mm_setzero_ps();
        for(size_t ii = 0; ii < size; ii += 4) {

            size_t bjj;
            temp = _mm_mul_ps(_mm_set1_ps(gain * _b0), _mm_skipload_ps(input+ii*stride,stride));

            // Loop through coefficients, handling hand-off from buffer to input
            for(bjj = 0; ii+bjj+3 < bsize; bjj++) {
                base = _mm_skipload_ps(_inns+(ii+bjj)*stride+channel,stride);
                temp = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj]),base,temp);
            }
            for(; ii+bjj < bsize; bjj++) {
                size_t brem = bsize-ii-bjj;
                for(size_t dkk = 0; dkk < brem; dkk++) {
                    base[dkk] = _inns[(ii+bjj+dkk)*stride+channel];
                }
                for(size_t dkk = 0; dkk < 4-brem; dkk++) {
                    base[dkk+brem] = gain * input[(ii+bjj+dkk+brem-bsize)*stride];
                }
                temp = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj]),base,temp);
            }
            for(; bjj < bsize; bjj++) {
                base = _mm_mul_ps(_mm_set1_ps(gain), _mm_skipload_ps(input+(ii+bjj-bsize)*stride,stride));
                temp = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj]),base,temp);
            }
            _mm_skipstore_ps(output+ii*stride,temp,stride);
        }
        
        for(size_t kk = 0; kk < bsize; kk++) {
            _inns[kk*stride+channel] = gain * input[(size-bsize+kk)*stride];
        }
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        unsigned stride = _channels;
        size_t bsize = _bval.size();
        
        float32x4_t temp, base;
        base = {0,0,0,0};
        float32x4_t thegain = vld1q_dup_f32(&gain);
        for(size_t ii = 0; ii < size; ii += 4) {
            
            size_t bjj;
            temp = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), vld1q_skip_f32(input+ii*stride,stride)));
            
            // Loop through coefficients, handling hand-off from buffer to input
            for(bjj = 0; ii+bjj+3 < bsize; bjj++) {
                base = vld1q_skip_f32(_inns+(ii+bjj)*stride+channel,stride);
                temp = vmlaq_f32(temp, vld1q_dup_f32(_bval+bjj), base);
            }
            for(; ii+bjj < bsize; bjj++) {
                size_t brem = bsize-ii-bjj;
                for(size_t dkk = 0; dkk < brem; dkk++) {
                    base[dkk] = _inns[(ii+bjj+dkk)*stride+channel];
                }
                for(size_t dkk = 0; dkk < 4-brem; dkk++) {
                    base[dkk+brem] = gain * input[(ii+bjj+dkk+brem-bsize)*stride];
                }
                temp = vmlaq_f32(temp, vld1q_dup_f32(_bval+bjj), base);
            }
            for(; bjj < bsize; bjj++) {
                base = vmulq_f32(thegain, vld1q_skip_f32(input+(ii+bjj-bsize)*stride,stride));
                temp = vmlaq_f32(temp, vld1q_dup_f32(_bval+bjj), base);
            }
            vst1q_skip_f32(output+ii*stride,temp,stride);
        }
        
        for(size_t kk = 0; kk < bsize; kk++) {
            _inns[kk*stride+channel] = gain * input[(size-bsize+kk)*stride];
        }
    } else {
#else
    {
#endif
        unsigned stride = _channels;
        size_t bsize = _bval.size();
        
        for(size_t ii = 0; ii < size; ii++) {
            
            float temp = gain * _b0 * input[ii*stride];
            size_t bjj;
            for(bjj = 0; ii+bjj < bsize; bjj++) {
                temp += _bval[bjj]*_inns[(ii+bjj)*stride+channel];
            }
            for(; bjj < bsize; bjj++) {
                temp += gain * _bval[bjj]*input[(ii+bjj-bsize)*stride];
            }
            output[ii*stride] = temp;
            
        }
        
        for(size_t bjj = 0; bjj < bsize; bjj++) {
            _inns[bjj*stride+channel] = gain * input[(size-bsize+bjj)*stride];
        }
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
void FIRFilter::single(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        __m128 temp, base;
        size_t bsize = _bval.size();
        
        for(size_t ii = 0; ii < size; ii += 4) {
            
            size_t bjj;
            temp = _mm_mul_ps(_mm_set1_ps(gain * _b0), _mm_loadu_ps(input+ii));
            base = _mm_loadu_ps(input+ii);
            
            // Loop through coefficients, handling hand-off from buffer to input
            for(bjj = 0; ii+bjj+3 < bsize; bjj++) {
                temp = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj]),_mm_loadu_ps(_inns+ii+bjj),temp);
            }
            for(; ii+bjj < bsize; bjj++) {
                size_t brem = bsize-ii-bjj;
                for(size_t dkk = 0; dkk < brem; dkk++) {
                    base[dkk] = _inns[ii+bjj+dkk];
                }
                for(size_t dkk = 0; dkk < 4-brem; dkk++) {
                    base[dkk+brem] = gain * input[ii+bjj+dkk-bsize+brem];
                }
                temp = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj]),base,temp);
            }
            for(; bjj < bsize; bjj++) {
                base = _mm_mul_ps(_mm_set1_ps(gain), _mm_loadu_ps(input+ii+bjj-bsize));
                temp = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj]),base,temp);
            }
            
            _mm_storeu_ps(output+ii,temp);
        }
        
        size_t kk;
        for(kk = 0; kk+3 < bsize; kk += 4) {
            _mm_store_ps(_inns+kk,_mm_mul_ps(_mm_set1_ps(gain), _mm_loadu_ps(input+size-bsize+kk)));
        }
        for(; kk < bsize; kk++) {
            _inns[kk] = gain * input[size-bsize+kk];
        }
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float32x4_t temp, base;
        size_t bsize = _bval.size();
        
        float32x4_t thegain = vld1q_dup_f32(&gain);
        for(size_t ii = 0; ii < size; ii += 4) {
            
            size_t bjj;
            base = vld1q_f32(input+ii);
            temp = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), base));
            
            // Loop through coefficients, handling hand-off from buffer to input
            for(bjj = 0; ii+bjj+3 < bsize; bjj++) {
                temp = vmlaq_f32(temp, vld1q_dup_f32(_bval+bjj), vld1q_f32(_inns+ii+bjj));
            }
            for(; ii+bjj < bsize; bjj++) {
                size_t brem = bsize-ii-bjj;
                for(size_t dkk = 0; dkk < brem; dkk++) {
                    base[dkk] = _inns[ii+bjj+dkk];
                }
                for(size_t dkk = 0; dkk < 4-brem; dkk++) {
                    base[dkk+brem] = gain * input[ii+bjj+dkk-bsize+brem];
                }
                temp = vmlaq_f32(temp, vld1q_dup_f32(_bval+bjj), base);
            }
            for(; bjj < bsize; bjj++) {
                base = vmulq_f32(thegain, vld1q_f32(input+ii+bjj-bsize));
                temp = vmlaq_f32(temp, vld1q_dup_f32(_bval+bjj), base);
            }
            
            vst1q_f32(output+ii,temp);
        }
        
        size_t kk;
        for(kk = 0; kk+3 < bsize; kk += 4) {
            vst1q_f32(_inns+kk,vmulq_f32(thegain, vld1q_f32(input+size-bsize+kk)));
        }
        for(; kk < bsize; kk++) {
            _inns[kk] = gain * input[size-bsize+kk];
        }
    } else {
#else
    {
#endif
        size_t bsize = _bval.size();
        
        for(size_t ii = 0; ii < size; ii++) {
            
            float temp = gain * _b0 * input[ii];
            size_t bjj;
            for(bjj = 0; ii+bjj < bsize; bjj++) {
                temp += _bval[bjj]*_inns[ii+bjj];
            }
            for(; bjj < bsize; bjj++) {
                temp += gain * _bval[bjj]*input[ii+bjj-bsize];
            }
            output[ii] = temp;
            
        }
        
        for(size_t bjj = 0; bjj < bsize; bjj++) {
            _inns[bjj] = gain * input[size-bsize+bjj];
        }
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
void FIRFilter::dual(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        __m128 temp, base;
        size_t bsize = _bval.size();
        
        base = _mm_setzero_ps();
        for(size_t ii = 0; ii < 2*size; ii += 4) {

            size_t bjj;
            temp = _mm_mul_ps(_mm_set1_ps(gain * _b0), _mm_loadu_ps(input+ii));
            
            // Loop through coefficients, handling hand-off from buffer to input
            for(bjj = 0; ii+bjj+3 < 2*bsize; bjj += 2) {
                base = _mm_loadu_ps(_inns+ii+bjj);
                temp = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj/2]),base,temp);
            }
            
            for(; ii+bjj < 2*bsize; bjj += 2) {
                size_t brem = 2*bsize-ii-bjj;
                for(size_t dkk = 0; dkk < brem; dkk++) {
                    base[dkk] = _inns[ii+bjj+dkk];
                }
                for(size_t dkk = 0; dkk < 4-brem; dkk++) {
                    base[dkk+brem] = gain * input[ii+bjj+dkk-2*bsize+brem];
                }
                temp = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj/2]),base,temp);
            }
            for(; bjj < 2*bsize; bjj += 2) {
                base = _mm_mul_ps(_mm_set1_ps(gain), _mm_loadu_ps(input+ii+bjj-2*bsize));
                temp = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj/2]),base,temp);
            }
            
            _mm_storeu_ps(output+ii,temp);
        }
        
        size_t kk;
        for(kk = 0; kk+3 < 2*bsize; kk += 4) {
            _mm_store_ps(_inns+kk,_mm_mul_ps(_mm_set1_ps(gain), _mm_loadu_ps(input+2*(size-bsize)+kk)));
        }
        for(; kk < 2*bsize; kk++) {
            _inns[kk] = gain * input[2*(size-bsize)+kk];
        }
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float32x4_t temp, base;
        size_t bsize = _bval.size();
        
        base = {0,0,0,0};
        float32x4_t thegain = vld1q_dup_f32(&gain);
        for(size_t ii = 0; ii < 2*size; ii += 4) {
            size_t bjj;
            temp = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), vld1q_f32(input+ii)));
            
            // Loop through coefficients, handling hand-off from buffer to input
            for(bjj = 0; ii+bjj+3 < 2*bsize; bjj += 2) {
                base = vld1q_f32(_inns+ii+bjj);
                temp = vmlaq_f32(temp, vld1q_dup_f32(_bval+(bjj/2)), base);
            }
            
            for(; ii+bjj < 2*bsize; bjj += 2) {
                size_t brem = 2*bsize-ii-bjj;
                for(size_t dkk = 0; dkk < brem; dkk++) {
                    base[dkk] = _inns[ii+bjj+dkk];
                }
                for(size_t dkk = 0; dkk < 4-brem; dkk++) {
                    base[dkk+brem] = gain * input[ii+bjj+dkk-2*bsize+brem];
                }
                temp = vmlaq_f32(temp, vld1q_dup_f32(_bval+(bjj/2)), base);
            }
            for(; bjj < 2*bsize; bjj += 2) {
                base = vmulq_f32(thegain, vld1q_f32(input+ii+bjj-2*bsize));
                temp = vmlaq_f32(temp, vld1q_dup_f32(_bval+(bjj/2)), base);
            }
            
            vst1q_f32(output+ii,temp);
        }
        
        size_t kk;
        for(kk = 0; kk+3 < 2*bsize; kk += 4) {
            vst1q_f32(_inns+kk, vmulq_f32(thegain, vld1q_f32(input+2*(size-bsize)+kk)));
        }
        for(; kk < 2*bsize; kk++) {
            _inns[kk] = gain * input[2*(size-bsize)+kk];
        }
    } else {
#else
    {
#endif
        size_t bsize = _bval.size();
        
        for(size_t ii = 0; ii < size; ii++) {
            size_t bjj;
            float temp;
            
            temp = gain * _b0 * input[2*ii+0];
            for(bjj = 0; ii+bjj < bsize; bjj++) {
                temp += _bval[bjj]*_inns[2*(ii+bjj)+0];
            }
            for(; bjj < bsize; bjj++) {
                temp += gain * _bval[bjj]*input[2*(ii+bjj-bsize)+0];
            }
            output[2*ii+0] = temp;
            
            temp = gain * _b0 * input[2*ii+1];
            for(bjj = 0; ii+bjj < bsize; bjj++) {
                temp += _bval[bjj]*_inns[2*(ii+bjj)+1];
            }
            for(; bjj < bsize; bjj++) {
                temp += gain * _bval[bjj]*input[2*(ii+bjj-bsize)+1];
            }
            output[2*ii+1] = temp;
            
        }
        
        for(size_t bjj = 0; bjj < bsize; bjj++) {
            _inns[2*bjj+0] = gain * input[2*(size-bsize+bjj)+0];
            _inns[2*bjj+1] = gain * input[2*(size-bsize+bjj)+1];
        }
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
void FIRFilter::trio(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float32x4x3_t temp, base;
        size_t bsize = _bval.size();
        
        float32x4_t thegain = vld1q_dup_f32(&gain);
        for(size_t ii = 0; ii < 3*size; ii += 12) {
            size_t bjj;
            base = vld3q_f32(input+ii);
            temp.val[0] = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), base.val[0]));
            temp.val[1] = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), base.val[1]));
            temp.val[2] = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), base.val[2]));

            // Loop through coefficients, handling hand-off from buffer to input
            for(bjj = 0; ii+bjj+11 < 3*bsize; bjj += 3) {
                base = vld3q_f32(_inns+ii+bjj);
                temp.val[0] = vmlaq_f32(temp.val[0], vld1q_dup_f32(_bval+(bjj/3)), base.val[0]);
                temp.val[1] = vmlaq_f32(temp.val[1], vld1q_dup_f32(_bval+(bjj/3)), base.val[1]);
                temp.val[2] = vmlaq_f32(temp.val[2], vld1q_dup_f32(_bval+(bjj/3)), base.val[2]);
            }
            for(; ii+bjj < 3*bsize; bjj+=3) {
                size_t brem = 3*bsize-ii-bjj;
                for(size_t dkk = 0; dkk < brem; dkk++) {
                    base.val[dkk % 3][(dkk)/3] = _inns[ii+bjj+dkk];
                }
                for(size_t dkk = 0; dkk < 12-brem; dkk++) {
                    base.val[(dkk+brem) % 3][(dkk+brem)/3] = gain * input[ii+bjj+dkk-3*bsize+brem];
                }
                temp.val[0] = vmlaq_f32(temp.val[0], vld1q_dup_f32(_bval+(bjj/3)), base.val[0]);
                temp.val[1] = vmlaq_f32(temp.val[1], vld1q_dup_f32(_bval+(bjj/3)), base.val[1]);
                temp.val[2] = vmlaq_f32(temp.val[2], vld1q_dup_f32(_bval+(bjj/3)), base.val[2]);
            }
            for(; bjj < 3*bsize; bjj+=3) {
                base = vld3q_f32(input+ii+bjj-3*bsize);
                base.val[0] = vmulq_f32(thegain, base.val[0]);
                base.val[1] = vmulq_f32(thegain, base.val[1]);
                base.val[2] = vmulq_f32(thegain, base.val[2]);
                temp.val[0] = vmlaq_f32(temp.val[0], vld1q_dup_f32(_bval+(bjj/3)), base.val[0]);
                temp.val[1] = vmlaq_f32(temp.val[1], vld1q_dup_f32(_bval+(bjj/3)), base.val[1]);
                temp.val[2] = vmlaq_f32(temp.val[2], vld1q_dup_f32(_bval+(bjj/3)), base.val[2]);
            }
            
            vst3q_f32(output+ii,temp);
        }
        
        size_t kk;
        for(kk = 0; kk+3 < 3*bsize; kk += 4) {
            vst1q_f32(_inns+kk,vmulq_f32(thegain, vld1q_f32(input+3*(size-bsize)+kk)));
        }
        for(; kk < 3*bsize; kk++) {
            _inns[kk] = gain * input[3*(size-bsize)+kk];
        }
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
void FIRFilter::quad(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
       __m128 temp;
        size_t bsize = _bval.size();
        
        for(size_t ii = 0; ii < 4*size; ii += 4) {

            size_t bjj;
            temp = _mm_mul_ps(_mm_set1_ps(gain * _b0), _mm_loadu_ps(input+ii));
            
            for(bjj = 0; ii+bjj < 4*bsize; bjj += 4) {
                temp = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj/4]),_mm_loadu_ps(_inns+ii+bjj),temp);
            }
            for(; bjj < 4*bsize; bjj += 4) {
                temp = _mm_fmadd_ps(_mm_set1_ps(gain * _bval[bjj/4]),_mm_loadu_ps(input+ii+bjj-4*bsize),temp);
            }
            _mm_storeu_ps(output+ii,temp);
        }
        
        size_t kk;
        for(kk = 0; kk+3 < 4*bsize; kk += 4) {
            _mm_store_ps(_inns+kk,_mm_mul_ps(_mm_set1_ps(gain), _mm_loadu_ps(input+4*(size-bsize)+kk)));
        }
        for(; kk < 4*bsize; kk++) {
            _inns[kk] = gain*input[4*(size-bsize)+kk];
        }
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float32x4_t temp, base;
        size_t bsize = _bval.size();
        
        float32x4_t thegain = vld1q_dup_f32(&gain);
        for(size_t ii = 0; ii < 4*size; ii += 4) {
            
            size_t bjj;
            temp = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), vld1q_f32(input+ii)));

            for(bjj = 0; ii+bjj < 4*bsize; bjj += 4) {
                temp = vmlaq_f32(temp, vld1q_dup_f32(_bval+(bjj/4)), vld1q_f32(_inns+ii+bjj));
            }
            for(; bjj < 4*bsize; bjj += 4) {
                base = vmulq_f32(thegain, vld1q_f32(input+ii+bjj-4*bsize));
                temp = vmlaq_f32(temp, vld1q_dup_f32(_bval+(bjj/4)), base);
            }
            vst1q_f32(output+ii,temp);
        }
        
        size_t kk;
        for(kk = 0; kk+3 < 4*bsize; kk += 4) {
            vst1q_f32(_inns+kk,vmulq_f32(thegain, vld1q_f32(input+4*(size-bsize)+kk)));
        }
        for(; kk < 4*bsize; kk++) {
            _inns[kk] = gain*input[4*(size-bsize)+kk];
        }
    } else {
#else
    {
#endif
        size_t bsize = _bval.size();
        
        for(size_t ii = 0; ii < size; ii++) {
            for(size_t ckk = 0; ckk < 4; ckk++) {
                
                size_t bjj;
                float temp = gain * _b0 * input[4*ii+ckk];
                for(bjj = 0; ii+bjj < bsize; bjj++) {
                    temp += _bval[bjj]*_inns[4*(ii+bjj)+ckk];
                }
                for(; bjj < bsize; bjj++) {
                    temp += gain * _bval[bjj]*input[4*(ii+bjj-bsize)+ckk];
                }
                output[4*ii+ckk] = temp;
                
            }
        }
        
        for(size_t bjj = 0; bjj < bsize; bjj++) {
            for(size_t ckk = 0; ckk < 4; ckk++) {
                _inns[4*bjj+ckk] = gain * input[4*(size-bsize+bjj)+ckk];
            }
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
void FIRFilter::quart(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        __m128 temp;
        size_t bsize = _bval.size();
        
        for(size_t ii = 0; ii < 8*size; ii += 4) {

            size_t bjj;
            temp = _mm_mul_ps(_mm_set1_ps(gain * _b0), _mm_loadu_ps(input+ii));
            for(bjj = 0; ii+bjj < 8*bsize; bjj += 8) {
                temp = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj/8]),_mm_loadu_ps(_inns+ii+bjj),temp);
            }
            for(; bjj < 8*bsize; bjj += 8) {
                temp = _mm_fmadd_ps(_mm_set1_ps(gain*_bval[bjj/8]),_mm_loadu_ps(input+ii+bjj-8*bsize),temp);
            }
            _mm_storeu_ps(output+ii,temp);
        }
        
        size_t kk;
        for(kk = 0; kk+3 < 8*bsize; kk += 4) {
            _mm_store_ps(_inns+kk,_mm_mul_ps(_mm_set1_ps(gain), _mm_loadu_ps(input+8*(size-bsize)+kk)));
        }
        for(; kk < 8*bsize; kk++) {
            _inns[kk] = input[8*(size-bsize)+kk];
        }
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float32x4_t temp, base;
        size_t bsize = _bval.size();
        
        float32x4_t thegain = vld1q_dup_f32(&gain);
        for(size_t ii = 0; ii < 8*size; ii += 4) {
            size_t bjj;
            temp = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), vld1q_f32(input+ii)));
            for(bjj = 0; ii+bjj < 8*bsize; bjj += 8) {
                temp = vmlaq_f32(temp, vld1q_dup_f32(_bval+(bjj/8)), vld1q_f32(_inns+ii+bjj));
            }
            for(; bjj < 8*bsize; bjj += 8) {
                base = vmulq_f32(thegain, vld1q_f32(input+ii+bjj-8*bsize));
                temp = vmlaq_f32(temp, vld1q_dup_f32(_bval+(bjj/8)), base);
            }
            vst1q_f32(output+ii,temp);
        }
        
        size_t kk;
        for(kk = 0; kk+3 < 8*bsize; kk += 4) {
            vst1q_f32(_inns+kk,vmulq_f32(thegain, vld1q_f32(input+8*(size-bsize)+kk)));
        }
        for(; kk < 8*bsize; kk++) {
            _inns[kk] = input[8*(size-bsize)+kk];
        }
    } else {
#else
    {
#endif
        size_t bsize = _bval.size();
        
        for(size_t ii = 0; ii < size; ii++) {
            for(size_t ckk = 0; ckk < 8; ckk++) {
                
                size_t bjj;
                float temp = gain * _b0 * input[8*ii+ckk];
                for(bjj = 0; ii+bjj < bsize; bjj++) {
                    temp += _bval[bjj]*_inns[8*(ii+bjj)+ckk];
                }
                for(; bjj < bsize; bjj++) {
                    temp += gain * _bval[bjj]*input[8*(ii+bjj-bsize)+ckk];
                }
                output[8*ii+ckk] = temp;
                
            }
        }
        
        for(size_t bjj = 0; bjj < bsize; bjj++) {
            for(size_t ckk = 0; ckk < 8; ckk++) {
                _inns[8*bjj+ckk] = gain * input[8*(size-bsize+bjj)+ckk];
            }
        }
    }
}


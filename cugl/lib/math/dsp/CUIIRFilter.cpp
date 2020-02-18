//
//  CUIIRFilter.cpp
//  Cornell University Game Library (CUGL)
//
//  This class is represents a infinite impulse response filter. It is a
//  general purpose filter that allows an arbitrary number of coefficients.
//  It should only be used for 3-order or higher IIRs.  In all other cases,
//  you should use one of the specific classes for performance reasons.
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
#include <cugl/math/dsp/CUIIRFilter.h>
#include <cugl/util/CUDebug.h>
#include "cuDSP128.inl"

using namespace cugl;
using namespace cugl::dsp;

/** Whether to use a vectorization algorithm */
bool IIRFilter::VECTORIZE = true;

#pragma mark Vector Support
/**
 * Multiplies a single row a, with the matrix b, storing the result in out.
 *
 * The row is assumed to have length dim, while the matrix is assumed to be
 * square of size dim x dim.  The matrix should be in row major order.
 *
 * @param a     The row on the left
 * @param b     The matrix on the right
 * @param out   The row to store the output
 * @param dim   The row size
 */
static inline void rowMult(float* a, float* b, float* out, size_t dim) {
    for(size_t ii = 0; ii < dim; ii++) {
        float tmp = 0;
        for(size_t jj = 0; jj < dim; jj++) {
            tmp += a[jj]*b[ii+jj*dim];
        }
        out[ii] = tmp;
    }
}

/**
 * Multiplies two matrices (a*b) in row major order, storing the result in out.
 *
 * The matrices are assumed to be square of size dim x dim.
 *
 * @param a     The left matrix
 * @param b     The right matrix
 * @param out   The matrix to store the output
 * @param dim   The matrix dimension
 */
static inline void squareMult(float* a, float* b, float* out, size_t dim) {
    for(size_t ii = 0; ii < dim; ii++) {
        rowMult(a+ii*dim,b,out+ii*dim,dim);
    }
}

/**
 * Copies src into a strided output array dst.
 *
 * The value dim is size of src.  The destination array has a stride of 4, and
 * so it should be 4 times this size.  This function is primarily used to
 * copy a value into the column of a matrix.
 *
 * @param dst   The destination array
 * @param src   The source array
 * @param dim   The source array size
 */
static inline void colcpy(float* dst, float* src, size_t dim) {
    for(size_t ii = 0; ii < dim; ii++) {
        dst[ii*4] = src[dim-1-ii];
    }
}

#pragma mark -
#pragma mark Constructors

/**
 * Creates a zero-order pass-through filter for a single channel.
 */
IIRFilter::IIRFilter() :
_b0(1),
_channels(1) {
    _aval.reset(0, 16);
    _bval.reset(0, 16);
    _outs.reset(0, 16);
    _inns.reset(0, 16);
    reset();
}

/**
 * Creates a zero-order pass-through filter for the given number of channels.
 *
 * @param channels  The number of channels
 */
IIRFilter::IIRFilter(unsigned channels) :
_b0(1) {
    _channels = channels;
    _aval.reset(0, 16);
    _bval.reset(0, 16);
    _outs.reset(0, 16);
    _inns.reset(0, 16);
    reset();
}

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
IIRFilter::IIRFilter(unsigned channels, const std::vector<float> &bvals, const std::vector<float> &avals) :
_b0(1) {
    _channels = channels;
    setCoeff(bvals, avals);
}

/**
 * Creates a copy of the IIR filter.
 *
 * @param filter    The filter to copy
 */
IIRFilter::IIRFilter(const IIRFilter& copy) {
    _b0 = copy._b0;
    _channels = copy._channels;
    std::memcpy(_c1,copy._c1,16*sizeof(float));
    std::memcpy(_c2,copy._c2,16*sizeof(float));
    std::memcpy(_d1,copy._d1,16*sizeof(float));
    std::memcpy(_d2,copy._d2,16*sizeof(float));
    _aval = copy._aval;
    _bval = copy._bval;
    _inns = copy._inns;
    _outs = copy._outs;
}

/**
 * Creates and IIR filter with  the resources of the original.
 *
 * @param filter    The filter to acquire
 */
IIRFilter::IIRFilter(IIRFilter&& filter) {
    _b0 = filter._b0;
    _channels = filter._channels;
    std::memcpy(_c1,filter._c1,16*sizeof(float));
    std::memcpy(_c2,filter._c2,16*sizeof(float));
    std::memcpy(_d1,filter._d1,16*sizeof(float));
    std::memcpy(_d2,filter._d2,16*sizeof(float));
    _aval = std::move(filter._aval);
    _bval = std::move(filter._bval);
    _inns = std::move(filter._inns);
    _outs = std::move(filter._outs);
}

/**
 * Destroys the filter, releasing all resources.
 */
IIRFilter::~IIRFilter() {}

/**
 * Resets the caching data structures for this filter
 *
 * This must be called if the number of channels or coefficients change.
 */
void IIRFilter::reset() {
    size_t asize = _aval.size();
    size_t bsize = _bval.size();

    if (asize > 0) {
        float* u = (float*)malloc(asize*sizeof(float));
        for(size_t ii = 0; ii < asize; ii++) {
            u[ii] = _aval[asize-ii-1];
        }
        float* v = (float*)malloc(asize*sizeof(float));
    
        float* a = (float*)malloc(asize*asize*sizeof(float));
        float* b1 = (float*)malloc(asize*asize*sizeof(float));
        float* b2 = (float*)malloc(asize*asize*sizeof(float));
        std::memset(a,0,asize*asize*sizeof(float));
        std::memset(b1,0,asize*asize*sizeof(float));
        std::memset(b2,0,asize*asize*sizeof(float));

        std::memcpy(a,u,asize*sizeof(float));
        for(size_t ii = 0; ii < asize-1; ii++) {
            a[(ii+1)*asize+ii] = 1.0f;
        }
    
        for(size_t ii = 0; ii < asize; ii++) {
            b1[ii+ii*asize] = 1.0f;
            b2[ii+ii*asize] = 1.0f;
        }
    
        _c1.reset(asize*4,16);
        _c2.reset(asize*8,16);
        rowMult(u,b1,v,asize);
        colcpy(_c1+(size_t)0,v,asize);

        squareMult(b1,a,b2,asize);
        rowMult(u,b2,v,asize);
        colcpy(_c1+(size_t)1,v,asize);

        squareMult(b2,a,b1,asize);
        rowMult(u,b1,v,asize);
        colcpy(_c1+(size_t)2,v,asize);
    
        squareMult(b1,a,b2,asize);
        rowMult(u,b2,v,asize);
        colcpy(_c1+(size_t)3,v,asize);
    
#if defined (CU_MATH_VECTOR_SSE)
        _mm_store_ps(_d1,       _mm_setr_ps(1, _c1[4*asize-4], _c1[4*asize-3], _c1[4*asize-2]));
        _mm_store_ps(_d1+4,     _mm_setr_ps(0,     1,          _c1[4*asize-4], _c1[4*asize-3]));
        _mm_store_ps(_d1+8,     _mm_setr_ps(0,     0,              1,          _c1[4*asize-4]));
        _mm_store_ps(_d1+12,    _mm_setr_ps(0,     0,              0,              1));

        for(size_t ii = 0; ii < asize; ii++) {
            __m128 rows;
            __m128 temp = _mm_load_ps(_c1+ii*4);
            temp = _mm_movelh_ps(temp,_mm_setzero_ps());
            rows = _mm_shuffle_ps(temp,temp,_MM_SHUFFLE(3,1,2,0));
            _mm_store_ps(_c2+ii*8,rows);
            rows = _mm_shuffle_ps(temp,temp,_MM_SHUFFLE(1,3,0,2));
            _mm_store_ps(_c2+ii*8+4,rows);
        }

        _mm_store_ps(_d2,       _mm_setr_ps(1,    0,  _c2[8*asize-8],      0));
        _mm_store_ps(_d2+4,     _mm_setr_ps(0,    1,      0,           _c2[8*asize-3]));
        _mm_store_ps(_d2+8,     _mm_setr_ps(0,    0,      1,               0));
        _mm_store_ps(_d2+12,    _mm_setr_ps(0,    0,      0,               1));
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
        if ( android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
            (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
        {
#endif
            float32x4_t temp;
            temp = {1, _c1[4*asize-4], _c1[4*asize-3], _c1[4*asize-2]};
            vst1q_f32(_d1,    temp);
            temp = {0,     1,          _c1[4*asize-4], _c1[4*asize-3]};
            vst1q_f32(_d1+4 , temp);
            temp = {0,     0,              1,          _c1[4*asize-4]};
            vst1q_f32(_d1+8 , temp);
            temp = {0,     0,              0,            1};
            vst1q_f32(_d1+12, temp);
            
            float32x4_t zeroes = {0,0,0,0};
            for(size_t ii = 0; ii < asize; ii++) {
                float32x4_t temp   = vld1q_f32(_c1+ii*4);
                float32x4x2_t zipd = vzipq_f32(temp, zeroes);
                vst1q_f32(_c2+ii*8, zipd.val[0]);
                temp = vextq_f32(zipd.val[0], zipd.val[0], 3);
                vst1q_f32(_c2+ii*8+4, temp);
            }
            
            temp = {1,    0,  _c2[8*asize-8],      0};
            vst1q_f32(_d2,    temp);
            temp = {0,    1,      0,           _c2[8*asize-3]};
            vst1q_f32(_d2+4 , temp);
            temp = {0,    0,      1,               0};
            vst1q_f32(_d2+8 , temp);
            temp = {0,    0,      0,               1};
            vst1q_f32(_d2+12, temp);
        }
#endif
        free(a);
        free(b1);
        free(b2);
    } else {
        _c1.reset(0,16);
        _c2.reset(0,16);

#if defined (CU_MATH_VECTOR_SSE)
        _mm_store_ps(_d1,       _mm_setr_ps(1, 0, 0, 0));
        _mm_store_ps(_d1+4,     _mm_setr_ps(0, 1, 0, 0));
        _mm_store_ps(_d1+8,     _mm_setr_ps(0, 0, 1, 0));
        _mm_store_ps(_d1+12,    _mm_setr_ps(0, 0, 0, 1));
 
        _mm_store_ps(_d2,       _mm_setr_ps(1, 0, 0, 0));
        _mm_store_ps(_d2+4,     _mm_setr_ps(0, 1, 0, 0));
        _mm_store_ps(_d2+8,     _mm_setr_ps(0, 0, 1, 0));
        _mm_store_ps(_d2+12,    _mm_setr_ps(0, 0, 0, 1));
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
        if ( android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
            (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
        {
#endif
            float32x4_t temp;
            temp = { 1, 0, 0, 0 };
            vst1q_f32(_d1,    temp);
            vst1q_f32(_d2,    temp);
            temp = { 0, 1, 0, 0 };
            vst1q_f32(_d1+4 , temp);
            vst1q_f32(_d2+4 , temp);
            temp = { 0, 0, 1, 0 };
            vst1q_f32(_d1+8 , temp);
            vst1q_f32(_d2+8 , temp);
            temp = { 0, 0, 0, 1 };
            vst1q_f32(_d1+12, temp);
            vst1q_f32(_d2+12, temp);
        }
#endif
    }
    
    _inns.reset(bsize*_channels,16);
    _outs.reset(asize*_channels,16);
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
void IIRFilter::setChannels(unsigned channels) {
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
void IIRFilter::setCoeff(const std::vector<float> &bvals, const std::vector<float> &avals) {
    size_t bsize = bvals.size() > 0 ? bvals.size()-1 : 0;
    size_t asize = avals.size() > 0 ? avals.size()-1 : 0;
    _bval.reset(bsize,16);
    _aval.reset(asize,16);

    // Lower coefficients are in reverse order
    float a0 = avals.size() == 0 ? 1.0f : avals[0];
    for(int ii = 0; ii < asize; ii++) {
        _aval[asize-ii-1] = -avals[ii+1]/a0;
    }

    // Upper coefficients are also in reverse order
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
std::vector<float> IIRFilter::getBCoeff() const {
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
std::vector<float> IIRFilter::getACoeff() const {
    std::vector<float> result;
    result.push_back(1.0f);  // Assume normalization
    for(int ii = (int)_aval.size()-1; ii >= 0; ii--) {
        result.push_back(-_aval[(size_t)ii]);
    }
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
void IIRFilter::setTransfer(const Polynomial& p, const Polynomial& q) {
    size_t bsize = p.degree();
    size_t asize = q.degree();
    _bval.reset(bsize,16);
    _aval.reset(asize,16);
    
    // All coefficients are in polynomials order
    float a0 = q.back();
    for(size_t ii = 0; ii < asize; ii++) {
        _aval[ii] = -q[ii]/a0;
    }
    
    // Upper coefficients are also in reverse order
    _b0 = p.back()/a0;
    for(size_t ii = 0; ii < bsize; ii++) {
        _bval[ii] = p[ii]/a0;
    }
    
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
Polynomial IIRFilter::getNumerator() const {
    Polynomial result(_bval.size());
    for(size_t ii = 0; ii < _bval.size(); ii++) {
        result[ii] = _bval[ii];
    }
    result[_bval.size()] = _b0;
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
Polynomial IIRFilter::getDenominator() const {
    Polynomial result(_aval.size());
    for(size_t ii = 0; ii < _aval.size(); ii++) {
        result[ii] = -_aval[ii];
    }
    result[_aval.size()] = 1.0f;  // Assume normalization
    return result;
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
void IIRFilter::step(float gain, float* input, float* output) {
    size_t asize = _aval.size();
    size_t bsize = _bval.size();
    
    if (asize > 0) {
        for(size_t ckk = 0; ckk < _channels; ckk++) {
            output[ckk] = _outs[ckk];
            float temp = gain * _b0 * input[ckk];
            temp += _aval[(size_t)0] * _outs[ckk];
            for(size_t ajj=1; ajj < asize; ajj++) {
                temp += _aval[ajj]*_outs[_channels*(ajj)+ckk];
                _outs[_channels*(ajj-1)+ckk] = _outs[_channels*(ajj)+ckk];
            }
            for(size_t bjj = 0; bjj < bsize; bjj++) {
                temp += _bval[bjj]*_inns[bjj*_channels+ckk];
            }
            _outs[_channels*(asize-1)+ckk] = temp;
        }
    } else {
        for(size_t ckk=0; ckk < _channels; ckk++) {
            float temp = gain * _b0 * input[ckk];
            for(size_t bjj = 0; bjj < bsize; bjj++) {
                temp += _bval[bjj]*_inns[bjj*_channels+ckk];
            }
            output[ckk] = temp;
        }
    }

    if (bsize > 0) {
        for(size_t bjj = 0; bjj < _channels*(bsize-1); bjj++) {
            _inns[bjj] = _inns[bjj+_channels];
        }
        for(size_t ckk = 0; ckk < _channels; ckk++) {
            _inns[ckk+_channels*(bsize-1)] = gain * input[ckk];
        }
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
void IIRFilter::calculate(float gain,float* input, float* output, size_t size) {
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
void IIRFilter::clear() {
    for(size_t ii = 0; ii < _inns.size(); ii++) {
        _inns[ii] = 0.0f;
    }
    for(size_t ii = 0; _outs && ii < _outs.size(); ii++) {
        _outs[ii] = 0.0f;
    }
}

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
size_t IIRFilter::flush(float* output) {
    for(size_t ii = 0; _outs && ii < _outs.size(); ii++) {
        output[ii] = _outs[ii];
        _outs[ii] = 0.0f;
    }
    for(size_t ii = 0; ii < _inns.size(); ii++) {
        _inns[ii] = 0.0f;
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
void IIRFilter::stride(float gain, float* input, float* output, size_t size, unsigned channel) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        unsigned stride = _channels;
        __m128 tmp1, tmp2, tmp3;
        __m128 base = _mm_setzero_ps();

        size_t asize = _aval.size();
        size_t bsize = _bval.size();

        size_t ii;
        for(ii=0; ii < asize; ii++) {
            output[ii*stride] = _outs[channel+ii*stride];
        }
        
        for(ii = 0; ii+3 < size-asize; ii += 4) {
            // C[r] * y
            tmp1 = _mm_setzero_ps();
            for(size_t ajj = 0; ajj < asize; ajj++) {
                tmp1 = _mm_fmadd_ps(_mm_set1_ps(output[(ii+ajj)*stride]),_mm_load_ps(_c1+4*ajj),tmp1);
            }
            
            // FIR
            size_t bjj;
            tmp2 = _mm_mul_ps(_mm_set1_ps(gain * _b0), _mm_skipload_ps(input+ii*stride,stride));
            for(bjj = 0; ii+bjj+3 < bsize; bjj++) {
                base = _mm_skipload_ps(_inns+(ii+bjj)*stride+channel,stride);
                tmp2 = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj]),base,tmp2);
            }
            for(; ii+bjj < bsize; bjj++) {
                size_t brem = bsize-ii-bjj;
                for(size_t dkk = 0; dkk < brem; dkk++) {
                    base[dkk] = _inns[(ii+bjj+dkk)*stride+channel];
                }
                for(size_t dkk = 0; dkk < 4-brem; dkk++) {
                    base[dkk+brem] = gain * input[(ii+bjj+dkk+brem-bsize)*stride];
                }
                tmp2 = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj]),base,tmp2);
            }
            for(; bjj < bsize; bjj++) {
                base = _mm_mul_ps(_mm_set1_ps(gain), _mm_skipload_ps(input+(ii+bjj-bsize)*stride,stride));
                tmp2 = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj]),base,tmp2);
            }
            
            // D[r] * x
            tmp3 = _mm_mul_ps(_mm_set1_ps(tmp2[0]),  _mm_load_ps(_d1));
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[1]),_mm_load_ps(_d1+4),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[2]),_mm_load_ps(_d1+8),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[3]),_mm_load_ps(_d1+12),tmp3);
            
            // Shift to output and repeat
            tmp2 = _mm_add_ps(tmp1,tmp3);
            _mm_skipstore_ps(output+(ii+asize)*stride, tmp2, stride);
        }
        
        // Intermediate case
        size_t arem = (asize % 4);
        if (arem) {
            // C[r] * y
            tmp1 = _mm_setzero_ps();
            for(int ajj = 0; ajj < asize; ajj++) {
                tmp1 = _mm_fmadd_ps(_mm_set1_ps(output[(ii+ajj)*stride]),_mm_load_ps(_c1+4*ajj),tmp1);
            }
            
            // FIR
            size_t bjj;
            tmp2 = _mm_mul_ps(_mm_set1_ps(gain*_b0), _mm_skipload_ps(input+ii*stride,stride));
            for(bjj = 0; ii+bjj+3 < bsize; bjj++) {
                base = _mm_skipload_ps(_inns+(ii+bjj)*stride+channel,stride);
                tmp2 = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj]),base,tmp2);
            }
            for(; ii+bjj < bsize; bjj++) {
                size_t brem = bsize-ii-bjj;
                for(size_t dkk = 0; dkk < brem; dkk++) {
                    base[dkk] = _inns[(ii+bjj+dkk)*stride+channel];
                }
                for(size_t dkk = 0; dkk < 4-brem; dkk++) {
                    base[dkk+brem] = gain * input[(ii+bjj+dkk+brem-bsize)*stride];
                }
                tmp2 = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj]),base,tmp2);
            }
            for(; bjj < bsize; bjj++) {
                base = _mm_mul_ps(_mm_set1_ps(gain),_mm_skipload_ps(input+(ii+bjj-bsize)*stride,stride));
                tmp2 = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj]),base,tmp2);
            }
            
            // D[r] * x
            tmp3 = _mm_mul_ps(_mm_set1_ps(tmp2[0]),_mm_load_ps(_d1));
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[1]),_mm_load_ps(_d1+4),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[2]),_mm_load_ps(_d1+8),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[3]),_mm_load_ps(_d1+12),tmp3);
            
            // Shift to output and handoff
            tmp2 = _mm_add_ps(tmp1,tmp3);
            for(size_t dkk = 0; dkk < 4-arem; dkk++) {
                output[(ii+asize+dkk)*stride] = tmp2[dkk];
            }
            for(size_t dkk = 0; dkk < arem; dkk++) {
                _outs[channel+dkk*stride] = tmp2[4-arem+dkk];
            }
            ii += 4;
        }
        
        // Pure buffer (when coeff > 4)
        for(size_t kk=arem; kk < asize; kk += 4) {
            // C[r] * y
            tmp1 = _mm_setzero_ps();
            
            size_t ajj = 0;
            for(ajj = 0; kk+ajj < asize; ajj++) {
                tmp1 = _mm_fmadd_ps(_mm_set1_ps(output[(ii+ajj)*stride]),_mm_load_ps(_c1+4*ajj),tmp1);
            }
            for(; ajj < asize; ajj++) {
                tmp1 = _mm_fmadd_ps(_mm_set1_ps(_outs[channel+(kk+ajj-asize)*stride]),_mm_load_ps(_c1+4*ajj),tmp1);
            }
            
            // FIR
            size_t bjj;
            size_t mm = ii+kk-arem;
            tmp2 = _mm_mul_ps(_mm_set1_ps(gain*_b0), _mm_skipload_ps(input+mm*stride,stride));
            for(bjj = 0; mm+bjj+3 < bsize; bjj++) {
                base = _mm_skipload_ps(_inns+(mm+bjj)*stride+channel,stride);
                tmp2 = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj]),base,tmp2);
            }
            for(; mm+bjj < bsize; bjj++) {
                size_t brem = bsize-mm-bjj;
                for(size_t dkk = 0; dkk < brem; dkk++) {
                    base[dkk] = _inns[(mm+bjj+dkk)*stride+channel];
                }
                for(size_t dkk = 0; dkk < 4-brem; dkk++) {
                    base[dkk+brem] = gain * input[(mm+bjj+dkk+brem-bsize)*stride];
                }
                tmp2 = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj]),base,tmp2);
            }
            for(; bjj < bsize; bjj++) {
                base = _mm_mul_ps(_mm_set1_ps(gain),_mm_skipload_ps(input+(mm+bjj-bsize)*stride,stride));
                tmp2 = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj]),base,tmp2);
            }
            
            // D[r] * x
            tmp3 = _mm_mul_ps(_mm_set1_ps(tmp2[0]),_mm_load_ps(_d1));
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[1]),_mm_load_ps(_d1+4),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[2]),_mm_load_ps(_d1+8),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[3]),_mm_load_ps(_d1+12),tmp3);
            
            // Shift to output buffer
            tmp2 = _mm_add_ps(tmp1,tmp3);
            _mm_skipstore_ps(_outs+channel+kk*stride,tmp2,stride);
        }
        
        for(size_t bjj = 0; bjj < bsize; bjj++) {
            _inns[bjj*stride+channel] = gain * input[(size-bsize+bjj)*stride];
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
        float32x4_t tmp1, tmp2, tmp3;
        float32x4_t base = {0,0,0,0};
        
        size_t asize = _aval.size();
        size_t bsize = _bval.size();
        
        size_t ii;
        for(ii=0; ii < asize; ii++) {
            output[ii*stride] = _outs[channel+ii*stride];
        }
        
        float32x4_t thegain = vld1q_dup_f32(&gain);
        for(ii = 0; ii+3 < size-asize; ii += 4) {
            // C[r] * y
            tmp1 = {0,0,0,0};
            for(size_t ajj = 0; ajj < asize; ajj++) {
                tmp1 = vmlaq_f32(tmp1, vld1q_dup_f32(output+(ii+ajj)*stride),vld1q_f32(_c1+4*ajj));
            }
            
            // FIR
            size_t bjj;
            tmp2 = vld1q_skip_f32(input+ii*stride,stride);
            tmp2 = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), tmp2));
            for(bjj = 0; ii+bjj+3 < bsize; bjj++) {
                base = vld1q_skip_f32(_inns+(ii+bjj)*stride+channel,stride);
                tmp2 = vmlaq_f32(tmp2, vld1q_dup_f32(_bval+bjj), base);
            }
            for(; ii+bjj < bsize; bjj++) {
                size_t brem = bsize-ii-bjj;
                for(size_t dkk = 0; dkk < brem; dkk++) {
                    base[dkk] = _inns[(ii+bjj+dkk)*stride+channel];
                }
                for(size_t dkk = 0; dkk < 4-brem; dkk++) {
                    base[dkk+brem] = gain * input[(ii+bjj+dkk+brem-bsize)*stride];
                }
                tmp2 = vmlaq_f32(tmp2, vld1q_dup_f32(_bval+bjj), base);
            }
            for(; bjj < bsize; bjj++) {
                base = vmulq_f32(thegain, vld1q_skip_f32(input+(ii+bjj-bsize)*stride,stride));
                tmp2 = vmlaq_f32(tmp2, vld1q_dup_f32(_bval+bjj), base);
            }
            
            // D[r] * x
            tmp3 = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2),0),vld1q_f32(_d1));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_low_f32(tmp2),1), vld1q_f32(_d1+4));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),0),vld1q_f32(_d1+8));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),1),vld1q_f32(_d1+12));

            // Shift to output and repeat
            tmp2 = vaddq_f32(tmp1,tmp3);
            vst1q_skip_f32(output+(ii+asize)*stride, tmp2, stride);
        }
        
        // Intermediate case
        size_t arem = (asize % 4);
        if (arem) {
            // C[r] * y
            tmp1 = {0,0,0,0};
            for(int ajj = 0; ajj < asize; ajj++) {
                tmp1 = vmlaq_f32(tmp1, vld1q_dup_f32(output+(ii+ajj)*stride), vld1q_f32(_c1+4*ajj));
            }
            
            // FIR
            size_t bjj;
            tmp2 = vld1q_skip_f32(input+ii*stride,stride);
            tmp2 = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), tmp2));
            for(bjj = 0; ii+bjj+3 < bsize; bjj++) {
                base = vld1q_skip_f32(_inns+(ii+bjj)*stride+channel,stride);
                tmp2 = vmlaq_f32(tmp2, vld1q_dup_f32(_bval+bjj), base);
            }
            for(; ii+bjj < bsize; bjj++) {
                size_t brem = bsize-ii-bjj;
                for(size_t dkk = 0; dkk < brem; dkk++) {
                    base[dkk] = _inns[(ii+bjj+dkk)*stride+channel];
                }
                for(size_t dkk = 0; dkk < 4-brem; dkk++) {
                    base[dkk+brem] = gain * input[(ii+bjj+dkk+brem-bsize)*stride];
                }
                tmp2 = vmlaq_f32(tmp2, vld1q_dup_f32(_bval+bjj), base);
            }
            for(; bjj < bsize; bjj++) {
                base = vmulq_f32(thegain, vld1q_skip_f32(input+(ii+bjj-bsize)*stride,stride));
                tmp2 = vmlaq_f32(tmp2, vld1q_dup_f32(_bval+bjj), base);
            }
            
            // D[r] * x
            tmp3 = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2),0),vld1q_f32(_d1));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_low_f32(tmp2),1), vld1q_f32(_d1+4));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),0),vld1q_f32(_d1+8));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),1),vld1q_f32(_d1+12));

            // Shift to output and handoff
            tmp2 = vaddq_f32(tmp1,tmp3);
            for(size_t dkk = 0; dkk < 4-arem; dkk++) {
                output[(ii+asize+dkk)*stride] = tmp2[dkk];
            }
            for(size_t dkk = 0; dkk < arem; dkk++) {
                _outs[channel+dkk*stride] = tmp2[4-arem+dkk];
            }
            ii += 4;
        }
        
        // Pure buffer (when coeff > 4)
        for(size_t kk=arem; kk < asize; kk += 4) {
            // C[r] * y
            tmp1 = {0,0,0,0};
            
            size_t ajj = 0;
            for(ajj = 0; kk+ajj < asize; ajj++) {
                tmp1 = vmlaq_f32(tmp1, vld1q_dup_f32(output+(ii+ajj)*stride), vld1q_f32(_c1+4*ajj));
            }
            for(; ajj < asize; ajj++) {
                tmp1 = vmlaq_f32(tmp1, vld1q_dup_f32(_outs+channel+(kk+ajj-asize)*stride), vld1q_f32(_c1+4*ajj));
            }
            
            // FIR
            size_t bjj;
            size_t mm = ii+kk-arem;
            tmp2 = vld1q_skip_f32(input+mm*stride,stride);
            tmp2 = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), tmp2));
            for(bjj = 0; mm+bjj+3 < bsize; bjj++) {
                base = vld1q_skip_f32(_inns+(mm+bjj)*stride+channel,stride);
                tmp2 = vmlaq_f32(tmp2, vld1q_dup_f32(_bval+bjj), base);
            }
            for(; mm+bjj < bsize; bjj++) {
                size_t brem = bsize-mm-bjj;
                for(size_t dkk = 0; dkk < brem; dkk++) {
                    base[dkk] = _inns[(mm+bjj+dkk)*stride+channel];
                }
                for(size_t dkk = 0; dkk < 4-brem; dkk++) {
                    base[dkk+brem] = gain * input[(mm+bjj+dkk+brem-bsize)*stride];
                }
                tmp2 = vmlaq_f32(tmp2, vld1q_dup_f32(_bval+bjj), base);
            }
            for(; bjj < bsize; bjj++) {
                base = vmulq_f32(thegain,vld1q_skip_f32(input+(mm+bjj-bsize)*stride,stride));
                tmp2 = vmlaq_f32(tmp2, vld1q_dup_f32(_bval+bjj), base);
            }
            
            // D[r] * x
            tmp3 = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2),0),vld1q_f32(_d1));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_low_f32(tmp2),1), vld1q_f32(_d1+4));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),0),vld1q_f32(_d1+8));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),1),vld1q_f32(_d1+12));

            // Shift to output buffer
            tmp2 = vaddq_f32(tmp1,tmp3);
            vst1q_skip_f32(_outs+channel+kk*stride,tmp2,stride);
        }
        
        for(size_t bjj = 0; bjj < bsize; bjj++) {
            _inns[bjj*stride+channel] = gain * input[(size-bsize+bjj)*stride];
        }
    } else {
#else
    {
#endif
        unsigned stride = _channels;
        size_t asize = _aval.size();
        size_t bsize = _bval.size();
        
        for(size_t ii = 0; ii < asize; ii++) {
            output[ii*stride] = _outs[channel+ii*stride];
        }
        
        for(size_t ii = 0; ii < size-asize; ii++) {
            float temp = gain * _b0 * input[ii*stride];
            size_t bjj;
            for(bjj = 0; ii+bjj < bsize; bjj++) {
                temp += _bval[bjj]*_inns[(ii+bjj)*stride+channel];
            }
            for(; bjj < bsize; bjj++) {
                temp += gain * _bval[bjj]*input[(ii+bjj-bsize)*stride];
            }
            for(size_t ajj = 0; ajj < asize; ajj++) {
                temp += _aval[ajj]*output[(ii+ajj)*stride];
            }
            output[(ii+asize)*stride] = temp;
        }
        
        for(size_t ii = 0; ii < asize; ii++) {
            float temp = gain * _b0 * input[(size-asize+ii)*stride];
            size_t bjj;
            for(bjj = 0; ii+bjj+size-asize < bsize; bjj++) {
                temp += _bval[bjj]*_inns[(ii+bjj+size-asize)*stride+channel];
            }
            for(; bjj < bsize; bjj++) {
                temp += gain * _bval[bjj]*input[(ii+bjj-bsize+size-asize)*stride];
            }
            size_t ajj;
            for(ajj = 0; ii+ajj < asize; ajj++) {
                temp += _aval[ajj]*output[(size-asize+ii+ajj)*stride];
            }
            for(; ajj < asize; ajj++) {
                temp += _aval[ajj]*_outs[channel+(ii+ajj-asize)*stride];
            }
            _outs[channel+ii*stride] = temp;
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
void IIRFilter::single(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        __m128 tmp1, tmp2, tmp3;
        __m128 base = _mm_setzero_ps();
        
        size_t asize = _aval.size();
        size_t bsize = _bval.size();

        size_t ii;
        for(ii=0; ii+3 < asize; ii += 4) {
            _mm_store_ps(output+ii,_mm_load_ps(_outs+ii));
        }
        for(; ii < asize; ii++) {
            output[ii] = _outs[ii];
        }
        
        for(ii = 0; ii+3 < size-asize; ii += 4) {
            // C[r] * y
            tmp1 = _mm_setzero_ps();
                for(size_t ajj = 0; ajj < asize; ajj++) {
                tmp1 = _mm_fmadd_ps(_mm_set1_ps(output[ii+ajj]),_mm_load_ps(_c1+4*ajj),tmp1);
            }

            // FIR
            size_t bjj;
            tmp2 = _mm_mul_ps(_mm_set1_ps(gain*_b0), _mm_load_ps(input+ii));
            for(bjj = 0; ii+bjj+3 < bsize; bjj++) {
                tmp2 = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj]),_mm_loadu_ps(_inns+ii+bjj),tmp2);
            }
            for(; ii+bjj < bsize; bjj++) {
                size_t brem = bsize-ii-bjj;
                for(size_t dkk = 0; dkk < brem; dkk++) {
                    base[dkk] = _inns[ii+bjj+dkk];
                }
                for(size_t dkk = 0; dkk < 4-brem; dkk++) {
                    base[dkk+brem] = gain * input[ii+bjj+dkk-bsize+brem];
                }
                tmp2 = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj]),base,tmp2);
            }
            for(; bjj < bsize; bjj++) {
                base = _mm_mul_ps(_mm_set1_ps(gain),_mm_loadu_ps(input+ii+bjj-bsize));
                tmp2 = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj]),base,tmp2);
            }

            // D[r] * x
            tmp3 = _mm_mul_ps(_mm_set1_ps(tmp2[0]),_mm_load_ps(_d1));
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[1]),_mm_load_ps(_d1+4),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[2]),_mm_load_ps(_d1+8),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[3]),_mm_load_ps(_d1+12),tmp3);

            // Shift to output and repeat
            tmp2 = _mm_add_ps(tmp1,tmp3);
            _mm_storeu_ps(output+ii+asize,tmp2);
        }

        // Intermediate case
        size_t arem = (asize % 4);
        if (arem) {
            // C[r] * y
            tmp1 = _mm_setzero_ps();
            for(size_t ajj = 0; ajj < asize; ajj++) {
                tmp1 = _mm_fmadd_ps(_mm_set1_ps(output[ii+ajj]),_mm_load_ps(_c1+4*ajj),tmp1);
            }

            // FIR
            size_t bjj;
            tmp2 = _mm_mul_ps(_mm_set1_ps(gain*_b0), _mm_load_ps(input+ii));
            for(bjj = 0; ii+bjj+3 < bsize; bjj++) {
                tmp2 = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj]),_mm_loadu_ps(_inns+ii+bjj),tmp2);
            }
            for(; ii+bjj < bsize; bjj++) {
                size_t brem = bsize-ii-bjj;
                for(size_t dkk = 0; dkk < brem; dkk++) {
                    base[dkk] = _inns[ii+bjj+dkk];
                }
                for(size_t dkk = 0; dkk < 4-brem; dkk++) {
                    base[dkk+brem] = gain*input[ii+bjj+dkk-bsize+brem];
                }
                tmp2 = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj]),base,tmp2);
            }
            for(; bjj < bsize; bjj++) {
                base = _mm_mul_ps(_mm_set1_ps(gain),_mm_loadu_ps(input+ii+bjj-bsize));
                tmp2 = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj]),base,tmp2);
            }
            
            // D[r] * x
            tmp3 = _mm_mul_ps(_mm_set1_ps(tmp2[0]),_mm_load_ps(_d1));
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[1]),_mm_load_ps(_d1+4),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[2]),_mm_load_ps(_d1+8),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[3]),_mm_load_ps(_d1+12),tmp3);

            // Shift to output and handoff
            tmp2 = _mm_add_ps(tmp1,tmp3);
            for(size_t dkk = 0; dkk < 4-arem; dkk++) {
                output[ii+asize+dkk] = tmp2[dkk];
            }
            for(size_t dkk = 0; dkk < arem; dkk++) {
                _outs[dkk] = tmp2[4-arem+dkk];
            }
            ii += 4;
        }
        
        // Pure buffer (when coeff > 4)
        for(size_t kk=arem; kk < asize; kk += 4) {
            // C[r] * y
            tmp1 = _mm_setzero_ps();
            
            size_t ajj = 0;
            for(ajj = 0; kk+ajj < asize; ajj++) {
                tmp1 = _mm_fmadd_ps(_mm_set1_ps(output[ii+ajj]),_mm_load_ps(_c1+4*ajj),tmp1);
            }
            for(; ajj < asize; ajj++) {
                tmp1 = _mm_fmadd_ps(_mm_set1_ps(_outs[kk+ajj-asize]),_mm_load_ps(_c1+4*ajj),tmp1);
            }

            // FIR
            size_t bjj;
            size_t mm = ii+kk-arem;
            tmp2 = _mm_mul_ps(_mm_set1_ps(gain*_b0), _mm_load_ps(input+mm));
            for(bjj = 0; mm+bjj+3 < bsize; bjj++) {
                tmp2 = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj]),_mm_loadu_ps(_inns+mm+bjj),tmp2);
            }
            for(; mm+bjj < bsize; bjj++) {
                size_t brem = bsize-mm-bjj;
                for(size_t dkk = 0; dkk < brem; dkk++) {
                    base[dkk] = _inns[mm+bjj+dkk];
                }
                for(size_t dkk = 0; dkk < 4-brem; dkk++) {
                    base[dkk+brem] = gain*input[mm+bjj+dkk-bsize+brem];
                }
                tmp2 = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj]),base,tmp2);
            }
            for(; bjj < bsize; bjj++) {
                base = _mm_mul_ps(_mm_set1_ps(gain),_mm_loadu_ps(input+mm+bjj-bsize));
                tmp2 = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj]),base,tmp2);
            }
            
            // D[r] * x
            tmp3 = _mm_mul_ps(_mm_set1_ps(tmp2[0]),_mm_load_ps(_d1));
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[1]),_mm_load_ps(_d1+4),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[2]),_mm_load_ps(_d1+8),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[3]),_mm_load_ps(_d1+12),tmp3);
            
            // Shift to output buffer
            tmp2 = _mm_add_ps(tmp1,tmp3);
            _mm_storeu_ps(_outs+kk,tmp2);
        }
        
        size_t bjj = 0;
        for(bjj = 0; bjj+3 < bsize; bjj += 4) {
            _mm_store_ps(_inns+bjj,_mm_mul_ps(_mm_set1_ps(gain),_mm_loadu_ps(input+size-bsize+bjj)));
        }
        for(; bjj < bsize; bjj++) {
            _inns[bjj] = gain*input[size-bsize+bjj];
        }
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float32x4_t tmp1, tmp2, tmp3;
        float32x4_t base = {0,0,0,0};
        
        size_t asize = _aval.size();
        size_t bsize = _bval.size();
        
        size_t ii;
        for(ii=0; ii+3 < asize; ii += 4) {
            vst1q_f32(output+ii,vld1q_f32(_outs+ii));
        }
        for(; ii < asize; ii++) {
            output[ii] = _outs[ii];
        }
        
        float32x4_t thegain = vld1q_dup_f32(&gain);
        for(ii = 0; ii+3 < size-asize; ii += 4) {
            // C[r] * y
            tmp1 = {0,0,0,0};
            for(size_t ajj = 0; ajj < asize; ajj++) {
                tmp1 = vmlaq_f32(tmp1, vld1q_dup_f32(output+ii+ajj),vld1q_f32(_c1+4*ajj));
            }
            
            // FIR
            size_t bjj;
            tmp2 = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), vld1q_f32(input+ii)));
            for(bjj = 0; ii+bjj+3 < bsize; bjj++) {
                tmp2 = vmlaq_f32(tmp2, vld1q_dup_f32(_bval+bjj), vld1q_f32(_inns+ii+bjj));
            }
            for(; ii+bjj < bsize; bjj++) {
                size_t brem = bsize-ii-bjj;
                for(size_t dkk = 0; dkk < brem; dkk++) {
                    base[dkk] = _inns[ii+bjj+dkk];
                }
                for(size_t dkk = 0; dkk < 4-brem; dkk++) {
                    base[dkk+brem] = gain * input[ii+bjj+dkk-bsize+brem];
                }
                tmp2 = vmlaq_f32(tmp2, vld1q_dup_f32(_bval+bjj), base);
            }
            for(; bjj < bsize; bjj++) {
                base = vmulq_f32(thegain, vld1q_f32(input+ii+bjj-bsize));
                tmp2 = vmlaq_f32(tmp2, vld1q_dup_f32(_bval+bjj), base);
            }
            
            // D[r] * x
            tmp3 = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2),0),vld1q_f32(_d1));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_low_f32(tmp2),1), vld1q_f32(_d1+4));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),0),vld1q_f32(_d1+8));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),1),vld1q_f32(_d1+12));

            // Shift to output and repeat
            tmp2 = vaddq_f32(tmp1,tmp3);
            vst1q_f32(output+ii+asize,tmp2);
        }
        
        // Intermediate case
        size_t arem = (asize % 4);
        if (arem) {
            // C[r] * y
            tmp1 = {0,0,0,0};
            for(size_t ajj = 0; ajj < asize; ajj++) {
                tmp1 = vmlaq_f32(tmp1, vld1q_dup_f32(output+ii+ajj), vld1q_f32(_c1+4*ajj));
            }
            
            // FIR
            size_t bjj;
            tmp2 = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), vld1q_f32(input+ii)));
            for(bjj = 0; ii+bjj+3 < bsize; bjj++) {
                tmp2 = vmlaq_f32(tmp2, vld1q_dup_f32(_bval+bjj), vld1q_f32(_inns+ii+bjj));
            }
            for(; ii+bjj < bsize; bjj++) {
                size_t brem = bsize-ii-bjj;
                for(size_t dkk = 0; dkk < brem; dkk++) {
                    base[dkk] = _inns[ii+bjj+dkk];
                }
                for(size_t dkk = 0; dkk < 4-brem; dkk++) {
                    base[dkk+brem] = gain*input[ii+bjj+dkk-bsize+brem];
                }
                tmp2 = vmlaq_f32(tmp2, vld1q_dup_f32(_bval+bjj), base);
            }
            for(; bjj < bsize; bjj++) {
                base = vmulq_f32(thegain, vld1q_f32(input+ii+bjj-bsize));
                tmp2 = vmlaq_f32(tmp2, vld1q_dup_f32(_bval+bjj), base);
            }
            
            // D[r] * x
            tmp3 = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2),0),vld1q_f32(_d1));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_low_f32(tmp2),1), vld1q_f32(_d1+4));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),0),vld1q_f32(_d1+8));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),1),vld1q_f32(_d1+12));

            // Shift to output and handoff
            tmp2 = vaddq_f32(tmp1,tmp3);
            for(size_t dkk = 0; dkk < 4-arem; dkk++) {
                output[ii+asize+dkk] = tmp2[dkk];
            }
            for(size_t dkk = 0; dkk < arem; dkk++) {
                _outs[dkk] = tmp2[4-arem+dkk];
            }
            ii += 4;
        }
        
        // Pure buffer (when coeff > 4)
        for(size_t kk=arem; kk < asize; kk += 4) {
            // C[r] * y
            tmp1 = {0,0,0,0};
            size_t ajj = 0;
            for(ajj = 0; kk+ajj < asize; ajj++) {
                tmp1 = vmlaq_f32(tmp1, vld1q_dup_f32(output+ii+ajj), vld1q_f32(_c1+4*ajj));
            }
            for(; ajj < asize; ajj++) {
                tmp1 = vmlaq_f32(tmp1, vld1q_dup_f32(_outs+kk+ajj-asize), vld1q_f32(_c1+4*ajj));
            }
            
            // FIR
            size_t bjj;
            size_t mm = ii+kk-arem;
            tmp2 = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), vld1q_f32(input+mm)));
            for(bjj = 0; mm+bjj+3 < bsize; bjj++) {
                tmp2 = vmlaq_f32(tmp2, vld1q_dup_f32(_bval+bjj), vld1q_f32(_inns+mm+bjj));
            }
            for(; mm+bjj < bsize; bjj++) {
                size_t brem = bsize-mm-bjj;
                for(size_t dkk = 0; dkk < brem; dkk++) {
                    base[dkk] = _inns[mm+bjj+dkk];
                }
                for(size_t dkk = 0; dkk < 4-brem; dkk++) {
                    base[dkk+brem] = gain*input[mm+bjj+dkk-bsize+brem];
                }
                tmp2 = vmlaq_f32(tmp2, vld1q_dup_f32(_bval+bjj), base);
            }
            for(; bjj < bsize; bjj++) {
                base = vmulq_f32(thegain, vld1q_f32(input+mm+bjj-bsize));
                tmp2 = vmlaq_f32(tmp2, vld1q_dup_f32(_bval+bjj), base);
            }
            
            // D[r] * x
            tmp3 = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2),0),vld1q_f32(_d1));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_low_f32(tmp2),1), vld1q_f32(_d1+4));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),0),vld1q_f32(_d1+8));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),1),vld1q_f32(_d1+12));

            // Shift to output buffer
            tmp2 = vaddq_f32(tmp1,tmp3);
            vst1q_f32(_outs+kk,tmp2);
        }
        
        size_t bjj = 0;
        for(bjj = 0; bjj+3 < bsize; bjj += 4) {
            vst1q_f32(_inns+bjj,vmulq_f32(thegain,vld1q_f32(input+size-bsize+bjj)));
        }
        for(; bjj < bsize; bjj++) {
            _inns[bjj] = gain*input[size-bsize+bjj];
        }
    } else {
#else
    {
#endif
        size_t asize = _aval.size();
        size_t bsize = _bval.size();
        
        for(size_t ii=0; ii < asize; ii++) {
            output[ii] = _outs[ii];
        }
        
        for(size_t ii = 0; ii < size-asize; ii++) {
            float temp = gain * _b0 * input[ii];
            size_t bjj;
            for(bjj = 0; ii+bjj < bsize; bjj++) {
                temp += _bval[bjj]*_inns[ii+bjj];
            }
            for(; bjj < bsize; bjj++) {
                temp += gain * _bval[bjj]*input[ii+bjj-bsize];
            }
            for(size_t ajj = 0; ajj < asize; ajj++) {
                temp += _aval[ajj]*output[ii+ajj];
            }
            output[ii+asize] = temp;
        }
        
        for(size_t ii=0; ii < asize; ii++) {
            float temp = gain * _b0 * input[size-asize+ii];
            size_t bjj;
            for(bjj = 0; ii+bjj+size-asize < bsize; bjj++) {
                temp += _bval[bjj]*_inns[ii+bjj+size-asize];
            }
            for(; bjj < bsize; bjj++) {
                temp += gain * _bval[bjj]*input[ii+bjj-bsize+size-asize];
            }
            size_t ajj;
            for(ajj = 0; ii+ajj < asize; ajj++) {
                temp += _aval[ajj]*output[size-asize+ii+ajj];
            }
            for(; ajj < asize; ajj++) {
                temp += _aval[ajj]*_outs[ii+ajj-asize];
            }
            _outs[ii] = temp;
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
void IIRFilter::dual(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        __m128 tmp1, tmp2, tmp3;
        __m128 base = _mm_setzero_ps();
        
        size_t asize = _aval.size();
        size_t bsize = _bval.size();

        size_t ii;
        for(ii=0; ii+3 < 2*asize; ii += 4) {
            _mm_store_ps(output+ii,_mm_load_ps(_outs+ii));
        }
        for(; ii < 2*asize; ii++) {
            output[ii] = _outs[ii];
        }
        
        for(ii = 0; ii+3 < 2*(size-asize); ii += 4) {
            // C[r] * y
            tmp1 = _mm_setzero_ps();
            for(size_t ajj = 0; ajj < 2*asize; ajj++) {
                tmp1 = _mm_fmadd_ps(_mm_set1_ps(output[ii+ajj]),_mm_load_ps(_c2+4*ajj),tmp1);
            }
            
            // FIR
            size_t bjj;
            tmp2 = _mm_mul_ps(_mm_set1_ps(gain*_b0), _mm_load_ps(input+ii));
            for(bjj = 0; ii+bjj+3 < 2*bsize; bjj += 2) {
                base = _mm_loadu_ps(_inns+ii+bjj);
                tmp2 = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj/2]),base,tmp2);
            }
            for(; ii+bjj < 2*bsize; bjj += 2) {
                size_t brem = 2*bsize-ii-bjj;
                for(size_t dkk = 0; dkk < brem; dkk++) {
                    base[dkk] = _inns[ii+bjj+dkk];
                }
                for(size_t dkk = 0; dkk < 4-brem; dkk++) {
                    base[dkk+brem] = gain*input[ii+bjj+dkk-2*bsize+brem];
                }
                tmp2 = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj/2]),base,tmp2);
            }
            for(; bjj < 2*bsize; bjj += 2) {
                base = _mm_mul_ps(_mm_set1_ps(gain),_mm_loadu_ps(input+ii+bjj-2*bsize));
                tmp2 = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj/2]),base,tmp2);
            }

            // D[r] * x
            tmp3 = _mm_mul_ps(_mm_set1_ps(tmp2[0]),_mm_load_ps(_d2));
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[1]),_mm_load_ps(_d2+4),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[2]),_mm_load_ps(_d2+8),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[3]),_mm_load_ps(_d2+12),tmp3);


            // Shift to output and repeat
            tmp2 = _mm_add_ps(tmp1,tmp3);
            _mm_storeu_ps(output+ii+2*asize,tmp2);
        }
        
        // Intermediate case
        size_t arem = ((2*asize) % 4);
        if (arem) {
            // C[r] * y
            tmp1 = _mm_setzero_ps();
            for(size_t ajj = 0; ajj < 2*asize; ajj++) {
                tmp1 = _mm_fmadd_ps(_mm_set1_ps(output[ii+ajj]),_mm_load_ps(_c2+4*ajj),tmp1);
            }
            
            // FIR
            size_t bjj;
            tmp2 = _mm_mul_ps(_mm_set1_ps(gain*_b0), _mm_load_ps(input+ii));
            for(bjj = 0; ii+bjj+3 < 2*bsize; bjj += 2) {
                base = _mm_loadu_ps(_inns+ii+bjj);
                tmp2 = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj/2]),base,tmp2);
            }
            for(; ii+bjj < 2*bsize; bjj += 2) {
                size_t brem = 2*bsize-ii-bjj;
                for(size_t dkk = 0; dkk < brem; dkk++) {
                    base[dkk] = _inns[ii+bjj+dkk];
                }
                for(size_t dkk = 0; dkk < 4-brem; dkk++) {
                    base[dkk+brem] = gain*input[ii+bjj+dkk-2*bsize+brem];
                }
                tmp2 = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj/2]),base,tmp2);
            }
            for(; bjj < 2*bsize; bjj += 2) {
                base = _mm_mul_ps(_mm_set1_ps(gain),_mm_loadu_ps(input+ii+bjj-2*bsize));
                tmp2 = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj/2]),base,tmp2);
            }
            
            // D[r] * x
            tmp3 = _mm_mul_ps(_mm_set1_ps(tmp2[0]),_mm_load_ps(_d2));
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[1]),_mm_load_ps(_d2+4),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[2]),_mm_load_ps(_d2+8),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[3]),_mm_load_ps(_d2+12),tmp3);
            
            // Shift to output and handoff
            tmp2 = _mm_add_ps(tmp1,tmp3);
            for(size_t dkk = 0; dkk < 4-arem; dkk++) {
                output[ii+2*asize+dkk] = tmp2[dkk];
            }
            for(size_t dkk = 0; dkk < arem; dkk++) {
                _outs[dkk] = tmp2[4-arem+dkk];
            }
            ii += 4;
        }
        
        // Pure buffer (when coeff > 4)
        for(size_t kk=arem; kk < 2*asize; kk += 4) {
            // C[r] * y
            tmp1 = _mm_setzero_ps();
            size_t ajj = 0;
            for(ajj = 0; kk+ajj < 2*asize; ajj++) {
                tmp1 = _mm_fmadd_ps(_mm_set1_ps(output[2*(size-asize)+kk+ajj]),_mm_load_ps(_c2+4*ajj),tmp1);
            }
            for(; ajj < 2*asize; ajj++) {
                tmp1 = _mm_fmadd_ps(_mm_set1_ps(_outs[kk+ajj-2*asize]),_mm_load_ps(_c2+4*ajj),tmp1);
            }
            
            // FIR
            size_t bjj;
            size_t mm = ii+kk-arem;
            tmp2 = _mm_mul_ps(_mm_set1_ps(gain*_b0), _mm_load_ps(input+mm));
            for(bjj = 0; ii+bjj+3 < 2*bsize; bjj += 2) {
                base = _mm_loadu_ps(_inns+mm+bjj);
                tmp2 = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj/2]),base,tmp2);
            }
            for(; ii+bjj < 2*bsize; bjj += 2) {
                size_t brem = 2*bsize-mm-bjj;
                for(size_t dkk = 0; dkk < brem; dkk++) {
                    base[dkk] = _inns[mm+bjj+dkk];
                }
                for(size_t dkk = 0; dkk < 4-brem; dkk++) {
                    base[dkk+brem] = gain*input[mm+bjj+dkk-2*bsize+brem];
                }
                tmp2 = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj/2]),base,tmp2);
            }
            for(; bjj < 2*bsize; bjj += 2) {
                base = _mm_mul_ps(_mm_set1_ps(gain),_mm_loadu_ps(input+mm+bjj-2*bsize));
                tmp2 = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj/2]),base,tmp2);
            }
            
            // D[r] * x
            tmp3 = _mm_mul_ps(_mm_set1_ps(tmp2[0]),_mm_load_ps(_d2));
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[1]),_mm_load_ps(_d2+4),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[2]),_mm_load_ps(_d2+8),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[3]),_mm_load_ps(_d2+12),tmp3);
            
            // Shift to output buffer
            tmp2 = _mm_add_ps(tmp1,tmp3);
            _mm_storeu_ps(_outs+kk,tmp2);
        }
        
        size_t kk;
        for(kk = 0; kk+3 < 2*bsize; kk += 4) {
            _mm_store_ps(_inns+kk,_mm_mul_ps(_mm_set1_ps(gain),_mm_loadu_ps(input+2*(size-bsize)+kk)));
        }
        for(; kk < 2*bsize; kk++) {
            _inns[kk] = gain*input[2*(size-bsize)+kk];
        }
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float32x4_t tmp1, tmp2, tmp3;
        float32x4_t base = {0,0,0,0};
        
        size_t asize = _aval.size();
        size_t bsize = _bval.size();
        
        size_t ii;
        for(ii=0; ii+3 < 2*asize; ii += 4) {
            vst1q_f32(output+ii,vld1q_f32(_outs+ii));
        }
        for(; ii < 2*asize; ii++) {
            output[ii] = _outs[ii];
        }
        
        float32x4_t thegain = vld1q_dup_f32(&gain);
        for(ii = 0; ii+3 < 2*(size-asize); ii += 4) {
            // C[r] * y
            tmp1 = {0,0,0,0};
            for(size_t ajj = 0; ajj < 2*asize; ajj++) {
                tmp1 = vmlaq_f32(tmp1, vld1q_dup_f32(output+ii+ajj), vld1q_f32(_c2+4*ajj));
            }
            
            // FIR
            size_t bjj;
            tmp2 = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), vld1q_f32(input+ii)));
            for(bjj = 0; ii+bjj+3 < 2*bsize; bjj += 2) {
                base = vld1q_f32(_inns+ii+bjj);
                tmp2 = vmlaq_f32(tmp2, vld1q_dup_f32(_bval+(bjj/2)), base);
            }
            for(; ii+bjj < 2*bsize; bjj += 2) {
                size_t brem = 2*bsize-ii-bjj;
                for(size_t dkk = 0; dkk < brem; dkk++) {
                    base[dkk] = _inns[ii+bjj+dkk];
                }
                for(size_t dkk = 0; dkk < 4-brem; dkk++) {
                    base[dkk+brem] = gain*input[ii+bjj+dkk-2*bsize+brem];
                }
                tmp2 = vmlaq_f32(tmp2, vld1q_dup_f32(_bval+(bjj/2)), base);
            }
            for(; bjj < 2*bsize; bjj += 2) {
                base = vmulq_f32(thegain, vld1q_f32(input+ii+bjj-2*bsize));
                tmp2 = vmlaq_f32(tmp2, vld1q_dup_f32(_bval+(bjj/2)), base);
            }
            
            // D[r] * x
            tmp3 = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2),0),vld1q_f32(_d2));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_low_f32(tmp2),1), vld1q_f32(_d2+4));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),0),vld1q_f32(_d2+8));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),1),vld1q_f32(_d2+12));
            
            // Shift to output and repeat
            tmp2 = vaddq_f32(tmp1,tmp3);
            vst1q_f32(output+ii+2*asize,tmp2);
        }
        
        // Intermediate case
        size_t arem = ((2*asize) % 4);
        if (arem) {
            // C[r] * y
            tmp1 = {0,0,0,0};
            for(size_t ajj = 0; ajj < 2*asize; ajj++) {
                tmp1 = vmlaq_f32(tmp1, vld1q_dup_f32(output+ii+ajj), vld1q_f32(_c2+4*ajj));
            }
            
            // FIR
            size_t bjj;
            tmp2 = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), vld1q_f32(input+ii)));
            for(bjj = 0; ii+bjj+3 < 2*bsize; bjj += 2) {
                base = vld1q_f32(_inns+ii+bjj);
                tmp2 = vmlaq_f32(tmp2, vld1q_dup_f32(_bval+(bjj/2)), base);
            }
            for(; ii+bjj < 2*bsize; bjj += 2) {
                size_t brem = 2*bsize-ii-bjj;
                for(size_t dkk = 0; dkk < brem; dkk++) {
                    base[dkk] = _inns[ii+bjj+dkk];
                }
                for(size_t dkk = 0; dkk < 4-brem; dkk++) {
                    base[dkk+brem] = gain*input[ii+bjj+dkk-2*bsize+brem];
                }
                tmp2 = vmlaq_f32(tmp2, vld1q_dup_f32(_bval+(bjj/2)), base);
            }
            for(; bjj < 2*bsize; bjj += 2) {
                base = vmulq_f32(thegain, vld1q_f32(input+ii+bjj-2*bsize));
                tmp2 = vmlaq_f32(tmp2, vld1q_dup_f32(_bval+(bjj/2)), base);
            }
            
            // D[r] * x
            tmp3 = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2),0),vld1q_f32(_d2));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_low_f32(tmp2),1), vld1q_f32(_d2+4));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),0),vld1q_f32(_d2+8));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),1),vld1q_f32(_d2+12));

            // Shift to output and handoff
            tmp2 = vaddq_f32(tmp1,tmp3);
            for(size_t dkk = 0; dkk < 4-arem; dkk++) {
                output[ii+2*asize+dkk] = tmp2[dkk];
            }
            for(size_t dkk = 0; dkk < arem; dkk++) {
                _outs[dkk] = tmp2[4-arem+dkk];
            }
            ii += 4;
        }
        
        // Pure buffer (when coeff > 4)
        for(size_t kk=arem; kk < 2*asize; kk += 4) {
            // C[r] * y
            tmp1 = {0,0,0,0};
            size_t ajj = 0;
            for(ajj = 0; kk+ajj < 2*asize; ajj++) {
                tmp1 = vmlaq_f32(tmp1, vld1q_dup_f32(output+2*(size-asize)+kk+ajj), vld1q_f32(_c2+4*ajj));
            }
            for(; ajj < 2*asize; ajj++) {
                tmp1 = vmlaq_f32(tmp1, vld1q_dup_f32(_outs+kk+ajj-2*asize), vld1q_f32(_c2+4*ajj));
            }
            
            // FIR
            size_t bjj;
            size_t mm = ii+kk-arem;
            tmp2 = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), vld1q_f32(input+mm)));
            for(bjj = 0; ii+bjj+3 < 2*bsize; bjj += 2) {
                base = vld1q_f32(_inns+mm+bjj);
                tmp2 = vmlaq_f32(tmp2, vld1q_dup_f32(_bval+(bjj/2)), base);
            }
            for(; ii+bjj < 2*bsize; bjj += 2) {
                size_t brem = 2*bsize-mm-bjj;
                for(size_t dkk = 0; dkk < brem; dkk++) {
                    base[dkk] = _inns[mm+bjj+dkk];
                }
                for(size_t dkk = 0; dkk < 4-brem; dkk++) {
                    base[dkk+brem] = gain*input[mm+bjj+dkk-2*bsize+brem];
                }
                tmp2 = vmlaq_f32(tmp2, vld1q_dup_f32(_bval+(bjj/2)), base);
            }
            for(; bjj < 2*bsize; bjj += 2) {
                base = vmulq_f32(thegain, vld1q_f32(input+mm+bjj-2*bsize));
                tmp2 = vmlaq_f32(tmp2, vld1q_dup_f32(_bval+(bjj/2)), base);
            }
            
            // D[r] * x
            tmp3 = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2),0),vld1q_f32(_d2));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_low_f32(tmp2),1), vld1q_f32(_d2+4));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),0),vld1q_f32(_d2+8));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),1),vld1q_f32(_d2+12));

            // Shift to output buffer
            tmp2 = vaddq_f32(tmp1,tmp3);
            vst1q_f32(_outs+kk,tmp2);
        }
        
        size_t kk;
        for(kk = 0; kk+3 < 2*bsize; kk += 4) {
            vst1q_f32(_inns+kk,vmulq_f32(thegain, vld1q_f32(input+2*(size-bsize)+kk)));
        }
        for(; kk < 2*bsize; kk++) {
            _inns[kk] = gain*input[2*(size-bsize)+kk];
        }
    } else {
#else
    {
#endif
        size_t asize = _aval.size();
        size_t bsize = _bval.size();
        
        for(size_t ii=0; ii < 2*asize; ii++) {
            output[ii] = _outs[ii];
        }
        
        for(size_t ii = 0; ii < size-asize; ii++) {
            size_t bjj;
            
            float temp = gain * _b0 * input[2*ii];
            for(bjj = 0; ii+bjj < bsize; bjj++) {
                temp += _bval[bjj]*_inns[2*(ii+bjj)+0];
            }
            for(; bjj < bsize; bjj++) {
                temp += gain * _bval[bjj]*input[2*(ii+bjj-bsize)+0];
            }
            for(size_t ajj = 0; ajj < asize; ajj++) {
                temp += _aval[ajj]*output[2*(ii+ajj)  ];
            }
            output[2*(ii+asize)  ] = temp;
            
            temp = gain*_b0 * input[2*ii+1];
            for(bjj = 0; ii+bjj < bsize; bjj++) {
                temp += _bval[bjj]*_inns[2*(ii+bjj)+1];
            }
            for(; bjj < bsize; bjj++) {
                temp += gain * _bval[bjj]*input[2*(ii+bjj-bsize)+1];
            }
            for(size_t ajj = 0; ajj < asize; ajj++) {
                temp += _aval[ajj]*output[2*(ii+ajj)+1];
            }
            output[2*(ii+asize)+1] = temp;
        }
        
        for(size_t ii=0; ii < asize; ii++) {
            size_t ajj;
            size_t bjj;
            
            float temp = gain*_b0 * input[2*(size-asize+ii)];
            for(bjj = 0; size-asize+ii+bjj < bsize; bjj++) {
                temp += _bval[bjj]*_inns[2*(size-asize+ii+bjj)+0];
            }
            for(; bjj < bsize; bjj++) {
                temp += gain * _bval[bjj]*input[2*(ii+bjj+size-asize-bsize)+0];
            }
            for(ajj = 0; ii+ajj < asize; ajj++) {
                temp += _aval[ajj]*output[2*(size-asize+ii+ajj)];
            }
            for(; ajj < asize; ajj++) {
                temp += _aval[ajj]*_outs[2*(ii+ajj-asize)];
            }
            _outs[2*ii  ] = temp;
            
            temp = gain*_b0 * input[2*(size-asize+ii)+1];
            for(bjj = 0; size-asize+ii+bjj < bsize; bjj++) {
                temp += _bval[bjj]*_inns[2*(size-asize+ii+bjj)+1];
            }
            for(; bjj < bsize; bjj++) {
                temp += gain * _bval[bjj]*input[2*(ii+bjj+size-asize-bsize)+1];
            }
            for(ajj = 0; ii+ajj < asize; ajj++) {
                temp += _aval[ajj]*output[2*(size-asize+ii+ajj)+1];
            }
            for(; ajj < asize; ajj++) {
                temp += _aval[ajj]*_outs[2*(ii+ajj-asize)+1];
            }
            _outs[2*ii+1] = temp;
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
void IIRFilter::trio(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        // This is REALLY complicated code for a negligible speed-up over strideOpt
        float32x4x3_t tmp1, tmp2, tmp3;
        float32x4x3_t base;
        base.val[0] = {0,0,0,0};
        base.val[1] = {0,0,0,0};
        base.val[2] = {0,0,0,0};

        size_t asize = _aval.size();
        size_t bsize = _bval.size();
        
        size_t ii;
        for(ii=0; ii+3 < 3*asize; ii += 4) {
            vst1q_f32(output+ii,vld1q_f32(_outs+ii));
        }
        for(; ii < 3*asize; ii++) {
            output[ii] = _outs[ii];
        }
        
        float32x4_t thegain = vld1q_dup_f32(&gain);
        for(ii = 0; ii+11 < 3*(size-asize); ii += 12) {
            // C[r] * y
            tmp1.val[0] = {0,0,0,0};
            tmp1.val[1] = {0,0,0,0};
            tmp1.val[2] = {0,0,0,0};
            for(size_t ajj = 0; ajj < 3*asize; ajj++) {
                tmp1.val[ajj % 3] = vmlaq_f32(tmp1.val[ajj % 3], vld1q_dup_f32(output+ii+ajj), vld1q_f32(_c1+4*(ajj/3)));
            }
            
            // FIR
            size_t bjj;
            base = vld3q_f32(input+ii);
            tmp2.val[0] = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), base.val[0]));
            tmp2.val[1] = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), base.val[1]));
            tmp2.val[2] = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), base.val[2]));
            for(bjj = 0; ii+bjj+11 < 3*bsize; bjj += 3) {
                base = vld3q_f32(_inns+ii+bjj);
                tmp2.val[0] = vmlaq_f32(tmp2.val[0], vld1q_dup_f32(_bval+(bjj/3)), base.val[0]);
                tmp2.val[1] = vmlaq_f32(tmp2.val[1], vld1q_dup_f32(_bval+(bjj/3)), base.val[1]);
                tmp2.val[2] = vmlaq_f32(tmp2.val[2], vld1q_dup_f32(_bval+(bjj/3)), base.val[2]);
            }
            for(; ii+bjj < 3*bsize; bjj += 3) {
                size_t brem = 3*bsize-ii-bjj;
                for(size_t dkk = 0; dkk < brem; dkk++) {
                    base.val[dkk % 3][(dkk)/3] = _inns[ii+bjj+dkk];
                }
                for(size_t dkk = 0; dkk < 12-brem; dkk++) {
                    base.val[(dkk+brem) % 3][(dkk+brem)/3] = gain*input[ii+bjj+dkk-3*bsize+brem];
                }
                tmp2.val[0] = vmlaq_f32(tmp2.val[0], vld1q_dup_f32(_bval+(bjj/3)), base.val[0]);
                tmp2.val[1] = vmlaq_f32(tmp2.val[1], vld1q_dup_f32(_bval+(bjj/3)), base.val[1]);
                tmp2.val[2] = vmlaq_f32(tmp2.val[2], vld1q_dup_f32(_bval+(bjj/3)), base.val[2]);
            }
            for(; bjj < 3*bsize; bjj += 3) {
                base = vld3q_f32(input+ii+bjj-3*bsize);
                base.val[0] = vmulq_f32(thegain, base.val[0]);
                base.val[1] = vmulq_f32(thegain, base.val[1]);
                base.val[2] = vmulq_f32(thegain, base.val[2]);
                tmp2.val[0] = vmlaq_f32(tmp2.val[0], vld1q_dup_f32(_bval+(bjj/3)), base.val[0]);
                tmp2.val[1] = vmlaq_f32(tmp2.val[1], vld1q_dup_f32(_bval+(bjj/3)), base.val[1]);
                tmp2.val[2] = vmlaq_f32(tmp2.val[2], vld1q_dup_f32(_bval+(bjj/3)), base.val[2]);
            }
            
            // D[r] * x
            for(int ll = 0; ll < 3; ll++) {
                tmp3.val[ll] = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2.val[ll]),0),vld1q_f32(_d1));
                tmp3.val[ll] = vmlaq_f32(tmp3.val[ll],vdupq_lane_f32(vget_low_f32(tmp2.val[ll]),1), vld1q_f32(_d1+4));
                tmp3.val[ll] = vmlaq_f32(tmp3.val[ll],vdupq_lane_f32(vget_high_f32(tmp2.val[ll]),0),vld1q_f32(_d1+8));
                tmp3.val[ll] = vmlaq_f32(tmp3.val[ll],vdupq_lane_f32(vget_high_f32(tmp2.val[ll]),1),vld1q_f32(_d1+12));
            }

            // Shift to output and repeat
            tmp2.val[0] = vaddq_f32(tmp1.val[0],tmp3.val[0]);
            tmp2.val[1] = vaddq_f32(tmp1.val[1],tmp3.val[1]);
            tmp2.val[2] = vaddq_f32(tmp1.val[2],tmp3.val[2]);
            vst3q_f32(output+ii+3*asize,tmp2);
        }
        
        // Intermediate case
        size_t arem = ((3*asize) % 12);
        if (arem) {
            // C[r] * y
            tmp1.val[0] = {0,0,0,0};
            tmp1.val[1] = {0,0,0,0};
            tmp1.val[2] = {0,0,0,0};
            for(size_t ajj = 0; ajj < 3*asize; ajj++) {
                tmp1.val[ajj % 3] = vmlaq_f32(tmp1.val[ajj % 3], vld1q_dup_f32(output+ii+ajj), vld1q_f32(_c1+4*(ajj/3)));
            }
            
            // FIR
            size_t bjj;
            base = vld3q_f32(input+ii);
            tmp2.val[0] = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), base.val[0]));
            tmp2.val[1] = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), base.val[1]));
            tmp2.val[2] = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), base.val[2]));
            for(bjj = 0; ii+bjj+11 < 3*bsize; bjj += 3) {
                base = vld3q_f32(_inns+ii+bjj);
                tmp2.val[0] = vmlaq_f32(tmp2.val[0], vld1q_dup_f32(_bval+(bjj/3)), base.val[0]);
                tmp2.val[1] = vmlaq_f32(tmp2.val[1], vld1q_dup_f32(_bval+(bjj/3)), base.val[1]);
                tmp2.val[2] = vmlaq_f32(tmp2.val[2], vld1q_dup_f32(_bval+(bjj/3)), base.val[2]);
            }
            for(; ii+bjj < 3*bsize; bjj += 3) {
                size_t brem = 3*bsize-ii-bjj;
                for(size_t dkk = 0; dkk < brem; dkk++) {
                    base.val[dkk % 3][dkk/3] = _inns[ii+bjj+dkk];
                }
                for(size_t dkk = 0; dkk < 12-brem; dkk++) {
                    base.val[(dkk+brem) % 3][(dkk+brem)/3] = gain*input[ii+bjj+dkk-3*bsize+brem];
                }
                tmp2.val[0] = vmlaq_f32(tmp2.val[0], vld1q_dup_f32(_bval+(bjj/3)), base.val[0]);
                tmp2.val[1] = vmlaq_f32(tmp2.val[1], vld1q_dup_f32(_bval+(bjj/3)), base.val[1]);
                tmp2.val[2] = vmlaq_f32(tmp2.val[2], vld1q_dup_f32(_bval+(bjj/3)), base.val[2]);
            }
            for(; bjj < 3*bsize; bjj += 3) {
                base = vld3q_f32(input+ii+bjj-3*bsize);
                base.val[0] = vmulq_f32(thegain, base.val[0]);
                base.val[1] = vmulq_f32(thegain, base.val[1]);
                base.val[2] = vmulq_f32(thegain, base.val[2]);
                tmp2.val[0] = vmlaq_f32(tmp2.val[0], vld1q_dup_f32(_bval+(bjj/3)), base.val[0]);
                tmp2.val[1] = vmlaq_f32(tmp2.val[1], vld1q_dup_f32(_bval+(bjj/3)), base.val[1]);
                tmp2.val[2] = vmlaq_f32(tmp2.val[2], vld1q_dup_f32(_bval+(bjj/3)), base.val[2]);
            }
            
            // D[r] * x
            for(int ll = 0; ll < 3; ll++) {
                tmp3.val[ll] = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2.val[ll]),0),vld1q_f32(_d1));
                tmp3.val[ll] = vmlaq_f32(tmp3.val[ll],vdupq_lane_f32(vget_low_f32(tmp2.val[ll]),1), vld1q_f32(_d1+4));
                tmp3.val[ll] = vmlaq_f32(tmp3.val[ll],vdupq_lane_f32(vget_high_f32(tmp2.val[ll]),0),vld1q_f32(_d1+8));
                tmp3.val[ll] = vmlaq_f32(tmp3.val[ll],vdupq_lane_f32(vget_high_f32(tmp2.val[ll]),1),vld1q_f32(_d1+12));
            }

            // Shift to output and handoff
            tmp2.val[0] = vaddq_f32(tmp1.val[0],tmp3.val[0]);
            tmp2.val[1] = vaddq_f32(tmp1.val[1],tmp3.val[1]);
            tmp2.val[2] = vaddq_f32(tmp1.val[2],tmp3.val[2]);
            for(size_t dkk = 0; dkk < 12-arem; dkk++) {
                output[ii+3*asize+dkk] = tmp2.val[dkk % 3][dkk/3];
            }
            for(size_t dkk = 0; dkk < arem; dkk++) {
                _outs[dkk] = tmp2.val[(12-arem+dkk) % 3][(12-arem+dkk)/3];
            }
            ii += 12;
        }
        
        // Pure buffer (when coeff > 4)
        for(size_t kk=arem; kk < 3*asize; kk += 12) {
            // C[r] * y
            tmp1.val[0] = {0,0,0,0};
            tmp1.val[1] = {0,0,0,0};
            tmp1.val[2] = {0,0,0,0};
            size_t ajj = 0;
            for(ajj = 0; kk+ajj < 3*asize; ajj++) {
                tmp1.val[ajj % 3] = vmlaq_f32(tmp1.val[ajj % 3], vld1q_dup_f32(output+3*(size-asize)+kk+ajj), vld1q_f32(_c1+4*(ajj/3)));
            }
            for(; ajj < 3*asize; ajj++) {
                tmp1.val[ajj % 3] = vmlaq_f32(tmp1.val[ajj % 3], vld1q_dup_f32(_outs+kk+ajj-3*asize), vld1q_f32(_c1+4*(ajj/3)));
            }
            
            // FIR
            size_t bjj;
            size_t mm = ii+kk-arem;
            base = vld3q_f32(input+mm);
            tmp2.val[0] = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), base.val[0]));
            tmp2.val[1] = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), base.val[1]));
            tmp2.val[2] = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), base.val[2]));
            for(bjj = 0; ii+bjj+11 < 3*bsize; bjj += 3) {
                base = vld3q_f32(_inns+mm+bjj);
                tmp2.val[0] = vmlaq_f32(tmp2.val[0], vld1q_dup_f32(_bval+(bjj/3)), base.val[0]);
                tmp2.val[1] = vmlaq_f32(tmp2.val[1], vld1q_dup_f32(_bval+(bjj/3)), base.val[1]);
                tmp2.val[2] = vmlaq_f32(tmp2.val[2], vld1q_dup_f32(_bval+(bjj/3)), base.val[2]);
            }
            for(; ii+bjj < 3*bsize; bjj += 3) {
                size_t brem = 3*bsize-mm-bjj;
                for(size_t dkk = 0; dkk < brem; dkk++) {
                    base.val[dkk % 3][(dkk)/3] = _inns[mm+bjj+dkk];
                }
                for(size_t dkk = 0; dkk < 4-brem; dkk++) {
                    base.val[(dkk+brem) % 3][(dkk+brem)/3] = gain*input[mm+bjj+dkk-3*bsize+brem];
                }
                tmp2.val[0] = vmlaq_f32(tmp2.val[0], vld1q_dup_f32(_bval+(bjj/3)), base.val[0]);
                tmp2.val[1] = vmlaq_f32(tmp2.val[1], vld1q_dup_f32(_bval+(bjj/3)), base.val[1]);
                tmp2.val[2] = vmlaq_f32(tmp2.val[2], vld1q_dup_f32(_bval+(bjj/3)), base.val[2]);
            }
            for(; bjj < 3*bsize; bjj += 3) {
                base = vld3q_f32(input+mm+bjj-3*bsize);
                base.val[0] = vmulq_f32(thegain, base.val[0]);
                base.val[1] = vmulq_f32(thegain, base.val[1]);
                base.val[2] = vmulq_f32(thegain, base.val[2]);
                tmp2.val[0] = vmlaq_f32(tmp2.val[0], vld1q_dup_f32(_bval+(bjj/3)), base.val[0]);
                tmp2.val[1] = vmlaq_f32(tmp2.val[1], vld1q_dup_f32(_bval+(bjj/3)), base.val[1]);
                tmp2.val[2] = vmlaq_f32(tmp2.val[2], vld1q_dup_f32(_bval+(bjj/3)), base.val[2]);
            }
            
            // D[r] * x
            for(int ll = 0; ll < 3; ll++) {
                tmp3.val[ll] = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2.val[ll]),0),vld1q_f32(_d1));
                tmp3.val[ll] = vmlaq_f32(tmp3.val[ll],vdupq_lane_f32(vget_low_f32(tmp2.val[ll]),1), vld1q_f32(_d1+4));
                tmp3.val[ll] = vmlaq_f32(tmp3.val[ll],vdupq_lane_f32(vget_high_f32(tmp2.val[ll]),0),vld1q_f32(_d1+8));
                tmp3.val[ll] = vmlaq_f32(tmp3.val[ll],vdupq_lane_f32(vget_high_f32(tmp2.val[ll]),1),vld1q_f32(_d1+12));
            }
            
            // Shift to output buffer
            tmp2.val[0] = vaddq_f32(tmp1.val[0],tmp3.val[0]);
            tmp2.val[1] = vaddq_f32(tmp1.val[1],tmp3.val[1]);
            tmp2.val[2] = vaddq_f32(tmp1.val[2],tmp3.val[2]);
            vst3q_f32(_outs+kk,tmp2);
        }
        
        size_t kk;
        for(kk = 0; kk+3 < 3*bsize; kk += 4) {
            vst1q_f32(_inns+kk,vmulq_f32(thegain, vld1q_f32(input+3*(size-bsize)+kk)));
        }
        for(; kk < 3*bsize; kk++) {
            _inns[kk] = gain*input[3*(size-bsize)+kk];
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
void IIRFilter::quad(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        __m128 temp;
        
        size_t asize = _aval.size();
        size_t bsize = _bval.size();
        
        for(size_t ii=0; ii < 4*asize; ii += 4) {
            _mm_store_ps(output+ii,_mm_load_ps(_outs+ii));
        }

        for(size_t ii = 0; ii < 4*(size-asize); ii += 4) {
            size_t bjj;
            temp = _mm_mul_ps(_mm_set1_ps(gain*_b0), _mm_load_ps(input+ii));
            for(bjj = 0; ii+bjj < 4*bsize; bjj += 4) {
                temp = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj/4]),_mm_loadu_ps(_inns+ii+bjj),temp);
            }
            for(; bjj < 4*bsize; bjj += 4) {
                temp = _mm_fmadd_ps(_mm_set1_ps(gain*_bval[bjj/4]),_mm_loadu_ps(input+ii+bjj-4*bsize),temp);
            }
            for(size_t ajj = 0; ajj < 4*asize; ajj += 4) {
                temp = _mm_fmadd_ps(_mm_set1_ps(_aval[ajj/4]),_mm_loadu_ps(output+ii+ajj),temp);
            }
            _mm_store_ps(output+ii+4*asize, temp);
        }
        
        // There is NO intermediate case for this width
        for(size_t ii=0; ii < 4*asize; ii += 4) {
            size_t ajj;
            size_t bjj;
            size_t mm = ii+4*(size-asize);
            temp = _mm_mul_ps(_mm_set1_ps(gain*_b0), _mm_load_ps(input+mm));
            for(bjj = 0; mm+bjj < 4*bsize; bjj += 4) {
                temp = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj/4]),_mm_loadu_ps(_inns+mm+bjj),temp);
            }
            for(; bjj < 4*bsize; bjj += 4) {
                temp = _mm_fmadd_ps(_mm_set1_ps(gain*_bval[bjj/4]),_mm_loadu_ps(input+mm+bjj-4*bsize),temp);
            }
            for(ajj = 0; ajj+ii < 4*asize; ajj += 4) {
                temp = _mm_fmadd_ps(_mm_set1_ps(_aval[ajj/4]),_mm_loadu_ps(output+mm+ajj),temp);
            }
            for(; ajj < 4*asize; ajj += 4) {
                temp = _mm_fmadd_ps(_mm_set1_ps(_aval[ajj/4]),_mm_load_ps(_outs+ajj+ii-4*asize),temp);
            }
            _mm_store_ps(_outs+ii, temp);
        }
        
        size_t kk;
        for(kk = 0; kk+3 < 4*bsize; kk += 4) {
            _mm_store_ps(_inns+kk,_mm_mul_ps(_mm_set1_ps(gain),_mm_loadu_ps(input+4*(size-bsize)+kk)));
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
        
        size_t asize = _aval.size();
        size_t bsize = _bval.size();
        
        for(size_t ii=0; ii < 4*asize; ii += 4) {
            vst1q_f32(output+ii, vld1q_f32(_outs+ii));
        }
        
        float32x4_t thegain = vld1q_dup_f32(&gain);
        for(size_t ii = 0; ii < 4*(size-asize); ii += 4) {
            size_t bjj;
            temp = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), vld1q_f32(input+ii)));
            for(bjj = 0; ii+bjj < 4*bsize; bjj += 4) {
                temp = vmlaq_f32(temp, vld1q_dup_f32(_bval+(bjj/4)), vld1q_f32(_inns+ii+bjj));
            }
            for(; bjj < 4*bsize; bjj += 4) {
                base = vmulq_f32(vld1q_dup_f32(_bval+(bjj/4)), vld1q_f32(input+ii+bjj-4*bsize));
                temp = vmlaq_f32(temp, thegain, base);
            }
            for(size_t ajj = 0; ajj < 4*asize; ajj += 4) {
                temp = vmlaq_f32(temp, vld1q_dup_f32(_aval+(ajj/4)), vld1q_f32(output+ii+ajj));
            }
            vst1q_f32(output+ii+4*asize, temp);
        }
        
        // There is NO intermediate case for this width
        for(size_t ii=0; ii < 4*asize; ii += 4) {
            size_t ajj;
            size_t bjj;
            size_t mm = ii+4*(size-asize);
            temp = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), vld1q_f32(input+mm)));
            for(bjj = 0; mm+bjj < 4*bsize; bjj += 4) {
                temp = vmlaq_f32(temp, vld1q_dup_f32(_bval+(bjj/4)), vld1q_f32(_inns+mm+bjj));
            }
            for(; bjj < 4*bsize; bjj += 4) {
                base = vmulq_f32(vld1q_dup_f32(_bval+(bjj/4)), vld1q_f32(input+mm+bjj-4*bsize));
                temp = vmlaq_f32(temp, thegain, base);
            }
            for(ajj = 0; ajj+ii < 4*asize; ajj += 4) {
                temp = vmlaq_f32(temp, vld1q_dup_f32(_aval+(ajj/4)), vld1q_f32(output+mm+ajj));
            }
            for(; ajj < 4*asize; ajj += 4) {
                temp = vmlaq_f32(temp, vld1q_dup_f32(_aval+(ajj/4)), vld1q_f32(_outs+ajj+ii-4*asize));
            }
            vst1q_f32(_outs+ii, temp);
        }
        
        size_t kk;
        for(kk = 0; kk+3 < 4*bsize; kk += 4) {
            vst1q_f32(_inns+kk,vmulq_f32(thegain,vld1q_f32(input+4*(size-bsize)+kk)));
        }
        for(; kk < 4*bsize; kk++) {
            _inns[kk] = gain*input[4*(size-bsize)+kk];
        }
    } else {
#else
    {
#endif
        size_t asize = _aval.size();
        size_t bsize = _bval.size();
        
        for(size_t ii=0; ii < 4*asize; ii++) {
            output[ii] = _outs[ii];
        }
        
        for(size_t ckk = 0; ckk < 4; ckk++) {
            for(size_t ii = 0; ii < size-asize; ii++) {
                size_t bjj;
                float temp = gain * _b0 * input[4*ii+ckk];
                for(bjj = 0; ii+bjj < bsize; bjj++) {
                    temp += _bval[bjj]*_inns[4*(ii+bjj)+ckk];
                }
                for(; bjj < bsize; bjj++) {
                    temp += gain * _bval[bjj]*input[4*(ii+bjj-bsize)+ckk];
                }
                for(size_t ajj = 0; ajj < asize; ajj++) {
                    temp += _aval[ajj]*output[4*(ii+ajj)+ckk];
                }
                output[4*(ii+asize)+ckk] = temp;
            }
            
            for(size_t ii=0; ii < asize; ii++) {
                size_t ajj;
                size_t bjj;
                
                size_t mm = size-asize+ii;
                float temp = gain * _b0 * input[4*mm+ckk];
                for(bjj = 0; mm+bjj < bsize; bjj++) {
                    temp += _bval[bjj]*_inns[4*(mm+bjj)+ckk];
                }
                for(; bjj < bsize; bjj++) {
                    temp += gain * _bval[bjj]*input[4*(mm+bjj-bsize)+ckk];
                }
                for(ajj = 0; ii+ajj < asize; ajj++) {
                    temp += _aval[ajj]*output[4*(mm+ajj)+ckk];
                }
                for(; ajj < asize; ajj++) {
                    temp += _aval[ajj]*_outs[4*(ii+ajj-asize)+ckk];
                }
                _outs[4*ii+ckk] = temp;
            }
            
            for(size_t bjj = 0; bjj < bsize; bjj++) {
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
void IIRFilter::quart(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
       __m128 temp;
        
        size_t asize = _aval.size();
        size_t bsize = _bval.size();

        for(size_t ii=0; ii < 8*asize; ii += 4) {
            _mm_store_ps(output+ii,  _mm_load_ps(_outs+ii));
        }
        

        for(size_t ii = 0; ii < 8*(size-asize); ii += 8) {
            size_t bjj;

            temp = _mm_mul_ps(_mm_set1_ps(gain*_b0), _mm_load_ps(input+ii));
            for(bjj = 0; ii+bjj < 8*bsize; bjj += 8) {
                temp = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj/8]),_mm_loadu_ps(_inns+ii+bjj),temp);
            }
            for(; bjj < 8*bsize; bjj += 8) {
                temp = _mm_fmadd_ps(_mm_set1_ps(gain*_bval[bjj/8]),_mm_loadu_ps(input+ii+bjj-8*bsize),temp);
            }
            for(size_t ajj = 0; ajj < 4*asize; ajj += 4) {
                temp = _mm_fmadd_ps(_mm_set1_ps(_aval[ajj/4]),_mm_loadu_ps(output+ii+2*ajj),temp);
            }
            _mm_store_ps(output+ii+8*asize, temp);
            
            temp = _mm_mul_ps(_mm_set1_ps(gain*_b0), _mm_load_ps(input+ii+4));
            for(bjj = 0; ii+bjj < 8*bsize; bjj += 8) {
                temp = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj/8]),_mm_loadu_ps(_inns+ii+bjj+4),temp);
            }
            for(; bjj < 8*bsize; bjj += 8) {
                temp = _mm_fmadd_ps(_mm_set1_ps(gain*_bval[bjj/8]),_mm_loadu_ps(input+ii+bjj-8*bsize+4),temp);
            }
            for(size_t ajj = 0; ajj < 4*asize; ajj += 4) {
                temp = _mm_fmadd_ps(_mm_set1_ps(_aval[ajj/4]),_mm_loadu_ps(output+ii+2*ajj+4),temp);
            }
            _mm_store_ps(output+ii+8*asize+4, temp);
        }
        
        // There is NO intermediate case for this width
        for(size_t ii=0; ii < 8*asize; ii += 8) {
            size_t ajj;
            size_t bjj;
            size_t mm = ii+8*(size-asize);

            temp = _mm_mul_ps(_mm_set1_ps(gain*_b0), _mm_load_ps(input+mm));
            for(bjj = 0; mm+bjj < 8*bsize; bjj += 8) {
                temp = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj/8]),_mm_loadu_ps(_inns+mm+bjj),temp);
            }
            for(; bjj < 8*bsize; bjj += 8) {
                temp = _mm_fmadd_ps(_mm_set1_ps(gain*_bval[bjj/8]),_mm_loadu_ps(input+mm+bjj-8*bsize),temp);
            }
            for(ajj = 0; 2*ajj+ii < 8*asize; ajj += 4) {
                temp = _mm_fmadd_ps(_mm_set1_ps(_aval[ajj/4]),_mm_loadu_ps(output+2*ajj+mm),temp);
            }
            for(; ajj < 4*asize; ajj += 4) {
                temp = _mm_fmadd_ps(_mm_set1_ps(_aval[ajj/4]),_mm_load_ps(_outs+2*ajj+ii-8*asize),temp);
            }
            _mm_store_ps(_outs+ii, temp);
            
            temp = _mm_mul_ps(_mm_set1_ps(gain*_b0), _mm_load_ps(input+mm+4));
            for(bjj = 0; mm+bjj < 8*bsize; bjj += 8) {
                temp = _mm_fmadd_ps(_mm_set1_ps(_bval[bjj/8]),_mm_loadu_ps(_inns+mm+bjj+4),temp);
            }
            for(; bjj < 8*bsize; bjj += 8) {
                temp = _mm_fmadd_ps(_mm_set1_ps(gain*_bval[bjj/8]),_mm_loadu_ps(input+mm+bjj-8*bsize+4),temp);
            }
            for(ajj = 0; 2*ajj+ii < 8*asize; ajj += 4) {
                temp = _mm_fmadd_ps(_mm_set1_ps(_aval[ajj/4]),_mm_loadu_ps(output+2*ajj+mm+4),temp);
            }
            for(; ajj < 4*asize; ajj += 4) {
                temp = _mm_fmadd_ps(_mm_set1_ps(_aval[ajj/4]),_mm_load_ps(_outs+2*ajj+ii-8*asize+4),temp);
            }
            _mm_store_ps(_outs+ii+4, temp);
        }
        
        size_t kk;
        for(kk = 0; kk+3 < 8*bsize; kk += 4) {
            _mm_store_ps(_inns+kk,_mm_mul_ps(_mm_set1_ps(gain),_mm_loadu_ps(input+8*(size-bsize)+kk)));
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
        
        size_t asize = _aval.size();
        size_t bsize = _bval.size();
        
        for(size_t ii=0; ii < 8*asize; ii += 4) {
            vst1q_f32(output+ii, vld1q_f32(_outs+ii));
        }
        
        float32x4_t thegain = vld1q_dup_f32(&gain);
        for(size_t ii = 0; ii < 8*(size-asize); ii += 8) {
            size_t bjj;
            
            temp = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), vld1q_f32(input+ii)));
            for(bjj = 0; ii+bjj < 8*bsize; bjj += 8) {
                temp = vmlaq_f32(temp, vld1q_dup_f32(_bval+(bjj/8)), vld1q_f32(_inns+ii+bjj));
            }
            for(; bjj < 8*bsize; bjj += 8) {
                base = vmulq_f32(vld1q_dup_f32(_bval+(bjj/8)), vld1q_f32(input+ii+bjj-8*bsize));
                temp = vmlaq_f32(temp, thegain, base);
            }
            for(size_t ajj = 0; ajj < 4*asize; ajj += 4) {
                temp = vmlaq_f32(temp, vld1q_dup_f32(_aval+(ajj/4)), vld1q_f32(output+ii+2*ajj));
            }
            vst1q_f32(output+ii+8*asize, temp);
            
            temp = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), vld1q_f32(input+ii+4)));
            for(bjj = 0; ii+bjj < 8*bsize; bjj += 8) {
                temp = vmlaq_f32(temp, vld1q_dup_f32(_bval+(bjj/8)), vld1q_f32(_inns+ii+bjj+4));
            }
            for(; bjj < 8*bsize; bjj += 8) {
                base = vmulq_f32(vld1q_dup_f32(_bval+(bjj/8)), vld1q_f32(input+ii+bjj-8*bsize+4));
                temp = vmlaq_f32(temp, thegain, base);
            }
            for(size_t ajj = 0; ajj < 4*asize; ajj += 4) {
                temp = vmlaq_f32(temp, vld1q_dup_f32(_aval+(ajj/4)), vld1q_f32(output+ii+2*ajj+4));
            }
            vst1q_f32(output+ii+8*asize+4, temp);
        }
        
        // There is NO intermediate case for this width
        for(size_t ii=0; ii < 8*asize; ii += 8) {
            size_t ajj;
            size_t bjj;
            size_t mm = ii+8*(size-asize);
            
            temp = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), vld1q_f32(input+mm)));
            for(bjj = 0; mm+bjj < 8*bsize; bjj += 8) {
                temp = vmlaq_f32(temp, vld1q_dup_f32(_bval+(bjj/8)), vld1q_f32(_inns+mm+bjj));
            }
            for(; bjj < 8*bsize; bjj += 8) {
                base = vmulq_f32(vld1q_dup_f32(_bval+(bjj/8)), vld1q_f32(input+mm+bjj-8*bsize));
                temp = vmlaq_f32(temp, thegain, base);
            }
            for(ajj = 0; 2*ajj+ii < 8*asize; ajj += 4) {
                temp = vmlaq_f32(temp, vld1q_dup_f32(_aval+(ajj/4)), vld1q_f32(output+mm+2*ajj));
            }
            for(; ajj < 4*asize; ajj += 4) {
                temp = vmlaq_f32(temp, vld1q_dup_f32(_aval+(ajj/4)), vld1q_f32(_outs+2*ajj+ii-8*asize));
            }
            vst1q_f32(_outs+ii, temp);
            
            temp = vmulq_f32(thegain, vmulq_f32(vld1q_dup_f32(&_b0), vld1q_f32(input+mm+4)));
            for(bjj = 0; mm+bjj < 8*bsize; bjj += 8) {
                temp = vmlaq_f32(temp, vld1q_dup_f32(_bval+(bjj/8)), vld1q_f32(_inns+mm+bjj+4));
            }
            for(; bjj < 8*bsize; bjj += 8) {
                base = vmulq_f32(vld1q_dup_f32(_bval+(bjj/8)), vld1q_f32(input+mm+bjj-8*bsize+4));
                temp = vmlaq_f32(temp, thegain, base);
            }
            for(ajj = 0; 2*ajj+ii < 8*asize; ajj += 4) {
                temp = vmlaq_f32(temp, vld1q_dup_f32(_aval+(ajj/4)), vld1q_f32(output+mm+2*ajj+4));
            }
            for(; ajj < 4*asize; ajj += 4) {
                temp = vmlaq_f32(temp, vld1q_dup_f32(_aval+(ajj/4)), vld1q_f32(_outs+2*ajj+ii-8*asize+4));
            }
            vst1q_f32(_outs+ii+4, temp);
        }
        
        size_t kk;
        for(kk = 0; kk+3 < 8*bsize; kk += 4) {
            vst1q_f32(_inns+kk, vmulq_f32(thegain, vld1q_f32(input+8*(size-bsize)+kk)));
        }
        for(; kk < 8*bsize; kk++) {
            _inns[kk] = input[8*(size-bsize)+kk];
        }
    } else {
#else
    {
#endif
        size_t asize = _aval.size();
        size_t bsize = _bval.size();
        
        for(size_t ii=0; ii < 8*asize; ii++) {
            output[ii] = _outs[ii];
        }
        
        for(int ckk = 0; ckk < 8; ckk++) {
            for(size_t ii = 0; ii < size-asize; ii++) {
                size_t bjj;
                
                float temp = gain * _b0 * input[8*ii+ckk];
                for(bjj = 0; ii+bjj < bsize; bjj++) {
                    temp += _bval[bjj]*_inns[8*(ii+bjj)+ckk];
                }
                for(; bjj < bsize; bjj++) {
                    temp += gain * _bval[bjj]*input[8*(ii+bjj-bsize)+ckk];
                }
                for(size_t ajj = 0; ajj < asize; ajj++) {
                    temp += _aval[ajj]*output[8*(ii+ajj)+ckk];
                }
                output[8*(ii+asize)+ckk] = temp;
            }
            
            for(size_t ii=0; ii < asize; ii++) {
                size_t ajj;
                size_t bjj;
                size_t mm = size-asize+ii;
                
                float temp = gain*_b0 * input[8*mm+ckk];
                for(bjj = 0; mm+bjj < bsize; bjj++) {
                    temp += _bval[bjj]*_inns[8*(mm+bjj)+ckk];
                }
                for(; bjj < bsize; bjj++) {
                    temp += gain * _bval[bjj]*input[8*(mm+bjj-bsize)+ckk];
                }
                for(ajj = 0; ii+ajj < asize; ajj++) {
                    temp += _aval[ajj]*output[8*(mm+ajj)+ckk];
                }
                for(; ajj < asize; ajj++) {
                    temp += _aval[ajj]*_outs[8*(ii+ajj-asize)+ckk];
                }
                
                _outs[8*ii+ckk] = temp;
            }
            
            for(size_t bjj = 0; bjj < bsize; bjj++) {
                _inns[8*bjj+ckk] = gain * input[8*(size-bsize+bjj)+ckk];
            }
        }
    }
}

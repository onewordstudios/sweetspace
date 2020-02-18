//
//  CUBiquadIIR.cpp
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
#include <cugl/math/dsp/CUBiquadIIR.h>
#include <cugl/util/CUDebug.h>
#include "cuDSP128.inl"

using namespace cugl;
using namespace cugl::dsp;

/** Conversion factor for bandwidth calculations */
#define LN2_OVER2   0.3465735903

/** Whether to use a vectorization algorithm */
bool BiquadIIR::VECTORIZE = true;

#pragma mark -
#pragma mark Constructors

/**
 * Creates a second-order pass-through filter for a single channel.
 */
BiquadIIR::BiquadIIR() :
_b0(1.0f),
_b1(0.0f),
_b2(0.0f),
_a1(0.0f),
_a2(0.0f),
_channels(1) {
    _inns.reset(0, 16);
    _outs.reset(0, 16);
    reset();
}

/**
 * Creates a second-order pass-through filter for the given number of channels.
 *
 * @param channels  The number of channels
 */
BiquadIIR::BiquadIIR(unsigned channels) :
_b0(1.0f),
_b1(0.0f),
_b2(0.0f),
_a1(0.0f),
_a2(0.0f) {
    _inns.reset(0, 16);
    _outs.reset(0, 16);
    _channels = channels;
    reset();
}

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
 * @param type      The filter type
 * @param frequency The (normalized) target frequency
 * @param gainDB    The gain at the target frequency in decibels
 * @param qVal      The special Q factor
 */
BiquadIIR::BiquadIIR(unsigned channels, Type type, float frequency, float gainDB, float qVal) {
    _inns.reset(0, 16);
    _outs.reset(0, 16);
    _channels = channels;
    setType(type,frequency,gainDB,qVal);
}

/**
 * Creates a copy of the biquad filter.
 *
 * @param filter    The filter to copy
 */
BiquadIIR::BiquadIIR(const BiquadIIR& copy) {
    _a1 = copy._a1;
    _a2 = copy._a2;
    _b0 = copy._b0;
    _b1 = copy._b1;
    _b2 = copy._b2;
    _channels = copy._channels;
    std::memcpy(_c1,copy._c1,8*sizeof(float));
    std::memcpy(_c2,copy._c2,16*sizeof(float));
    std::memcpy(_d1,copy._d1,16*sizeof(float));
    std::memcpy(_d2,copy._d2,16*sizeof(float));
    _inns = copy._inns;
    _outs = copy._outs;
}

/**
 * Creates a biquad filter with the resources of the original.
 *
 * @param filter    The filter to acquire
 */
BiquadIIR::BiquadIIR(BiquadIIR&& filter) {
    _a1 = filter._a1;
    _a2 = filter._a2;
    _b0 = filter._b0;
    _b1 = filter._b1;
    _b2 = filter._b2;
    _channels = filter._channels;
    std::memcpy(_c1,filter._c1,8*sizeof(float));
    std::memcpy(_c2,filter._c2,16*sizeof(float));
    std::memcpy(_d1,filter._d1,16*sizeof(float));
    std::memcpy(_d2,filter._d2,16*sizeof(float));
    _inns = std::move(filter._inns);
    _outs = std::move(filter._outs);
}

/**
 * Destroys the filter, releasing all resources.
 */
BiquadIIR::~BiquadIIR() {}

/**
 * Resets the caching data structures for this filter
 *
 * This must be called if the number of channels or coefficients change.
 */
void BiquadIIR::reset() {
    // Single channel optimizations
    _c1[0] = -_a2;
    _c1[4] = -_a1;
    _c1[1] = -_a1*_c1[0];
    _c1[5] = -_a1*_c1[4]-_a2;
    _c1[2] = -_a1*_c1[1]-_a2*_c1[0];
    _c1[6] = -_a1*_c1[5]-_a2*_c1[4];
    _c1[3] = -_a1*_c1[2]-_a2*_c1[1];
    _c1[7] = -_a1*_c1[6]-_a2*_c1[5];
    
#if defined (CU_MATH_VECTOR_SSE)
    _mm_store_ps(_d1,       _mm_setr_ps(   1,   _c1[4], _c1[5],      _c1[6]));
    _mm_store_ps(_d1+4,     _mm_setr_ps(   0,    1,     _c1[4],      _c1[5]));
    _mm_store_ps(_d1+8,     _mm_setr_ps(   0,    0,      1,          _c1[4]));
    _mm_store_ps(_d1+12,    _mm_setr_ps(   0,    0,      0,           1));
    
    // Dual channel optimizations
    _mm_store_ps(_c2,       _mm_setr_ps( -_a2,   0,     _a1*_a2,      0        ));
    _mm_store_ps(_c2+4,     _mm_setr_ps(   0,  -_a2,     0,          _a1*_a2   ));
    _mm_store_ps(_c2+8,     _mm_setr_ps( -_a1,   0,     _a1*_a1-_a2,  0        ));
    _mm_store_ps(_c2+12,    _mm_setr_ps(   0,  -_a1,     0,          _a1*_a1-_a2));
    
    _mm_store_ps(_d2,       _mm_setr_ps(   1,    0,     _c2[8],       0 ));
    _mm_store_ps(_d2+4,     _mm_setr_ps(   0,    1,      0,          _c2[13] ));
    _mm_store_ps(_d2+8,     _mm_setr_ps(   0,    0,      1,           0 ));
    _mm_store_ps(_d2+12,    _mm_setr_ps(   0,    0,      0,           1 ));
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if ( android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    {
#endif
        float32x4_t temp;
        temp = {   1,   _c1[4], _c1[5],      _c1[6] };
        vst1q_f32(_d1   , temp);
        temp = {   0,    1,     _c1[4],      _c1[5] };
        vst1q_f32(_d1+4 , temp);
        temp = {   0,    0,      1,          _c1[4] };
        vst1q_f32(_d1+8 , temp);
        temp = {   0,    0,      0,           1     };
        vst1q_f32(_d1+12, temp);
    
        temp = { -_a2,     0,  _a1*_a2,       0      };
        vst1q_f32(_c2   , temp);
        temp = {    0,  -_a2,        0,  _a1*_a2     };
        vst1q_f32(_c2+4 , temp);
        temp = { -_a1,    0,  _a1*_a1-_a2,       0      };
        vst1q_f32(_c2+8 , temp);
        temp = {    0, -_a1,        0,  _a1*_a1-_a2     };
        vst1q_f32(_c2+12, temp);
    
        temp = {   1,    0,     _c2[8],       0      };
        vst1q_f32(_d2   , temp);
        temp = {   0,    1,      0,          _c2[13]  };
        vst1q_f32(_d2+4 , temp);
        temp = {   0,    0,      1,           0      };
        vst1q_f32(_d2+8 , temp);
        temp = {   0,    0,      0,           1      };
        vst1q_f32(_d2+12, temp);
    }
#endif
    
    _outs.reset(2*_channels,16);
    _inns.reset(2*_channels,16);
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
void BiquadIIR::setChannels(unsigned channels) {
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
 * All b-coefficients and a-coefficients after the third are ignored.  If any
 * coefficients are missing, they are replaced with 1 for b[0] and a[0], and
 * 0 otherwise.
 *
 * @param bvals The upper coefficients
 * @param avals The lower coefficients
 */
void BiquadIIR::setCoeff(const std::vector<float> &bvals, const std::vector<float> &avals) {
    float a0 = avals.size() == 0 ? 1.0f : avals[0];
    _b0 = (bvals.size() == 0 ? 1.0f : bvals[0])/a0;
    _b1 = (bvals.size() <= 1 ? 0.0f : bvals[1])/a0;
    _b2 = (bvals.size() <= 2 ? 0.0f : bvals[2])/a0;
    _a1 = (avals.size() <= 1 ? 0.0f : avals[1])/a0;
    _a2 = (avals.size() <= 2 ? 0.0f : avals[2])/a0;
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
const std::vector<float> BiquadIIR::getBCoeff() const {
    std::vector<float> result;
    result.push_back(_b0);
    result.push_back(_b1);
    result.push_back(_b2);
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
const std::vector<float> BiquadIIR::getACoeff() const {
    std::vector<float> result;
    result.push_back(1.0f);  // Assume normalization
    result.push_back(_a1);
    result.push_back(_a2);
    return result;
}

#pragma mark -
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
void BiquadIIR::setBCoeff(float b0, float b1, float b2) {
    _b0 = b0;
    _b1 = b1;
    _b2 = b2;
    reset();
}

/**
 * Sets the lower coefficients.
 *
 * Setting this leaves the upper coefficients unchanged.
 *
 * @param a1    The lower first-order coefficient
 * @param a2    The lower second-order coefficient
 */
void BiquadIIR::setACoeff(float a1, float a2) {
    _a1 = a1;
    _a2 = a2;
    reset();
}

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
void BiquadIIR::setType(Type type, float frequency, float gainDB, float qVal) {
    double norm;
    double vval = pow(10, fabs(gainDB) / 20.0);
    double kval = tan(M_PI * frequency);
    switch (type) {
        case Type::LOWPASS:
            norm = 1 / (1 + kval / qVal + kval * kval);
            _b0 = kval * kval * norm;
            _b1 = 2 * _b0;
            _b2 = _b0;
            _a1 = 2 * (kval * kval - 1) * norm;
            _a2 = (1 - kval / qVal + kval * kval) * norm;
            break;
            
        case Type::HIGHPASS:
            norm = 1 / (1 + kval / qVal + kval * kval);
            _b0 = 1 * norm;
            _b1 = -2 * _b0;
            _b2 = _b0;
            _a1 = 2 * (kval * kval - 1) * norm;
            _a2 = (1 - kval / qVal + kval * kval) * norm;
            break;
            
        case Type::BANDPASS:
            norm = 1 / (1 + kval / qVal + kval * kval);
            _b0 = kval / qVal * norm;
            _b1 = 0;
            _b2 = -_b0;
            _a1 = 2 * (kval * kval - 1) * norm;
            _a2 = (1 - kval / qVal + kval * kval) * norm;
            break;
            
        case Type::NOTCH:
            norm = 1 / (1 + kval / qVal + kval * kval);
            _b0 = (1 + kval * kval) * norm;
            _b1 = 2 * (kval * kval - 1) * norm;
            _b2 = _b0;
            _a1 = _b1;
            _a2 = (1 - kval / qVal + kval * kval) * norm;
            break;
            
        case Type::PEAK:
            if (gainDB >= 0) {       // boost
                norm = 1 / (1 + kval / qVal + kval * kval);
                _b0 = (1 + vval/qVal * kval + kval * kval) * norm;
                _b1 = 2 * (kval * kval - 1) * norm;
                _b2 = (1 - vval/qVal * kval + kval * kval) * norm;
                _a1 = _b1;
                _a2 = (1 - kval/qVal + kval * kval) * norm;
            } else {                    // cut
                norm = 1 / (1 + vval/qVal * kval + kval * kval);
                _b0 = (1 + kval/qVal + kval * kval) * norm;
                _b1 = 2 * (kval * kval - 1) * norm;
                _b2 = (1 - kval/qVal + kval * kval) * norm;
                _a1 = _b1;
                _a2 = (1 - vval/qVal * kval + kval * kval) * norm;
            }
            break;
            
        case Type::LOWSHELF:
            if (gainDB >= 0) {       // boost
                norm = 1 / (1 + sqrt(2) * kval + vval * kval * kval);
                _b0 = (1 + sqrt(2*vval) * kval + vval * kval * kval) * norm;
                _b1 = 2 * (vval * kval * kval - 1) * norm;
                _b2 = (1 - sqrt(2*vval) * kval + vval * kval * kval) * norm;
                _a1 = 2 * (kval * kval - 1) * norm;
                _a2 = (1 - sqrt(2) * kval + kval * kval) * norm;
            } else {                    // cut
                norm = 1 / (1 + sqrt(2*vval) * kval + vval * kval * kval);
                _b0 = (1 + sqrt(2) * kval + kval * kval) * norm;
                _b1 = 2 * (kval * kval - 1) * norm;
                _b2 = (1 - sqrt(2) * kval + kval * kval) * norm;
                _a1 = 2 * (vval * kval * kval - 1) * norm;
                _a2 = (1 - sqrt(2*vval) * kval + vval * kval * kval) * norm;
            }
            break;
            
        case Type::HIGHSHELF:
            if (gainDB >= 0) {       // boost
                norm = 1 / (1 + sqrt(2) * kval + kval * kval);
                _b0 = (vval + sqrt(2*vval) * kval + kval * kval) * norm;
                _b1 = 2 * (kval * kval - vval) * norm;
                _b2 = (vval - sqrt(2*vval) * kval + kval * kval) * norm;
                _a1 = 2 * (kval * kval - 1) * norm;
                _a2 = (1 - sqrt(2) * kval + kval * kval) * norm;
            } else {                    // cut
                norm = 1 / (vval + sqrt(2*vval) * kval + kval * kval);
                _b0 = (1 + sqrt(2) * kval + kval * kval) * norm;
                _b1 = 2 * (kval * kval - 1) * norm;
                _b2 = (1 - sqrt(2) * kval + kval * kval) * norm;
                _a1 = 2 * (kval * kval - vval) * norm;
                _a2 = (vval - sqrt(2*vval) * kval + kval * kval) * norm;
            }
            break;
            
            // Taken from http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
        case Type::ALLPASS:
            norm = 1 / (1 + kval / qVal + kval * kval);
            _b0 = (1 - kval / qVal + kval * kval) * norm;
            _b1 = 2 * (kval * kval - 1) * norm;
            _b2 = 1;
            _a1 = _b1;
            _a2 = _b0;
            break;
            
            // Taken from STK (Unsure if this is appropriate)
        case Type::RESONANCE:
            assert(frequency >= 0.0 && frequency <= 0.5);
            _b0 = 0.5 - 0.5 * qVal * qVal;
            _b1 = 0.0;
            _b2 = -_b1;
            _a1 = -2.0 * qVal * cos( 2 * M_PI * frequency );
            _a2 = qVal * qVal;
            break;
            
        default:
            break;
    }
    
    reset();
    return;
}

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
float BiquadIIR::db2gain(float gainDB) {
    return pow(10, gainDB / 20.0);
}

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
float BiquadIIR::gain2db(float gain) {
    return log10f(gain)*20.0f;
}

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
float BiquadIIR::bandwidth2q(float width) {
    return 1/(2.0*sinh(LN2_OVER2*width));
}

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
float BiquadIIR::q2Bandwidth(float qVal) {
    double z = 1.0/(2.0*qVal);
    return asinh(z)/LN2_OVER2;
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
void BiquadIIR::step(float gain, float* input, float* output) {
    for(size_t ckk = 0; ckk < _channels; ckk++) {
        output[ckk] = _outs[ckk];
        float temp = gain * _b0 * input[ckk] + _b1 * _inns[ckk+_channels] +  _b2 * _inns[ckk];
        temp -= _a1 * _outs[ckk+_channels] + _a2 * _outs[ckk];
        _inns[ckk] = _inns[ckk+_channels];
        _inns[ckk+_channels] = gain * input[ckk];
        _outs[ckk] = _outs[ckk+_channels];
        _outs[ckk+_channels] = temp;
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
void BiquadIIR::calculate(float gain, float* input, float* output, size_t size) {
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
                float temp = gain * _b0 * input[ii*_channels+ckk] + _b1 * _inns[ckk+_channels] +  _b2 * _inns[ckk];
                temp -= _a1 * _outs[ckk+_channels] + _a2 * _outs[ckk];
                _inns[ckk] = _inns[ckk+_channels];
                _inns[ckk+_channels] = gain * input[ii*_channels+ckk];
                _outs[ckk] = _outs[ckk+_channels];
                _outs[ckk+_channels] = temp;
            }
        }
    }
}

/**
 * Clears the filter buffer of any delayed outputs or cached inputs
 */
void BiquadIIR::clear() {
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
 * The array size should be the number of channels times one less the
 * number of a-coefficients.
 *
 * This method will also clear the buffer.
 *
 * @return The number of frames (not samples) written
 */
size_t BiquadIIR::flush(float* output) {
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
void BiquadIIR::stride(float gain, float* input, float* output, size_t size, unsigned channel) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        __m128 pout, pinn;
        __m128 tmp1, tmp2, tmp3;
        __m128 data, shuf;
    
        unsigned stride = _channels;
        __m128 factor0 = _mm_set1_ps(_b0);
        __m128 factor1 = _mm_set1_ps(_b1);
        __m128 factor2 = _mm_set1_ps(_b2);
    
        pout = _mm_set_ps(_outs[channel+stride],_outs[channel],0,0);
        pinn = _mm_set_ps(_inns[channel+stride],_inns[channel],0,0);
        for(int ii = 0; ii < size; ii += 4) {
            // C[r] * y
            tmp2 = _mm_set1_ps(pout[2]);
            tmp3 = _mm_set1_ps(pout[3]);
            tmp1 = _mm_fmadd_ps(tmp2,_mm_load_ps(_c1),_mm_mul_ps(tmp3,_mm_load_ps(_c1+4)));
        
            // Pack to alignment
            data = _mm_mul_ps(_mm_set1_ps(gain),_mm_skipload_ps(input+ii*stride,stride));
        
            // FIR
            shuf = _mm_shuffle_ps(pinn,data,_MM_SHUFFLE(1,0,3,2));
            tmp2 = _mm_add_ps(_mm_mul_ps(factor0,data),_mm_mul_ps(factor2,shuf));
            shuf = _mm_shuffle_ps(shuf,data,_MM_SHUFFLE(2,1,2,1));
            tmp2 = _mm_add_ps(tmp2,_mm_mul_ps(factor1,shuf));
            pinn = data;

            // D[r] * x
            tmp3 = _mm_mul_ps(_mm_set1_ps(tmp2[0]),_mm_load_ps(_d1));
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[1]),_mm_load_ps(_d1+4),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[2]),_mm_load_ps(_d1+8),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[3]),_mm_load_ps(_d1+12),tmp3);
        
            // Unpack to store
            tmp2 = _mm_add_ps(tmp1,tmp3);
            data = _mm_shuffle_ps(pout, tmp2, _MM_SHUFFLE(1,0,3,2));
            _mm_skipstore_ps(output+ii*stride,data,stride);
            pout = tmp2;
        }
    
        _outs[channel] = pout[2];
        _outs[stride+channel] = pout[3];
        _inns[channel] = pinn[2];
        _inns[stride+channel] = pinn[3];
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        unsigned stride = _channels;
        float32x2_t pout, pinn;
        float32x4_t tmp1, tmp2, tmp3;
        float32x4_t data, shuf;
    
        float32x4_t thegain = vld1q_dup_f32(&gain);
        float32x4_t factor0 = vld1q_dup_f32(&_b0);
        float32x4_t factor1 = vld1q_dup_f32(&_b1);
        float32x4_t factor2 = vld1q_dup_f32(&_b2);
    
        pout = vld1_skip_f32(_outs+channel,stride);
        pinn = vld1_skip_f32(_inns+channel,stride);
        for(int ii = 0; ii < size; ii += 4) {
            // C[r] * y
            tmp2 = vdupq_lane_f32(pout,0);
            tmp3 = vdupq_lane_f32(pout,1);
            tmp1 = vmlaq_f32(vmulq_f32(tmp2,vld1q_f32(_c1)),tmp3,vld1q_f32(_c1+4));
        
            // FIR
            data = vmulq_f32(thegain,vld1q_skip_f32(input+ii*stride,stride));
            shuf = vcombine_f32(pinn,vget_low_f32(data));
            tmp2 = vmlaq_f32(vmulq_f32(factor0,data),factor2,shuf);
            shuf = vextq_f32(vcombine_f32(pinn,pinn), data, 3);
            tmp2 = vmlaq_f32(tmp2,factor1,shuf);
            pinn = vget_high_f32(data);
        
            // D[r] * x
            tmp3 = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2),0),vld1q_f32(_d1));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_low_f32(tmp2),1), vld1q_f32(_d1+4));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),0),vld1q_f32(_d1+8));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),1),vld1q_f32(_d1+12));

            // Unpack to store
            tmp2 = vaddq_f32(tmp1,tmp3);
            tmp3 = vcombine_f32(pout,vget_low_f32(tmp2));
            vst1q_skip_f32(output+ii*stride, tmp3, stride);
            pout = vget_high_f32(tmp2);
        }
        vst1_skip_f32(_outs+channel,pout,stride);
        vst1_skip_f32(_inns+channel,pinn,stride);
    } else {
#else
    {
#endif
        unsigned stride = _channels;
        output[0] = _outs[channel];
        output[stride] = _outs[stride+channel];
    
        output[2*stride]  = gain * _b0 * input[0] + _b1 * _inns[stride+channel] + _b2 * _inns[channel];
        output[2*stride] -= _a1 * output[stride] + _a2 * output[0];
    
        output[3*stride]  = gain * _b0 * input[stride] + gain * _b1 * input[0] + _b2 * _inns[stride+channel];
        output[3*stride] -= _a1 * output[2*stride] + _a2 * output[stride];
    
        for(size_t ii = 2; ii < size-2; ii++) {
            output[(ii+2)*stride]  = gain * (_b0 * input[ii*stride] + _b1 * input[(ii-1)*stride] + _b2 * input[(ii-2)*stride]);
            output[(ii+2)*stride] -= _a1 * output[(ii+1)*stride] + _a2 * output[ii*stride];
        }
    
        _outs[channel]  = gain * (_b0 * input[stride*(size-2)] + _b1 * input[stride*(size-3)] +  _b2 *  input[stride*(size-4)]);
        _outs[channel] -= _a1 * output[stride*(size-1)] + _a2 * output[stride*(size-2)];
        _outs[stride+channel]  = gain * (_b0 * input[stride*(size-1)] + _b1 * input[stride*(size-2)] +  _b2 * input[stride*(size-3)]);
        _outs[stride+channel] -= _a1 * _outs[channel] + _a2 * output[stride*(size-1)];
    
        _inns[channel]  = gain * input[(size-2)*stride];
        _inns[stride+channel]  = gain * input[(size-1)*stride];
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
void BiquadIIR::single(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        __m128 pout, pinn;
        __m128 tmp1, tmp2, tmp3;
        __m128 data, shuf;
    
        __m128 factor0 = _mm_set1_ps(_b0);
        __m128 factor1 = _mm_set1_ps(_b1);
        __m128 factor2 = _mm_set1_ps(_b2);
    
        pout = _mm_set_ps(_outs[1],_outs[0],0,0);
        pinn = _mm_set_ps(_inns[1],_inns[0],0,0);
        for(int ii = 0; ii < size; ii += 4) {
            // C[r] * y
            tmp2 = _mm_set1_ps(pout[2]);
            tmp3 = _mm_set1_ps(pout[3]);
            tmp1 = _mm_fmadd_ps(tmp2,_mm_load_ps(_c1),_mm_mul_ps(tmp3,_mm_load_ps(_c1+4)));
        
            // FIR
            data = _mm_mul_ps(_mm_set1_ps(gain),_mm_load_ps(input+ii));
            shuf = _mm_shuffle_ps(pinn,data,_MM_SHUFFLE(1,0,3,2));
            tmp2 = _mm_add_ps(_mm_mul_ps(factor0,data),_mm_mul_ps(factor2,shuf));
            shuf = _mm_shuffle_ps(shuf,data,_MM_SHUFFLE(2,1,2,1));
            tmp2 = _mm_add_ps(tmp2,_mm_mul_ps(factor1,shuf));
            pinn = data;
        
            // D[r] * x
            tmp3 = _mm_mul_ps(_mm_set1_ps(tmp2[0]),_mm_load_ps(_d1));
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[1]),_mm_load_ps(_d1+4),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[2]),_mm_load_ps(_d1+8),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[3]),_mm_load_ps(_d1+12),tmp3);
        
            // Shift to output and repeat
            tmp2 = _mm_add_ps(tmp1,tmp3);
            tmp3 = _mm_shuffle_ps(pout, tmp2, _MM_SHUFFLE(1,0,3,2));
            _mm_store_ps(output+ii, tmp3);
            pout = tmp2;
        }
    
        _outs[0] = pout[2];
        _outs[1] = pout[3];
        _inns[0] = pinn[2];
        _inns[1] = pinn[3];
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float32x2_t pout, pinn;
        float32x4_t tmp1, tmp2, tmp3;
        float32x4_t data, shuf;
    
        float32x4_t thegain = vld1q_dup_f32(&gain);
        float32x4_t factor0 = vld1q_dup_f32(&_b0);
        float32x4_t factor1 = vld1q_dup_f32(&_b1);
        float32x4_t factor2 = vld1q_dup_f32(&_b2);
    
        pout = vld1_f32(_outs+0);
        pinn = vld1_f32(_inns+0);
        for(int ii = 0; ii < size; ii += 4) {
            // C[r] * y
            tmp2 = vdupq_lane_f32(pout,0);
            tmp3 = vdupq_lane_f32(pout,1);
            tmp1 = vmlaq_f32(vmulq_f32(tmp2,vld1q_f32(_c1)),tmp3,vld1q_f32(_c1+4));
        
            // FIR
            data = vmulq_f32(thegain,vld1q_f32(input+ii));
            shuf = vcombine_f32(pinn,vget_low_f32(data));
            tmp2 = vmlaq_f32(vmulq_f32(factor0,data),factor2,shuf);
            shuf = vextq_f32(vcombine_f32(pinn,pinn), data, 3);
            tmp2 = vmlaq_f32(tmp2,factor1,shuf);
            pinn = vget_high_f32(data);
        
            // D[r] * x
            tmp3 = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2),0),vld1q_f32(_d1));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_low_f32(tmp2),1), vld1q_f32(_d1+4));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),0),vld1q_f32(_d1+8));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),1),vld1q_f32(_d1+12));
        
            // Shift to output and repeat
            tmp2 = vaddq_f32(tmp1,tmp3);
            tmp3 = vcombine_f32(pout,vget_low_f32(tmp2));
            vst1q_f32(output+ii, tmp3);
            pout = vget_high_f32(tmp2);
        }
    
        vst1_f32(_outs+0,pout);
        vst1_f32(_inns+0,pinn);
    } else {
#else
    {
#endif
        output[0] = _outs[(size_t)0];
        output[1] = _outs[(size_t)1];
    
        output[2]  = gain * _b0 * input[0] + _b1 * _inns[(size_t)1] + _b2 * _inns[(size_t)0];
        output[2] -= _a1 * output[1] + _a2 * output[0];
    
        output[3]  = gain * _b0 * input[1] + gain * _b1 * input[0] + _b2 * _inns[(size_t)1];
        output[3] -= _a1 * output[2] + _a2 * output[1];
    
        for(size_t ii = 2; ii < size-2; ii++) {
            output[ii+2] = gain * (_b0 * input[ii] + _b1 * input[ii-1] + _b2 * input[ii-2]);
            output[ii+2] -= _a1 * output[ii+1] + _a2 * output[ii];
        }
    
        _outs[(size_t)0]  = gain * (_b0 * input[size-2] + _b1 * input[size-3] + _b2 * input[size-4]);
        _outs[(size_t)0] -= _a1 * output[size-1] + _a2 * output[size-2];
    
        _outs[(size_t)1]  = gain * (_b0 * input[size-1] + _b1 * input[size-2] + _b2 * input[size-3]);
        _outs[(size_t)1] -= _a1 * _outs[(size_t)0] + _a2 * output[size-1];
    
        _inns[(size_t)0]  = gain * input[size-2];
        _inns[(size_t)1]  = gain * input[size-1];
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
void BiquadIIR::dual(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        __m128 pout, pinn;
        __m128 tmp1, tmp2, tmp3;
        __m128 data, shuf;

        pout = _mm_load_ps(_outs+0);
        pinn = _mm_load_ps(_inns+0);
    
        __m128 factor0 = _mm_set1_ps(_b0);
        __m128 factor1 = _mm_set1_ps(_b1);
        __m128 factor2 = _mm_set1_ps(_b2);

        for(int ii = 0; ii < 2*size; ii += 4) {
            // C[r] * y
            tmp1 = _mm_mul_ps(_mm_set1_ps(pout[0]),_mm_load_ps(_c2));
            tmp1 = _mm_fmadd_ps(_mm_set1_ps(pout[1]),_mm_load_ps(_c2+4), tmp1);
            tmp1 = _mm_fmadd_ps(_mm_set1_ps(pout[2]),_mm_load_ps(_c2+8), tmp1);
            tmp1 = _mm_fmadd_ps(_mm_set1_ps(pout[3]),_mm_load_ps(_c2+12),tmp1);
        
            // FIR
            data = _mm_mul_ps(_mm_set1_ps(gain),_mm_loadu_ps(input+ii));
            shuf = _mm_shuffle_ps(pinn,data,_MM_SHUFFLE(1,0,3,2));
            tmp2 = _mm_add_ps(_mm_mul_ps(factor0,data),_mm_mul_ps(factor1,shuf));
            tmp2 = _mm_add_ps(tmp2,_mm_mul_ps(factor2,pinn));
            pinn = data;
        
            // D[r] * x
            tmp3 = _mm_mul_ps(_mm_set1_ps(tmp2[0]),_mm_load_ps(_d2));
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[1]),_mm_load_ps(_d2+4),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[2]),_mm_load_ps(_d2+8),tmp3);
            tmp3 = _mm_fmadd_ps(_mm_set1_ps(tmp2[3]),_mm_load_ps(_d2+12),tmp3);
        
            // Shift to output and repeat
            tmp2 = _mm_add_ps(tmp1,tmp3);
            _mm_store_ps(output+ii, pout);
            pout = tmp2;
        }
    
        _mm_store_ps(_outs+0,pout);
        _mm_store_ps(_inns+0,pinn);
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float32x4_t pout, pinn;
        float32x4_t tmp1, tmp2, tmp3;
        float32x4_t data, shuf;
    
        pout = vld1q_f32(_outs+0);
        pinn = vld1q_f32(_inns+0);
    
        float32x4_t thegain = vld1q_dup_f32(&gain);
        float32x4_t factor0 = vld1q_dup_f32(&_b0);
        float32x4_t factor1 = vld1q_dup_f32(&_b1);
        float32x4_t factor2 = vld1q_dup_f32(&_b2);
    
        for(int ii = 0; ii < 2*size; ii += 4) {
            // C[r] * y
            tmp1 = vmulq_f32(vdupq_lane_f32(vget_low_f32(pout),0), vld1q_f32(_c2));
            tmp1 = vmlaq_f32(tmp1, vdupq_lane_f32(vget_low_f32(pout),1), vld1q_f32(_c2+4));
            tmp1 = vmlaq_f32(tmp1, vdupq_lane_f32(vget_high_f32(pout),0), vld1q_f32(_c2+8));
            tmp1 = vmlaq_f32(tmp1, vdupq_lane_f32(vget_high_f32(pout),1), vld1q_f32(_c2+12));
        
            // FIR
            data = vmulq_f32(thegain,vld1q_f32(input+ii));
            shuf = vextq_f32(pinn,data,2);
            tmp2 = vmlaq_f32(vmulq_f32(factor0,data),factor1,shuf);
            tmp2 = vmlaq_f32(tmp2,factor2,pinn);
            pinn = data;
        
            // D[r] * x
            tmp3 = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2),0),vld1q_f32(_d2));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_low_f32(tmp2),1), vld1q_f32(_d2+4));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),0),vld1q_f32(_d2+8));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),1),vld1q_f32(_d2+12));

            // Shift to output and repeat
            tmp2 = vaddq_f32(tmp1,tmp3);
            vst1q_f32(output+ii, pout);
            pout = tmp2;
        }
    
        vst1q_f32(_outs+0,pout);
        vst1q_f32(_inns+0,pinn);
    } else {
#else
    {
#endif
        output[0] = _outs[(size_t)0];
        output[1] = _outs[(size_t)1];
        output[2] = _outs[(size_t)2];
        output[3] = _outs[(size_t)3];
    
        output[4]  = gain * _b0 * input[0] + _b1 * _inns[(size_t)2] + _b2 * _inns[(size_t)0];
        output[4] -= _a1 * output[2] + _a2 * output[0];
        output[5]  = gain * _b0 * input[1] + _b1 * _inns[(size_t)3] + _b2 * _inns[(size_t)1];
        output[5] -= _a1 * output[3] + _a2 * output[1];
        output[6]  = gain * _b0 * input[2] + gain * _b1 * input[0] + _b2 * _inns[(size_t)2];
        output[6] -= _a1 * output[4] + _a2 * output[2];
        output[7]  = gain * _b0 * input[3] + gain * _b1 * input[1] + _b2 * _inns[(size_t)3];
        output[7] -= _a1 * output[5] + _a2 * output[3];
    
        for(size_t ii = 2; ii < size-2; ii++) {
            output[2*(ii+2)  ]  = gain * (_b0 * input[2*ii  ] + _b1 * input[2*(ii-1)  ] + _b2 * input[2*(ii-2)  ]);
            output[2*(ii+2)  ] -= _a1 * output[2*(ii+1)  ] + _a2 * output[2*ii  ];
            output[2*(ii+2)+1]  = gain * (_b0 * input[2*ii+1] + _b1 * input[2*(ii-1)+1] + _b2 * input[2*(ii-2)+1]);
            output[2*(ii+2)+1] -= _a1 * output[2*(ii+1)+1] + _a2 * output[2*ii+1];
        }
    
        _outs[(size_t)0]  = gain * (_b0 * input[2*(size-2)  ] + _b1 * input[2*(size-3)  ] + _b2 * input[2*(size-4)  ]);
        _outs[(size_t)0] -= _a1 * output[2*(size-1)  ] + _a2 * output[2*(size-2)  ];
        _outs[(size_t)1]  = gain * (_b0 * input[2*(size-2)+1] + _b1 * input[2*(size-3)+1] + _b2 * input[2*(size-4)+1]);
        _outs[(size_t)1] -= _a1 * output[2*(size-1)+1] + _a2 * output[2*(size-2)+1];
        _outs[(size_t)2]  = gain * (_b0 * input[2*(size-1)  ] + _b1 * input[2*(size-2)  ] + _b2 * input[2*(size-3)  ]);
        _outs[(size_t)2] -= _a1 * _outs[(size_t)0] + _a2 * output[2*(size-1)  ];
        _outs[(size_t)3]  = gain * (_b0 * input[2*(size-1)+1] + _b1 * input[2*(size-2)+1] + _b2 * input[2*(size-3)+1]);
        _outs[(size_t)3] -= _a1 * _outs[(size_t)1] + _a2 * output[2*(size-1)+1];
    
        _inns[(size_t)0] = gain * input[2*size-4];
        _inns[(size_t)1] = gain * input[2*size-3];
        _inns[(size_t)2] = gain * input[2*size-2];
        _inns[(size_t)3] = gain * input[2*size-1];
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
void BiquadIIR::trio(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float32x4x3_t data, outr;
        float32x4_t tmp1, tmp2, tmp3, shuf;
    
        float32x2x3_t pinn, pout;
        pinn = vld3_f32(_inns+0);
        pout = vld3_f32(_outs+0);
    
        float32x4_t thegain = vld1q_dup_f32(&gain);
        float32x4_t factor0 = vld1q_dup_f32(&_b0);
        float32x4_t factor1 = vld1q_dup_f32(&_b1);
        float32x4_t factor2 = vld1q_dup_f32(&_b2);

        for(size_t ii = 0; ii < 3*size; ii += 12) {
            data = vld3q_f32(input+ii);
            data.val[0] = vmulq_f32(thegain,data.val[0]);
            data.val[1] = vmulq_f32(thegain,data.val[1]);
            data.val[2] = vmulq_f32(thegain,data.val[2]);
        
            // Channel 0
            // C[r] * y
            tmp2 = vdupq_lane_f32(pout.val[0],0);
            tmp3 = vdupq_lane_f32(pout.val[0],1);
            tmp1 = vmlaq_f32(vmulq_f32(tmp2,vld1q_f32(_c1)),tmp3,vld1q_f32(_c1+4));

            // FIR
            shuf = vcombine_f32(pinn.val[0],vget_low_f32(data.val[0]));
            tmp2 = vmlaq_f32(vmulq_f32(factor0,data.val[0]),factor2,shuf);
            shuf = vextq_f32(vcombine_f32(pinn.val[0],pinn.val[0]), data.val[0], 3);
            tmp2 = vmlaq_f32(tmp2,factor1,shuf);
            pinn.val[0] = vget_high_f32(data.val[0]);

            // D[r] * x
            tmp3 = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2),0),vld1q_f32(_d1));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_low_f32(tmp2),1), vld1q_f32(_d1+4));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),0),vld1q_f32(_d1+8));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),1),vld1q_f32(_d1+12));
        
            // Shift to output and repeat
            tmp2 = vaddq_f32(tmp1,tmp3);
            tmp3 = vcombine_f32(pout.val[0],vget_low_f32(tmp2));
            outr.val[0] = tmp3;
            pout.val[0] = vget_high_f32(tmp2);
        
            // Channel 1
            // C[r] * y
            tmp2 = vdupq_lane_f32(pout.val[1],0);
            tmp3 = vdupq_lane_f32(pout.val[1],1);
            tmp1 = vmlaq_f32(vmulq_f32(tmp2,vld1q_f32(_c1)),tmp3,vld1q_f32(_c1+4));
        
            // FIR
            shuf = vcombine_f32(pinn.val[1],vget_low_f32(data.val[1]));
            tmp2 = vmlaq_f32(vmulq_f32(factor0,data.val[1]),factor2,shuf);
            shuf = vextq_f32(vcombine_f32(pinn.val[1],pinn.val[1]), data.val[1], 3);
            tmp2 = vmlaq_f32(tmp2,factor1,shuf);
            pinn.val[1] = vget_high_f32(data.val[1]);
        
            // D[r] * x
            tmp3 = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2),0),vld1q_f32(_d1));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_low_f32(tmp2),1), vld1q_f32(_d1+4));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),0),vld1q_f32(_d1+8));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),1),vld1q_f32(_d1+12));
        
            // Shift to output and repeat
            tmp2 = vaddq_f32(tmp1,tmp3);
            tmp3 = vcombine_f32(pout.val[1],vget_low_f32(tmp2));
            outr.val[1] = tmp3;
            pout.val[1] = vget_high_f32(tmp2);
        
            // Channel 2
            // C[r] * y
            tmp2 = vdupq_lane_f32(pout.val[2],0);
            tmp3 = vdupq_lane_f32(pout.val[2],1);
            tmp1 = vmlaq_f32(vmulq_f32(tmp2,vld1q_f32(_c1)),tmp3,vld1q_f32(_c1+4));
        
            // FIR
            shuf = vcombine_f32(pinn.val[2],vget_low_f32(data.val[2]));
            tmp2 = vmlaq_f32(vmulq_f32(factor0,data.val[2]),factor2,shuf);
            shuf = vextq_f32(vcombine_f32(pinn.val[2],pinn.val[2]), data.val[2], 3);
            tmp2 = vmlaq_f32(tmp2,factor1,shuf);
            pinn.val[2] = vget_high_f32(data.val[2]);
        
            // D[r] * x
            tmp3 = vmulq_f32(vdupq_lane_f32(vget_low_f32(tmp2),0),vld1q_f32(_d1));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_low_f32(tmp2),1), vld1q_f32(_d1+4));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),0),vld1q_f32(_d1+8));
            tmp3 = vmlaq_f32(tmp3,vdupq_lane_f32(vget_high_f32(tmp2),1),vld1q_f32(_d1+12));
        
            // Shift to output and repeat
            tmp2 = vaddq_f32(tmp1,tmp3);
            tmp3 = vcombine_f32(pout.val[2],vget_low_f32(tmp2));
            outr.val[2] = tmp3;
            pout.val[2] = vget_high_f32(tmp2);
        
            vst3q_f32(output+ii, outr);
        }
    
        vst3_f32(_outs+0,pout);
        vst3_f32(_inns+0,pinn);
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
void BiquadIIR::quad(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        __m128 pout1,pout2;
        __m128 pinn1,pinn2;
        __m128 data, temp;
    
        pout2 = _mm_load_ps(_outs+0);
        pout1 = _mm_load_ps(_outs+4);

        pinn2 = _mm_load_ps(_inns+0);
        pinn1 = _mm_load_ps(_inns+4);

        __m128 factor0 = _mm_set1_ps(_b0);
        __m128 factor1 = _mm_set1_ps(_b1);
        __m128 factor2 = _mm_set1_ps(_b2);

        for(int ii = 0; ii < 4*size; ii += 4) {
            data = _mm_mul_ps(_mm_set1_ps(gain),_mm_loadu_ps(input+ii));
            temp = _mm_add_ps(_mm_mul_ps(factor0,data),_mm_mul_ps(factor1,pinn1));
            temp = _mm_add_ps(temp,_mm_mul_ps(factor2,pinn2));

            temp = _mm_fmadd_ps(_mm_set1_ps(-_a1),pout1,temp);
            temp = _mm_fmadd_ps(_mm_set1_ps(-_a2),pout2,temp);
            _mm_store_ps(output+ii, pout2);
        
            pout2 = pout1;
            pout1 = temp;
            pinn2 = pinn1;
            pinn1 = data;
        }
        _mm_store_ps(_outs+0,pout2);
        _mm_store_ps(_outs+4,pout1);
        _mm_store_ps(_inns+0,pinn2);
        _mm_store_ps(_inns+4,pinn1);
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float32x4_t pout1,pout2;
        float32x4_t pinn1,pinn2;
        float32x4_t data, temp;
    
        pout2 = vld1q_f32(_outs+0);
        pout1 = vld1q_f32(_outs+4);
    
        pinn2 = vld1q_f32(_inns+0);
        pinn1 = vld1q_f32(_inns+4);
    
        float32x4_t thegain = vld1q_dup_f32(&gain);
        float32x4_t factor0 = vld1q_dup_f32(&_b0);
        float32x4_t factor1 = vld1q_dup_f32(&_b1);
        float32x4_t factor2 = vld1q_dup_f32(&_b2);
    
        for(int ii = 0; ii < 4*size; ii += 4) {
            data = vmulq_f32(thegain,vld1q_f32(input+ii));
            temp = vmlaq_f32(vmulq_f32(factor0,data),factor1,pinn1);
            temp = vmlaq_f32(temp,factor2,pinn2);
        
            temp = vmlaq_f32(temp, vld1q_dup_f32(_c1+4), pout1);
            temp = vmlaq_f32(temp, vld1q_dup_f32(_c1),   pout2);
            vst1q_f32(output+ii, pout2);
        
            pout2 = pout1;
            pout1 = temp;
            pinn2 = pinn1;
            pinn1 = data;
        }
        vst1q_f32(_outs+0,pout2);
        vst1q_f32(_outs+4,pout1);
        vst1q_f32(_inns+0,pinn2);
        vst1q_f32(_inns+4,pinn1);
    } else {
#else
    {
#endif
        for(size_t ii = 0; ii < 8; ii++) {
            output[ii] = _outs[ii];
        }
    
        for(size_t ii = 0; ii < 4; ii++) {
            output[ii+8]  = gain * _b0 * input[ii] + _b1 * _inns[ii+4] + _b2 * _inns[ii];
            output[ii+8] -= _a1 * output[ii+4] + _a2 * output[ii];
        }
        for(size_t ii = 4; ii < 8; ii++) {
            output[ii+8]  = gain * _b0 * input[ii] + gain * _b1 * input[ii-4] + _b2 * _inns[ii];
            output[ii+8] -= _a1 * output[ii+4] + _a2 * output[ii];
        }
        for(size_t ii = 8; ii < 4*(size-2); ii++) {
            output[ii+8]  = gain * (_b0 * input[ii] + _b1 * input[ii-4] + _b2 * input[ii-8]);
            output[ii+8] -= _a1 * output[ii+4] + _a2 * output[ii];
        }
    
        for(size_t ii = 0; ii < 4; ii++) {
            _outs[ii]  = gain * (_b0 * input[4*(size-2)+ii] + _b1 * input[4*(size-3)+ii] + _b2 * input[4*(size-4)+ii]);
            _outs[ii] -= _a1 * output[4*(size-1)+ii] + _a2 * output[4*(size-2)+ii];
        }
        for(size_t ii = 4; ii < 8; ii++) {
            _outs[ii]  = gain * (_b0 * input[4*(size-2)+ii] + _b1 * input[4*(size-3)+ii] + _b2 * input[4*(size-4)+ii]);
            _outs[ii] -= _a1 * _outs[ii-4] + _a2 * output[4*(size-2)+ii];
        }
        for(size_t ii = 0; ii < 8; ii++) {
            _inns[ii] = gain * input[4*size-8+ii];
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
void BiquadIIR::quart(float gain, float* input, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        __m128 pout1a,pout2a,pout1b,pout2b;
        __m128 pinn1a,pinn2a,pinn1b,pinn2b;
        __m128 data, temp;
        
        pout2a = _mm_load_ps(_outs+0);
        pout2b = _mm_load_ps(_outs+4);
        pout1a = _mm_load_ps(_outs+8);
        pout1b = _mm_load_ps(_outs+12);
        pinn2a = _mm_load_ps(_inns+0);
        pinn2b = _mm_load_ps(_inns+4);
        pinn1a = _mm_load_ps(_inns+8);
        pinn1b = _mm_load_ps(_inns+12);
        
        __m128 factor0 = _mm_set1_ps(_b0);
        __m128 factor1 = _mm_set1_ps(_b1);
        __m128 factor2 = _mm_set1_ps(_b2);
        for(int ii = 0; ii < 8*size; ii += 8) {
            data = _mm_mul_ps(_mm_set1_ps(gain),_mm_loadu_ps(input+ii));
            temp = _mm_add_ps(_mm_mul_ps(factor0,data),_mm_mul_ps(factor1,pinn1a));
            temp = _mm_add_ps(temp,_mm_mul_ps(factor2,pinn2a));
            temp = _mm_fmadd_ps(_mm_set1_ps(-_a1),pout1a,temp);
            temp = _mm_fmadd_ps(_mm_set1_ps(-_a2),pout2a,temp);
            _mm_storeu_ps(output+ii, pout2a);
            pout2a = pout1a;
            pout1a = temp;
            pinn2a = pinn1a;
            pinn1a = data;
            
            data = _mm_mul_ps(_mm_set1_ps(gain),_mm_loadu_ps(input+ii+4));
            temp = _mm_add_ps(_mm_mul_ps(factor0,data),_mm_mul_ps(factor1,pinn1b));
            temp = _mm_add_ps(temp,_mm_mul_ps(factor2,pinn2b));
            temp = _mm_fmadd_ps(_mm_set1_ps(-_a1),pout1b,temp);
            temp = _mm_fmadd_ps(_mm_set1_ps(-_a2),pout2b,temp);
            _mm_storeu_ps(output+ii+4, pout2b);
            pout2b = pout1b;
            pout1b = temp;
            pinn2b = pinn1b;
            pinn1b = data;
        }
        
        _mm_store_ps(_outs+0,pout2a);
        _mm_store_ps(_outs+4,pout2b);
        _mm_store_ps(_outs+8,pout1a);
        _mm_store_ps(_outs+12,pout1b);
        _mm_store_ps(_inns+0,pinn2a);
        _mm_store_ps(_inns+4,pinn2b);
        _mm_store_ps(_inns+8,pinn1a);
        _mm_store_ps(_inns+12,pinn1b);
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float32x4_t pout1a,pout2a,pout1b,pout2b;
        float32x4_t pinn1a,pinn2a,pinn1b,pinn2b;
        float32x4_t data, temp;
    
        pout2a = vld1q_f32(_outs+0);
        pout2b = vld1q_f32(_outs+4);
        pout1a = vld1q_f32(_outs+8);
        pout1b = vld1q_f32(_outs+12);
        pinn2a = vld1q_f32(_inns+0);
        pinn2b = vld1q_f32(_inns+4);
        pinn1a = vld1q_f32(_inns+8);
        pinn1b = vld1q_f32(_inns+12);
    
        float32x4_t thegain = vld1q_dup_f32(&gain);
        float32x4_t factor0 = vld1q_dup_f32(&_b0);
        float32x4_t factor1 = vld1q_dup_f32(&_b1);
        float32x4_t factor2 = vld1q_dup_f32(&_b2);
    
        for(int ii = 0; ii < 8*size; ii += 8) {
            data = vmulq_f32(thegain,vld1q_f32(input+ii));
            temp = vmlaq_f32(vmulq_f32(factor0,data),factor1,pinn1a);
            temp = vmlaq_f32(temp,factor2,pinn2a);
            temp = vmlaq_f32(temp, vld1q_dup_f32(_c1+4), pout1a);
            temp = vmlaq_f32(temp, vld1q_dup_f32(_c1  ), pout2a);
            vst1q_f32(output+ii, pout2a);
            pout2a = pout1a;
            pout1a = temp;
            pinn2a = pinn1a;
            pinn1a = data;
        
            data = vmulq_f32(thegain,vld1q_f32(input+ii+4));
            temp = vmlaq_f32(vmulq_f32(factor0,data),factor1,pinn1b);
            temp = vmlaq_f32(temp,factor2,pinn2b);
            temp = vmlaq_f32(temp, vld1q_dup_f32(_c1+4), pout1b);
            temp = vmlaq_f32(temp, vld1q_dup_f32(_c1  ), pout2b);
            vst1q_f32(output+ii+4, pout2b);
            pout2b = pout1b;
            pout1b = temp;
            pinn2b = pinn1b;
            pinn1b = data;
        }
    
        vst1q_f32(_outs+0,pout2a);
        vst1q_f32(_outs+4,pout2b);
        vst1q_f32(_outs+8,pout1a);
        vst1q_f32(_outs+12,pout1b);
        vst1q_f32(_inns+0,pinn2a);
        vst1q_f32(_inns+4,pinn2b);
        vst1q_f32(_inns+8,pinn1a);
        vst1q_f32(_inns+12,pinn1b);
    } else {
#else
    {
#endif
        for(size_t ii = 0; ii < 16; ii++) {
            output[ii] = _outs[ii];
        }
        for(size_t ii = 0; ii < 8; ii++) {
            output[ii+16]  = gain * _b0 * input[ii] + _b1 * _inns[ii+8] + _b2 * _inns[ii];
            output[ii+16] -= _a1 * output[ii+8] + _a2 * output[ii];
        }
        for(size_t ii = 8; ii < 16; ii++) {
            output[ii+16]  = gain * _b0 * input[ii] + gain * _b1 * input[ii-8] + _b2 * _inns[ii];
            output[ii+16] -= _a1 * output[ii+8] + _a2 * output[ii];
        }
        for(size_t ii = 16; ii < 8*(size-2); ii++) {
            output[ii+16]  = gain * (_b0 * input[ii] + _b1 * input[ii-8] + _b2 * input[ii-16]);
            output[ii+16] -= _a1 * output[ii+8] + _a2 * output[ii];
        }
    
        for(size_t ii = 0; ii < 8; ii++) {
            _outs[ii]  = gain * (_b0 * input[8*(size-2)+ii] + _b1 * input[8*(size-3)+ii] + _b2 * input[8*(size-4)+ii]);
            _outs[ii] -= _a1 * output[8*(size-1)+ii] + _a2 * output[8*(size-2)+ii];
        }
        for(size_t ii = 8; ii < 16; ii++) {
            _outs[ii]  = gain * (_b0 * input[8*(size-2)+ii] + _b1 * input[8*(size-3)+ii] + _b2 * input[8*(size-4)+ii]);
            _outs[ii] -= _a1 * _outs[ii-8] + _a2 * output[8*(size-2)+ii];
        }
        for(size_t ii = 0; ii < 16; ii++) {
            _inns[ii] = gain * input[8*size-16+ii];
        }
    }
}

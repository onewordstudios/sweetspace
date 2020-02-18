//
//  CUDSPMath.cpp
//  Cornell University Game Library (CUGL)
//
//  This class is represents a class of static methods for performing basic
//  DSP calculations, like addition and multiplication.  As with the DSP
//  filters, this class supports vector optimizations for SSE and Neon 64.
//  Our implementation is limited to 128-bit words.  While 256-bit (e.g. AVX)
//  are more performant, they are not better for DSP filters and so we keep
//  the optimizations at the same level.
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
//  Version: 10/11/18
//
#include <cugl/math/dsp/CUDSPMath.h>
#include <cugl/util/CUDebug.h>
#include "cuDSP128.inl"

using namespace cugl;
using namespace cugl::dsp;

/** Whether to use a vectorization algorithm */
bool DSPMath::VECTORIZE = true;

#pragma mark -
#pragma mark Arithmetic Methods
/**
 * Adds two input signals together, storing the result in output
 *
 * It is safe for output to be the same as one of the two input buffers.
 *
 * @param input1    The first input buffer
 * @param input2    The second input buffer
 * @param output    The output buffer
 * @param size      The number of elements to add
 *
 * @return the number of elements successfully added
 */
size_t DSPMath::add(float* input1, float* input2, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        for(int ii = 0; ii < (int)size-3; ii += 4) {
            _mm_storeu_ps(output+ii, _mm_add_ps(_mm_loadu_ps(input1+ii),_mm_loadu_ps(input2+ii)));
        }
        if (size % 4 != 0) {
            Uint32 rem = size % 4;
            for(Uint32 ii = (Uint32)(size-rem); ii < size; ii++) {
                output[ii] = input1[ii]+input2[ii];
            }
        }
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        int ii;
        for(ii = 0; ii < (int)size-3; ii += 4) {
            vst1q_f32(output+ii, vaddq_f32(vld1q_f32(input1+ii),vld1q_f32(input2+ii)));
        }
        if (size % 4 != 0) {
            Uint32 rem = size % 4;
            for(Uint32 ii = (Uint32)(size-rem); ii < size; ii++) {
                output[ii] = input1[ii]+input2[ii];
            }
        }
    } else {
#else
    {
#endif
        for(int ii = 0; ii < size; ii++) {
            output[ii] = input1[ii]+input2[ii];
        }
    }
    return size;
}

/**
 * Multiplies two input signals together, storing the result in output
 *
 * It is safe for output to be the same as one of the two input buffers.
 *
 * @param input1    The first input buffer
 * @param input2    The second input buffer
 * @param output    The output buffer
 * @param size      The number of elements to multiply
 *
 * @return the number of elements successfully multiplied
 */
size_t DSPMath::multiply(float* input1, float* input2, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        for(int ii = 0; ii < (int)size-3; ii += 4) {
            _mm_storeu_ps(output+ii, _mm_mul_ps(_mm_loadu_ps(input1+ii),_mm_loadu_ps(input2+ii)));
        }
        if (size % 4 != 0) {
            Uint32 rem = size % 4;
            for(Uint32 ii = (Uint32)(size-rem); ii < size; ii++) {
                output[ii] = input1[ii]*input2[ii];
            }
        }
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        for(int ii = 0; ii < (int)size-3; ii += 4) {
            vst1q_f32(output+ii, vmulq_f32(vld1q_f32(input1+ii),vld1q_f32(input2+ii)));
        }
        if (size % 4 != 0) {
            Uint32 rem = size % 4;
            for(int ii = (Uint32)(size-rem); ii < size; ii++) {
                output[ii] = input1[ii]*input2[ii];
            }
        }
    } else {
#else
    {
#endif
        for(int ii = 0; ii < size; ii++) {
            output[ii] = input1[ii]*input2[ii];
        }
    }
    return size;
}

/**
 * Scales an input signal, storing the result in output
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param input     The input buffer
 * @param scalar    The scalar to mutliply by
 * @param output    The output buffer
 * @param size      The number of elements to multiply
 *
 * @return the number of elements successfully multiplied
 */
size_t DSPMath::scale(float* input, float scalar, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        const __m128 gain = _mm_set1_ps(scalar);
        for(int ii = 0; ii < (int)size-3; ii += 4) {
            _mm_storeu_ps(output+ii, _mm_mul_ps(_mm_loadu_ps(input+ii),gain));
        }
        if (size % 4 != 0) {
            Uint32 rem = size % 4;
            for(int ii = (Uint32)(size-rem); ii < size; ii++) {
                output[ii] = input[ii]*scalar;
            }
        }
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        const float32x4_t gain = vld1q_dup_f32(&scalar);
        for(int ii = 0; ii < (int)size-3; ii += 4) {
            vst1q_f32(output+ii, vmulq_f32(vld1q_f32(input+ii),gain));
        }
        if (size % 4 != 0) {
            Uint32 rem = size % 4;
            for(int ii = (Uint32)(size-rem); ii < size; ii++) {
                output[ii] = input[ii]*scalar;
            }
        }
    } else {
#else
    {
#endif
        for(int ii = 0; ii < size; ii++) {
            output[ii] = input[ii]*scalar;
        }
    }
    return size;
}

/**
 * Scales an input signal and adds it to another, storing the result in output
 *
 * It is safe for output to be the same as one of the two input buffers.
 *
 * @param input1    The first input buffer
 * @param input2    The second input buffer
 * @param scalar    The scalar to mutliply input1 by
 * @param output    The output buffer
 * @param size      The number of elements to process
 *
 * @return the number of elements successfully processed
 */
size_t DSPMath::scale_add(float* input1, float* input2, float scalar, float* output, size_t size) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        const __m128 gain = _mm_set1_ps(scalar);
        for(int ii = 0; ii < (int)size-3; ii += 4) {
            _mm_storeu_ps(output+ii,
                          _mm_fmadd_ps(_mm_loadu_ps(input1+ii),gain,_mm_loadu_ps(input2+ii)));
        }
        if (size % 4 != 0) {
            Uint32 rem = size % 4;
            for(int ii = (Uint32)(size-rem); ii < size; ii++) {
                output[ii] = input1[ii]*scalar+input2[ii];
            }
        }
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        const float32x4_t gain = vld1q_dup_f32(&scalar);
        for(int ii = 0; ii < (int)size-3; ii += 4) {
            vst1q_f32(output+ii,
                      vmlaq_f32(vld1q_f32(input2+ii),vld1q_f32(input1+ii),gain));
        }
        if (size % 4 != 0) {
            Uint32 rem = size % 4;
            for(int ii = (Uint32)(size-rem); ii < size; ii++) {
                output[ii] = input1[ii]*scalar+input2[ii];
            }
        }
    } else {
#else
    {
#endif
        for(int ii = 0; ii < size; ii++) {
            output[ii] = input1[ii]*scalar+input2[ii];
        }
    }
    return size;
}
        
        
#pragma mark -
#pragma mark Fade-In/Out Methods
/**
 * Scales an input signal, storing the result in output
 *
 * The scalar is a sliding factor linearly interpolated between start to end.
 * It will use start for the first element of input and end for the size
 * element.
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param input     The input buffer
 * @param start     The initial scalar value
 * @param end       The final scalar value
 * @param output    The output buffer
 * @param size      The number of elements to multiply
 *
 * @return the number of elements successfully multiplied
 */
size_t DSPMath::slide(float* input, float start, float end, float* output, size_t size) {
    float step = (end-start)/size;
    float curr = start;
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        __m128 left, rght;
        __m128 skip = _mm_setr_ps(0,step,2*step,3*step);
        for(int ii = 0; ii < (int)size-3; ii += 4) {
            left = _mm_loadu_ps(input+ii);
            rght = _mm_add_ps(_mm_set1_ps(curr),skip);
            _mm_storeu_ps(output+ii, _mm_mul_ps(left,rght));
            curr += 4*step;
        }
        if (size % 4 != 0) {
            Uint32 rem = size % 4;
            for(int ii = (Uint32)(size-rem); ii < size; ii++) {
                output[ii] = input[ii]*curr;
                curr += step;
            }
        }
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float32x4_t left, rght;
        float32x4_t skip = {0,step,2*step,3*step};
        for(int ii = 0; ii < (int)size-3; ii += 4) {
            left = vld1q_f32(input+ii);
            rght = vaddq_f32(vld1q_dup_f32(&curr),skip);
            vst1q_f32(output+ii, vmulq_f32(left,rght));
            curr += 4*step;
        }
        if (size % 4 != 0) {
            Uint32 rem = size % 4;
            for(int ii = (Uint32)(size-rem); ii < size; ii++) {
                output[ii] = input[ii]*curr;
                curr += step;
            }
        }
    } else {
#else
    {
#endif
        for(int ii = 0; ii < size; ii++) {
            output[ii] = input[ii]*curr;
            curr += step;
        }
    }
    return size;
}

/**
 * Scales an input signal and adds it to another, storing the result in output
 *
 * The scalar is a sliding factor linearly interpolated between start to end.
 * It will use start for the first element of input1 and end for the size
 * element.
 *
 * It is safe for output to be the same as one of the two input buffers.
 *
 * @param input1    The first input buffer
 * @param input2    The second input buffer
 * @param start     The initial scalar value
 * @param end       The final scalar value
 * @param output    The output buffer
 * @param size      The number of elements to process
 *
 * @return the number of elements successfully processed
 */
size_t DSPMath::slide_add(float* input1, float* input2, float start, float end, float* output, size_t size) {
    float step = (end-start)/size;
    float curr = start;
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        __m128 left, rght, gain;
        __m128 skip = _mm_setr_ps(0,step,2*step,3*step);
        for(int ii = 0; ii < (int)size-3; ii += 4) {
            left = _mm_loadu_ps(input1+ii);
            rght = _mm_loadu_ps(input2+ii);
            gain = _mm_add_ps(_mm_set1_ps(curr),skip);
            _mm_storeu_ps(output+ii, _mm_fmadd_ps(left,gain,rght));
            curr += 4*step;
        }
        if (size % 4 != 0) {
            Uint32 rem = size % 4;
            for(int ii = (Uint32)(size-rem); ii < size; ii++) {
                output[ii] = input1[ii]*curr+input2[ii];
                curr += step;
            }
        }
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float32x4_t left, rght, gain;
        float32x4_t skip = {0,step,2*step,3*step};
        for(int ii = 0; ii < (int)size-3; ii += 4) {
            left = vld1q_f32(input1+ii);
            rght = vld1q_f32(input2+ii);
            gain = vaddq_f32(vld1q_dup_f32(&curr),skip);
            vst1q_f32(output+ii, vmlaq_f32(rght,left,gain));
            curr += 4*step;
        }
        if (size % 4 != 0) {
            Uint32 rem = size % 4;
            for(int ii = (Uint32)(size-rem); ii < size; ii++) {
                output[ii] = input1[ii]*curr+input2[ii];
                curr += step;
            }
        }
    } else {
#else
    {
#endif
        for(int ii = 0; ii < size; ii++) {
            output[ii] = input1[ii]*curr+input2[ii];
            curr += step;
        }
    }
    return size;
}

        
#pragma mark -
#pragma mark Clamp Methods
/**
 * Hard clamps the data stream to the range [min,max]
 *
 * @param data      The stream buffer
 * @param min       The minimum allowed value
 * @param max       The maximum allowed value
 * @param size      The number of elements to clamp
 *
 * @return the number of elements successfully clamped
 */
size_t DSPMath::clamp(float* data, float min, float max, size_t size) {
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        const __m128 vmin = _mm_set1_ps(min);
        const __m128 vmax = _mm_set1_ps(max);
        for(int ii = 0; ii < (int)size-3; ii += 4) {
            _mm_storeu_ps(data+ii, _mm_min_ps(_mm_max_ps(_mm_loadu_ps(data+ii),vmin),vmax));
        }
        if (size % 4 != 0) {
            __m128 temp;
            Uint32 rem = size % 4;
            for(int ii = (Uint32)(size-rem); ii < size; ii++) {
                temp = _mm_min_ss(_mm_max_ss(_mm_set_ss(data[ii]),vmin),vmax);
                data[ii] = temp[0];
            }
        }
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        const float32x4_t vmin = vld1q_dup_f32(&min);
        const float32x4_t vmax = vld1q_dup_f32(&max);
        for(int ii = 0; ii < (int)size-3; ii += 4) {
            vst1q_f32(data+ii, vminq_f32(vmaxq_f32(vld1q_f32(data+ii),vmin),vmax));
        }
        if (size % 4 != 0) {
            Uint32 rem = size % 4;
            for(int ii = (Uint32)(size-rem); ii < size; ii++) {
                data[ii] = std::min(std::max(data[ii],min),max);
            }
        }
    } else {
#else
    {
#endif
        for(int ii = 0; ii < size; ii++) {
            data[ii] = std::min(std::max(data[ii],min),max);
        }
    }
    return size;
}

/**
 * Soft clamps the data stream to the range [-bound,bound]
 *
 * The clamp is a soft knee.  Values in the range [-knee, knee] are not
 * affected.  Values outside this range are asymptotically clamped to the
 * range [-bound,bound] with the formula
 *
 *     y = (bound*x - knee+knee*knee)/x
 *
 * @param data      The stream buffer
 * @param bound     The asymptotic bound
 * @param knee      The soft knee bound
 * @param size      The number of elements to clamp
 *
 * @return the number of elements successfully clamped
 */
size_t DSPMath::ease(float* data, float bound, float knee, size_t size) {
    float factor = bound*knee-knee*knee;
#if defined (CU_MATH_VECTOR_SSE)
    if (VECTORIZE) {
        const __m128 gain = _mm_set1_ps(bound);
        const __m128 uppr = _mm_set1_ps(knee);
        const __m128 lowr = _mm_set1_ps(-knee);
        const __m128 fact = _mm_set1_ps(factor);
        const __m128i mask = _mm_set1_epi32(~0);
        __m128 value, temp1, temp2, temp3, left, rght;
        for(int ii = 0; ii < (int)size-3; ii += 4) {
            value = _mm_loadu_ps(data+ii);
            temp1 = _mm_cmpgt_ps(value,uppr);
            temp2 = _mm_cmplt_ps(value,lowr);
            temp3 = _mm_or_ps(temp1,temp2);
            if (!_mm_test_all_zeros(temp3,mask)) {
                rght  = _mm_div_ps(fact,value);
                left  = _mm_and_ps(temp1,_mm_sub_ps(gain,rght));
                rght  = _mm_and_ps(temp2,_mm_add_ps(gain,rght));
                _mm_storeu_ps(data+ii,_mm_or_ps(_mm_andnot_ps(temp3,value),
                                                _mm_or_ps(left,rght)));
            }
        }
        if (size % 4 != 0) {
            Uint32 rem = size % 4;
            for(int ii = (Uint32)(size-rem); ii < size; ii++) {
                float tmp = data[ii];
                if (tmp > knee) {
                    data[ii] = (bound*tmp-factor)/tmp;
                } else if (tmp < - knee) {
                    data[ii] = (bound*tmp+factor)/tmp;
                }
            }
        }
    } else {
#elif defined (CU_MATH_VECTOR_NEON64)
#if defined (__ANDROID__)
    if (VECTORIZE && android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
#else
    if (VECTORIZE) {
#endif
        float bknee = -knee;
        const float32x4_t gain = vld1q_dup_f32(&bound);
        const float32x4_t uppr = vld1q_dup_f32(&knee);
        const float32x4_t lowr = vld1q_dup_f32(&bknee);
        const float32x4_t fact = vld1q_dup_f32(&factor);
        uint32x4_t  temp1, temp2, temp3;
        float32x4_t value, left, rght;
        for(int ii = 0; ii < (int)size-3; ii += 4) {
            value = vld1q_f32(data+ii);
            temp1 = vcgtq_f32(value,uppr);
            temp2 = vcltq_f32(value,lowr);
            temp3 = vorrq_u32((uint32x4_t)temp1,(uint32x4_t)temp2);
            if (vmaxvq_u32(temp3)) {
                left  = vrecpeq_f32(value);
                left  = vmulq_f32(vrecpsq_f32(value, left), left);
                rght  = vmulq_f32(fact,left);
                left  = vbslq_f32(temp1,vsubq_f32(gain,rght),vaddq_f32(gain,rght));
                vst1q_f32(data+ii,vbslq_f32(temp3,left,value));
            }
        }
        if (size % 4 != 0) {
            Uint32 rem = size % 4;
            for(int ii = (Uint32)(size-rem); ii < size; ii++) {
                float tmp = data[ii];
                if (tmp > knee) {
                    data[ii] = (bound*tmp-factor)/tmp;
                } else if (tmp < - knee) {
                    data[ii] = (bound*tmp+factor)/tmp;
                }
            }
        }
    } else {
#else
    {
#endif
        for(int ii = 0; ii < size; ii++) {
            float tmp = data[ii];
            if (tmp > knee) {
                data[ii] = (bound*tmp-factor)/tmp;
            } else if (tmp < - knee) {
                data[ii] = (bound*tmp+factor)/tmp;
            }
        }
    }
    return size;
}

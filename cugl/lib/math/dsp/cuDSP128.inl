//
//  cuDSP128.inl
//  Cornell University Game Library (CUGL)
//
//  This include file provides several static inline functions to aid with
//  code vectorization for DSP algorithms.  These functions are intended to
//  "harmonize" the differences between SSE and NEON intrinsics.
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

#if defined (CU_MATH_VECTOR_SSE)
/**
 * Stores a __m128 float vector into a strided array
 *
 * @param dst       The destination array
 * @param src       The float vector
 * @param stride    The destination stride
 */
static inline void _mm_skipstore_ps(float* dst, __m128 src, size_t stride) {
    dst[0       ] = src[0];
    dst[stride  ] = src[1];
    dst[stride*2] = src[2];
    dst[stride*3] = src[3];
}

/**
 * Returns a __m128 float vector from a strided array.
 *
 * @param src       The source array
 * @param stride    The source stride
 *
 * @return a __m128 float vector from a strided array.
 */
static inline __m128 _mm_skipload_ps(float* src, size_t stride) {
    __m128 result;
    result[0] = src[0];
    result[1] = src[stride];
    result[2] = src[stride*2];
    result[3] = src[stride*3];
    return result;
}

#elif defined (CU_MATH_VECTOR_NEON64)
/**
 * Stores a float32x4_t vector into a strided array
 *
 * @param dst       The destination array
 * @param src       The float vector
 * @param stride    The destination stride
 */
static inline void vst1q_skip_f32(float* dst, float32x4_t src, size_t stride) {
    dst[0] = vgetq_lane_f32(src,0);
    dst[  stride] = vgetq_lane_f32(src,1);
    dst[2*stride] = vgetq_lane_f32(src,2);
    dst[3*stride] = vgetq_lane_f32(src,3);
}

/**
 * Stores a float32x2_t vector into a strided array
 *
 * @param dst       The destination array
 * @param src       The float vector
 * @param stride    The destination stride
 */
static inline void vst1_skip_f32(float* dst, float32x2_t src, size_t stride) {
    dst[0] = vget_lane_f32(src,0);
    dst[  stride] = vget_lane_f32(src,1);
}

/**
 * Returns a float32x4_t vector from a strided array.
 *
 * @param src   The source array
 * @param dim   The source stride
 *
 * @return a float32x4_t vector from a strided array.
 */
static inline float32x4_t vld1q_skip_f32(float* src, size_t stride) {
    float32x4_t result = { src[0], src[stride], src[2*stride], src[3*stride] };
    return result;
}

/**
 * Returns a float32x4_t vector from a strided array.
 *
 * @param src   The source array
 * @param dim   The source stride
 *
 * @return a float32x4_t vector from a strided array.
 */
static inline float32x2_t vld1_skip_f32(float* src, size_t stride) {
    float32x2_t result = { src[0], src[stride] };
    return result;
}

#endif

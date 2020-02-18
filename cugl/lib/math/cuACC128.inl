
#if defined (CU_MATH_VECTOR_NEON64)
/**
 * Returns a 4-bit mask from the most significant bits of the four floats
 *
 * @param v the float vector
 */
static inline uint32_t vmaskq_f32(float32x4_t v) {
    uint32x4_t mmA = vandq_u32(vreinterpretq_u32_f32(v), (uint32x4_t) {0x1, 0x2, 0x4, 0x8}); // [0 1 2 3]
    uint32x4_t mmB = vextq_u32(mmA, mmA, 2);                        // [2 3 0 1]
    uint32x4_t mmC = vorrq_u32(mmA, mmB);                           // [0+2 1+3 0+2 1+3]
    uint32x4_t mmD = vextq_u32(mmC, mmC, 3);                        // [1+3 0+2 1+3 0+2]
    uint32x4_t mmE = vorrq_u32(mmC, mmD);                           // [0+1+2+3 ...]
    return vgetq_lane_u32(mmE, 0);
}

/**
 * Returns the dot product of two 4-element vectors
 *
 * @param a the left vector
 * @param b the right vector
 */
static inline float vdotq_f32(float32x4_t a, float32x4_t b) {
    float32x4_t prod = vmulq_f32(a, b);
    float32x4_t sum1 = vaddq_f32(prod, vrev64q_f32(prod));
    float32x4_t sum2 = vaddq_f32(sum1, vcombine_f32(vget_high_f32(sum1), vget_low_f32(sum1)));
    return sum2[0];
}

/**
 * Returns the _MM_SHUFFLE(3,0,2,1) permutation of v
 *
 * @param v the vector to permute
 */
static inline float32x4_t vpermq_3021_f32(float32x4_t v) {
    float32x4_t temp = vextq_f32(v,v,1);
    return vcombine_f32(vget_low_f32(temp),vrev64_f32(vget_high_f32(temp)));
}

/**
 * Returns the _MM_SHUFFLE(3,1,0,2) permutation of v
 *
 * @param v the vector to permute
 */
static inline float32x4_t vpermq_3102_f32(float32x4_t v) {
    float32x4_t temp = vcombine_f32(vget_low_f32(v),vrev64_f32(vget_high_f32(v)));
    return vextq_f32(temp,temp,3);
}

#endif

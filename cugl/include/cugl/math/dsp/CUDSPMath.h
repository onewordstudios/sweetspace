//
//  CUDSPMath.h
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
#ifndef __CU_DSP_MATH_H__
#define __CU_DSP_MATH_H__
#include "../CUMathBase.h"

namespace cugl {
    namespace dsp {

/**
 * This class is a collection of static methods for basic DSP calculations
 *
 * As with the DSP filters, this class supports vector optimizations for SSE
 * and Neon 64. Our implementation is limited to 128-bit words.  While 256-bit
 * (e.g. AVX) are more performant, they are not better for DSP filters and so
 * we keep the optimizations at the same level.
 *
 * This class is not thread safe.  External locking may be required when
 * the filter is shared between multiple threads (such as between an audio
 * thread and the main thread).
 */
class DSPMath {
private:
    /**
     * Default constructor (does nothing)
     */
    DSPMath() {}
    
    /**
     * Default destructor (does nothing)
     */
    ~DSPMath() {}
    
    
public:
    /** Whether to use a vectorization algorithm (Access not thread safe) */
    static bool VECTORIZE;
    
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
    static size_t add(float* input1, float* input2, float* output, size_t size);
    
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
    static size_t multiply(float* input1, float* input2, float* output, size_t size);

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
    static size_t scale(float* input, float scalar, float* output, size_t size);

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
    static size_t scale_add(float* input1, float* input2, float scalar, float* output, size_t size);
    
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
    static size_t slide(float* input, float start, float end, float* output, size_t size);
    
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
    static size_t slide_add(float* input1, float* input2, float start, float end, float* output, size_t size);

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
    static size_t clamp(float* data, float min, float max, size_t size);

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
    static size_t ease(float* data, float bound, float knee, size_t size);

    // TODO: Add convolution

};
    }
}
#endif /* __CU_DSP_MATH_H__ */

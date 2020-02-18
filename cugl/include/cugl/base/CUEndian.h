//
//  CUEndian.h
//  Cornell University Game Library (CUGL)
//
//  This header includes several inline functions to force data into "network"
//  (or big-endian) order.  This guarantees that serialized binary data is the
//  same across all platforms.
//
//  All of the functions in this header are idempotent. To decode a previously
//  encoded piece of data, use the function again.
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
//  Version: 11/28/16
//
#ifndef __CU_SENDIAN_H__
#define __CU_SENDIAN_H__
#include <SDL/SDL.h>

namespace cugl {

/**
 * Returns the given value encoded in network order
 *
 * On a big-endian system, this function has no effect.  On a little-endian
 * system, it swaps the bytes to put them in big-endian order.
 *
 * This function is idempotent. To decode an encoded value, call this function
 * on the value again.
 *
 * @param value The value to encode
 *
 * @return the given value encoded in network order
 */
SDL_FORCE_INLINE Uint16 marshall(Sint16 value) {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    return (Sint16)SDL_Swap16((Uint16)value);
#else
    return value;
#endif
}

/**
 * Returns the given value encoded in network order
 *
 * On a big-endian system, this function has no effect.  On a little-endian
 * system, it swaps the bytes to put them in big-endian order.
 *
 * This function is idempotent. To decode an encoded value, call this function
 * on the value again.
 *
 * @param value The value to encode
 *
 * @return the given value encoded in network order
 */
SDL_FORCE_INLINE Uint16 marshall(Uint16 value) {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    return SDL_Swap16(value);
#else
    return value;
#endif
}

/**
 * Returns the given value encoded in network order
 *
 * On a big-endian system, this function has no effect.  On a little-endian
 * system, it swaps the bytes to put them in big-endian order.
 *
 * This function is idempotent. To decode an encoded value, call this function
 * on the value again.
 *
 * @param value The value to encode
 *
 * @return the given value encoded in network order
 */
SDL_FORCE_INLINE Uint32 marshall(Sint32 value) {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    return (Sint32)SDL_Swap32((Uint32)value);
#else
    return value;
#endif
}

/**
 * Returns the given value encoded in network order
 *
 * On a big-endian system, this function has no effect.  On a little-endian
 * system, it swaps the bytes to put them in big-endian order.
 *
 * This function is idempotent. To decode an encoded value, call this function
 * on the value again.
 *
 * @param value The value to encode
 *
 * @return the given value encoded in network order
 */
SDL_FORCE_INLINE Uint32 marshall(Uint32 value) {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    return SDL_Swap32(value);
#else
    return value;
#endif
}

/**
 * Returns the given value encoded in network order
 *
 * On a big-endian system, this function has no effect.  On a little-endian
 * system, it swaps the bytes to put them in big-endian order.
 *
 * This function is idempotent. To decode an encoded value, call this function
 * on the value again.
 *
 * @param value The value to encode
 *
 * @return the given value encoded in network order
 */
SDL_FORCE_INLINE Uint64 marshall(Sint64 value) {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    return (Sint64)SDL_Swap64((Uint64)value);
#else
    return value;
#endif
}

/**
 * Returns the given value encoded in network order
 *
 * On a big-endian system, this function has no effect.  On a little-endian
 * system, it swaps the bytes to put them in big-endian order.
 *
 * This function is idempotent. To decode an encoded value, call this function
 * on the value again.
 *
 * @param value The value to encode
 *
 * @return the given value encoded in network order
 */
SDL_FORCE_INLINE Uint64 marshall(Uint64 value) {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    return SDL_Swap64(value);
#else
    return value;
#endif
}

/**
 * Returns the given value encoded in network order
 *
 * On a big-endian system, this function has no effect.  On a little-endian
 * system, it swaps the bytes to put them in big-endian order.
 *
 * This function is idempotent. To decode an encoded value, call this function
 * on the value again.
 *
 * @param value The value to encode
 *
 * @return the given value encoded in network order
 */
SDL_FORCE_INLINE float marshall(float value) {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    union
    {
        float  f;
        Uint32 ui32;
    } swapper;
    swapper.f = value;
    swapper.ui32 = SDL_Swap32(swapper.ui32);
    return swapper.f;
#else
    return value;
#endif
}

/**
 * Returns the given value encoded in network order
 *
 * On a big-endian system, this function has no effect.  On a little-endian
 * system, it swaps the bytes to put them in big-endian order.
 *
 * This function is idempotent. To decode an encoded value, call this function
 * on the value again.
 *
 * @param value The value to encode
 *
 * @return the given value encoded in network order
 */
SDL_FORCE_INLINE double marshall(double value) {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    union
    {
        double d;
        Uint64 ui64;
    } swapper;
    swapper.d = value;
    swapper.ui64 = SDL_Swap64(swapper.ui64);
    return swapper.d;
#else
    return value;
#endif
}

}
#endif /* __CU_ENDIAN_H__ */

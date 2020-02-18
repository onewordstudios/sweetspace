//
//  CUAligned.h
//  Cornell University Game Library (CUGL)
//
//  This module provides an aligned array that is C++11.  Aligned arrays are
//  added to C++17, but there is no guaranteed Android support for C++17.
//
//  This tempalted class does not uses our standard shared-pointer architecture.
//  We often want to copy or move these arrays, so we have non-trivial constructors
//  for this class. With that said, we do have a static method which allocates
//  a shared pointer.
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
//  Version: 6/30/18
#ifndef __CU_ALIGNED_H__
#define __CU_ALIGNED_H__
#include "CUDebug.h"
#include <memory.h>

namespace cugl {
    
#pragma mark -
#pragma mark Aligned Arrays Template
    
/**
 * This template provides support for a aligned arrays
 *
 * Explicit aligned memory allocation is supported in C++17, but not in C++11.
 * As Android can only guarantee C++11, this poses a bit of a problem for Neon
 * and vector optimization.  This class presents a workaround that is compatible
 * with C++11.
 */
template <class T>
class Aligned {
private:
    /** The aligned array */
    T*     _pntr;
    /** The original allocated pointer */
    void*  _orig;
    /** The pointer raw capacity */
    size_t _capacity;
    /** The pointer size as an array of T */
    size_t _length;
    /** The alignment stride */
    size_t _alignm;

#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates an empty (nullptr) aligned array.
     */
    Aligned() :
    _pntr(nullptr),
    _orig(nullptr),
    _capacity(0),
    _length(0),
    _alignm(0) { }
    
    
    
    /**
     * Creates an aligned array of the given size and alignment.
     *
     * @param size      The number of elements in the array
     * @param alignment The alignment stride
     */
    Aligned(size_t size, size_t alignment) {
        _capacity = size*sizeof(T)+alignment;
        _length   = size;
        _alignm   = alignment;
        _orig = malloc(_capacity);
        _pntr = (T*)std::align(alignment,_length*sizeof(T),_orig,_capacity);
    }

    /**
     * Creates an (aligned) copy of the given aligned array.
     *
     * @param copy  The aligned array to copy
     */
    Aligned(const Aligned<T>& copy) {
        _capacity = copy._capacity;
        _length   = copy._length;
        _alignm   = copy._alignm;
        _orig = malloc(_capacity);
        _pntr = (T*)std::align(_alignm,_length*sizeof(T),_orig,_capacity);
        std::memcpy(_pntr,copy._pntr,_length*sizeof(T)); // Only copy visible space
    }

    /**
     * Acquires ownership of the contents of an aligned array.
     *
     * @param other The aligned array to move
     */
    Aligned(Aligned<T>&& other) {
        _capacity = other._capacity;
        _length   = other._length;
        _alignm   = other._alignm;
        _orig = other._orig;
        _pntr = other._pntr;
        other._orig = nullptr;
        other._pntr = nullptr;
    }

    /**
     * Deletes the given aligned array, releasing all resources.
     */
    ~Aligned() { dispose(); }
    
    /**
     * Disposes the resources of given aligned array, making it a null pointer.
     */
    void dispose() {
        if (_orig) free(_orig);
        _pntr = nullptr;
        _orig = nullptr;
        _capacity = 0;
        _length = 0;
    }

    /**
     * Returns an aligned array of the given size and alignment.
     *
     * @param size      The number of elements in the array
     * @param alignment The alignment stride
     *
     * @return an aligned array of the given size and alignment.
     */
    static std::shared_ptr<Aligned<T>> alloc(size_t size, size_t alignment) {
        std::shared_ptr<Aligned<T>> result = std::make_shared<Aligned<T>>();
        return (result->reset(size,alignment) ? result : nullptr);
    }

#pragma mark -
#pragma mark Array Methods
    /**
     * Resets an aligned array to an empty one of the given size and alignment.
     *
     * @param size      The number of elements in the array
     * @param alignment The alignment stride
     */
    bool reset(size_t size, size_t alignment) {
        if (_orig) dispose();
        
        _capacity = size*sizeof(T)+alignment;
        _length   = size;
        _alignm   = alignment;
        
        _orig = malloc(_capacity);
        _pntr = (T*)std::align(_alignm,size*sizeof(T),_orig,_capacity);
        return _pntr;
    }
    
    /**
     * Clears the contents of the given aligned array.
     */
    void clear() {
        std::memset(_pntr,0,_length*sizeof(T));
    }
    
    /**
     * Returns the size of this array.
     *
     * @return the size of this array.
     */
    size_t size() const {
        return _length;
    }

#pragma mark -
#pragma mark Copy Behavior
    /**
     * Copies the given aligned array.
     *
     * @param copy  The aligned array to copy
     *
     * @return a reference to this aligned array for chaining
     */
    Aligned<T>& operator=(const Aligned<T>& copy) {
        _capacity = copy._capacity;
        _length   = copy._length;
        _alignm   = copy._alignm;
        _orig = malloc(_capacity);
        _pntr = (T*)std::align(_alignm,_length*sizeof(T),_orig,_capacity);
        std::memcpy(_pntr,copy._pntr,_length*sizeof(T)); // Only copy visible space
        return *this;
    }
    
    /**
     * Acquires ownership of the contents of the given aligned array.
     *
     * @param other  The aligned array to subsume
     *
     * @return a reference to this aligned array for chaining
     */
    Aligned<T>& operator=(Aligned<T>&& other) {
        _capacity = other._capacity;
        _length   = other._length;
        _alignm   = other._alignm;
        _orig = other._orig;
        _pntr = other._pntr;
        other._orig = nullptr;
        other._pntr = nullptr;
        return *this;
    }

#pragma mark -
#pragma mark Pointer Behavior
    /**
     * Returns a pointer to the first element in the array
     *
     * @return a pointer to the first element in the array
     */
    operator T*() const {
        return _pntr;
    }
    
    
    /**
     * Returns a reference to the first element in the array
     *
     * @return a reference to the first element in the array
     */
    T& operator*() {
        return _pntr[0];
    }

    /**
     * Returns a pointer to the offset element of the array
     *
     * @param offset    The array position to access
     *
     * @return a pointer to the offset element of the array
     */
    T* operator+(size_t offset) {
        return _pntr+offset;
    }
    
    /**
     * Returns a reference to the offset element of the array
     *
     * @param idx    The array position to access
     *
     * @return a reference to the offset element of the array
     */
    T& operator[](size_t idx) {
        return _pntr[idx];
    }

    /**
     * Returns the value of the offset element of the array
     *
     * @param idx    The array position to access
     *
     * @return the value of the offset element of the array
     */
    const T& operator[](size_t idx) const {
        return _pntr[idx];
    }
    

};

}

#endif /* __CU_ALIGNED_H__ */

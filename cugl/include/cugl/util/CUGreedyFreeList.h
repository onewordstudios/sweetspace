//
//  CUGreedyFreeList.h
//  Cornell University Game Library (CUGL)
//
//  This header provides a template for a free list subclass. This version of
//  free is very aggressive at recycling memory.  It is not expandable, and
//  never allocates memory beyond the preallocated capacity.  Instead, if you
//  attempt to allocate beyond the capacity, it will immediately recycle the
//  oldest allocated object, even if it is not freed.
//
//  This sounds a bit unsafe. In order to use it safely, object pointers have
//  to be prepared to be working with a reset object at any given time.
//
//  This is not a class. It is a class template. Templates do not have cpp
//  files. They only have a header file.  When you include the header, it
//  compiles the specific template used by your program. Hence all of the code
//  for this templated class is in this header.
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
//  Version: 11/29/16
//
#ifndef __CU_GREEDY_LIST_H__
#define __CU_GREEDY_LIST_H__
#include "CUFreeList.h"

namespace cugl {
    
#pragma mark -
#pragma mark GreedyFreeList Template
/**
 * Template for a free list class with aggressive recycling.
 *
 * This free list is not expandable, and never allocates memory beyond the 
 * preallocated capacity.  Instead, if you attempt to allocate beyond the 
 * capacity, it will immediately recycle the oldest allocated object, even 
 * if it is not freed.
 *
 * This sounds a bit unsafe. In order to use it safely, object pointers have to 
 * be prepared to be working with a reset object at any given time.  In 
 * particular, it is designed for particle systems, where the particles are 
 * managed by a set that does not permit duplicates.  That way, an allocation 
 * of a forceably recycled object will only appear once in the list.
 *
 * In order to work properly, the objects allocated must all have the method
 *
 *    void reset();
 *
 * This method resets the object when it is recycled. It is like a destructor,
 * except that the object is not actually deleted. The class does not have to
 * formally subclass anything or implement an interface (C++ does not work that
 * way). It just has to have this method. In addition, the class must have a
 * default constructor with no arguments.  You should have an init() method if
 * you need to initialize the object after allocation.
 *
 * This class owns all memory that it allocates.  When the free list is deleted,
 * all of the objects that it allocated will be deleted also.
 *
 * A free list is not an all-purpose memory allocator.  It is restricted to a
 * single class.  It should only be used for specialized applications.
 *
 * WARNING: Templates cannot support virual methods.  Therefore it is unsafe
 * to downcast a GreedyFreeList pointer to a FreeList pointer.
 */
template <class T>
class GreedyFreeList : public FreeList<T> {
protected:
    /** Tracks all of the memory that has been allocated, allowing forceable recycling */
    std::queue<T*> _allocation;
    
#pragma mark Constructors
public:
    /**
     * Creates a new greedy free list with no capacity.
     *
     * You must initialize this greedy free list before use.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a free list on
     * the heap, use one of the static constructors instead.
     */
    GreedyFreeList() : FreeList<T>() { }
    
    /**
     * Deletes this free list, releasing all memory.
     *
     * A free list is the owner of all memory it allocates. Any object allocated
     * by this free list will be deleted and unsafe to access.
     */
    ~GreedyFreeList() { }
    
    /**
     * Initializes a greedy free list with the given capacity
     *
     * As it is not expandable, it will never allocate any objects beyond those
     * preallocated in this constructor.  Hence the capacity must be non-zero.
     *
     * @param  capacity the number of objects to preallocate
     *
     * @return true if initialization was successful.
     */
    bool init(size_t capacity) {
        return FreeList<T>::init(capacity);
    }
    
    /**
     * This inherited initializer is disabled
     */
    bool init(size_t capacity, bool expand) {
        CUAssertLog(false, "This initialzier cannot be used with GreedyFreeList");
        return false;
    }
    
    /**
     * Returns a newly allocated greedy free list with the given capacity.
     *
     * As it is not expandable, it will never allocate any objects beyond those
     * preallocated in this constructor.  Hence the capacity must be non-zero.
     *
     * @param  capacity the number of objects to preallocate
     *
     * @return a newly allocated greedy free list with the given capacity.
     */
    static std::shared_ptr<GreedyFreeList<T>> alloc(size_t capacity) {
        std::shared_ptr<GreedyFreeList<T>> result = std::make_shared<GreedyFreeList<T>>();
        return (result->init(capacity) ? result : nullptr);
    }
    
#pragma mark Memory Managment
    /**
     * Returns a pointer to a newly allocated T object.
     *
     * If there are any objects on the free list, it will recycle them.  Next, if
     * there are any preallocated objects, it will use one of those.  Finally, it
     * will forceably recycle the oldest allocated object.
     *
     * @return a pointer to a newly allocated T object.
     */
    T* malloc();
};


#pragma mark -
#pragma mark Method Implementations
/**
 * Returns a pointer to a newly allocated T object.
 *
 * If there are any objects on the free list, it will recycle them.  Next, if
 * there are any preallocated objects, it will use one of those.  Finally, it
 * will forceably recycle the oldest allocated object.
 *
 * @return a pointer to a newly allocated T object.
 */
template <class T>
T* GreedyFreeList<T>::malloc() {
    T* result = FreeList<T>::malloc();
    if (result == nullptr && !(_allocation.empty())) {
        result = _allocation.front();
        _allocation.pop();
        result->reset();
        _allocation.push(result);
    } else if (result != nullptr) {
        _allocation.push(result);
    }
    return result;
}

}

#endif /* __CU_GREEDY_LIST_H__ */

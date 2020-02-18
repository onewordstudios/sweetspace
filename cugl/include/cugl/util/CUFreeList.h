//
//  CUFreeList.h
//  Cornell University Game Library (CUGL)
//
//  This header provides a template for a free list class.  A free list allows
//  you to "recycle" memory.  As allocating and deleting memory is an expensive
//  operation, this has significant performance problems with systems that
//  create a lot of short live objects (e.g. particle systems).  Free lists
//  solve this problem by replacing the new operator with a method to manage
//  your memory.
//
//  A free list can also allocate a block of memory at creation.  This allows
//  you to group all your initial allocations ahead of time. See the class
//  documentation for more information.
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
#ifndef __CU_FREE_LIST_H__
#define __CU_FREE_LIST_H__
#include <queue>

namespace cugl {
    
#pragma mark -
#pragma mark FreeList Template

/**
 * Template for a free list class
 *
 * A free list provides a way to recycle heap allocations.  Instead of using 
 * the operations new and delete, you use the methods {@link malloc()} and 
 * {@link free()} in this class (remember that {@link alloc()} is used to 
 * allocate the free list itself). Hence an instance of this class effectively 
 * plays the role of your heap.
 *
 * The method ,alloc() looks to see if there is any recycled memory, and uses
 * that before allocating a new object.  In addition, the user can allocated 
 * memory in the constructor of the free list, providing an initial list of 
 * preallocated memory at the start.  If both the recycled and preallocated 
 * memory are exhausted, then this class will start to allocate new memory.
 *
 * The exception is the case in which the free list is not expandable. In that 
 * case, the free list never has any more memory beyond what was allocated at 
 * the beginning. Any attempts to allocate memory beyond this bound will return 
 * a null pointer.
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
 * This class owns all memory that it allocates. When the free list is deleted,
 * all of the objects that it allocated will be deleted also.
 *
 * A free list is not an all-purpose memory allocator.  It is restricted to a
 * single class.  It should only be used for specialized applications.
 */
template <class T>
class FreeList {
protected:
    /** The number of objects allocated so far */
    size_t _allocated;
    /** The number of objects released. */
    size_t _released;
    /** The memory high water mark */
    size_t _peaksize;
    
    /** The array of preallocated objects */
    T* _prealloc;
    /** The capacity of the preallocated objects */
    size_t _capacity;
    
    /** The list of objects in the free list */
    std::queue<T*> _freeobjs;
    
    /** Whether or not we can add objects beyond the ones preallocated */
    bool _expandable;
    /** Place to put the expanded objects */
    std::queue<T*> _expansion;
   
#pragma mark Constructors
public:
    /**
     * Creates a new free list with no capacity.
     *
     * You must initialize this free list before use.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a free list on
     * the heap, use one of the static constructors instead.
     */
    FreeList() : _allocated(0), _released(0), _peaksize(0), _prealloc(nullptr),
                 _capacity(0), _expandable(false) { }
    
    /**
     * Deletes this free list, releasing all memory.
     *
     * A free list is the owner of all memory it allocates. Any object allocated
     * by this free list will be deleted and unsafe to access.
     */
    ~FreeList()	{ dispose(); }
    
    /**
     * Disposes this free list, releasing all memory.
     *
     * A disposed free list can be safely reinitialized. However, a free list 
     * is the owner of all memory it allocates.  Any object allocated by this 
     * free list will be deleted and unsafe to access.
     */
    void dispose() {
        clear();
        if (_prealloc != nullptr) {
            delete[] _prealloc;
        }
    }

    /**
     * Initializes a free list with the given capacity, but not expandable.
     *
     * As it is not expandable, it will never allocate any objects beyond those 
     * preallocated in this constructor.  Hence the capacity must be non-zero.
     *
     * @param  capacity the number of objects to preallocate
     *
     * @return true if initialization was successful.
     */
    bool init(size_t capacity) {
        CUAssertLog(capacity, "An unexapandable free list must have non-zero capacity");
        _expandable = false;
        _capacity  = capacity;
        _released  = 0;
        _allocated = 0;
        _prealloc = (_capacity > 0 ? new T[_capacity] : nullptr);
        return _prealloc;
    }
        
    /**
     * Initializes a free list with the given capacity.
     *
     * If capacity is non-zero, then it will allocate that many objects ahead 
     * of time. If expand is false, then it will never allocate any objects 
     * beyond those preallocated in this constructor.
     *
     * @param  capacity the number of objects to preallocate
     * @param  expand   whether to allow non-preallocated objects
     *
     * @return true if initialization was successful.
     */
    bool init(size_t capacity, bool expand) {
        CUAssertLog(capacity || expand, "The free list must be expandable or have capacity non-zero");
        _expandable = expand;
        _capacity  = capacity;
        _released  = 0;
        _allocated = 0;
        _prealloc = (_capacity > 0 ? new T[_capacity] : nullptr);
        return  _capacity == 0 || _prealloc;
    }

    /**
     * Returns a newly allocated free list with the given capacity.
     *
     * If capacity is non-zero, then it will allocate that many objects ahead
     * of time. If expand is false, then it will never allocate any objects
     * beyond those preallocated in this constructor.
     *
     * @param  capacity the number of objects to preallocate
     * @param  expand   whether to allow non-preallocated objects
     *
     * @return a newly allocated free list with the given capacity.
     */
    static std::shared_ptr<FreeList<T>> alloc(size_t capacity=0, bool expand=false) {
        std::shared_ptr<FreeList<T>> result = std::make_shared<FreeList<T>>();
        return (result->init(capacity,expand) ? result : nullptr);
    }
    
    
#pragma mark Accessors
    /**
     * Returns the number of objects that can be allocated without more memory.
     *
     * This value is the number of elements in the free list plus the number of
     * elements remaining in the preallocation list.
     *
     * @return the number of objects that can be allocated without more memory.
     */
    size_t getAvailable() const	{ return (_capacity-(long)_allocated)+_freeobjs.size(); }
    
    /**
     * Returns the preallocated capacity of this list.
     *
     * If the free list is not expandable, this it the maximum number of objects
     * that may be allocated at any given time.
     *
     * @return the preallocated capacity of this list.
     */
    size_t getCapacity() const { return _capacity; }
    
    /**
     * Returns the number of objects that have been allocated but not released yet.
     *
     * Allocating an object will increase this value; free an object will decrease
     * the value.
     *
     * @return the number of objects that have been allocated but not released yet.
     */
    size_t getUsage() const { return _allocated-_released; }
    
    /**
     * Returns the maximum usage value at any given time in this object's lifecycle.
     *
     * This value represents the high-water mark for memory.  It is very useful if
     * this is an expandable free list.
     *
     * @return the maximum usage value at any given time in this object's lifecycle.
     */
    size_t getPeakUsage() const { return _peaksize; }
    
    /**
     * Returns whether this free list is allowed to allocate additional memory.
     *
     * If the free list is not expandable, the capacity is the maximum number of
     * objects that may be allocated at any given time.
     *
     * @return whether this free list is allowed to allocate additional memory.
     */
    bool isExpandable() const { return _expandable; }
    
    /**
     * Returns the pointer to the preallocated memory (if any) of this object.
     *
     * It is unsafe to modify this pointer.  Hence it is returned as a constant.
     *
     * @return the pointer to the preallocated memory (if any) of this object.
     */
    const T* getPreallocated() const { return _prealloc; }
    
    
#pragma mark Memory Managment
    /**
     * Returns a pointer to a newly allocated T object.
     *
     * If there are any objects on the free list, it will recycle them.  Next,
     * if there are any preallocated objects, it will use one of those. Finally,
     * it checks to see if the list is expandable or not.  If so, it will 
     * allocate an additional object. Otherwise, it will return nullptr.
     *
     * @return a pointer to a newly allocated T object.
     */
    T* malloc();
    
    /**
     * Frees the object, adding it to the free list.
     *
     * This method will call the reset() method in the object, erasing its
     * contents.  The class should be designed so that it cannot be used until
     * it is reintialized.
     *
     * It is possible to add an object that was not originally allocated by
     * this free list.  Doing so will make the object available for allocation.
     * However, the free list will not assert ownership of the object, and will
     * not delete it when it is cleaning up.
     *
     * @param  obj  the object to free
     */
    void free(T* obj);
    
    /**
     * Clears this free list, restoring it to its original state.
     *
     * This method (1) empties the free list, (2) resets all preallocated objects
     * allowing them to be reused and (3) deletes any other objects that might
     * have been allocated.
     */
    virtual void clear();
};


#pragma mark -
#pragma mark Method Implementations

/**
 * Returns a pointer to a newly allocated T object.
 *
 * If there are any objects on the free list, it will recycle them.  Next, if
 * there are any preallocated objects, it will use one of those.  Finally, it
 * checks to see if the list is expandable or not.  If so, it will allocate
 * an additional object.  Otherwise, it will return nullptr.
 *
 * @return a pointer to a newly allocated T object.
 */
template <class T>
T* FreeList<T>::malloc() {
    T* result = nullptr;
    if (!_freeobjs.empty()) {
        result = _freeobjs.front();
        _freeobjs.pop();
        _allocated++;
    } else if (_allocated < _capacity) {
        result = &_prealloc[_allocated];
        _allocated++;
    } else if (_expandable) {
        result = new T();
        _expansion.push(result);
        _allocated++;
    }
    _peaksize = (_peaksize < getUsage() ? getUsage() : _peaksize);
    return result;
}

/**
 * Frees the object, adding it to the free list.
 *
 * This method will call the reset() method in the object, erasing its
 * contents.  The class should be designed so that it cannot be used until
 * it is reintialized.
 *
 * It is possible to add an object that was not originally allocated by
 * this free list.  Doing so will make the object available for allocation.
 * However, the free list will not assert ownership of the object, and will
 * not delete it when it is cleaning up.
 *
 * @param  obj  the object to free
 */
template <class T>
void FreeList<T>::free(T* obj) {
    CUAssertLog(obj != nullptr, "Attempt to free null pointer");
    _freeobjs.push(obj);
    obj->reset();
    _released++;
}

/**
 * Clears this free list, restoring it to its original state.
 *
 * This method (1) empties the free list, (2) resets all preallocated objects
 * allowing them to be reused and (3) deletes any other objects that might
 * have been allocated.
 */
template <class T>
void FreeList<T>::clear() {
    // We own anything in expansion.  Clear it.
    while (!_expansion.empty()) {
        T* a = _expansion.front();
        _expansion.pop();
        delete a;
    }
    
    // Clear the preallocated
    for(int ii =0; ii < _capacity; ii++) {
        _prealloc[ii].reset();
    }
    
    // Clear everything else
    while (!_expansion.empty()) {
        _freeobjs.pop();
    }
    
    _allocated = 0;
    _released  = 0;
}

}
#endif /* __CU_FREE_LIST_H__ */

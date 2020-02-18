//
//  CUThreadPool.h
//  Cornell University Game Library (CUGL)
//
//  Module for a pool of threads capable of executing asynchronous tasks.  Each
//  task is specified by a void function.  There are no guarantees about thread
//  safety; that is responsibility of the author of each task.
//
//  This code is largely inspired from the Cocos2d file AudioEngine.cpp, from
//  the code for asynchronous asset loading. We generalized that class added
//  some notable safety changes.
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
#ifndef __CU_THREAD_POOL_H__
#define __CU_THREAD_POOL_H__
#include <cugl/base/CUBase.h>
#include <SDL/SDL.h>
#include <condition_variable>
#include <stdio.h>
#include <queue>
#include <vector>
#include <thread>

// std::thread is not safe on Android and Windows platforms
#if defined (__WINDOWS__) || defined (__ANDROID__)
    #define CU_SDL_THREADS 1
#endif

namespace cugl {

#pragma mark -
#pragma mark Thread Pool

/**
 *  Class to providing a collection of worker threads.
 *
 *  This is a general purpose class for performing tasks asynchronously.  There 
 *  is no notification process for when a task is complete.  Instead, your task 
 *  should either set a flag, or execute a callback when it is done.
 *
 *  There are some important safety considerations for using this class over
 *  direct thread objects. For example, stopping a thread pool does not shut it 
 *  down immediately; it just marks it for shutdown.  Because of mutex locks, 
 *  it is not safe to delete a thread pool until it is completely shutdown.
 *
 *  More importantly, we do not allow for detached threads. This makes no sense
 *  in this application, because the threads share a resource (_taskQueue) with 
 *  the main thread that will be deleted.  It is therefore unsafe for the 
 *  threads to ever detach.
 *
 *  See the class {@link AssetManager} for an example of how to use a thread 
 *  pool.
 */
class ThreadPool {
private:
    /** The individual worker threads for this thread pool */
#ifdef CU_SDL_THREADS
    std::vector<SDL_Thread*> _workers;
#else
    std::vector<std::thread> _workers;
#endif
    
    /** Tasks waiting to be assigned to a thread */
    std::queue< std::function<void()> > _taskQueue;
    
    /** A mutex lock for the task queue */
    std::mutex _queueMutex;
    /** A condition variable to manage tasks waiting for a worker */
    std::condition_variable _taskCondition;
    
    /** Whether or not the thread pool has been marked for shutdown */
    bool _stop;
    /** The number of child threads that are completed */
    int _complete;
    
    /**
     * The body function of a single thread.
     *
     * This function pulls tasks from the task queue.  
     *
     * This implementation is safe to use with std::thread.
     */
    void threadFunc();

    /**
     * The body function of a single thread.
     *
     * This function pulls tasks from the task queue.
     *
     * This static implementation uses the SDL thread API.  It should be used
     * on Android and Windows, which have special thread requirements.
     */
    static int sdlThreadFunc(void* ptr);
    

#pragma mark Constructors
public:
    /**
     * Creates a thread pool with no active threads.
     *
     * You must initialize this thread pool before use.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a thread pool 
     * on the heap, use one of the static constructors instead.
     */
    ThreadPool() :_stop(false), _complete(0) { }
    
    /**
     * Deletes this thread pool, destroying all resources.
     *
     * It is a bad idea to destroy the thread pool if the pool is not yet shut
     * down. The task queue is shared by the child threads, so we cannot delete
     * it until all the threads complete.  This destructor will block unti 
     * showndown.
     */
    ~ThreadPool() { dispose(); }
    
    /**
     * Disposes this thread pool, releasing all memory.
     *
     * A disposed thread pool can be safely reinitialized. However, it is a bad 
     * idea to destroy the thread pool if the pool is not yet shut down. The 
     * task queue is shared by the child threads, so we cannot delete it until 
     * all the threads complete.  This destructor will block unti showndown.
     */
    void dispose();
    
    /**
     * Initializes a thread pool with the given number of threads.
     *
     * You can specify the number of simultaneous worker threads. We find that 
     * 4 is generally a good number, even if you have a lot of tasks.  Much
     * more than the number of cores on a machine is counter-productive.
     *
     * @param threads   the number of threads in this pool
     *
     * @return true if the threed pool is initialized properly, false otherwise.
     */
    virtual bool init(int threads = 4);

    
#pragma mark Static Constructors
    /**
     * Returns a newly allocated thread pool with the given number of threads.
     *
     * You can specify the number of simultaneous worker threads. We find that
     * 4 is generally a good number, even if you have a lot of tasks.  Much
     * more than the number of cores on a machine is counter-productive.
     *
     * @param threads   the number of threads in this pool
     *
     * @return a newly allocated thread pool with the given number of threads.
     */
    static std::shared_ptr<ThreadPool> alloc(int threads = 4) {
        std::shared_ptr<ThreadPool> result = std::make_shared<ThreadPool>();
        return (result->init(threads) ? result : nullptr);
    }
    
    
#pragma mark Task Management
    /**
     * Adds a task to the thread pool.
     *
     * A task is a void returning function with no parameters.  If you need 
     * state in the task, you should use a method call for the state.  The task 
     * will not be executed immediately, but must wait for the first available 
     * worker.
     *
     * @param  task     the task function to add to the thread pool
     */
    void addTask(const std::function<void()> &task);
    
    /**
     * Stop the thread pool, marking it for shut down.
     *
     * A stopped thread pool is marked for shutdown, but it shutdown has not 
     * necessarily completed.  Shutdown will be complete when the current child 
     * threads have finished with their tasks.
     */
    void stop();
    
    /**
     * Returns whether the thread pool has been stopped.
     *
     * A stopped thread pool is marked for shutdown, but it shutdown has not
     * necessarily completed.  Shutdown will be complete when the current child 
     * threads have finished with their tasks.
     *
     * @return whether the thread pool has been stopped.
     */
    bool isStopped() const { return _stop; }
    
    /**
     * Returns whether the thread pool has been shut down.
     *
     * A shut down thread pool has no active threads and is safe for deletion.
     *
     * @return whether the thread pool has been shut down.
     */
    bool isShutdown() const { return _workers.size() == _complete; }
  
private:  
    /** Copying is only allowed via shared pointer. */
    CU_DISALLOW_COPY_AND_ASSIGN(ThreadPool);
};

}

#endif /* __CU_THREAD_POOL_H__ */

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
#include <cugl/util/CUThreadPool.h>

using namespace cugl;

#pragma mark -
#pragma mark Constructors
/**
 * Disposes this thread pool, releasing all memory.
 *
 * A disposed thread pool can be safely reinitialized. However, it is a bad
 * idea to destroy the thread pool if the pool is not yet shut down. The
 * task queue is shared by the child threads, so we cannot delete it until
 * all the threads complete.  This destructor will block unti showndown.
 */
void ThreadPool::dispose() {
    stop();
    while (!isShutdown());
}

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
bool ThreadPool::init(int threads) {
    for (int index = 0; index < threads; ++index) {
#ifdef CU_SDL_THREADS
        _workers.emplace_back(SDL_CreateThread(ThreadPool::sdlThreadFunc,"Pool Dispatch",(void*)this));
#else
        _workers.emplace_back(std::thread(std::bind(&ThreadPool::threadFunc, this)));
#endif
    }
    return true;
}


#pragma mark -
#pragma mark Thread Execution

/**
 * The body function of a single thread.
 *
 * This function pulls tasks from the task queue.
 *
 * This implementation is safe to use with std::thread.
 */
void ThreadPool::threadFunc() {
    while (!_stop) {
        std::function<void()> task = nullptr;
        {   // Lock for save queue access
            std::unique_lock<std::mutex> lk(_queueMutex);
            if (_stop) {
                break;
            }
            // Pull the next task off the queue
            if (!_taskQueue.empty()) {
                task = std::move(_taskQueue.front());
                _taskQueue.pop();
            } else {
                _taskCondition.wait(lk);
                continue;
            }
        }
        // Perform the current task
        task();
    }
    _complete++;
}

/**
 * The body function of a single thread.
 *
 * This function pulls tasks from the task queue.
 *
 * This static implementation uses the SDL thread API.  It should be used
 * on Android and Windows, which have special thread requirements.
 */
int ThreadPool::sdlThreadFunc(void* ptr) {
    ThreadPool* self = (ThreadPool*)ptr;
    while (!self->_stop) {
        std::function<void()> task = nullptr;
        {   // Lock for save queue access
            std::unique_lock<std::mutex> lk(self->_queueMutex);
            if (self->_stop) {
                break;
            }
            // Pull the next task off the queue
            if (!self->_taskQueue.empty()) {
                task = std::move(self->_taskQueue.front());
                self->_taskQueue.pop();
            } else {
                self->_taskCondition.wait(lk);
                continue;
            }
        }
        // Perform the current task
        task();
    }
    self->_complete++;
    return 0;
}



#pragma mark -
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
void ThreadPool::addTask(const std::function<void()> &task){
    std::unique_lock<std::mutex> lk(_queueMutex);
    _taskQueue.emplace(task);
    _taskCondition.notify_one();
}

/**
 * Stop the thread pool, marking it for shut down.
 *
 * A stopped thread pool is marked for shutdown, but it shutdown has not
 * necessarily completed.  Shutdown will be complete when the current child
 * threads have finished with their tasks.
 */
void ThreadPool::stop() {
    {
        std::unique_lock<std::mutex> lk(_queueMutex);
        _stop = true;
        _taskCondition.notify_all();
    }
    
    for (auto&& worker : _workers) {
#ifdef CU_SDL_THREADS
        int status;
        SDL_WaitThread(worker,&status);
#else
        worker.join();
#endif
    }
}

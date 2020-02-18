//
//  CUTimestamp.h
//  Cornell University Game Library (CUGL)
//
//  This module simplifies timestamp support a bit.  Accurate timestamp support
//  is necessary for touch and mouse support, but timestamps requires some
//  really arcane C++11.
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
//  Version: 2/10/15
//
#ifndef __CU_TIMESTAMP_H__
#define __CU_TIMESTAMP_H__
#include <chrono>
#include <cugl/base/CUBase.h>
#include "CUDebug.h"

#pragma mark -
#pragma mark Clock Data Types

/** The clock data type, using the steady clock from chrono */
typedef std::chrono::steady_clock cuclock_t;
/** The timestamp data type, relative to our clock */
typedef cuclock_t::time_point   timestamp_t;

namespace cugl {

#pragma mark -
#pragma mark TimeStamp Class
/**
 * Class to mark a moment in time.
 *
 * The advantage of using this class over timestamp_t is that it has many
 * built-in methods for converting the timestamp to milliseconds, nanoseconds,
 * and so on.  It is a lot easier to use this class that to use the more 
 * arcane chrono package in C++.
 *
 * While this is a class, this is meant to be created on the stack.  Therefore
 * there is no support for shared pointers or initialization like in our 
 * other, more heavy-weight classes.
 */
class Timestamp {
protected:
    /** The current timestamp using the steady clock from chrono. */
    timestamp_t _time;

#pragma mark Constructors
public:
    /**
     * Constructs a new time stamp.
     *
     * The constructor marks the exact moment that it was created.
     */
    Timestamp() { mark(); }
    
    /**
     * Constructs a copy of the given time stamp.
     *
     * The constructor does not mark the time it copies, but instead uses the 
     * value taken from stamp.
     *
     * @param stamp The time stamp to copy
     */
    Timestamp(const Timestamp& stamp) { set(stamp); }

    /**
     * Sets this time stamp to be a copy of stamp.
     *
     * This operator does not mark the time it copies, but instead uses the
     * value taken from stamp.
     *
     * @param stamp The time stamp to copy
     *
     * @return a reference to this timestamp for chaining.
     */
    const Timestamp& operator=(const Timestamp& stamp) {
        return set(stamp);
    }

    /**
     * Sets this time stamp to be a copy of stamp.
     *
     * This operator does not mark the time it copies, but instead uses the
     * value taken from stamp.
     *
     * @param stamp The time stamp to copy
     *
     * @return a reference to this timestamp for chaining.
     */
    const Timestamp& set(const Timestamp& stamp) {
        _time = stamp.getTime();
        return *this;
    }
    
    /**
     * Sets this time stamp to be the current moment in time.
     *
     * The previous value of this time stamp will be erased.
     */
    void mark() {
        _time = cuclock_t::now();
    }
    
#pragma mark Reading Time
    /**
     * Returns the current time stamp in its internal format
     *
     * This format is not much use by itself.  It only allows you to use
     * functions in chrono not supported by this class.  For most purposes
     * you should used one of the ellapsed methods instead.
     *
     * @return the current time stamp in its internal format
     */
    timestamp_t getTime() const { return _time; }

    /**
     * Returns the ellapsed time between the given two timestamps.
     *
     * The value is returned in milliseconds.
     *
     * @param start The start time
     * @param end   The finish time
     *
     * @return the ellapsed time between the given two timestamps.
     */
    static Uint64 ellapsedMillis(const Timestamp& start, const Timestamp& end) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end.getTime()-start.getTime());
        return (Uint64)elapsed.count();
    }

    /**
     * Returns the ellapsed time from the provided time.
     *
     * This method measures the time from stamp until the current time.  Hence
     * the current time is the end of the interval.
     *
     * The value is returned in milliseconds.
     *
     * @param stamp The start time
     *
     * @return the ellapsed time from the provided time.
     */
    Uint64 ellapsedMillis(const Timestamp& stamp) const  {
        return Timestamp::ellapsedMillis(stamp,*this);
    }
    
    /**
     * Returns the ellapsed time between the given two timestamps.
     *
     * The value is returned in microseconds.
     *
     * @param start The start time
     * @param end   The finish time
     *
     * @return the ellapsed time between the given two timestamps.
     */
    static Uint64 ellapsedMicros(const Timestamp& start, const Timestamp& end)  {
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end.getTime()-start.getTime());
        return (Uint64)elapsed.count();
    }
    
    /**
     * Returns the ellapsed time from the provided time.
     *
     * This method measures the time from stamp until the current time.  Hence
     * the current time is the end of the interval.
     *
     * The value is returned in microseconds.
     *
     * @param stamp The start time
     *
     * @return the ellapsed time from the provided time.
     */
    Uint64 ellapsedMicros(const Timestamp& stamp) const  {
        return Timestamp::ellapsedMicros(stamp,*this);
    }

    /**
     * Returns the ellapsed time between the given two timestamps.
     *
     * The value is returned in nanoseconds.
     *
     * @param start The start time
     * @param end   The finish time
     *
     * @return the ellapsed time between the given two timestamps.
     */
    static Uint64 ellapsedNanos(const Timestamp& start, const Timestamp& end) {
        auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end.getTime()-start.getTime());
        return (Uint64)elapsed.count();
    }
    
    /**
     * Returns the ellapsed time from the provided time.
     *
     * This method measures the time from stamp until the current time.  Hence
     * the current time is the end of the interval.
     *
     * The value is returned in nanoseconds.
     *
     * @param stamp The start time
     *
     * @return the ellapsed time from the provided time.
     */
    Uint64 ellapsedNanos(const Timestamp& stamp) {
        return Timestamp::ellapsedNanos(stamp,*this);
    }
    
#pragma mark Operators

    /**
     * Increments this time stamp by the given number of milliseconds
     *
     * While we can support timestamps more accurate than milliseconds,
     * SDL events only mark time increments at the millisecond level.
     * As this is the primary reason for incrementing a time stamp by
     * a integer value, we only support milliseconds for this operation.
     *
     * @param millis    The number of milliseconds to increment
     *
     * @return a reference to this time stamp for chaining
     */
    Timestamp& operator+=(Uint32 millis) {
        _time += std::chrono::milliseconds(millis);
        return *this;
    }

    /**
     * Decrements this time stamp by the given number of milliseconds
     *
     * While we can support timestamps more accurate than milliseconds,
     * SDL events only mark time increments at the millisecond level.
     * As this is the primary reason for incrementing a time stamp by
     * a integer value, we only support milliseconds for this operation.
     *
     * @param millis    The number of milliseconds to decrement
     *
     * @return a reference to this time stamp for chaining
     */
    Timestamp& operator-=(Uint32 millis) {
        _time -= std::chrono::milliseconds(millis);
        return *this;
    }

    /**
     * Returns a copy of this time stamp incremented by the number of milliseconds
     *
     * While we can support timestamps more accurate than milliseconds,
     * SDL events only mark time increments at the millisecond level.
     * As this is the primary reason for incrementing a time stamp by
     * a integer value, we only support milliseconds for this operation.
     *
     * @param millis    The number of milliseconds to increment
     *
     * @return a copy of this time stamp incremented by the number of milliseconds
     */
    const Timestamp operator+(Uint32 millis) const {
        return Timestamp(*this) += millis;
    }
    
    /**
     * Returns a copy of this time stamp decremented by the number of milliseconds
     *
     * While we can support timestamps more accurate than milliseconds,
     * SDL events only mark time increments at the millisecond level.
     * As this is the primary reason for incrementing a time stamp by
     * a integer value, we only support milliseconds for this operation.
     *
     * @param millis    The number of milliseconds to decrement
     *
     * @return a copy of this time stamp decremented by the number of milliseconds
     */
    const Timestamp operator-(Uint32 millis) const {
        return Timestamp(*this) -= millis;
    }

    /**
     * Returns whether this timestamp is less than another timestamp.
     *
     * @param other	The timestamp to compare
     *
     * @return whether this timestamp is less than another timestamp.
     */
    bool operator<(const Timestamp& other) const {
        return _time < other._time;
    }

};

}
#endif /* __CU_TIMESTAMP_H__ */

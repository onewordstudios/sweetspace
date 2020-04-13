#ifndef __JS_EVENT_MODEL_H__
#define __JS_EVENT_MODEL_H__
#include <cugl/cugl.h>
#include <cugl/io/CUJsonReader.h>

#include <vector>

#include "LevelConstants.h"

using namespace cugl;

#pragma mark -
#pragma mark Event Model

/**
 * Class that represents a dynamically loaded event in the game
 *
 */
class EventModel {
   public:
   private:
	/**The name of the block to generate*/
	string block;

	/**The time to start this event*/
	int timeStart;

	/**The time to stop this event*/
	int timeStop;

	/**The probability per update frame of occurrence */
	float probability;

   public:
#pragma mark Static Constructors

	/**
	 * Creates a new building block with the given JSON file.
	 *
	 * @return a new building block
	 */
	static std::shared_ptr<EventModel> alloc(const std::shared_ptr<cugl::JsonValue>& json) {
		std::shared_ptr<EventModel> result = std::make_shared<EventModel>();
		return (result->init(json) ? result : nullptr);
	}

#pragma mark Event Attributes

	/**
	 * Returns the name of the block used
	 *
	 * @return name of block used
	 */
	string getBlock() { return block; }

	/**
	 * Returns the start time
	 *
	 * @return start time
	 */
	int getStart() { return timeStart; }

	/**
	 * Returns the end time
	 *
	 * @return end time
	 */
	int getEnd() { return timeStop; }

	/**
	 * Returns the probability this event is generated per update frame
	 *
	 * @return probability per update frame
	 */
	float getProbability() { return probability; }

	/**
	 * Returns whether this event is active
	 *
	 * @param time the current game time
	 * @return whether this event is active
	 */
	bool isActive(int time) { return time <= timeStop && time >= timeStart; }

	/**
	 * Returns whether this event is one time
	 *
	 * @return whether this event is one time
	 */
	bool isOneTime() { return timeStop == timeStart; }

#pragma mark -
#pragma mark Initializers
	/**
	 * Creates a new, empty level.
	 */
	EventModel(void){};

	bool init(const std::shared_ptr<cugl::JsonValue>& json) {
		block = json->get(BLOCK_FIELD)->asString();
		timeStart = json->get(TIME_START_FIELD)->asInt();
		timeStop = json->get(TIME_STOP_FIELD)->asInt();
		probability = json->get(PROBABILITY_FIELD)->asFloat();
		return true;
	}

	/**
	 * Destroys this level, releasing all resources.
	 */
	virtual ~EventModel(void){};
};

#endif /* defined(__JS_LEVEL_MODEL_H__) */

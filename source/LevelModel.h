#ifndef JS_LEVEL_MODEL_H
#define JS_LEVEL_MODEL_H
#include <cugl/2d/physics/CUObstacleWorld.h>
#include <cugl/assets/CUAsset.h>
#include <cugl/cugl.h>
#include <cugl/io/CUJsonReader.h>

#include <map>
#include <vector>

#include "BuildingBlockModel.h"
#include "EventModel.h"
#include "LevelConstants.h"

#pragma mark -
#pragma mark Level Model

const unsigned int DEFAULT_MAX_BUTTONS = 2;
const unsigned int DEFAULT_MAX_BREACHES = 3;
const unsigned int DEFAULT_MAX_DOORS = 1;
const unsigned int DEFAULT_BASE_SIZE = 360;
const int DEFAULT_PER_PLAYER = 45;
const int DEFAULT_INIT_HEALTH = 11;
const float DEFAULT_TIME = 45;

/**
 * Class that represents a dynamically loaded level in the game
 *
 * This class is a subclass of Asset so that we can use it with a GenericLoader.
 */
class LevelModel : public cugl::Asset {
   private:
	/** The maximum number of breaches on ship at any one time. This will probably need to scale
	 * with the number of players*/
	uint8_t maxBreaches;
	/** The maximum number of doors on ship at any one time. This will probably need to scale with
	 * the number of players*/
	uint8_t maxDoors;
	/** The maximum number of buttons on ship at any one time. This will probably need to scale with
	 * the number of players*/
	uint8_t maxButtons;
	/** Base size of the ship in degrees*/
	float baseShipSize;
	/** Incremental size of the ship in degrees*/
	float perPlayer;
	/** Starting time for the timer*/
	float time;
	/** Starting health for the ship*/
	float initHealth;
	/** List of building blocks for this level*/
	map<std::string, std::shared_ptr<BuildingBlockModel>> blocks;
	/** List of events for this level*/
	vector<std::shared_ptr<EventModel>> events;

   public:
#pragma mark Static Constructors
	/**
	 * Creates a new game level with no source file.
	 *
	 * The source file can be set at any time via the setFile() method. This method
	 * does NOT load the asset.  You must call the load() method to do that.
	 *
	 * @return  an autoreleased level file
	 */
	static std::shared_ptr<LevelModel> alloc() {
		std::shared_ptr<LevelModel> result = std::make_shared<LevelModel>();
		return (result->init("") ? result : nullptr);
	}

	/**
	 * Creates a new game level with the given source file.
	 *
	 * This method does NOT load the level. You must call the load() method to do that.
	 * This method returns false if file does not exist.
	 *
	 * @return  an autoreleased level file
	 */
	static std::shared_ptr<LevelModel> alloc(std::string file) {
		std::shared_ptr<LevelModel> result = std::make_shared<LevelModel>();
		return (result->init(file) ? result : nullptr);
	}

#pragma mark Level Attributes
	/**
	 * Returns the max breaches
	 *
	 * @return the max breaches
	 */
	const uint8_t getMaxBreaches() const { return maxBreaches; }

	/**
	 * Returns the max doors
	 *
	 * @return the max doors
	 */
	const uint8_t getMaxDoors() const { return maxDoors; }

	/**
	 * Returns the max buttons
	 *
	 * @return the max buttons
	 */
	const uint8_t getMaxButtons() const { return maxButtons; }

	/**
	 * Returns the ship size given a number of players
	 *
	 * @param players the number of players
	 * @return the ship size
	 */
	const float getShipSize(uint8_t players) const {
		return baseShipSize + (float)players * perPlayer;
	}

	/**
	 * Returns the starting time
	 *
	 * @return the starting time
	 */
	const float getTime() const { return time; }

	/**
	 * Returns the init health
	 *
	 * @return the init health
	 */
	const float getInitHealth() const { return initHealth; }

	/**
	 * Returns the list of events in the level
	 *
	 * @return the list of events in the level
	 */
	vector<std::shared_ptr<EventModel>> getEvents() const { return events; }

	/**
	 * Returns the list of building blocks for this level
	 *
	 * @return list of building blocks for this level
	 */
	map<std::string, std::shared_ptr<BuildingBlockModel>> getBlocks() const { return blocks; }

#pragma mark -
#pragma mark Asset Loading
	/**
	 * Loads this game level from the source file
	 *
	 * This load method should NEVER access the AssetManager.  Assets are loaded in
	 * parallel, not in sequence.  If an asset (like a game level) has references to
	 * other assets, then these should be connected later, during scene initialization.
	 *
	 * @return true if successfully loaded the asset from a file
	 */
	bool preload(const std::string& file) override {
		std::shared_ptr<cugl::JsonReader> reader = cugl::JsonReader::allocWithAsset(file);
		return preload(reader->readJson());
	}

	/**
	 * Loads this game level from the source file
	 *
	 * This load method should NEVER access the AssetManager.  Assets are loaded in
	 * parallel, not in sequence.  If an asset (like a game level) has references to
	 * other assets, then these should be connected later, during scene initialization.
	 *
	 * @return true if successfully loaded the asset from a file
	 */
	bool preload(const std::shared_ptr<cugl::JsonValue>& json) override {
		if (json == nullptr) {
			CUAssertLog(false, "Failed to load level file");
			return false;
		}
		maxBreaches = (uint8_t)json->get(MAX_BREACH_FIELD)->asInt();
		maxDoors = (uint8_t)json->get(MAX_DOOR_FIELD)->asInt();
		maxButtons = (uint8_t)json->get(MAX_BUTTON_FIELD)->asInt();
		baseShipSize = json->get(BASE_SHIP_FIELD)->asFloat();
		perPlayer = json->get(PER_PLAYER_FIELD)->asFloat();
		time = json->get(TIME_FIELD)->asFloat();
		initHealth = json->get(INIT_HEALTH_FIELD)->asFloat();
		std::shared_ptr<cugl::JsonValue> blocksJson = json->get(BLOCKS_FIELD);
		for (int i = 0; i < blocksJson->size(); i++) {
			std::shared_ptr<cugl::JsonValue> block = blocksJson->get(i);
			blocks.insert(pair<string, std::shared_ptr<BuildingBlockModel>>(
				block->get(NAME_FIELD)->asString(), BuildingBlockModel::alloc(blocksJson->get(i))));
		}
		std::shared_ptr<cugl::JsonValue> eventsJson = json->get(EVENTS_FIELD);
		for (int i = 0; i < eventsJson->size(); i++) {
			std::shared_ptr<cugl::JsonValue> event = eventsJson->get(i);
			events.push_back(EventModel::alloc(event));
		}
		return true;
	}

	/**
	 * Unloads this game level, releasing all sources
	 *
	 * This load method should NEVER access the AssetManager.  Assets are loaded and
	 * unloaded in parallel, not in sequence.  If an asset (like a game level) has
	 * references to other assets, then these should be disconnected earlier.
	 */
	void unload() {}

	//#pragma mark -
	//#pragma mark Initializers
	/**
	 * Creates a new, empty level.
	 */
	LevelModel(void) {
		maxButtons = DEFAULT_MAX_BUTTONS;
		maxBreaches = DEFAULT_MAX_BREACHES;
		maxDoors = DEFAULT_MAX_DOORS;
		baseShipSize = DEFAULT_BASE_SIZE;
		perPlayer = DEFAULT_PER_PLAYER;
		time = DEFAULT_TIME;
		initHealth = DEFAULT_INIT_HEALTH;
	};

	LevelModel(const LevelModel&) = delete;

	/**
	 * Destroys this level, releasing all resources.
	 */
	virtual ~LevelModel(void){};
};

#endif /* defined(__JS_LEVEL_MODEL_H__) */

#ifndef __JS_BUILDING_BLOCK_MODEL_H__
#define __JS_BUILDING_BLOCK_MODEL_H__
#include <cugl/assets/CUAsset.h>
#include <cugl/cugl.h>
#include <cugl/io/CUJsonReader.h>

#include <vector>

#include "Globals.h"
#include "LevelConstants.h"

using namespace cugl;

#pragma mark -
#pragma mark Building Block Model

/**
 * Class that represents a dynamically loaded building block in the game
 *
 */
class BuildingBlockModel {
   public:
	enum PlayerDistType { MinDist = 0, SpecificPlayer = 1, Random = 2 };
	enum ObjectType { Breach = 0, Door = 1, Button = 2, Roll = 3 };
	struct Object {
		int type, angle, player;
	};

   private:
	/**An ordered list of objects used in this building block*/
	vector<Object> objects;

	/**The type of player distance this building block uses*/
	PlayerDistType type;

	/**The (relative) player this block is placed relative to */
	int player = -1;

	/**The distance used for placing this building block*/
	int distance = -1;

	/** The total amount of space used by this building block */
	int range = 0;

	/** The minimum relative angle used */
	int min = 0;

	/** The number of breaches needed for this block*/
	int breachesNeeded = 0;

	/** The number of doors needed for this block*/
	int doorsNeeded = 0;

   public:
#pragma mark Static Constructors

	/**
	 * Creates a new building block with the given JSON file.
	 *
	 * @return a new building block
	 */
	static std::shared_ptr<BuildingBlockModel> alloc(const std::shared_ptr<cugl::JsonValue>& json) {
		std::shared_ptr<BuildingBlockModel> result = std::make_shared<BuildingBlockModel>();
		return (result->init(json) ? result : nullptr);
	}

#pragma mark Building Block Attributes

	/**
	 * Returns the objects
	 *
	 * @return vector of objects
	 */
	vector<Object> getObjects() { return objects; }

	/**
	 * Returns the type of player distance used
	 *
	 * @return type
	 */
	PlayerDistType getType() { return type; }

	/**
	 * Returns the relative player used
	 *
	 * @return player
	 */
	int getPlayer() { return player; }

	/**
	 * Returns the distance at which this should be generated
	 *
	 * @return distance
	 */
	int getDistance() { return distance; }

	/**
	 * Returns the total width taken up by this building block
	 *
	 * @return range of building block
	 */
	int getRange() { return range; }

	/**
	 * Returns the min
	 *
	 * @return min
	 */
	int getMin() { return min; }
	int getBreachesNeeded() { return breachesNeeded; }
	int getDoorsNeeded() { return doorsNeeded; }

#pragma mark -
#pragma mark Initializers
	/**
	 * Creates a new, empty level.
	 */
	BuildingBlockModel(void)
		: type(Random),
		  player(0),
		  distance(-1),
		  range(0),
		  min(0),
		  breachesNeeded(0),
		  doorsNeeded(0){};

	bool init(const std::shared_ptr<cugl::JsonValue>& json) {
		std::shared_ptr<cugl::JsonValue> playerDist = json->get(PLAYER_DIST_FIELD);
		type = (PlayerDistType)playerDist->get(SPAWN_RULE_FIELD)->asInt();
		switch (type) {
			case MinDist:
				distance = playerDist->get(DISTANCE_FIELD)->asInt();
				break;
			case SpecificPlayer:
				distance = playerDist->get(DISTANCE_FIELD)->asInt();
				player = playerDist->get(PLAYER_ID_FIELD)->asInt();
				break;
			case Random:
				break;
		}
		std::shared_ptr<cugl::JsonValue> objectJson = json->get(OBJECTS_FIELD);
		int numObjects = (int)objectJson->size();
		int maxAngle = 0;
		int minAngle = 0;
		int leftWidth = 0;
		int rightWidth = 0;
		for (int i = 0; i < numObjects; i++) {
			std::shared_ptr<cugl::JsonValue> object = objectJson->get(i);
			Object obj = {object->get(OBJECT_TYPE_FIELD)->asInt(),
						  object->get(OBJECT_ANGLE_FIELD)->asInt(),
						  object->get(OBJECT_PLAYER_FIELD)->asInt()};
			objects.push_back(obj);
			if (obj.type == Roll) continue;
			if (obj.angle >= maxAngle) {
				maxAngle = obj.angle;
				switch (obj.type) {
					case Breach:
						rightWidth = globals::BREACH_WIDTH;
						break;
					case Door:
						rightWidth = globals::DOOR_WIDTH;
						break;
					case Button:
						// TODO
						break;
				};
			}
			if (obj.angle <= minAngle) {
				minAngle = obj.angle;
				switch (obj.type) {
					case Breach:
						leftWidth = globals::BREACH_WIDTH;
						break;
					case Door:
						leftWidth = globals::DOOR_WIDTH;
						break;
					case Button:
						// TODO
						break;
				};
			}
			min = minAngle - leftWidth;
			range = maxAngle + rightWidth - min;
		}
		breachesNeeded = (int)count_if(
			objects.begin(), objects.end(),
			[](BuildingBlockModel::Object o) { return o.type == BuildingBlockModel::Breach; });
		doorsNeeded = (int)count_if(
			objects.begin(), objects.end(),
			[](BuildingBlockModel::Object o) { return o.type == BuildingBlockModel::Door; });
		return true;
	}

	/**
	 * Destroys this level, releasing all resources.
	 */
	virtual ~BuildingBlockModel(void){};
};

#endif /* defined(__JS_LEVEL_MODEL_H__) */

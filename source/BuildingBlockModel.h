#ifndef __JS_BUILDING_BLOCK_MODEL_H__
#define __JS_BUILDING_BLOCK_MODEL_H__
#include <cugl/assets/CUAsset.h>
#include <cugl/cugl.h>
#include <cugl/io/CUJsonReader.h>

#include <vector>

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
#pragma mark -
#pragma mark Initializers
	/**
	 * Creates a new, empty level.
	 */
	BuildingBlockModel(void){};

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
		int numObjects = objectJson->size();
		for (int i = 0; i < numObjects; i++) {
			std::shared_ptr<cugl::JsonValue> object = objectJson->get(i);
			Object obj = {object->get(OBJECT_TYPE_FIELD)->asInt(),
						  object->get(OBJECT_ANGLE_FIELD)->asInt(),
						  object->get(OBJECT_PLAYER_FIELD)->asInt()};
			objects.push_back(obj);
		}
		return true;
	}

	/**
	 * Destroys this level, releasing all resources.
	 */
	virtual ~BuildingBlockModel(void){};
};

#endif /* defined(__JS_LEVEL_MODEL_H__) */

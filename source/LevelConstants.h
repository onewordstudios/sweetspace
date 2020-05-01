#ifndef __JS_LEVEL_CONSTANTS_H__
#define __JS_LEVEL_CONSTANTS_H__

#include <initializer_list>

constexpr auto MAX_BREACH_FIELD = "maxBreaches";   // NOLINT
constexpr auto MAX_DOOR_FIELD = "maxDoors";		   // NOLINT
constexpr auto MAX_BUTTON_FIELD = "maxButtons";	   // NOLINT
constexpr auto BASE_SHIP_FIELD = "baseShipSize";   // NOLINT
constexpr auto PER_PLAYER_FIELD = "perPlayerSize"; // NOLINT
constexpr auto TIME_FIELD = "time";				   // NOLINT
constexpr auto INIT_HEALTH_FIELD = "initHealth";   // NOLINT
constexpr auto BLOCKS_FIELD = "blocks";			   // NOLINT
constexpr auto NAME_FIELD = "name";				   // NOLINT
constexpr auto PLAYER_DIST_FIELD = "playerDist";   // NOLINT
constexpr auto SPAWN_RULE_FIELD = "spawnRule";	   // NOLINT
constexpr auto PLAYER_ID_FIELD = "player";		   // NOLINT
constexpr auto DISTANCE_FIELD = "distance";		   // NOLINT
constexpr auto OBJECTS_FIELD = "objects";		   // NOLINT
constexpr auto OBJECT_TYPE_FIELD = "type";		   // NOLINT
constexpr auto OBJECT_ANGLE_FIELD = "angle";	   // NOLINT
constexpr auto OBJECT_PLAYER_FIELD = "player";	   // NOLINT
constexpr auto EVENTS_FIELD = "events";			   // NOLINT
constexpr auto BLOCK_FIELD = "blockName";		   // NOLINT
constexpr auto TIME_START_FIELD = "timeStart";	   // NOLINT
constexpr auto TIME_STOP_FIELD = "timeStop";	   // NOLINT
constexpr auto PROBABILITY_FIELD = "probability";  // NOLINT

/** List of all level names, used as both keys and values */
constexpr auto LEVEL_NAMES = { // NOLINT
	"json/tutorial1.owslevel", "json/tutorial2.owslevel", "json/tutorial3.owslevel",
	"json/tutorial4.owslevel", "json/level1.owslevel",	  "json/level2.owslevel",
	"json/level3.owslevel"};

/** Easy level index */
constexpr unsigned int EASY_LEVEL = 0; // NOLINT
/** Medium level index */
constexpr unsigned int MED_LEVEL = 4; // NOLINT
/** Hard level index */
constexpr unsigned int HARD_LEVEL = 6; // NOLINT

#endif /* defined(__JS_LEVEL_CONSTANTS_H__) */

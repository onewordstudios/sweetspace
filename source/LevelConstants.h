#ifndef __JS_LEVEL_CONSTANTS_H__
#define __JS_LEVEL_CONSTANTS_H__

#include <array>

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

/** Total number of levels in the game */
constexpr unsigned int MAX_NUM_LEVELS = 16;

/** Number of buttons in the level select */
static constexpr unsigned int NUM_LEVEL_BTNS = 5;

/** List of all level names, used as both keys and values NOLINTNEXTLINE */
constexpr std::array<const char*, MAX_NUM_LEVELS> LEVEL_NAMES = {"",
																 "json/tutorial2.owslevel",
																 "",
																 "json/tutorial4.owslevel",
																 "json/level1.owslevel",
																 "json/level2.owslevel",
																 "",
																 "json/tutorial6.owslevel",
																 "json/tutorial7.owslevel",
																 "json/level3.owslevel",
																 "",
																 "json/level4.owslevel",
																 "json/level5.owslevel",
																 "json/level6.owslevel",
																 "json/level7.owslevel",
																 "json/level8.owslevel"};

/** List of where the buttons on the level select map */
constexpr std::array<unsigned int, NUM_LEVEL_BTNS> LEVEL_ENTRY_POINTS = {0, 6, 9, 10, 11}; // NOLINT

/** Easy level index */
constexpr unsigned int EASY_LEVEL = 0; // NOLINT
/** Medium level index */
constexpr unsigned int MED_LEVEL = 8; // NOLINT
/** Hard level index */
constexpr unsigned int HARD_LEVEL = 9; // NOLINT

#endif /* defined(__JS_LEVEL_CONSTANTS_H__) */

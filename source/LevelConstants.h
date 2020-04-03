#ifndef __JS_LEVEL_CONSTANTS_H__
#define __JS_LEVEL_CONSTANTS_H__

constexpr auto MAX_BREACH_FIELD = "maxBreaches";  // NOLINT
constexpr auto MAX_DOOR_FIELD = "maxDoors";		  // NOLINT
constexpr auto SPAWN_RATE_FIELD = "spawnRate";	  // NOLINT
constexpr auto MIN_ANGLE_DIFF_FIELD = "minAngle"; // NOLINT
constexpr auto SHIP_SIZE_FIELD = "shipSize";	  // NOLINT
constexpr auto TIME_FIELD = "time";				  // NOLINT
constexpr auto INIT_HEALTH_FIELD = "initHealth";  // NOLINT

/** The source for our level file */
constexpr auto LEVEL_ONE_FILE = "json/level.json"; // NOLINT
/** The key for our loaded level */
constexpr auto LEVEL_ONE_KEY = "level1"; // NOLINT

#endif /* defined(__JS_LEVEL_CONSTANTS_H__) */

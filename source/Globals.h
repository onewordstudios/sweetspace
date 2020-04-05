#ifndef __GLOBALS_H__
#define __GLOBALS_H__

/**
 * A namespace for storing constants that are useful across the program.
 *
 * Do not stick all possible constants you'll ever use in here. Only place constants here if they
 * will be useful in multiple classes. The purpose of this namespace is to avoid duplicating
 * constants that are used in multiple places.
 */
namespace globals {

/** Number of players needed for the game - will be replaced with a range in the future */
constexpr unsigned int NUM_PLAYERS = 3; // NOLINT

/** Network tick frequency */
constexpr unsigned int NETWORK_TICK = 12; // NOLINT

/** Number of characters in a ship ID */
constexpr unsigned int ROOM_LENGTH = 5; // NOLINT

/** Minimum of ship health for win condition */
constexpr int SHIP_HEALTH_WIN_LIMIT = 10; // NOLINT

/** Minimum number of players per game */
constexpr unsigned int MIN_PLAYERS = 3; // NOLINT

/** Maximum number of players per game */
constexpr unsigned int MAX_PLAYERS = 6; // NOLINT

#pragma region Math
#pragma mark Math

/** Pi */
constexpr float PI = 3.14159265358979323846264338327950288f; // NOLINT

/** Two Pi */
constexpr float TWO_PI = 2 * PI; // NOLINT

/** Pi divided by 180 for converting between degrees and radians */
constexpr float PI_180 = PI / 180.0f; // NOLINT

#pragma mark -
#pragma endregion
#pragma region Scene Graph
#pragma mark Scenegraph
/** Width of the screen, used in windowed mode */
constexpr unsigned int SCENE_WIDTH = 1024; // NOLINT

/** Radius of the ship for the scene graph */
constexpr unsigned int RADIUS = 550; // NOLINT

/** Maximum number of possibly visible ship segments at a time */
constexpr unsigned int VISIBLE_SEGS = 6; // NOLINT

/** The angle in degree of a single ship segment */
constexpr float SEG_SIZE = 45 * PI_180; // NOLINT

/** The screen angle at which a ship segment is no longer visible */
constexpr float SEG_CUTOFF_ANGLE = 90 * PI_180; // NOLINT

#pragma endregion
} // namespace globals
#endif

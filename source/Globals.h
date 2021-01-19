#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <cmath>
#include <cstddef>
#include <cstdint>

/**
 * A namespace for storing constants that are useful across the program.
 *
 * Do not stick all possible constants you'll ever use in here. Only place constants here if they
 * will be useful in multiple classes. The purpose of this namespace is to avoid duplicating
 * constants that are used in multiple places.
 */
namespace globals {

#pragma warning(push)
#pragma warning(disable : 4244)

/** Network tick frequency */
constexpr unsigned int NETWORK_TICK = 12; // NOLINT

/** API version number. Bump this everytime a backwards incompatible API change happens. */
constexpr uint8_t API_VER = 0; // NOLINT

/** ID marker for unops in sound effects*/
constexpr int UNOP_MARKER = 12; // NOLINT

/** Number of tutorial levels */
constexpr uint8_t NUM_TUTORIAL_LEVELS = 11; // NOLINT

/** Number of characters in a ship ID */
constexpr size_t ROOM_LENGTH = 5; // NOLINT

/** Minimum number of players per game */
constexpr uint8_t MIN_PLAYERS = 2; // NOLINT

/** Maximum number of players per game */
constexpr uint8_t MAX_PLAYERS = 6; // NOLINT

/** The Angle in degrees for which a collision occurs*/
constexpr float DOOR_WIDTH = 10.0f; // NOLINT

/** The Angle in degrees for which a breach donut collision occurs*/
constexpr float BREACH_WIDTH = 6.0f; // NOLINT

/** The Angle in degrees for which a door can be activated*/
constexpr float BUTTON_ACTIVE_ANGLE = 15.0f; // NOLINT

/** The Angle in degrees for anything can be near a button*/
constexpr float BUTTON_WIDTH = 30.0f; // NOLINT

/** The min angle in degrees for which two buttons can be placed*/
constexpr int BUTTON_DIST = 60; // NOLINT

/** The duration in seconds of the everyone roll challenge*/
constexpr int ROLL_CHALLENGE_LENGTH = 6; // NOLINT

#pragma region Math
#pragma mark Math

/** Size of generic ship, in degrees */
constexpr float DEG_ORIG_CIRCLE = 360.0f; // NOLINT

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

/** Radius ratio for calculating spin speed */
constexpr float SPIN_RATIO = 3.0f; // NOLINT

/** Maximum number of possibly visible ship segments at a time */
constexpr unsigned int VISIBLE_SEGS = 6; // NOLINT

/** The angle in degrees of a single ship segment */
constexpr unsigned int SEG_DEG = 45; // NOLINT

/** The angle in radians of a single ship segment */
constexpr float SEG_SIZE = 45 * PI_180; // NOLINT

/** The screen angle at which a ship segment is no longer visible */
constexpr float SEG_CUTOFF_ANGLE = 90 * PI_180; // NOLINT

/** How much the player count needle is offset by */
constexpr float NEEDLE_OFFSET = 0.9f; // NOLINT

/** Music Fade-in time in seconds*/
constexpr float MUSIC_FADE_IN = 0.2f; // NOLINT

/** Music Fade-out time in seconds*/
constexpr float MUSIC_FADE_OUT = 0.2f; // NOLINT

/** Max color channel value */
constexpr int MAX_BYTE = 255; // NOLINT
#pragma endregion

/** Positive remainder of x/y */
inline float remainderPos(float x, float y) { return fmod(fmod(x, y) + y, y); }

} // namespace globals

#pragma warning(pop)
#endif

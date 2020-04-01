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

/** Width of the screen, used in windowed mode */
constexpr unsigned int SCENE_WIDTH = 1024; // NOLINT

/** Number of players needed for the game - will be replaced with a range in the future */
constexpr unsigned int NUM_PLAYERS = 3; // NOLINT

/** Network tick frequency */
constexpr unsigned int NETWORK_TICK = 12; // NOLINT

/** Health of the ship to start */
constexpr int INITIAL_SHIP_HEALTH = 11; // NOLINT

/** Radius of the ship for the scene graph */
constexpr unsigned int RADIUS = 550; // NOLINT

} // namespace globals

#endif /* __GLOBALS_H__ */
#ifndef __TUTORIAL_CONSTANTS_H__
#define __TUTORIAL_CONSTANTS_H__

#include <array>

#include "Globals.h"

namespace tutorial {
constexpr uint8_t NUM_REAL_LEVELS = 7;
constexpr std::array<uint8_t, NUM_REAL_LEVELS> REAL_LEVELS =			 // NOLINT
	{1, 3, 4, 5, 7, 8, 9};												 // NOLINT
constexpr std::array<uint8_t, globals::NUM_TUTORIAL_LEVELS> MAX_BREACH = // NOLINT
	{10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};						 // NOLINT
constexpr std::array<uint8_t, globals::NUM_TUTORIAL_LEVELS> MAX_DOOR =	 // NOLINT
	{0, 0, 2, 10, 0, 0, 10, 0, 0, 0, 0};								 // NOLINT
constexpr std::array<uint8_t, globals::NUM_TUTORIAL_LEVELS> MAX_BUTTON = // NOLINT
	{10, 10, 10, 10, 10, 10, 15, 10, 10, 10, 10};
constexpr std::array<float, globals::NUM_TUTORIAL_LEVELS> HEALTH = { // NOLINT
	1000, 10, 1000, 10, 10, 10, 1000, 10, 10, 10, 1000};			 // NOLINT
constexpr std::array<float, globals::NUM_TUTORIAL_LEVELS> SIZE_PER = // NOLINT
	{180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180};		 // NOLINT
constexpr std::array<bool, globals::NUM_TUTORIAL_LEVELS> SECTIONED = // NOLINT
	{1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0};
constexpr std::array<unsigned int, globals::NUM_TUTORIAL_LEVELS> CUSTOM_EVENTS = // NOLINT
	{2, 1, 1, 1, 1, 1, 1, 10, 10, 10, 10};										 // NOLINT

constexpr float BUTTON_PADDING = 30.0f;	   // NOLINT
constexpr float FAKE_DOOR_PADDING = 25.0f; // NOLINT
constexpr float BREACH_DIST = 45.0f;	   // NOLINT
constexpr int BREACH_LEVEL = 0;			   // NOLINT
constexpr int DOOR_LEVEL = 2;			   // NOLINT
constexpr int BUTTON_LEVEL = 6;			   // NOLINT
constexpr int STABILIZER_LEVEL = 10;	   // NOLINT
constexpr int B_L_PART1 = 5;			   // NOLINT
constexpr int B_L_PART2 = 10;			   // NOLINT
constexpr int B_L_LOC1 = -25;			   // NOLINT
constexpr int B_L_LOC2 = -50;			   // NOLINT
constexpr int B_L_LOC3 = 25;			   // NOLINT
constexpr int B_L_LOC4 = 50;			   // NOLINT

/**
 * Return true iff lvl is a tutorial level
 */
constexpr bool IS_TUTORIAL_LEVEL(uint8_t lvl) {
	if (lvl >= globals::NUM_TUTORIAL_LEVELS) {
		return false;
	}

	return std::find(tutorial::REAL_LEVELS.begin(), tutorial::REAL_LEVELS.end(), lvl) ==
		   std::end(tutorial::REAL_LEVELS);
}

} // namespace tutorial

#endif /* defined(__JS_TUTORIAL_CONSTANTS_H__) */

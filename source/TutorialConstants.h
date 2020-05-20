#ifndef __TUTORIAL_CONSTANTS_H__
#define __TUTORIAL_CONSTANTS_H__

#include <array>

#include "Globals.h"

namespace tutorial {
constexpr int NUM_REAL_LEVELS = 7;
constexpr std::array<int, NUM_REAL_LEVELS> REAL_LEVELS = {1, 3, 4, 5, 7, 8, 9};		  // NOLINT
constexpr std::array<int, globals::NUM_TUTORIAL_LEVELS> MAX_BREACH = {10, 10, 10, 10, // NOLINT
																	  10, 10, 10, 10,
																	  10, 10, 10}; // NOLINT
constexpr std::array<int, globals::NUM_TUTORIAL_LEVELS> MAX_DOOR = {0, 0, 2,  10,  // NOLINT
																	0, 0, 10, 0,
																	0, 0, 0};		  // NOLINT
constexpr std::array<int, globals::NUM_TUTORIAL_LEVELS> MAX_BUTTON = {10, 10, 10, 10, // NOLINT
																	  10, 10, 15, 10,
																	  10, 10, 10};		// NOLINT
constexpr std::array<int, globals::NUM_TUTORIAL_LEVELS> HEALTH = {						// NOLINT
	1000, 10, 1000, 10, 10, 10, 1000, 10, 10, 10, 1000};								// NOLINT
constexpr std::array<int, globals::NUM_TUTORIAL_LEVELS> SIZE_PER = {180, 180, 180, 180, // NOLINT
																	180, 180, 180, 180,
																	180, 180, 180};	   // NOLINT
constexpr std::array<int, globals::NUM_TUTORIAL_LEVELS> SECTIONED = {1, 0, 0, 0,	   // NOLINT
																	 1, 0, 0, 0};	   // NOLINT
constexpr std::array<int, globals::NUM_TUTORIAL_LEVELS> CUSTOM_EVENTS = {2,	 1,	 1, 1, // NOLINT
																		 1,	 1,	 1, 10,
																		 10, 10, 10}; // NOLINT
constexpr float BUTTON_PADDING = 30.0f;												  // NOLINT
constexpr float FAKE_DOOR_PADDING = 25.0f;											  // NOLINT
constexpr float BREACH_DIST = 45.0f;												  // NOLINT
constexpr int BREACH_LEVEL = 0;														  // NOLINT
constexpr int DOOR_LEVEL = 2;														  // NOLINT
constexpr int BUTTON_LEVEL = 6;														  // NOLINT
constexpr int STABILIZER_LEVEL = 10;												  // NOLINT
constexpr int B_L_PART1 = 10;														  // NOLINT
constexpr int B_L_PART2 = 15;														  // NOLINT
constexpr int B_L_LOC1 = -25;														  // NOLINT
constexpr int B_L_LOC2 = -50;														  // NOLINT
constexpr int B_L_LOC3 = 25;														  // NOLINT
constexpr int B_L_LOC4 = 50;														  // NOLINT

} // namespace tutorial

#endif /* defined(__JS_TUTORIAL_CONSTANTS_H__) */

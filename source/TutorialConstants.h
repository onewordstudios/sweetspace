#ifndef __TUTORIAL_CONSTANTS_H__
#define __TUTORIAL_CONSTANTS_H__

#include <array>
namespace tutorial {
constexpr int REAL_LEVELS[] = {1, 3, 5, 6};							 // NOLINT
constexpr int MAX_BREACH[] = {10, 10, 10, 10, 10, 10, 10, 10};		 // NOLINT
constexpr int MAX_DOOR[] = {0, 0, 2, 10, 0, 0, 10, 0};				 // NOLINT
constexpr int MAX_BUTTON[] = {10, 10, 10, 10, 10, 10, 10, 10};		 // NOLINT
constexpr int HEALTH[] = {1000, 10, 1000, 10, 1000, 10, 10, 1000};	 // NOLINT
constexpr int SIZE_PER[] = {180, 180, 180, 180, 180, 180, 180, 180}; // NOLINT
constexpr int SECTIONED[] = {1, 0, 0, 0, 1, 0, 0, 0};				 // NOLINT
constexpr int CUSTOM_EVENTS[] = {2, 1, 1, 1, 1, 1, 1, 10};			 // NOLINT
constexpr float BUTTON_PADDING = 30.0f;								 // NOLINT
constexpr float FAKE_DOOR_PADDING = 25.0f;							 // NOLINT
constexpr float BREACH_DIST = 45.0f;								 // NOLINT
constexpr int BREACH_LEVEL = 0;										 // NOLINT
constexpr int DOOR_LEVEL = 2;										 // NOLINT
constexpr int BUTTON_LEVEL = 4;										 // NOLINT
constexpr int STABILIZER_LEVEL = 7;									 // NOLINT
constexpr int B_L_PART1 = 10;										 // NOLINT
constexpr int B_L_PART2 = 15;										 // NOLINT

} // namespace tutorial

#endif /* defined(__JS_TUTORIAL_CONSTANTS_H__) */

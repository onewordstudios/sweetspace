#include "DonutNode.h"

#include <cugl/2d/CUNode.h>

#include "Globals.h"

using namespace cugl;

/** The radius of the ship. Also the y coordinate of the center of the ship */
constexpr unsigned int RADIUS_OFFSET = 30;

/** Position to place node offscreen. */
constexpr float OFF_SCREEN_POS = 1500;

void DonutNode::animateJumping() {}

void DonutNode::animateFacialExpression() {}
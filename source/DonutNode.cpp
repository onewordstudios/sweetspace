#include "DonutNode.h"

#include <cugl/2d/CUNode.h>

#include "GameGraphRoot.h"
#include "Globals.h"
#include "Tween.h"

using namespace cugl;

/** The scale by which donut stretches when jumping */
constexpr float JUMP_SCALE = 1.6f;

void DonutNode::animateJumping() {
	setScale(GameGraphRoot::DONUT_SCALE,
			 Tween::linear(GameGraphRoot::DONUT_SCALE, GameGraphRoot::DONUT_SCALE * JUMP_SCALE,
						   (int)(donutModel->getJumpOffset() * 100), 35));
}

void DonutNode::animateFacialExpression() {}
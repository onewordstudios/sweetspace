#include "DonutNode.h"

#include <cugl/2d/CUNode.h>

#include "GameGraphRoot.h"
#include "Globals.h"
#include "Tween.h"

using namespace cugl;

/** The scale by which donut stretches when jumping */
constexpr float JUMP_SCALE = 0.6f;

/** Percentage of jump at which distortion begins */
constexpr float SCALING_BEGIN = 0.1f;

/** Percentage of jump at which distortion stops */
constexpr float SCALING_END = 1.2f;

bool DonutNode::init(const std::shared_ptr<cugl::Texture>& bodyTexture,
					 std::shared_ptr<DonutModel> donut) {
	donutModel = donut;

	rotationNode = cugl::Node::alloc();
	bodyNode = cugl::PolygonNode::allocWithTexture(bodyTexture);
	bodyNode->setAnchor(cugl::Vec2::ANCHOR_CENTER);
	bodyNode->setPosition(0, 0);
	rotationNode->addChild(bodyNode);
	addChild(rotationNode);

	setScale(DONUT_SCALE);
	return true;
}

void DonutNode::animateJumping() {
	float halfJumpTime =
		sqrt(2 * DonutModel::GRAVITY * DonutModel::JUMP_HEIGHT) / DonutModel::GRAVITY;
	float scalingWindowSize = halfJumpTime * (SCALING_END - SCALING_BEGIN);
	bool isInScalingWindow = donutModel->getJumpTime() > halfJumpTime * SCALING_BEGIN &&
							 donutModel->getJumpTime() < halfJumpTime * SCALING_END;
	if (!donutModel->isJumping()) {
		// Not jumping. Set scale to normal and return
		setScale(DONUT_SCALE, DONUT_SCALE);
		return;
	}
	float xScale;
	if (donutModel->getJumpTime() <= halfJumpTime * SCALING_BEGIN) {
		// First animation stage
		xScale = Tween::linear(DONUT_SCALE, DONUT_SCALE * JUMP_SCALE,
							   (int)((donutModel->getJumpTime()) * 100),
							   (int)(halfJumpTime * SCALING_BEGIN * 100));
	} else if (isInScalingWindow) {
		// Second animation stage
		xScale =
			Tween::linear(DONUT_SCALE * JUMP_SCALE, DONUT_SCALE,
						  (int)((donutModel->getJumpTime() - halfJumpTime * SCALING_BEGIN) * 100),
						  (int)(scalingWindowSize * 100));
	} else {
		// Not in animation stage
		xScale = DONUT_SCALE;
	}
	setScale(xScale, DONUT_SCALE);
}

void DonutNode::animateFacialExpression() {}
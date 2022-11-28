#include "DonutNode.h"

#include <cugl/2d/CUNode.h>

#include <utility>

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

/** Number of frames between blinking */
constexpr int FACE_ANIMATION_BLINK_INTERVAL = 100;

/** Controls speed of facial animation. Inverse relationship to speed */
constexpr int FACE_ANIMATION_SPEED = 3;

/** Percentage conversion */
constexpr float PERCENTAGE_SCALE = 100;

bool DonutNode::init(const std::shared_ptr<cugl::Texture> &bodyTexture,
					 const std::shared_ptr<cugl::Texture> &faceIdleTexture,
					 const std::shared_ptr<cugl::Texture> &faceDizzyTexture,
					 const std::shared_ptr<cugl::Texture> &faceWorkTexture,
					 std::shared_ptr<DonutModel> donut) {
	referencedDonutModel = std::move(donut);

	rotationNode = cugl::Node::alloc();
	bodyNode = cugl::PolygonNode::allocWithTexture(bodyTexture);
	bodyNode->setAnchor(cugl::Vec2::ANCHOR_CENTER);
	bodyNode->setPosition(0, 0);
	rotationNode->addChild(bodyNode);

	faceNodeIdle = cugl::AnimationNode::alloc(faceIdleTexture, ANIMATION_IDLE_H, ANIMATION_IDLE_W,
											  ANIMATION_IDLE_FRAMES);
	faceNodeIdle->setAnchor(cugl::Vec2::ANCHOR_CENTER);
	faceNodeIdle->setPosition(0, 0);
	rotationNode->addChild(faceNodeIdle);

	faceNodeDizzy = cugl::AnimationNode::alloc(faceDizzyTexture, ANIMATION_NOTIDLE_H,
											   ANIMATION_NOTIDLE_W, ANIMATION_NOTIDLE_FRAMES);
	faceNodeDizzy->setAnchor(cugl::Vec2::ANCHOR_CENTER);
	faceNodeDizzy->setPosition(0, 0);
	faceNodeDizzy->setVisible(false);
	rotationNode->addChild(faceNodeDizzy);

	faceNodeWorking = cugl::AnimationNode::alloc(faceWorkTexture, ANIMATION_NOTIDLE_H,
												 ANIMATION_NOTIDLE_W, ANIMATION_NOTIDLE_FRAMES);
	faceNodeWorking->setAnchor(cugl::Vec2::ANCHOR_CENTER);
	faceNodeWorking->setPosition(0, 0);
	faceNodeWorking->setVisible(false);
	rotationNode->addChild(faceNodeWorking);

	addChild(rotationNode);

	setScale(DONUT_SCALE);
	animationCounter = 0;
	lastFaceState = DonutModel::FaceState::Idle;
	return true;
}

void DonutNode::animateJumping() {
	const float halfJumpTime =
		sqrt(2 * DonutModel::GRAVITY * DonutModel::JUMP_HEIGHT) / DonutModel::GRAVITY;
	const float scalingWindowSize = halfJumpTime * (SCALING_END - SCALING_BEGIN);
	const bool isInScalingWindow =
		referencedDonutModel->getJumpTime() > halfJumpTime * SCALING_BEGIN &&
		referencedDonutModel->getJumpTime() < halfJumpTime * SCALING_END;
	if (!referencedDonutModel->isJumping()) {
		// Not jumping. Set scale to normal and return
		setScale(DONUT_SCALE, DONUT_SCALE);
		return;
	}
	float xScale = 0;
	if (referencedDonutModel->getJumpTime() <= halfJumpTime * SCALING_BEGIN) {
		// First animation stage
		xScale = Tween::linear(
			DONUT_SCALE, DONUT_SCALE * JUMP_SCALE,
			static_cast<int>((referencedDonutModel->getJumpTime()) * PERCENTAGE_SCALE),
			static_cast<int>(halfJumpTime * SCALING_BEGIN * PERCENTAGE_SCALE));
	} else if (isInScalingWindow) {
		// Second animation stage
		xScale = Tween::linear(
			DONUT_SCALE * JUMP_SCALE, DONUT_SCALE,
			static_cast<int>((referencedDonutModel->getJumpTime() - halfJumpTime * SCALING_BEGIN) *
							 PERCENTAGE_SCALE),
			static_cast<int>(scalingWindowSize * PERCENTAGE_SCALE));
	} else {
		// Not in animation stage
		xScale = DONUT_SCALE;
	}
	setScale(xScale, DONUT_SCALE);
}

void DonutNode::animateFacialExpression() {
	DonutModel::FaceState faceState = referencedDonutModel->getFaceState();
	std::shared_ptr<cugl::AnimationNode> visibleFaceNode;
	unsigned int nextFrame = 0;
	if (lastFaceState == DonutModel::FaceState::Working && faceState != lastFaceState &&
		faceNodeWorking->getFrame() != 0) {
		faceState = DonutModel::FaceState::Working;
	} else {
		lastFaceState = referencedDonutModel->getFaceState();
	}
	switch (faceState) {
		case DonutModel::FaceState::Idle:
			visibleFaceNode = faceNodeIdle;
			animationCounter += 1;
			if (visibleFaceNode->getFrame() == 0) {
				// No animation
				nextFrame = 0;
				if (animationCounter == FACE_ANIMATION_BLINK_INTERVAL) {
					// Start animation
					nextFrame = 1;
					animationCounter = 0;
				}
			} else {
				// Perform idle face animation
				if (animationCounter == FACE_ANIMATION_SPEED) {
					animationCounter = 0;
				}
				nextFrame = visibleFaceNode->getFrame() + (animationCounter == 0 ? 1 : 0);
			}
			break;
		case DonutModel::FaceState::Dizzy:
			visibleFaceNode = faceNodeDizzy;
			nextFrame = visibleFaceNode->getFrame() + 1;
			break;
		case DonutModel::FaceState::Working:
		case DonutModel::FaceState::Colliding:
			visibleFaceNode = faceNodeWorking;
			nextFrame = visibleFaceNode->getFrame() + 1;
			break;
	}
	faceNodeIdle->setVisible(false);
	faceNodeDizzy->setVisible(false);
	faceNodeWorking->setVisible(false);
	if (visibleFaceNode != nullptr) {
		visibleFaceNode->setVisible(true);
		visibleFaceNode->setFrame(
			nextFrame < visibleFaceNode->getSize() ? static_cast<int>(nextFrame) : 0);
	}
}
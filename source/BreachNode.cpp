#include "BreachNode.h"

#include <cugl/2d/CUPolygonNode.h>

#include "GameGraphRoot.h"
#include "Globals.h"
#include "Tween.h"

using namespace cugl;

/** The scale of the breach textures. */
static constexpr float BREACH_SCALE = 0.5f;

/** Position to place BreachNode offscreen. */
constexpr float OFF_SCREEN_POS = 1500;

/** How many idle animation frames there is */
constexpr int NUM_IDLE_FRAMES = 11;

/** Controls how fast idle animations proceed */
constexpr int NUM_SKIP_FRAMES = 3;

/** Minimum scale of pattern node */
constexpr float PATTERN_SCALE = 0.1f;

/** Vertical position offset for pattern animation */
constexpr int PATTERN_OFFSET = -60;

/** Vertical position offset for pattern animation */
constexpr int SPARKLE_OFFSET_BEGIN = 20;

/** Vertical position offset for pattern animation */
constexpr int SPARKLE_OFFSET_END = 60;

/** Scale of big sparkle effect */
constexpr float SPARKLE_SCALE_BIG = 1.0;

bool BreachNode::init(std::shared_ptr<BreachModel> breach, std::shared_ptr<DonutModel> player,
					  float shipSize, std::shared_ptr<cugl::Texture> filmstrip,
					  std::shared_ptr<cugl::Texture> pattern, cugl::Color4 color,
					  std::shared_ptr<SparkleNode> sparkleBig,
					  std::shared_ptr<SparkleNode> sparkleSmall) {
	CustomNode::init(player, shipSize, breach->getAngle(), globals::RADIUS);

	breachModel = breach;
	sparkleNodeBig = sparkleBig;
	sparkleNodeBig->setScale(SPARKLE_SCALE_BIG);
	sparkleNodeSmall = sparkleSmall;
	sparkleNodeSmall->setScale(0.6);
	setScale(BREACH_SCALE);
	prevHealth = breach->getHealth();
	setPosition(Vec2::ZERO);

	// Add shape node
	shapeNode = AnimationNode::alloc(filmstrip, BREACH_H, BREACH_W, BREACH_SIZE);
	shapeNode->setColor(color);
	shapeNode->setAnchor(Vec2::ANCHOR_CENTER);
	shapeNode->setPosition(0, 0);
	addChildWithName(shapeNode, "shape");

	// Add pattern node
	patternNode = AnimationNode::alloc(pattern, BREACH_H, BREACH_W, BREACH_SIZE);
	patternNode->setAnchor(Vec2::ANCHOR_CENTER);
	patternNode->setPosition(0, 0);
	addChildWithName(patternNode, "pattern");

	resetAnimation();

	return true;
}

bool BreachNode::isActive() { return breachModel->getIsActive(); }

void BreachNode::prePosition() { angle = breachModel->getAngle(); }

void BreachNode::postPosition() {
	if (prevHealth > breachModel->getHealth()) {
		// Start breach shrinking animation
		isAnimatingShrink = true;
		currentFrameIdle = 0;

		float yOffset = Tween::linear(SPARKLE_OFFSET_BEGIN, SPARKLE_OFFSET_END,
									  (int)shapeNode->getFrame(), shapeNode->getSize());
		sparkleNodeSmall->setRadius(radius + yOffset);
		sparkleNodeSmall->setAngle(getAngle());
		sparkleNodeSmall->setOnShipAngle(angle);
		sparkleNodeSmall->beginAnimation();
	}
	if (isAnimatingShrink) {
		// Update animation frame to shrink
		if (shapeNode->getFrame() == getFrameFromHealth(breachModel->getHealth()) - 1 ||
			shapeNode->getFrame() == shapeNode->getSize() - 1) {
			// End shrink animation
			isAnimatingShrink = false;
			if (shapeNode->getFrame() == shapeNode->getSize() - 1) {
				setPosition(OFF_SCREEN_POS, OFF_SCREEN_POS);
				isShown = false;
			}
		} else {
			// Continue shrink animation
			shapeNode->setFrame((int)shapeNode->getFrame() + 1);
			patternNode->setFrame((int)(shapeNode->getFrame()));
		}
	} else {
		// Play idle animation
		int magicNum =
			(currentFrameIdle < NUM_IDLE_FRAMES * NUM_SKIP_FRAMES
				 ? currentFrameIdle / NUM_SKIP_FRAMES
				 : (NUM_IDLE_FRAMES * NUM_SKIP_FRAMES * 2 - currentFrameIdle) / NUM_SKIP_FRAMES);
		shapeNode->setFrame((int)getFrameFromHealth(breachModel->getHealth()) + magicNum);
		patternNode->setFrame((int)shapeNode->getFrame());
		currentFrameIdle = currentFrameIdle == NUM_IDLE_FRAMES * 2 * NUM_SKIP_FRAMES - 1
							   ? 0
							   : currentFrameIdle + 1;
	}
	prevHealth = breachModel->getHealth();
	patternNode->setScale(
		(PATTERN_SCALE + (-PATTERN_SCALE + 1) *
							 (float)(shapeNode->getSize() - shapeNode->getFrame()) /
							 (float)shapeNode->getSize()));
}

void BreachNode::becomeInactive() {
	sparkleNodeBig->setRadius(radius + SPARKLE_OFFSET_BEGIN);
	sparkleNodeBig->setAngle(getAngle());
	sparkleNodeBig->setOnShipAngle(angle);
	sparkleNodeBig->beginAnimation();
}

void BreachNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
					  Color4 tint) {
	patternNode->setPositionY(
		Tween::linear(0, PATTERN_OFFSET, (int)shapeNode->getFrame(), shapeNode->getSize()));
	CustomNode::draw(batch, transform, tint);
}

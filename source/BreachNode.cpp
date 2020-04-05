#include "BreachNode.h"

#include <cugl/2d/CUPolygonNode.h>

#include "GameGraphRoot.h"
#include "Globals.h"

using namespace cugl;

/** Position to place BreachNode offscreen. */
constexpr float OFF_SCREEN_POS = 1500;

/** How many idle animation frames there is */
constexpr int NUM_IDLE_FRAMES = 22;

/** Controls how fast idle animations proceed */
constexpr int NUM_SKIP_FRAMES = 2;

void BreachNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
					  Color4 tint) {
	Vec2 breachPos;
	if (breachModel->getHealth() > 0 || isAnimatingShrink) {
		// Breach is currently active
		if (!isAnimatingShrink) {
			float onScreenAngle = breachModel->getAngle() - playerDonutModel->getAngle();
			onScreenAngle = onScreenAngle >= 0 ? onScreenAngle : shipSize + onScreenAngle;
			onScreenAngle = onScreenAngle > shipSize / 2 ? onScreenAngle - shipSize : onScreenAngle;
			onScreenAngle *= globals::PI_180;
			if (!isShown && onScreenAngle < globals::SEG_CUTOFF_ANGLE &&
				onScreenAngle > -globals::SEG_CUTOFF_ANGLE) {
				// Breach is coming into visible range
				float relativeAngle = onScreenAngle - getParent()->getParent()->getAngle();
				breachPos = Vec2(globals::RADIUS * sin(relativeAngle),
								 -(float)globals::RADIUS * cos(relativeAngle));
				setPosition(breachPos);
				setAngle(relativeAngle);
				isShown = true;
			} else if (isShown && (onScreenAngle >= globals::SEG_CUTOFF_ANGLE ||
								   onScreenAngle <= -globals::SEG_CUTOFF_ANGLE)) {
				// Breach is leaving visible range
				breachPos = Vec2(OFF_SCREEN_POS, OFF_SCREEN_POS);
				setPosition(breachPos);
				isShown = false;
			}
		}
		if (prevHealth > breachModel->getHealth()) {
			// Start breach shrinking animation
			isAnimatingShrink = true;
			currentFrameIdle = 0;
		}
		if (isAnimatingShrink) {
			// Update animation frame to shrink
			int framesPerHP = getSize() / BreachModel::HEALTH_DEFAULT;
			if (getFrame() == getFrameFromHealth(breachModel->getHealth()) - 1 ||
				getFrame() == getSize() - 1) {
				// End shrink animation
				isAnimatingShrink = false;
				if (getFrame() == getSize() - 1) {
					setPosition(OFF_SCREEN_POS, OFF_SCREEN_POS);
				}
			} else {
				// Continue shrink animation
				setFrame(getFrame() + 1);
			}
		} else {
			// Play idle animation
			setFrame(getFrameFromHealth(breachModel->getHealth()) +
					 (currentFrameIdle < NUM_IDLE_FRAMES / 2 * NUM_SKIP_FRAMES
						  ? currentFrameIdle / NUM_SKIP_FRAMES
						  : NUM_IDLE_FRAMES - currentFrameIdle / 2 - 1));
			currentFrameIdle = currentFrameIdle == NUM_IDLE_FRAMES * NUM_SKIP_FRAMES - 1
								   ? 0
								   : currentFrameIdle + 1;
		}
		prevHealth = breachModel->getHealth();
	} else {
		// Breach is currently inactive
		breachPos = Vec2(OFF_SCREEN_POS, OFF_SCREEN_POS);
		setPosition(breachPos);
		isShown = false;
	}
	AnimationNode::draw(batch, transform, tint);
}

#ifndef NEEDLE_ANIMATOR_H
#define NEEDLE_ANIMATOR_H

#include <cugl/cugl.h>

#include "Globals.h"
#include "MagicInternetBox.h"

/** A single static method to animate the needle pointing to the number of players on the dial */
class NeedleAnimator {
   private:
	/** How much the needle moves each frame to match its correct position */
	constexpr static float NEEDLE_SPEED = 0.3f;

	/** When to just snap the needle to its correct position */
	constexpr static float NEEDLE_CUTOFF = 0.01f;

   public:
	/**
	 * Animate the needle. Call this every frame.
	 *
	 * @param needle Scene graph node of the needle on the dial
	 */
	static void updateNeedle(const std::shared_ptr<cugl::Node>& needle) {
		float needlePer = static_cast<float>(MagicInternetBox::getInstance().getNumPlayers() - 1) /
						  static_cast<float>(globals::MAX_PLAYERS);
		float needleTarget = -needlePer * globals::TWO_PI * globals::NEEDLE_OFFSET;
		float currNeedle = needle->getAngle();
		if (needleTarget != needle->getAngle()) {
			float diff = needleTarget - currNeedle;
			float newNeedle = diff * NEEDLE_SPEED + currNeedle;
			if (abs(diff) < NEEDLE_CUTOFF) {
				newNeedle = needleTarget;
			}
			needle->setAngle(newNeedle);
		}
	}
};

#endif
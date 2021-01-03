#include "SparkleNode.h"

#include <cugl/2d/CUAnimationNode.h>

#include <utility>

#include "Globals.h"
#include "Tween.h"

using namespace cugl;

/** The scale of the breach textures. */
static constexpr float SPARKLE_SCALE = 0.5f;

/** How often to increase frame. Inversely proportional to animation speed. */
static constexpr int ANIMATION_SPEED = 2;

bool SparkleNode::init(std::shared_ptr<DonutModel> player, float shipSize,
					   const std::shared_ptr<cugl::Texture>& texture, cugl::Color4 color,
					   SparkleType type) {
	CustomNode::init(std::move(player), shipSize, 0, globals::RADIUS);

	setScale(SPARKLE_SCALE);
	setPosition(Vec2::ZERO);

	// Add filstrip node
	filmstrip = AnimationNode::alloc(texture, FILMSTRIP_H, FILMSTRIP_W,
									 type == Big ? FILMSTRIP_SIZE_BIG : FILMSTRIP_SIZE_SMALL);
	setFilmstripColor(color);
	filmstrip->setAnchor(Vec2::ANCHOR_CENTER);
	filmstrip->setPosition(0, 0);
	addChildWithName(filmstrip, "filmstrip");

	resetAnimation();

	return true;
}

bool SparkleNode::isActive() { return isAnimating; }

void SparkleNode::prePosition() {}

void SparkleNode::postPosition() {
	if (isAnimating) {
		animationCounter += 1;
		if (animationCounter >= filmstrip->getSize() * ANIMATION_SPEED) {
			// End of animation
			resetAnimation();
		} else if (animationCounter % ANIMATION_SPEED == 0) {
			// Advance frame
			filmstrip->setFrame((animationCounter / ANIMATION_SPEED));
		}
	}
}

void SparkleNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
					   Color4 tint) {
	CustomNode::draw(batch, transform, tint);
}

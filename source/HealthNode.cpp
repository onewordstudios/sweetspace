#include "HealthNode.h"

#include <cugl/2d/CUAnimationNode.h>

using namespace cugl;

/** The diameter of the ship. Also the x coordinate of the center of the ship */
constexpr unsigned int DIAMETER = 1280;

/** The radius of the ship. */
constexpr unsigned int RADIUS = 550;

constexpr unsigned int OFFSET = 345;

void HealthNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
        Color4 tint) {
    Vec2 pos = Vec2(DIAMETER + RADIUS * sin(0),
            DIAMETER / 2.0f - (RADIUS - OFFSET)*cos(0));

    setPosition(pos);
    setAngle(0);
    ship == nullptr ? setFrame(11) : setFrame(ship->getHealth());

    AnimationNode::draw(batch, transform, tint);
}

#include "HealthNode.h"

#include <cugl/2d/CUAnimationNode.h>

using namespace cugl;

/** The diameter of the ship. Also the x coordinate of the center of the ship */
constexpr unsigned int DIAMETER = 1280;

/** The radius used for placement of the doors. */
constexpr unsigned int DOOR_RADIUS = 650;

/** The height of the door. */
int height = 0;

void HealthNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
        Color4 tint) {
    Vec2 pos = Vec2(DIAMETER + DOOR_RADIUS,
            DIAMETER / 2.0f - (DOOR_RADIUS));

    setPosition(pos);
    setAngle(0);

    AnimationNode::draw(batch, transform, tint);
}
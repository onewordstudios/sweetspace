#include "BreachNode.h"
#include "GameGraphRoot.h"
#include <cugl/2d/CUPolygonNode.h>

using namespace cugl;

/** The diameter of the ship. Also the x coordinate of the center of the ship */
constexpr unsigned int DIAMETER = 1280;

/** The radius of the ship. Also the y coordinate of the center of the ship */
constexpr unsigned int RADIUS = 550;

/** Pi over 180 for converting between degrees and radians */
constexpr float PI_180 = (float)(M_PI / 180);

void BreachNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
        Color4 tint) {
    if (breachModel->getHealth() > 0) {
        Vec2 breachPos = Vec2(DIAMETER + RADIUS * sin(breachModel->getAngle()),
                DIAMETER / 2.0f - RADIUS * cos(breachModel->getAngle()));
        if (breachModel->getAngle() < 0) {
            breachPos = Vec2(0, 0);
        }
        setScale(GameGraphRoot::BREACH_SCALE * breachModel->getHealth() / 3.0f);
        setPosition(breachPos);
    } else {
        Vec2 breachPos = Vec2(0, 0);
        setPosition(breachPos);
    }
    AnimationNode::draw(batch, transform, tint);
}

#include "TutorialNode.h"

#include <cugl/2d/CUPolygonNode.h>

#include "GameGraphRoot.h"
#include "Globals.h"

using namespace cugl;

/** Position to place TutorialNode offscreen. */
constexpr float OFF_SCREEN_POS = 1500;

void TutorialNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
        Color4 tint) {
    Vec2 breachPos;
    if (breachNode->getIsShown()) {
        // Breach is currently active
        setPosition(breachNode->getPosition());
        CULog("tutorial pos: %f, %f", getPositionX(), getPositionY());
    } else {
        // Breach is currently inactive
        breachPos = Vec2(OFF_SCREEN_POS, OFF_SCREEN_POS);
        setPosition(breachPos);
    }
    Node::draw(batch, transform, tint);
}

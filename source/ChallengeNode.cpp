#include "ChallengeNode.h"

#include <cugl/2d/CUAnimationNode.h>

#include "Globals.h"

using namespace cugl;


void ChallengeNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
        Color4 tint) {

    if(ship->getChallenge()) {
        CULog("challenge");
        switch(display) {
            case hanger:
                setPosition(xPos, yPos);
            case panel:
                setPosition(xPos, yPos);
            case text:
                setPosition(xPos, yPos);
                setAngle(0);
            case arrows:
                setPosition(xPos - 130 + (arrowNum * 30), yPos - 125);
                ship->getRollDir() == -1 ? setAngle(180 * globals::PI_180) : setAngle(0);
        }
    } else {
        setPosition(1500, 1500);
    }

    AnimationNode::draw(batch, transform, tint);
}

#include "TutorialNode.h"

#include <cugl/2d/CUAnimationNode.h>

#include "Globals.h"

using namespace cugl;


void TutorialNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
        Color4 tint) {

    float posX = 0;
    float posY = 0;
    float angle = 0;
    if(breachNode != nullptr) {
        posX = breachNode->getPositionX();
        posY = breachNode->getPositionY() + 150;
        angle = breachNode->getAngle();
    } else if(doorNode != nullptr) {
        posX = doorNode->getPositionX();
        posY = doorNode->getPositionY();
        angle = doorNode->getAngle();
    } else if(buttonNode != nullptr) {
        posX = buttonNode->getPositionX();
        posY = buttonNode->getPositionY() + 100;
        angle = buttonNode->getAngle();
    }

    setPosition(posX, posY);
    setAngle(angle);

    AnimationNode::draw(batch, transform, tint);
}

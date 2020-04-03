#include "ChallengeNode.h"

#include <cugl/2d/CUAnimationNode.h>

#include "Globals.h"

using namespace cugl;


void ChallengeNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
        Color4 tint) {
    setAngle(0);

    AnimationNode::draw(batch, transform, tint);
}

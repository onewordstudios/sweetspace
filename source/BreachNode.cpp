#include "BreachNode.h"
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
    AnimationNode::draw(batch, transform, tint);
}

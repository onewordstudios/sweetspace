#include "DonutNode.h"

#include <cugl/2d/CUPolygonNode.h>

using namespace cugl;

void DonutNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
					 Color4 tint) {
	setPosition(donutModel->getSceneGraphPosition());
	PolygonNode::draw(batch, transform, tint);
}
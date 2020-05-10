#include "UnopenableNode.h"

#include <cugl/2d/CUAnimationNode.h>

#include "Globals.h"

using namespace cugl;

/** The radius used for placement of the doors. */
constexpr float DOOR_RADIUS = 660;

/** The scale of the doors. */
constexpr float DOOR_SCALE = 0.3f;

bool UnopenableNode::init(std::shared_ptr<Unopenable> unop, std::shared_ptr<DonutModel> player,
						  float shipSize, const std::shared_ptr<cugl::Texture>& texture) {
	CustomNode::init(player, shipSize, unop->getAngle(), DOOR_RADIUS);
	unopModel = unop;
	std::shared_ptr<PolygonNode> p = PolygonNode::allocWithTexture(texture);
	addChild(p);
	p->setAnchor(Vec2::ANCHOR_BOTTOM_CENTER);
	p->setPosition(0, 0);
	setScale(DOOR_SCALE);
	return true;
}

void UnopenableNode::prePosition() { angle = unopModel->getAngle(); }

void UnopenableNode::postPosition() {}

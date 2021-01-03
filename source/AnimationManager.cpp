#include "AnimationManager.h"

#include <utility>

#include "InputController.h"
#include "Tween.h"

AnimationManager::AnimationManager() : currentFrame(0) {}

void AnimationManager::reset() {
	for (const auto& n : initialData) {
		n.node->setPositionX(n.posX);
		n.node->setPositionY(n.posY);
		n.node->setColor(cugl::Color4::WHITE);
		n.node->setVisible(n.visible);
	}

	nodes.clear();
	inProgress.clear();
	initialData.clear();
}

void AnimationManager::registerNode(const std::string& name,
									const std::shared_ptr<cugl::AssetManager>& assets) {
	auto node = assets->get<cugl::Node>(name);

	nodes.insert({name, node});

	cugl::Color4 a;
	AnimationData currState = {node, node->isVisible(), node->getPositionX(), node->getPositionY()};
	initialData.push_back(currState);
}

bool AnimationManager::step() {
	if (inProgress.empty()) {
		if (currentFrame != 0) {
			InputController::getInstance()->clear();
		}
		currentFrame = 0;
		return false;
	}

	for (auto i = inProgress.begin(); i != inProgress.end();) {
		auto& anim = *i;

		// Ignore animations that haven't started
		if (currentFrame < anim.startFrame) {
			i++;
			continue;
		}

		// Special case for opacity transitions
		if (anim.property == Visibility) {
			if (currentFrame == anim.startFrame && anim.endVal == 1) {
				anim.node->setVisible(true);
			} else if (currentFrame == anim.endFrame && anim.endVal == 0) {
				anim.node->setVisible(false);
			}
		}

		// Compute the current value of the thing
		float val = 0; // The 0 is unused, but let's make the linter happy
		{
			// Scoping these temporary variables to avoid polluting the function
			float s1 = anim.startVal;
			float e1 = anim.endVal;
			int s2 = static_cast<int>(currentFrame - anim.startFrame);
			int e2 = static_cast<int>(anim.endFrame - anim.startFrame);

			switch (anim.ease) {
				case Tween::TweenType::Linear:
					val = Tween::linear(s1, e1, s2, e2);
					break;
				case Tween::TweenType::EaseIn:
					val = Tween::easeIn(s1, e1, s2, e2);
					break;
				case Tween::TweenType::EaseOut:
					val = Tween::easeOut(s1, e1, s2, e2);
					break;
				case Tween::TweenType::EaseInOut:
					val = Tween::easeInOut(s1, e1, s2, e2);
					break;
			}
		}

		// Set the value
		switch (anim.property) {
			case Visibility:
				anim.node->setColor(Tween::fade(val));
				break;
			case PositionX:
				anim.node->setPositionX(val);
				break;
			case PositionY:
				anim.node->setPositionY(val);
				break;
		}

		// Cleanup if animation is over
		if (currentFrame == anim.endFrame) {
			i = inProgress.erase(i);
		} else {
			i++;
		}
	}

	currentFrame++;
	return true;
}

void AnimationManager::queue(std::shared_ptr<cugl::Node> node, AnimationProperty property,
							 Tween::TweenType ease, float startVal, float endVal,
							 unsigned int duration, unsigned int delay) {
	inProgress.push_back({std::move(node), property, ease, startVal, endVal, delay + currentFrame,
						  duration + delay + currentFrame});
}

void AnimationManager::animateX(const std::string& node, Tween::TweenType ease, float destination,
								unsigned int duration, unsigned int delay) {
	std::shared_ptr<cugl::Node> n = nodes.at(node);
	queue(n, PositionX, ease, n->getPositionX(), destination, duration, delay);
}

void AnimationManager::animateY(const std::string& node, Tween::TweenType ease, float destination,
								unsigned int duration, unsigned int delay) {
	std::shared_ptr<cugl::Node> n = nodes.at(node);
	queue(n, PositionY, ease, n->getPositionY(), destination, duration, delay);
}

void AnimationManager::fadeIn(const std::string& node, unsigned int duration, unsigned int delay) {
	queue(nodes.at(node), Visibility, Tween::TweenType::Linear, 0, 1, duration, delay);
}

void AnimationManager::fadeOut(const std::string& node, unsigned int duration, unsigned int delay) {
	fadeOut(nodes.at(node), duration, delay);
}

void AnimationManager::fadeOut(std::shared_ptr<cugl::Node> node, unsigned int duration,
							   unsigned int delay) {
	queue(std::move(node), Visibility, Tween::TweenType::Linear, 1, 0, duration, delay);
}

#include "StabilizerNode.h"

#include "Globals.h"
#include "Tween.h"

/** Maximum number of health labels */
constexpr size_t MAX_HEALTH_LABELS = 10;

/** Number of frames to animate */
constexpr size_t ANIMATE_TIME = 30;

/** Initial zoom of fail panel when animating in */
constexpr float FAIL_ZOOM = 1.5f;

enum class StabilizerNode::NodeStatus {
	/** Nothing is showing */
	Off,
	/** Challenge panel is animating in */
	Activating,
	/** Challenge panel is animating to fail panel */
	ActiveToFail,
	/** Fail panel is animating in */
	OffToFail,
	/** Challenge panel is animating out */
	ActiveOut,
	/** Fail panel is animating out */
	FailOut
};

StabilizerNode::StabilizerNode(const std::shared_ptr<cugl::AssetManager>& assets,
							   const StabilizerModel& model)
	: model(model), state(NodeStatus::Off), currFrame(0) {
	Node::init();
	setAnchor({1.f / 2, 1.f / 2});
	setPosition(0, 0);
	setVisible(false);

	auto screen = assets->get<Node>("stabilizer");
	stabilizerPanel = assets->get<Node>("stabilizer_stabilizerPanel");
	failPanel = assets->get<Node>("stabilizer_failPanel");

	arrowDim = assets->get<cugl::Texture>("panel_progress_0");
	arrowLit = assets->get<cugl::Texture>("panel_progress_1");

	for (size_t i = 0; i < MAX_HEALTH_LABELS; i++) {
		std::string s = std::to_string(i);
		auto arrow = dynamic_pointer_cast<cugl::PolygonNode>(
			assets->get<Node>("stabilizer_stabilizerPanel_challengePanelArrow" + s));
		arrows.push_back(arrow);
	}

	cugl::Size dimen = cugl::Application::get()->getDisplaySize();
	dimen *= globals::SCENE_WIDTH / dimen.width;
	setContentSize(dimen);
	screen->setContentSize(dimen);

	addChild(screen);
	screen->setPosition(0, 0);

	doLayout();
}

StabilizerNode::~StabilizerNode() { removeAllChildren(); }

void StabilizerNode::update() {
	switch (state) {
		case NodeStatus::Off:
			if (model.getIsActive()) {
				state = NodeStatus::Activating;
				currFrame = 0;
				setVisible(true);
				stabilizerPanel->setVisible(true);
				failPanel->setVisible(false);

				stabilizerPanel->setAnchor({1.f / 2, 0});
				doLayout();

				for (const auto& arrow : arrows) {
					if (model.isLeft()) {
						arrow->setAngle(globals::PI);
					} else {
						arrow->setAngle(0);
					}
					arrow->setTexture(arrowDim);
					arrow->setVisible(true);
				}
			} else if (model.getState() == StabilizerModel::StabilizerState::Fail) {
				state = NodeStatus::OffToFail;
				currFrame = 0;
				setVisible(true);
				stabilizerPanel->setVisible(false);
				failPanel->setVisible(true);
			}
			break;
		case NodeStatus::Activating: {
			if (currFrame <= ANIMATE_TIME) {
				stabilizerPanel->setAnchor(
					{1.f / 2, Tween::easeOut(0, 1, currFrame, ANIMATE_TIME)});
				doLayout();
			}

			for (size_t i = 0; i < MAX_HEALTH_LABELS; i++) {
				float progress = model.getProgress() * MAX_HEALTH_LABELS;
				if (static_cast<float>(i) < progress) {
					arrows[i]->setTexture(arrowLit);
				}
			}

			switch (model.getState()) {
				case StabilizerModel::StabilizerState::Inactive:
					currFrame = 0;
					state = NodeStatus::ActiveOut;
					break;
				case StabilizerModel::StabilizerState::Fail:
					currFrame = 0;
					state = NodeStatus::ActiveToFail;
					break;
				default:
					break;
			}

			break;
		}
		case NodeStatus::ActiveToFail: {
			stabilizerPanel->setAnchor(1.f / 2, Tween::easeIn(1, 0, currFrame, ANIMATE_TIME));
			doLayout();
			if (currFrame == ANIMATE_TIME) {
				state = NodeStatus::OffToFail;
				stabilizerPanel->setVisible(false);
				failPanel->setVisible(true);
				failPanel->setAnchor(1.f / 2, 0);
				doLayout();
			}
			break;
		}
		case NodeStatus::OffToFail: {
			if (currFrame <= ANIMATE_TIME) {
				failPanel->setAnchor({1.f / 2, Tween::easeOut(0, 1, currFrame, ANIMATE_TIME)});
				doLayout();
				failPanel->setScale(Tween::easeOut(FAIL_ZOOM, 1, currFrame, ANIMATE_TIME));
			} else {
				switch (model.getState()) {
					case StabilizerModel::StabilizerState::Inactive:
						currFrame = 0;
						state = NodeStatus::FailOut;
						break;
					default:
						break;
				}
			}
			break;
		}
		case NodeStatus::ActiveOut: {
			stabilizerPanel->setAnchor({1.f / 2, Tween::easeIn(1, 0, currFrame, ANIMATE_TIME)});
			doLayout();
			if (currFrame == ANIMATE_TIME) {
				state = NodeStatus::Off;
				setVisible(false);
			}
			break;
		}
		case NodeStatus::FailOut: {
			failPanel->setAnchor({1.f / 2, Tween::easeIn(1, 0, currFrame, ANIMATE_TIME)});
			doLayout();
			if (currFrame == ANIMATE_TIME) {
				currFrame = 0;
				state = NodeStatus::Off;
				setVisible(false);
			}
			break;
		}
	}
	currFrame++;
}

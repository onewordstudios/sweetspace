#include "StabilizerNode.h"

#include "Globals.h"

/** Maximum number of health labels */
constexpr size_t MAX_HEALTH_LABELS = 10;

StabilizerNode::StabilizerNode(const std::shared_ptr<cugl::AssetManager>& assets,
							   const StabilizerModel& model)
	: model(model), active(false), currFrame(0) {
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
	if (!active) {
		// Inactive
		if (model.getIsActive()) {
			active = true;
			currFrame = 0;
			setVisible(true);
			stabilizerPanel->setVisible(true);
			failPanel->setVisible(false);

			for (const auto& arrow : arrows) {
				if (model.isLeft()) {
					arrow->setAngle(globals::PI);
				} else {
					arrow->setAngle(0);
				}
				arrow->setTexture(arrowDim);
				arrow->setVisible(true);
			}
		}
	} else {
		// Active

		for (size_t i = 0; i < MAX_HEALTH_LABELS; i++) {
			float progress = model.getProgress() * MAX_HEALTH_LABELS;
			if (static_cast<float>(i) < progress) {
				arrows[i]->setTexture(arrowLit);
			}
		}

		currFrame++;
	}
}

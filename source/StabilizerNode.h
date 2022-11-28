#ifndef STABILIZER_NODE_H
#define STABILIZER_NODE_H

#include <cugl/cugl.h>

#include "StabilizerModel.h"

class StabilizerNode : public cugl::Node {
   private:
	/** Underlying stabilizer model */
	const StabilizerModel& model;

	/** Possible states */
	enum class NodeStatus;

	/** Current state */
	NodeStatus state;

	/** Current frame of animation */
	size_t currFrame;

	/** Wrapper node around the stabilizer panel */
	std::shared_ptr<cugl::Node> stabilizerPanel;

	/** Wrapper node around the failure panel */
	std::shared_ptr<cugl::Node> failPanel;

	/** Vector of all the arrows in the stabilizer */
	std::vector<std::shared_ptr<cugl::PolygonNode>> arrows;
	/** Texture of unlit arrow */
	std::shared_ptr<cugl::Texture> arrowDim;
	/** Texture of lit arrow */
	std::shared_ptr<cugl::Texture> arrowLit;

   public:
	StabilizerNode(const std::shared_ptr<cugl::AssetManager>& assets, const StabilizerModel& model);
	virtual ~StabilizerNode();

	StabilizerNode(const StabilizerNode&) = delete;
	void operator=(const StabilizerNode&) = delete;

	void update();
};

#endif // STABILIZER_NODE_H

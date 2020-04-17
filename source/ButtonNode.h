#ifndef SWEETSPACE_BUTTONNODE_H
#define SWEETSPACE_BUTTONNODE_H

#include <cugl/2d/CUAnimationNode.h>

#include "ButtonModel.h"
#include "DonutModel.h"

class ButtonNode : public cugl::AnimationNode {
#pragma mark Values
   protected:
	shared_ptr<ButtonModel> buttonModel;
	/** Reference to the player donut model */
	std::shared_ptr<DonutModel> playerDonutModel;
	/** Size of the ship. Needed for visibility determination */
	float shipSize;
	/** Whether the breach is being shown right now */
	bool isShown;

	std::shared_ptr<cugl::Texture> btnBaseDown;
	std::shared_ptr<cugl::Texture> btnBaseUp;
	std::shared_ptr<cugl::Texture> btnDown;
	std::shared_ptr<cugl::Texture> btnUp;
	std::shared_ptr<cugl::Label> label;

	int buttonType;

   public:
#pragma mark -
#pragma mark Constructor
	/**
	 * Creates an empty polygon with the degenerate texture.
	 *
	 * You must initialize this PolygonNode before use.
	 *
	 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
	 * the heap, use one of the static constructors instead.
	 */
	ButtonNode() : cugl::AnimationNode(), shipSize(0), isShown(false), buttonType(0) {}

	/**
	 * Releases all resources allocated with this node.
	 *
	 * This will release, but not necessarily delete the associated texture.
	 * However, the polygon and drawing commands will be deleted and no
	 * longer safe to use.
	 */
	~ButtonNode() { dispose(); }

	/**
	 * Returns a newly allocated filmstrip node from the given texture.
	 *
	 * This constructor assumes that the filmstrip is rectangular, and that
	 * there are no unused frames.
	 *
	 * The size of the node is equal to the size of a single frame in the
	 * filmstrip. To resize the node, scale it up or down.  Do NOT change the
	 * polygon, as that will interfere with the animation.
	 *
	 * @param texture   The texture image to use
	 * @param rows      The number of rows in the filmstrip
	 * @param cols      The number of columns in the filmstrip
	 *
	 * @return a newly allocated filmstrip node from the given texture.
	 */
	static std::shared_ptr<ButtonNode> alloc(const std::shared_ptr<cugl::Texture> &texture) {
		std::shared_ptr<ButtonNode> node = std::make_shared<ButtonNode>();
		return (node->initWithTexture(texture) ? node : nullptr);
	}

#pragma mark -

	void setModel(std::shared_ptr<ButtonModel> model) { buttonModel = model; }

	void setDonutModel(std::shared_ptr<DonutModel> model) { playerDonutModel = model; }

	void setShipSize(float f) { shipSize = f; }

	std::shared_ptr<ButtonModel> getModel() { return buttonModel; }

	void setButtonBaseDown(std::shared_ptr<cugl::Texture> texture) { btnBaseDown = texture; }

	void setButtonBaseUp(std::shared_ptr<cugl::Texture> texture) { btnBaseUp = texture; }

	void setButtonDown(std::shared_ptr<cugl::Texture> texture) { btnDown = texture; }

	void setButtonUp(std::shared_ptr<cugl::Texture> texture) { btnUp = texture; }

	void setButtonLabel(std::shared_ptr<cugl::Label> l) { label = l; }

	std::shared_ptr<cugl::Texture> getButtonBaseDown() { return btnBaseDown; }
	std::shared_ptr<cugl::Texture> getButtonBaseUp() { return btnBaseUp; }
	std::shared_ptr<cugl::Texture> getButtonDown() { return btnDown; }
	std::shared_ptr<cugl::Texture> getButtonUp() { return btnUp; }

	void setButtonNodeType(int type) { buttonType = type; }

	void draw(const shared_ptr<cugl::SpriteBatch> &batch, const cugl::Mat4 &transform,
			  cugl::Color4 tint) override;
};

#endif // SWEETSPACE_BUTTONNODE_H

#ifndef SWEETSPACE_CUSTOMNODE_H
#define SWEETSPACE_CUSTOMNODE_H

#include <cugl/2d/CUNode.h>

#include "DonutModel.h"
#include "Globals.h"

/*
 * This class acts as an interface for game objects that exist in the level and require
 * view boundary calculations.
 * Note: if the node is not a child of nearSpace, it probably should not inherit this class.
 */
class CustomNode : public cugl::Node {
   private:
#pragma region Positioning
#pragma mark Positioning Methods
	/**
	 * Calculates the on-screen angle of node relative to the player avatar
	 *
	 * @param modelAngle Angle of this node's model
	 * @return
	 */
	float getOnScreenAngle(float modelAngle) {
		float onScreenAngle = modelAngle - playerDonutModel->getAngle();
		onScreenAngle = onScreenAngle >= 0 ? onScreenAngle : shipSize + onScreenAngle;
		onScreenAngle = onScreenAngle > shipSize / 2 ? onScreenAngle - shipSize : onScreenAngle;
		onScreenAngle *= globals::PI_180;
		return onScreenAngle;
	}

	/**
	 * Returns true if this node is just coming into viewing bounds
	 *
	 * @param onScreenAngle The on-screen angle of node relative to the player avatar
	 * @return
	 */
	bool isComingIntoView(float onScreenAngle) {
		return (!isShown || isDirty) && onScreenAngle < globals::SEG_CUTOFF_ANGLE &&
			   onScreenAngle > -globals::SEG_CUTOFF_ANGLE;
	}

	/**
	 * Returns true if this node is just going out of viewing bounds
	 *
	 * @param onScreenAngle The on-screen angle of node relative to the player avatar
	 * @return
	 */
	bool isGoingOutOfView(float onScreenAngle) {
		return isShown && (onScreenAngle >= globals::SEG_CUTOFF_ANGLE ||
						   onScreenAngle <= -globals::SEG_CUTOFF_ANGLE);
	}

	/**
	 * Returns relative position to nearSpace after polar coord calculation
	 *
	 * @param relAngle Relative angle to nearSpace
	 * @param radius Distance from nearSpace origin
	 * @return
	 */
	cugl::Vec2 getPositionVec(float relAngle, float radius) {
		return cugl::Vec2(radius * sin(relAngle), -radius * cos(relAngle));
	}
#pragma mark -
#pragma endregion
   protected:
#pragma region Fields
#pragma mark Fields
	/** Reference to the player donut model */
	std::shared_ptr<DonutModel> playerDonutModel;
	/** Size of the ship. Needed for visibility determination */
	float shipSize;
	/** Whether the node is being shown right now */
	bool isShown;
	/** The angle on the ship where this object is located */
	float angle;
	/** The radius on the ship where this object is located */
	float radius;
	/** Whether we need to force a redraw this frame */
	bool isDirty;
#pragma mark -
#pragma endregion
#pragma region Constructor
#pragma mark Constructor
	/**
	 * Constructor
	 */
	CustomNode() : cugl::Node(), shipSize(0), isShown(false), angle(0), radius(0), isDirty(false) {}

	/**
	 * Releases all resources allocated with this node.
	 *
	 * This will release, but not necessarily delete the associated texture.
	 * However, the polygon and drawing commands will be deleted and no
	 * longer safe to use.
	 */
	~CustomNode() { dispose(); }

	/**
	 * Properly initialize this node. Do NOT use the constructors in the parent class. They will not
	 * initialize everything.
	 *
	 * @param player Pointer to the player's donut model
	 * @param shipSize Size of the ship (in degrees)
	 * @param angle Angle on the ship where this node is located
	 * @param radius Radius on the ship where this node is located
	 */
	virtual bool init(std::shared_ptr<DonutModel> player, float shipSize, float angle,
					  float radius);

	/**
	 * Releases all resources allocated with this node.
	 *
	 * This will release, but not necessarily delete the associated texture.
	 * However, the polygon and drawing commands will be deleted and no
	 * longer safe to use.
	 */
	void dispose() override;
#pragma mark -
#pragma endregion
#pragma region State Methods
	/** Returns whether this node should be active. */
	virtual bool isActive() = 0;
	/**
	 * Compute any initialization and view state updates that need to happen before the object is
	 * positioned relative to the current angle of the ship.
	 *
	 * This method is not called if the associated model is inactive.
	 */
	virtual void prePosition() {}
	/**
	 * Compute any view state updates that need to happen after the object is positioned relative to
	 * the current angle of the ship.
	 *
	 * This method is not called if the associated model is inactive.
	 */
	virtual void postPosition() {}
#pragma endregion

   public:
	void draw(const shared_ptr<cugl::SpriteBatch> &batch, const cugl::Mat4 &transform,
			  cugl::Color4 tint) override;
};

#endif // SWEETSPACE_CUSTOMNODE_H

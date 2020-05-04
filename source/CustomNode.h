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
#pragma mark Fields
   protected:
	/** Reference to the player donut model */
	std::shared_ptr<DonutModel> playerDonutModel;
	/** Size of the ship. Needed for visibility determination */
	float shipSize;
	/** Whether the node is being shown right now */
	bool isShown;
#pragma mark -
#pragma mark Constructor
	/**
	 * Constructor
	 */
	CustomNode() : cugl::Node(), shipSize(0), isShown(false) {}

	/**
	 * Releases all resources allocated with this node.
	 *
	 * This will release, but not necessarily delete the associated texture.
	 * However, the polygon and drawing commands will be deleted and no
	 * longer safe to use.
	 */
	~CustomNode() { dispose(); }
#pragma mark -
#pragma mark Setters
   public:
	/**
	 * Sets playerDonutModel to given parameter
	 * @param model
	 */
	void setDonutModel(std::shared_ptr<DonutModel> model) { playerDonutModel = model; }

	/**
	 * Store size of ship level
	 * @param f
	 */
	void setShipSize(float f) { shipSize = f; }
#pragma mark -
#pragma mark Positioning Methods
   protected:
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
		return !isShown && onScreenAngle < globals::SEG_CUTOFF_ANGLE &&
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
};

#endif // SWEETSPACE_CUSTOMNODE_H
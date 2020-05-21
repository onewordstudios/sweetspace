#ifndef SWEETSPACE_CUSTOMNODE_H
#define SWEETSPACE_CUSTOMNODE_H

#include <cugl/2d/CUNode.h>

#include "DonutModel.h"
#include "Globals.h"

/*
 * This class acts as an interface for game objects that exist at a well-defined location on the
 * ship and thus require view boundary calculations.
 *
 * Objects that are fixed to the viewport and not to the ship generally should not extend this
 * class. Specifically, nodes of this class should only be added to the scene graph as the second
 * level descendant of nearSpace.
 *
 * This class is abstract and thus cannot be instantiated directly. The method isActive() must be
 * overridden by any children.
 *
 * All spawning and despawning from the screen as the player moves around the ship is handled by
 * this class's draw method. This class exposes five lifecycle methods that its children should
 * override to customize the functionality.
 *
 * 1 - {@link isActive()}. Called at the beginning of each frame. Should return true if and only if
 * this node needs to be drawn this frame. If this method returns false, the node is moved offscreen
 * and the rest of the lifecycle is skipped this frame. Is abstract and must be overriden.
 * Generally, should just query whether the associated model is active.
 *
 * 2 - {@link becomeActive()}. Called on the first frame where {@link isActive()} returns true
 * when it had returned false in the previous frame. Any updates that need to happen when the
 * associated model gets reactiviated should go here.
 *
 * 3 - {@link becomeInactive()}. Called on
 * the first frame where {@link isActive()} returns false when it had returned true in the previous
 * frame. Any cleanup that needs to happen when the associated model gets deactiveated should go
 * here. This method can also be used to spawn any lingering animations that occur after the model
 * is recycled, but should not be used for animations linked directly to the model itself (such as
 * doors opening).
 *
 * 4 - {@link prePosition()}. Called before this class does its positioning
 * calculations. Used to update any variables used by this class in the event they have changed.
 * Note that this class only processes objects once as they move onto screen. If an object's state
 * changes while it is already on screen, set isDirty to true to force a redraw.
 *
 * 5 - {@link postPosition()}. Called after this class does its positioning calculations. Used to
 * perform any additional custom calculations needed for each type of node. For example, if a donut
 * model needs to jump, that should be processed here.
 */
class CustomNode : public cugl::Node {
   private:
	/** The return value of {@code isActive()} the previous frame */
	bool wasActive;

	/** Set of all instantiated members of this class */
	static std::unordered_set<CustomNode *> allActiveNodes;

#pragma region Positioning
#pragma mark Positioning Methods
	/**
	 * Calculates the on-screen angle of node relative to the player avatar
	 *
	 * @param modelAngle Angle of this node's model
	 * @return
	 */
	float getOnScreenAngle(float modelAngle);

	/**
	 * Returns true if this node is just coming into viewing bounds
	 *
	 * @param onScreenAngle The on-screen angle of node relative to the player avatar
	 * @return
	 */
	bool isComingIntoView(float onScreenAngle);

	/**
	 * Returns true if this node is just going out of viewing bounds
	 *
	 * @param onScreenAngle The on-screen angle of node relative to the player avatar
	 * @return
	 */
	bool isGoingOutOfView(float onScreenAngle);

	/**
	 * Returns relative position to nearSpace after polar coord calculation
	 *
	 * @param relAngle Relative angle to nearSpace
	 * @param radius Distance from nearSpace origin
	 * @return
	 */
	cugl::Vec2 getPositionVec(float relAngle, float radius);
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
	CustomNode()
		: cugl::Node(),
		  wasActive(false),
		  shipSize(0),
		  isShown(false),
		  angle(0),
		  radius(0),
		  isDirty(false) {
		allActiveNodes.insert(this);
	}

	/**
	 * Releases all resources allocated with this node.
	 *
	 * This will release, but not necessarily delete the associated texture.
	 * However, the polygon and drawing commands will be deleted and no
	 * longer safe to use.
	 */
	~CustomNode() {
		dispose();
		allActiveNodes.erase(this);
	}

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
	 * Compute any initialization and view state updates that need to happen as the object
	 * becomes active when previously inactive.
	 */
	virtual void becomeActive() {}

	/**
	 * Compute any view state updates and perform any actions that need to happen as the
	 * object becomes inactive when previously active.
	 */
	virtual void becomeInactive() {}

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

	/**
	 * Manually recompute the positions of ALL instantiated custom nodes. Should be called
	 * after events like a stabilizer malfunction that might suddenly but drastically alter the
	 * player's position.
	 */
	static void recomputeAll();
};

#endif // SWEETSPACE_CUSTOMNODE_H

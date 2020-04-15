#ifndef __BUTTON_MODEL_H__
#define __BUTTON_MODEL_H__
#include <cugl/cugl.h>

#include <bitset>
/** The max height of the door*/
constexpr int HALF_PUSHED = 400;
/** The size to make the bitset */
constexpr unsigned int PLAYERS = 6;

class ButtonModel {
private:
    /** The height of the door */
    int height = 0;

protected:
    /** The angle at which the door exists */
    float angle;
    /** The state of the door */
    unsigned char playersOn;
    bool jumped;
    std::shared_ptr<ButtonModel> pairButton;
    int pairId;
    bool resolved = false;

public:
#pragma mark Constructors
    /*
     * Creates a new door at angle 0.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a model on
     * the heap, use one of the static constructors instead.
     */
    ButtonModel(void) : angle(0), playersOn(0) {}

    /**
     * Destroys this door, releasing all resources.
     */
    ~ButtonModel(void) { dispose(); }

    /**
     * Disposes all resources and assets of this door
     *
     * Any assets owned by this object will be immediately released.  Once
     * disposed, a door may not be used until it is initialized again.
     */
    void dispose();

    /**
     * Initializes a new door at an unassigned angle (-1).
     *
     * An initializer does the real work that the constructor does not.  It
     * initializes all assets and makes the object read for use.  By separating
     * them, we allow ourselfs non-pointer references to complex objects.
     *
     * @return true if the obstacle is initialized properly, false otherwise.
     */
    virtual bool init() { return init(-1.0f); }

    /**
     * Initializes a new door with the given angle
     *
     * An initializer does the real work that the constructor does not.  It
     * initializes all assets and makes the object read for use.  By separating
     * them, we allow ourselfs non-pointer references to complex objects.
     *
     * @param a   The angle at which the door exists
     *
     * @return true if the obstacle is initialized properly, false otherwise.
     */
    virtual bool init(const float a) {
        this->angle = a;
        return true;
    };

    static std::shared_ptr<ButtonModel> alloc() {
        std::shared_ptr<ButtonModel> result = std::make_shared<ButtonModel>();
        return (result->init() ? result : nullptr);
    }

#pragma mark -
#pragma mark Accessors
    /**
     * Returns the current angle of the door in degrees.
     *
     * @return the current angle of the door in degrees.
     */
    float getAngle() { return angle; }

    /**
     * Returns the current height of the door.
     *
     * @return the current height of the door.
     */
    int getHeight() { return height; }

    /**
     * Returns the number of players in range of the door.
     *
     * @return the number of players in range of the door.
     */
    int getPlayersOn() {
        std::bitset<PLAYERS> ids(playersOn);
        return (int)ids.count();
    }

    /**
     * Sets the current angle of the door in degrees.
     *
     * @param value The door angle in degrees
     */
    void setAngle(float value) { angle = value; }

    /**
     * Adds the given player's flag from the door.
     *
     */
    void addPlayer(int id) { playersOn = playersOn | (unsigned char)pow(2, id); }

    /**
     * Removes the given player's flag from the door. Requires that this player is on the door
     */
    void removePlayer(int id) {
        if (!isResolved()) {
            playersOn = playersOn ^ (unsigned char)pow(2, id);
        }
    }

    /**
     * Returns whether this player is on the door.
     */
    bool isPlayerOn(int id) { return (playersOn & (unsigned char)pow(2, id)) > 0; }

    /**
     * Returns whether this button is resolved.
     */
    bool isResolved() { return resolved; }

    /**
     * Sets whether this button is resolved.
     */
    void setResolved(bool r) { resolved = r; }
    /**
     * Returns whether this door can be passed under.
     */
    void setJumpedOn(bool jump) { jumped = jump; }
    /**
     * Returns whether this door can be passed under.
     */
    bool jumpedOn() { return jumped && getPlayersOn() == 1; }

    void setPair(std::shared_ptr<ButtonModel> b, int id) { pairButton = b; pairId = id; }
    std::shared_ptr<ButtonModel> getPair() { return pairButton; }
    int getPairID() { return pairId; }

    /**
     * Resets this door.
     */
	void clear() {
		playersOn = 0;
        height = 0;
        resolved = false;
	}
};
#endif /* __BUTTON_MODEL_H__ */

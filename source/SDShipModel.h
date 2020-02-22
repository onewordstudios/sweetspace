//
//  SDShipModel.h
//  Ship Demo
//
//  This model encapsulates all of the information for the ship demo. As there
//  are no collisions in ship demo, this class is pretty simple.
//
//  WARNING: There are a lot of shortcuts in this design that will do not adapt
//  well to data driven design.  This demo has a lot of simplifications to make
//  it a bit easier to see how everything fits together.  However, the model
//  classes and how they are initialized will need to be changed if you add
//  dynamic level loading.
//
//  Pay close attention to how this class designed. This class uses our standard
//  shared-pointer architecture which is common to the entire engine.
//
//  1. The constructor does not perform any initialization; it just sets all
//     attributes to their defaults.
//
//  2. All initialization takes place via init methods, which can fail if an
//     object is initialized more than once.
//
//  3. All allocation takes place via static constructors which return a shared
//     pointer.
//
//  Note that this object manages its own texture, but DOES NOT manage its own
//  scene graph node.  This is a very common way that we will approach complex
//  objects.
//
//  Author: Walker White
//  Version: 1/10/17
//
#ifndef __SD_SHIP_MODEL_H__
#define __SD_SHIP_MODEL_H__
#include <cugl/cugl.h>

/** Ship Frame Sprite numbers */
#define SHIP_IMG_LEFT   0   // Left bank frame
#define SHIP_IMG_FLAT   9   // Neutral frame
#define SHIP_IMG_RIGHT 17   // Right bank frame

/** Number of rows and cols in film strip */
#define SHIP_IMG_ROWS   4
#define SHIP_IMG_COLS   5
#define SHIP_IMG_SIZE  18


#pragma mark -
#pragma mark Ship Model

/**
 * Player avatar for the ship demo.
 *
 * All positional information about the ship goes in this class and not
 * in the sprite node.  That is because we are not animating the ship
 * (other than using the filmstrip to show banking).  We are animating
 * the background.  This forces us to decouple the model from the sprite.
 *
 * You should ALWAYS do this. If you do not do this, I will be most
 * displeased with you.
 */
class ShipModel {
private:
    /** This macro disables the copy constructor (not allowed on models) */
    CU_DISALLOW_COPY_AND_ASSIGN(ShipModel);

private:
    /**
     * Determines the next animation frame for the ship and applies it to the sprite.
     *
     * This method includes some dampening of the turn, and should be called before
     * moving the ship.
     */
    void advanceFrame();
    
protected:
    /** INITIAL position of the ship in world space */
    cugl::Vec2    _initial;
    /** Position of the ship in world space */
    cugl::Vec2    _position;
    /** Current ship velocity */
    cugl::Vec2    _velocity;
    /** Angle of the ship in the world space */
    float   _angle;
    /** Current turning thrust (stored to facilitate decay) */
    float   _turning;
    /** Current forward thrust (stored to facilitate decay) */
    float   _forward;
    /** Reference to image in SceneGraph for animation */
    std::shared_ptr<cugl::AnimationNode> _sprite;
    
    
public:
#pragma mark Constructors
    /*
     * Creates a new ship at the origin.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a model on
     * the heap, use one of the static constructors instead.
     */
    ShipModel(void) : _angle(0), _turning(0), _forward(0) { }
    
    /**
     * Destroys this ship, releasing all resources.
     */
    ~ShipModel(void) { dispose(); }
    
    /**
     * Disposes all resources and assets of this ship
     *
     * Any assets owned by this object will be immediately released.  Once
     * disposed, a ship may not be used until it is initialized again.
     */
    void dispose();
        
    /**
     * Initializes a new ship at the origin.
     *
     * An initializer does the real work that the constructor does not.  It
     * initializes all assets and makes the object read for use.  By separating
     * them, we allow ourselfs non-pointer references to complex objects.
     *
     * @return true if the obstacle is initialized properly, false otherwise.
     */
    virtual bool init() { return init(cugl::Vec2::ZERO); }
    
    /**
     * Initializes a new ship with the given position
     *
     * An initializer does the real work that the constructor does not.  It
     * initializes all assets and makes the object read for use.  By separating
     * them, we allow ourselfs non-pointer references to complex objects.
     *
     * @param pos   Initial position in world coordinates
     *
     * @return true if the obstacle is initialized properly, false otherwise.
     */
    virtual bool init(const cugl::Vec2& pos);
        
        
#pragma mark Static Constructors
    /**
     * Returns a newly allocated ship at the origin.
     *
     * This is a static constructor. You call it with the ShipModel::alloc().
     * We prefer static constructors as they make the usage of shared pointers
     * much simpler (and prevent the temptation of making a weak pointer on
     * the heap).
     *
     * @return a newly allocated ship at the origin.
     */
    static std::shared_ptr<ShipModel> alloc() {
        std::shared_ptr<ShipModel> result = std::make_shared<ShipModel>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated ship at the given position.
     *
     * This is a static constructor. You call it with the ShipModel::alloc().
     * We prefer static constructors as they make the usage of shared pointers
     * much simpler (and prevent the temptation of making a weak pointer on
     * the heap).
     *
     * @param pos   Initial position in world coordinates
     *
     * @return a newly allocated ship at the given position.
     */
    static std::shared_ptr<ShipModel> alloc(const cugl::Vec2& pos) {
        std::shared_ptr<ShipModel> result = std::make_shared<ShipModel>();
        return (result->init(pos) ? result : nullptr);
    }

    
#pragma mark -
#pragma mark Accessors
    // SHORT METHODS CAN BE IN-LINED IN C++
    
    /**
     * Returns the ship velocity as a reference.
     * 
     * This allows us to modify the value.
     *
     * @return the ship velocity as a reference.
     */
    cugl::Vec2& getVelocity()   { return _velocity;      }
    
    /**
     * Returns the ship position as a reference. 
     *
     * This allows us to modify the value.
     *
     * @return the ship position as a reference.
     */
    cugl::Vec2& getPosition()   { return _position;      }
    
    /**
     * Returns the current angle of the ship in radians.
     *
     * @return the current angle of the ship in radians.
     */
    float getAngle()            { return (float)M_PI*_angle/180.0f;    }
    
    /**
     * Sets the current angle of the ship in radians.
     *
     * @param value The ship angle in radians
     */
    void setAngle(float value)  { _angle = 180.0f*value/(float)M_PI;   }
    
    /**
     * Returns the current turning force on the ship
     *
     * @return the current turning force on the ship
     */
    float getTurning()          { return _turning;       }
    
    /**
     * Sets the current turning force on the ship
     *
     * @param value The ship turning force
     */
    void setTurning(float value)    { _turning = value;      }
    
    /**
     * Returns the current forward force on the ship
     *
     * @return The current forward force on the ship
     */
    float getForward()          { return _forward;       }
    
    /**
     * Sets the current forward force on the ship
     *
     * @param value The ship forward force
     */
    void setForward(float value)    { _forward = value;      }
    
    
#pragma mark -
#pragma mark Animation
    /**
     * Returns a reference to film strip representing this ship.
     *
     * It returns nullptr if there is no active film strip.
     *
     * @return a reference to film strip representing this ship.
     */
    std::shared_ptr<cugl::AnimationNode>& getSprite()   { return _sprite;   }
    
    /**
     * Sets the film strip representing this ship.
     *
     * Setting this to nullptr clears the value.
     *
     * @param value The ship film strip.
     */
    void    setSprite(const std::shared_ptr<cugl::AnimationNode>& value);
    
    /**
     * Updates the state of the model
     *
     * This method moves the ship forward, dampens the forces (if necessary)
     * and updates the sprite if it exists.
     *
     * @param timestep  Time elapsed since last called.
     */
    void    update(float timestep=0.0f);
    
    /**
     * Resets the ship back to its original settings
     */
    void    reset();

};

#endif /* __SD_SHIP_MODEL_H__ */

#ifndef __STATE_RECONCILER_H__
#define __STATE_RECONCILER_H__

#include <cugl/cugl.h>

#include "ShipModel.h"

/**
 * Helper class used by mib to reconcile states between state syncs.
 *
 * Can buffer discrepancies during one state sync so that it only gets resolved during the next one.
 */
class StateReconciler {
   private:
	/** One byte */
	unsigned int oneByte;
	/** The precision to multiply floating point numbers by */
	float floatPrecision;
	/** How close to consider floating point numbers identical */
	float floatEpsilon;

	/** Decode a float from the two bytes in the network packet */
	constexpr float DECODE_FLOAT(uint8_t m1, uint8_t m2);
	/** Encode a float and append it to the end of the given vector */
	void ENCODE_FLOAT(float f, std::vector<uint8_t>& out);

	/** Cache of previously unconforming breaches. Bool = active, float = position. */
	std::unordered_map<unsigned int, bool> breachCache;

	/** Cache of previously unconforming doors. Bool = active, float = position. */
	std::unordered_map<unsigned int, bool> doorCache;

	/** Cache of previously unconforming buttons. Bool = active, float = position. */
	std::unordered_map<unsigned int, bool> btnCache;

	/** Local cache of breaches for this cycle; pre-initialized as an optimization */
	std::unordered_map<unsigned int, bool> localBreach;
	/** Local cache of doors for this cycle; pre-initialized as an optimization */
	std::unordered_map<unsigned int, bool> localDoor;
	/** Local cache of buttons for this cycle; pre-initialized as an optimization */
	std::unordered_map<unsigned int, bool> localBtn;
	/** Local cache of unpaired buttons; pre-innitialized as an optimization */
	std::unordered_map<unsigned int, float> localUnpairedBtn;

   public:
	/** Construct a new state reconciler */
	StateReconciler(unsigned int oneByte, float floatPrecision, float floatEpsilon);

	/**
	 * Encode the state of the game into the specified vector.
	 *
	 * @param state The authoritative copy of the game state. PRECONDITION: Game must be going.
	 * @param data The vector for the state to be output into. The first element of the vector
	 * should be prepopulated with the appropriate network flag byte.
	 */
	void encode(std::shared_ptr<ShipModel> state, std::vector<uint8_t>& data);

	/**
	 * Reconcile the state of the game with the incoming message.
	 *
	 * @param state The local copy of the state, to be mutated
	 * @param message The full incoming state sync message, from mib, containing the authoritative
	 * state of the ship
	 *
	 * @return True iff successful. A return value of false indicates a catastrophic failure that
	 * cannot be recovered from (typically, the user has the wrong level loaded), and should boot
	 * the user from the current room.
	 */
	bool reconcile(std::shared_ptr<ShipModel> state, const std::vector<uint8_t>& message);

	/** Reset this class */
	void reset();
};

#endif /* __STATE_RECONCILER_H__ */
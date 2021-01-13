#ifndef STATE_RECONCILER_H
#define STATE_RECONCILER_H

#include <cugl/cugl.h>

#include "ShipModel.h"

/**
 * Helper class used by mib to reconcile states between state syncs.
 *
 * Can buffer discrepancies during one state sync so that it only gets resolved during the next one.
 */
class StateReconciler {
   private:
	/** Encode the current level into a single byte */
	static constexpr uint8_t ENCODE_LEVEL_NUM(uint8_t level, bool parity);

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
	/** Decode a float from the two bytes in the network packet */
	static float decodeFloat(uint8_t m1, uint8_t m2);

	/** Encode a float and append it to the end of the given vector */
	static void encodeFloat(float f, std::vector<uint8_t>& out);

	/** Decode a level byte into the current level and parity */
	static std::pair<uint8_t, bool> decodeLevelNum(uint8_t encodedLevel);

	/**
	 * Encode the state of the game into the specified vector.
	 *
	 * @param state The authoritative copy of the game state. PRECONDITION: Game must be going.
	 * @param data The vector for the state to be output into. The first element of the vector
	 * should be prepopulated with the appropriate network flag byte.
	 * @param level The current level number
	 * @paramm parity Level parity from mib
	 */
	static void encode(const std::shared_ptr<ShipModel>& state, std::vector<uint8_t>& data,
					   uint8_t level, bool parity);

	/**
	 * Reconcile the state of the game with the incoming message.
	 *
	 * @param state The local copy of the state, to be mutated
	 * @param message The full incoming state sync message, from mib, containing the authoritative
	 * state of the ship
	 * @param level The current level number
	 * @paramm parity Level parity from mib
	 *
	 * @return True iff successful. A return value of false indicates a catastrophic failure that
	 * cannot be recovered from (typically, the user has the wrong level loaded).
	 */
	bool reconcile(const std::shared_ptr<ShipModel>& state, const std::vector<uint8_t>& message,
				   uint8_t level, bool parity);

	/** Reset this class */
	void reset();
};

#endif /* STATE_RECONCILER_H */
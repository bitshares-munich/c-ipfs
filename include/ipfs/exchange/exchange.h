#pragma once
/**
 * This is the definition of an "Exchange"
 *
 * Anything that implements the Exchange interface can be used as
 * an IPFS block exchange protocol.
 */
#include "ipfs/blocks/block.h"
#include "ipfs/cid/cid.h"
#include "libp2p/utils/vector.h"

/**
 * These are methods that the local IPFS daemon (or client)
 * call to communicate with the local repository or network
 */

struct Exchange {
	/**
	 * Retrieve a block from peers within the deadline enforced
	 * by the context
	 *
	 * NOTE: Shouldn't the block parameter be a callback (function pointer)?
	 * Otherwise, this function is going to block. Is that what we want?
	 *
	 * @param context the context
	 * @param cid the hash of the block to retrieve
	 * @param block a pointer to the block (allocated by this method if return is true)
	 * @returns true(1) on success, false(0) otherwise
	 */
	int (*GetBlock)(struct Exchange* exchange, struct Cid* cid, struct Block** block);

	/**
	 * Retrieve several blocks
	 * @param context the context
	 * @param Cids a vector of hashes for the blocks to be retrieved
	 * @param blocks a pointer to a vector of retrieved blocks (will be NULL on error)
	 * @returns true(1) on success, otherwise false(0)
	 */
	int (*GetBlocks)(struct Exchange* exchange, struct Libp2pVector* Cids, struct Libp2pVector** blocks);

	/**
	 * Announces the existance of a block to this bitswap service. The service will
	 * potentially notify its peers.
	 * NOTE: This is mainly designed to announce blocks added by non-bitswap methods (i.e. the local user)
	 * @param block the block being announced
	 * @returns true(1) on success, false(0) if not
	 */
	int (*HasBlock)(struct Exchange* exchange, struct Block* block);

	/**
	 * Determine if we're online
	 * @returns true(1) if we're online
	 */
	int (*IsOnline)(struct Exchange*);

	/**
	 * Close up the exchange, and go offline
	 * @returns true(1);
	 */
	int (*Close)(struct Exchange*);

	/**
	 * Used by each implementation to maintain state
	 * (will be cast-ed to an implementation-specific structure)
	 */
	void* exchangeContext;
};

#pragma once

#include "ipfs/blocks/block.h"
#include "ipfs/cid/cid.h"
#include "ipfs/exchange/bitswap/bitswap.h"
#include "wantlist_queue.h"

/***
 * Add a Cid to the local wantlist
 * @param context the context
 * @param cid the Cid
 * @returns the added WantListQueueEntry
 */
struct WantListQueueEntry* ipfs_bitswap_want_manager_add(const struct BitswapContext* context, const struct Cid* cid, const struct WantListSession* session);

/***
 * Checks to see if the requested block has been received
 * @param context the context
 * @param cid the Cid
 * @returns true(1) if it has been received, false(0) otherwise
 */
int ipfs_bitswap_want_manager_received(const struct BitswapContext* context, const struct Cid* cid);

/***
 * retrieve a block from the WantManager.
 * @param context the context
 * @param cid the Cid to get
 * @param block a pointer to the block that will be allocated
 * @returns true(1) on success, false(0) otherwise
 */
int ipfs_bitswap_want_manager_get_block(const struct BitswapContext* context, const struct Cid* cid, struct Block** block);

/***
 * We no longer are requesting this block, so remove it from the queue
 * NOTE: This is reference counted, as another process may have asked for it.
 * @param context the context
 * @param cid the Cid
 * @returns true(1) on success, false(0) otherwise.
 */
int ipfs_bitswap_want_manager_remove(const struct BitswapContext* context, const struct Cid* cid);

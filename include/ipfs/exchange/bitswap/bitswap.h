#pragma once

/***
 * Bitswap implements the "exchange" and "Libp2pProtocolHandler" interfaces
 * @see ../exchange.h
 * @see libp2p/net/protocol.h
 */

#include "libp2p/net/protocol.h"
#include "ipfs/core/ipfs_node.h"
#include "ipfs/exchange/exchange.h"
#include "ipfs/exchange/bitswap/engine.h"

struct Libp2pProtocolHandler* ipfs_bitswap_build_protocol_handler(const struct IpfsNode* local_node);


struct BitswapContext {
	struct IpfsNode* ipfsNode;
	struct WantListQueue* localWantlist;
	struct PeerRequestQueue* peerRequestQueue;
	struct BitswapEngine* bitswap_engine;
};

/**
 * Start up the bitswap exchange
 * @param ipfsNode the context
 * @returns an Exchange struct that refers to the exchange
 */
struct Exchange* ipfs_bitswap_new(struct IpfsNode* ipfsNode);

/***
 * These are the implementation methods for the exchange "Interface"
 */

/***
 * Checks to see if the Bitswap service is online
 * @param exhcnageContext a pointer to a BitswapContext
 * @reutrns true(1) if online, false(0) otherwise.
 */
int ipfs_bitswap_is_online(struct Exchange* exchange);

/***
 * Closes down the Bitswap network
 * @param exchangeContext a pointer to a BitswapContext
 * @returns true(1)
 */
int ipfs_bitswap_close(struct Exchange* exchange);

/****
 * Notify the BitswapNetwork that we have this block
 * @param exchangeContext a pointer to a BitswapContext
 * @block the block that we have
 * @reutrns true(1) if successful, false(0) if not.
 */
int ipfs_bitswap_has_block(struct Exchange* exchange, struct Block* block);

/**
 * Retrieve a block from the BitswapNetwork
 * Note: This may pull the file from the local blockstore.
 * Note: If false(0) is returned, block will be NULL
 *
 * @param exchangeContext a pointer to a BitswapContext
 * @param cid the Cid of the block we're looking for
 * @param block a pointer to the block when we find it.
 * @returns true(1) on success, false(0) otherwise.
 */
int ipfs_bitswap_get_block(struct Exchange* exchange, struct Cid* cid, struct Block** block);

/***
 * Retrieve a collection of blocks from the BitswapNetwork
 * Note: The return of false(0) means that not all blocks were found.
 *
 * @param exchangeContext a pointer to a BitswapContext
 * @param cids a collection of Cid structs
 * @param blocks a collection that contains the results.
 * @param true(1) on success, false(0) otherwise
 */
int ipfs_bitswap_get_blocks(struct Exchange* exchange, struct Libp2pVector* cids, struct Libp2pVector** blocks);

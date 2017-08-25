#pragma once

#include "ipfs/merkledag/node.h"
#include "ipfs/core/ipfs_node.h"

/**
 * Implements a resover. EOM
 */

/**
 * Interogate the path and the current node, looking
 * for the desired node.
 * @param path the current path
 * @param from the current node (or NULL if it is the first call)
 * @returns what we are looking for, or NULL if it wasn't found
 */
struct HashtableNode* ipfs_resolver_get(const char* path, struct HashtableNode* from, const struct IpfsNode* ipfs_node);

/**
 * Interrogate the path, looking for the peer
 * @param path the peer path to search for
 * @param ipfs_node the context
 * @returns the MultiAddress that relates to the path, or NULL if not found
 */
struct Libp2pPeer* ipfs_resolver_find_peer(const char* path, const struct IpfsNode* ipfs_node);

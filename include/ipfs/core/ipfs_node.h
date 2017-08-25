#pragma once

#include "libp2p/peer/peerstore.h"
#include "libp2p/peer/providerstore.h"
#include "ipfs/blocks/blockstore.h"
#include "ipfs/exchange/exchange.h"
#include "ipfs/repo/config/identity.h"
#include "ipfs/repo/fsrepo/fs_repo.h"
#include "ipfs/routing/routing.h"

enum NodeMode { MODE_OFFLINE, MODE_ONLINE };

struct IpfsNode {
	enum NodeMode mode;
	struct Identity* identity;
	struct FSRepo* repo;
	struct Peerstore* peerstore;
	struct ProviderStore* providerstore;
	struct IpfsRouting* routing;
	struct Blockstore* blockstore;
	struct Exchange* exchange;
	struct Libp2pVector* protocol_handlers;
	//struct Pinner pinning; // an interface
	//struct Mount** mounts;
	// TODO: Add more here
};

/***
 * build an online IpfsNode
 * @param repo_path where the IPFS repository directory is
 * @param node the completed IpfsNode struct
 * @returns true(1) on success
 */
int ipfs_node_online_new(const char* repo_path, struct IpfsNode** node);
/***
 * Free resources from the creation of an IpfsNode
 * @param node the node to free
 * @returns true(1)
 */
int ipfs_node_free(struct IpfsNode* node);

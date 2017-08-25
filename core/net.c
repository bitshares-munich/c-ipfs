
#include "ipfs/core/net.h"

/**
 * Do a socket accept
 * @param listener the listener
 * @returns true(1) on success, false(0) otherwise
 */
int ipfs_core_net_accept(struct IpfsListener listener) {
	//TODO: Implement this
	return 0;
}

/**
 * Listen using a particular protocol
 * @param node the node
 * @param protocol the protocol to use
 * @param listener the results
 * @returns true(1) on success, false(0) otherwise
 */
int ipfs_core_net_listen(struct IpfsNode* node, char* protocol, struct IpfsListener* listener){
	// TODO: Implement this
	return 0;
}

/***
 * Dial a peer
 * @param node this node
 * @param peer_id who to dial (null terminated string)
 * @param protocol the protocol to use
 * @param stream the resultant stream
 * @returns true(1) on success, otherwise false(0)
 */
int ipsf_core_net_dial(const struct IpfsNode* node, const char* peer_id, const char* protocol, struct Stream* stream) {
	//TODO: Implement this
	// get the multiaddress from the peerstore
	struct Libp2pPeer* peer = libp2p_peerstore_get_peer(node->peerstore, peer_id, strlen(peer_id));
	// attempt to connect

	// attempt to use the protocol passed in
	return 0;
}

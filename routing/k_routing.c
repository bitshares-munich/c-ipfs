#include "ipfs/routing/routing.h"
#include "libp2p/routing/kademlia.h"
#include "libp2p/peer/providerstore.h"
#include "libp2p/utils/vector.h"
#include "ipfs/thirdparty/ipfsaddr/ipfs_addr.h"

/**
 * Routing using Kademlia and DHT
 *
 * The go version has "supernode" which is similar to this:
 */

/**
 * Put a value in the datastore
 * @param routing the struct that contains connection information
 * @param key the key
 * @param key_size the size of the key
 * @param value the value
 * @param value_size the size of the value
 * @returns 0 on success, otherwise -1
 */
int ipfs_routing_kademlia_put_value(struct IpfsRouting* routing, const unsigned char* key, size_t key_size, const void* value, size_t value_size) {
	return 0;
}

/**
 * Get a value from the datastore
 * @param 1 the struct that contains the connection information
 * @param 2 the key to look for
 * @param 3 the size of the key
 * @param 4 a place to store the value
 * @param 5 the size of the value
 */
int ipfs_routing_kademlia_get_value(struct IpfsRouting* routing, const unsigned char* key, size_t key_size, void** value, size_t* value_size) {
	return 0;
}

/**
 * Find a provider that can provide a particular key
 * NOTE: It will look locally before asking the network. So
 * if a peer announced, we already have an answer.
 *
 * @param routing the context
 * @param key the key to what we're looking for
 * @param key_size the size of the key
 * @param results the results, which is a vector of MultiAddress*
 * @param results_size the size of the results buffer
 * @returns true(1) on success, otherwise false(0)
 */
int ipfs_routing_kademlia_find_providers(struct IpfsRouting* routing, const unsigned char* key, size_t key_size, struct Libp2pVector** results) {
	*results = libp2p_utils_vector_new(1);
	struct Libp2pVector* vector = *results;
	// see if I can provide it
	unsigned char* peer_id = NULL;
	int peer_id_size = 0;
	if (libp2p_providerstore_get(routing->local_node->providerstore, (unsigned char*)key, key_size, &peer_id, &peer_id_size)) {
		struct Libp2pPeer* peer = libp2p_peerstore_get_peer(routing->local_node->peerstore, peer_id, peer_id_size);
		struct Libp2pLinkedList* current = peer->addr_head;
		while (current != NULL) {
			struct MultiAddress* ma = (struct MultiAddress*)current->item;
			if (multiaddress_is_ip(ma)) {
				libp2p_utils_vector_add(vector, ma);
			}
			current = current->next;
		}
	}
	//get a list of providers that are closest
	if (vector->total == 0) {
		// search requires null terminated key
		char* key_nt = malloc(key_size + 1);
		strncpy(key_nt, (char*)key, key_size);
		key_nt[key_size] = 0;
		struct MultiAddress** list = search_kademlia(key_nt, 3);
		free(key_nt);
		if (list != NULL) {
			int i = 0;
			while (list[i] != NULL) {
				struct MultiAddress* current = list[i];
				libp2p_utils_vector_add(vector, current);
				i++;
			}
		}
	}
	if (vector->total == 0) {
		// we were unable to find it, even on the network
		libp2p_utils_vector_free(vector);
		vector = NULL;
		return 0;
	}
	return 1;
}

/**
 * Find a peer
 */
int ipfs_routing_kademlia_find_peer(struct IpfsRouting* routing, const unsigned char* param1, size_t param2, struct Libp2pPeer **result) {
	return 0;
}

/**
 * Calling this method notifies the network that this peer can provide this key
 * @param routing the context
 * @param key the key that can be provided
 * @param key_size the size of the key
 * @returns true(1) on success, otherwise false(0)
 */
int ipfs_routing_kademlia_provide(struct IpfsRouting* routing, const unsigned char* key, size_t key_size) {
	//TODO: Announce to the network that I can provide this file
	// save in a cache
	// store key and address in cache. Key is the hash, peer id is the value
	libp2p_providerstore_add(routing->local_node->providerstore, (unsigned char*)key, key_size, (unsigned char*)routing->local_node->identity->peer->id, routing->local_node->identity->peer->id_size);

	return 1;
}

// declared here so as to have the code in 1 place
int ipfs_routing_online_ping(struct IpfsRouting*, struct Libp2pPeer*);
/**
 * Ping this instance
 */
int ipfs_routing_kademlia_ping(struct IpfsRouting* routing, struct Libp2pPeer* peer) {
	return ipfs_routing_online_ping(routing, peer);
}

int ipfs_routing_kademlia_bootstrap(struct IpfsRouting* routing) {
	struct IpfsNode *local_node = routing->local_node;
	// read the config file and get the bootstrap peers
	for(int i = 0; i < local_node->repo->config->bootstrap_peers->total; i++) { // loop through the peers
		struct IPFSAddr* ipfs_addr = (struct IPFSAddr*) libp2p_utils_vector_get(local_node->repo->config->bootstrap_peers, i);
		struct MultiAddress* ma = multiaddress_new_from_string(ipfs_addr->entire_string);
		// get the id
		char* ptr;
		if ( (ptr = strstr(ipfs_addr->entire_string, "/ipfs/")) != NULL) { // look for the peer id
			ptr += 6;
			if (ptr[0] == 'Q' && ptr[1] == 'm') { // things look good
				struct Libp2pPeer* peer = libp2p_peer_new_from_multiaddress(ma);
				if (peer) {
					peer->id = ptr;
					peer->id_size = strlen(ptr);
					libp2p_peerstore_add_peer(local_node->peerstore, peer);
				}
			}
			// TODO: attempt to connect to the peer

		} // we have a good peer ID

	}
	return 1;
}

struct IpfsRouting* ipfs_routing_new_kademlia(struct IpfsNode* local_node, struct RsaPrivateKey* private_key) {
	char kademlia_id[21];
	// generate kademlia compatible id by getting first 20 chars of peer id
	if (local_node->identity->peer->id == NULL || local_node->identity->peer->id_size < 20) {
		return NULL;
	}
	strncpy(kademlia_id, local_node->identity->peer->id, 20);
	kademlia_id[20] = 0;
	struct IpfsRouting* routing = (struct IpfsRouting*)malloc(sizeof(struct IpfsRouting));
	if (routing != NULL) {
		routing->local_node = local_node;
		routing->sk = private_key;
		routing->PutValue = ipfs_routing_kademlia_put_value;
		routing->GetValue = ipfs_routing_kademlia_get_value;
		routing->FindProviders = ipfs_routing_kademlia_find_providers;
		routing->FindPeer = ipfs_routing_kademlia_find_peer;
		routing->Provide = ipfs_routing_kademlia_provide;
		routing->Ping = ipfs_routing_kademlia_ping;
		routing->Bootstrap = ipfs_routing_kademlia_bootstrap;
	}
	// connect to nodes and listen for connections
	struct MultiAddress* address = multiaddress_new_from_string(local_node->repo->config->addresses->api);
	if (address != NULL && multiaddress_is_ip(address)) {
		start_kademlia_multiaddress(address, kademlia_id, 10, local_node->repo->config->bootstrap_peers);
	}
	local_node->routing = routing;
	return routing;
}

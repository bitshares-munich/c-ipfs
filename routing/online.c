#include <stdlib.h>

#include "ipfs/routing/routing.h"
#include "ipfs/core/null.h"
#include "libp2p/record/message.h"
#include "libp2p/net/stream.h"
#include "libp2p/conn/session.h"
#include "libp2p/routing/dht_protocol.h"
#include "libp2p/utils/logger.h"
#include "libp2p/conn/dialer.h"
#include "ipfs/core/null.h"

/**
 * Implements the routing interface for communicating with network clients
 */

/**
 * Helper method to send and receive a kademlia message
 * @param session_context the session context
 * @param message what to send
 * @returns what was received
 */
struct Libp2pMessage* ipfs_routing_online_send_receive_message(struct SessionContext* sessionContext, struct Libp2pMessage* message) {
	size_t protobuf_size = 0, results_size = 0;
	unsigned char* protobuf = NULL, *results = NULL;
	struct Libp2pMessage* return_message = NULL;

	protobuf_size = libp2p_message_protobuf_encode_size(message);
	protobuf = (unsigned char*)malloc(protobuf_size);
	libp2p_message_protobuf_encode(message, &protobuf[0], protobuf_size, &protobuf_size);


	// upgrade to kademlia protocol
	if (!libp2p_routing_dht_upgrade_stream(sessionContext)) {
		goto exit;
	}

	// send the message, and expect the same back
	if (!sessionContext->default_stream->write(sessionContext, protobuf, protobuf_size)) {
		libp2p_logger_error("online", "Attempted to write to Kademlia stream, but could not.\n");
		goto exit;
	}
	if (!sessionContext->default_stream->read(sessionContext, &results, &results_size, 5)) {
		libp2p_logger_error("online", "Attempted to read from Kademlia stream, but could not.\n");
	}

	if (results_size == 0) {
		libp2p_logger_error("online", "reading kademlia response returned nothing.\n");
		goto exit;
	}

	// see if we can unprotobuf
	if (!libp2p_message_protobuf_decode(results, results_size, &return_message)) {
		libp2p_logger_error("online", "Received kademlia response, but cannot decode it.\n");
		goto exit;
	}
	exit:

	if (protobuf != NULL)
		free(protobuf);
	if (results != NULL)
		free(results);

	return return_message;
}

/***
 * Ask the network for anyone that can provide a hash
 * @param routing the context
 * @param key the hash to look for
 * @param key_size the size of the hash
 * @param peers an array of Peer structs that can provide the hash
 * @returns true(1) on success, otherwise false(0)
 */
int ipfs_routing_online_find_remote_providers(struct IpfsRouting* routing, const unsigned char* key, size_t key_size, struct Libp2pVector** peers) {
	int found = 0;
	// build the message to be transmitted
	struct Libp2pMessage* message = libp2p_message_new();
	message->message_type = MESSAGE_TYPE_GET_PROVIDERS;
	message->key_size = key_size;
	message->key = malloc(message->key_size);
	memcpy(message->key, key, message->key_size);
	// loop through the connected peers, asking for the hash
	struct Libp2pLinkedList* current_entry = routing->local_node->peerstore->head_entry;
	while (current_entry != NULL) {
		struct Libp2pPeer* peer = ((struct PeerEntry*)current_entry->item)->peer;
		if (peer->connection_type == CONNECTION_TYPE_CONNECTED) {
			// Ask for hash, if it has it, break out of the loop and stop looking
			libp2p_logger_debug("online", "FindRemoteProviders: Asking for who can provide\n");
			struct Libp2pMessage* return_message = ipfs_routing_online_send_receive_message(peer->sessionContext, message);
			if (return_message != NULL && return_message->provider_peer_head != NULL) {
				libp2p_logger_debug("online", "FindRemoteProviders: Return value is not null\n");
				found = 1;
				*peers = libp2p_utils_vector_new(1);
				struct Libp2pLinkedList * current_provider_peer_list_item = return_message->provider_peer_head;
				while (current_provider_peer_list_item != NULL) {
					struct Libp2pPeer *current_peer = current_provider_peer_list_item->item;
					// if we can find the peer in the peerstore, use that one instead
					struct Libp2pPeer* peerstorePeer = libp2p_peerstore_get_peer(routing->local_node->peerstore, (unsigned char*)current_peer->id, current_peer->id_size);
					if (peerstorePeer != NULL) {
						// add it to the peerstore
						libp2p_peerstore_add_peer(routing->local_node->peerstore, current_peer);
						peerstorePeer = libp2p_peerstore_get_peer(routing->local_node->peerstore, (unsigned char*)current_peer->id, current_peer->id_size);
					}
					current_peer = peerstorePeer;
					libp2p_utils_vector_add(*peers, current_peer);
					current_provider_peer_list_item = current_provider_peer_list_item->next;
				}
				libp2p_message_free(return_message);
				break;
			} else {
				libp2p_logger_debug("online", "FindRemoteProviders: Return value is null or providers are empty.\n");
			}
			libp2p_message_free(return_message);
			// TODO: Make this multithreaded
		}
		if (found)
			current_entry = NULL;
		else
			current_entry = current_entry->next;
	}
	// clean up
	libp2p_message_free(message);
	return found;
}

/**
 * Looking for a provider of a hash. This first looks locally, then asks the network
 * @param routing the context
 * @param key the hash to look for
 * @param key_size the size of the hash
 * @param peers the results
 * @returns true(1) on success, otherwise false(0)
 */
int ipfs_routing_online_find_providers(struct IpfsRouting* routing, const unsigned char* key, size_t key_size, struct Libp2pVector** peers) {
	unsigned char* peer_id;
	int peer_id_size;
	struct Libp2pPeer *peer;

	// see if we can find the key, and retrieve the peer who has it
	if (!libp2p_providerstore_get(routing->local_node->providerstore, key, key_size, &peer_id, &peer_id_size)) {
		libp2p_logger_debug("online", "Unable to find provider locally... Asking network\n");
		// we need to look remotely
		return ipfs_routing_online_find_remote_providers(routing, key, key_size, peers);
	}

	libp2p_logger_debug("online", "FindProviders: Found provider locally. Searching for peer.\n");
	// now translate the peer id into a peer to get the multiaddresses
	peer = libp2p_peerstore_get_peer(routing->local_node->peerstore, peer_id, peer_id_size);
	free(peer_id);
	if (peer == NULL) {
		return 0;
	}

	*peers = libp2p_utils_vector_new(1);
	libp2p_utils_vector_add(*peers, peer);
	return 1;
}

/***
 * helper method. Connect to a peer and ask it for information
 * about another peer
 */
int ipfs_routing_online_ask_peer_for_peer(struct Libp2pPeer* whoToAsk, const unsigned char* peer_id, size_t peer_id_size, struct Libp2pPeer **result) {
	int retVal = 0;
	struct Libp2pMessage *message = NULL, *return_message = NULL;

	if (whoToAsk->connection_type == CONNECTION_TYPE_CONNECTED) {
		message = libp2p_message_new();
		if (message == NULL)
			goto exit;
		message->message_type = MESSAGE_TYPE_FIND_NODE;
		message->key_size = peer_id_size;
		message->key = malloc(peer_id_size);
		if (message->key == NULL)
			goto exit;
		memcpy(message->key, peer_id, peer_id_size);

		return_message = ipfs_routing_online_send_receive_message(whoToAsk->sessionContext, message);
		if (return_message == NULL) {
			// some kind of network error
			whoToAsk->connection_type = CONNECTION_TYPE_NOT_CONNECTED;
			char* id[whoToAsk->id_size + 1];
			memcpy(id, whoToAsk->id, whoToAsk->id_size);
			id[whoToAsk->id_size] = 0;
			libp2p_logger_error("online", "Connection to %s is broken\n", id);
			goto exit;
		}
		if ( return_message->provider_peer_head == NULL || return_message->provider_peer_head->item == NULL)
			goto exit;

		*result = libp2p_peer_copy(return_message->provider_peer_head->item);

	} else {
		goto exit;
	}

	retVal = 1;
	exit:
	if (return_message != NULL)
		libp2p_message_free(return_message);
	if (message != NULL)
		libp2p_message_free(message);

	return retVal;
}

/**
 * Find a peer
 * @param routing the context
 * @param peer_id the id to look for
 * @param peer_id_size the size of the id
 * @param peer the result of the search
 * @returns true(1) on success, otherwise false(0)
 */
int ipfs_routing_online_find_peer(struct IpfsRouting* routing, const unsigned char* peer_id, size_t peer_id_size, struct Libp2pPeer **result) {
	// first look to see if we have it in the local peerstore
	struct Peerstore* peerstore = routing->local_node->peerstore;
	*result = libp2p_peerstore_get_peer(peerstore, (unsigned char*)peer_id, peer_id_size);
	if (*result != NULL) {
		return 1;
	}
	//ask the swarm to find the peer
	// TODO: Multithread
	struct Libp2pLinkedList *current = peerstore->head_entry;
	while(current != NULL) {
		struct Libp2pPeer *current_peer = ((struct PeerEntry*)current->item)->peer;
		ipfs_routing_online_ask_peer_for_peer(current_peer, peer_id, peer_id_size, result);
		if (*result != NULL)
			return 1;
		current = current->next;
	}
	return 0;
}

struct Libp2pPeer* ipfs_routing_online_build_local_peer(struct IpfsRouting* routing) {
	// create the local_peer to be attached to the message
	struct Libp2pPeer* local_peer = libp2p_peer_copy(routing->local_node->identity->peer);
	local_peer->connection_type = CONNECTION_TYPE_CONNECTED;
	local_peer->addr_head = libp2p_utils_linked_list_new();
	struct MultiAddress* temp_ma = multiaddress_new_from_string((char*)routing->local_node->repo->config->addresses->swarm_head->item);
	int port = multiaddress_get_ip_port(temp_ma);
	multiaddress_free(temp_ma);
	char str[255];
	sprintf(str, "/ip4/127.0.0.1/tcp/%d/ipfs/%s/", port, routing->local_node->identity->peer->id);
	struct MultiAddress* ma = multiaddress_new_from_string(str);
	libp2p_logger_debug("online", "Adding local MultiAddress %s to peer.\n", ma->string);
	local_peer->addr_head->item = ma;
	return local_peer;
}

/**
 * Notify the network that this host can provide this key
 * @param routing information about this host
 * @param key the key (hash) of the data
 * @param key_size the length of the key
 * @returns true(1) on success, otherwise false
 */
int ipfs_routing_online_provide(struct IpfsRouting* routing, const unsigned char* key, size_t key_size) {
	// build a Libp2pPeer that represents this peer
	struct Libp2pPeer* local_peer = ipfs_routing_online_build_local_peer(routing);

	// create the message
	struct Libp2pMessage* msg = libp2p_message_new();
	msg->key_size = key_size;
	msg->key = malloc(msg->key_size);
	memcpy(msg->key, key, msg->key_size);
	msg->message_type = MESSAGE_TYPE_ADD_PROVIDER;
	msg->provider_peer_head = libp2p_utils_linked_list_new();
	msg->provider_peer_head->item = local_peer;

	// loop through all peers in peerstore, and let them know (if we're still connected)
	struct Libp2pLinkedList *current = routing->local_node->peerstore->head_entry;
	while (current != NULL) {
		struct PeerEntry* current_peer_entry = (struct PeerEntry*)current->item;
		struct Libp2pPeer* current_peer = current_peer_entry->peer;
		if (current_peer->is_local) {
			// don't bother adding it
		} else {
			// notify everyone we're connected to
			if (current_peer->connection_type == CONNECTION_TYPE_CONNECTED) {
				// ignoring results is okay this time
				struct Libp2pMessage* rslt = ipfs_routing_online_send_receive_message(current_peer->sessionContext, msg);
				if (rslt != NULL)
					libp2p_message_free(rslt);
			}
		}
		current = current->next;
	}

	// this will take care of freeing local_peer too
	libp2p_message_free(msg);

	return 1;
}

/**
 * Ping a remote
 * @param routing the context
 * @param message the message that we want to send
 * @returns true(1) on success, otherwise false(0)
 */
int ipfs_routing_online_ping(struct IpfsRouting* routing, struct Libp2pPeer* peer) {
	struct Libp2pMessage *outMsg = NULL, *inMsg = NULL;
	int retVal = 0;

	if (peer->connection_type != CONNECTION_TYPE_CONNECTED) {
		if (!libp2p_peer_connect(&routing->local_node->identity->private_key, peer, routing->local_node->peerstore, 5))
			goto exit;
	}
	if (peer->connection_type == CONNECTION_TYPE_CONNECTED) {

		// build the message
		outMsg = libp2p_message_new();
		if (outMsg == NULL)
			goto exit;
		outMsg->message_type = MESSAGE_TYPE_PING;
		// send the message
		inMsg = ipfs_routing_online_send_receive_message(peer->sessionContext, outMsg);

		if (inMsg == NULL) {
			goto exit;
		}

		if (inMsg == NULL || inMsg->message_type != MESSAGE_TYPE_PING)
			goto exit;
	}

	retVal = 1;
	exit:
	if (inMsg != NULL)
		libp2p_message_free(inMsg);
	if (outMsg != NULL)
		libp2p_message_free(outMsg);

	return retVal;
}

/***
 * Ask a peer for a value
 * @param routing the context
 * @param peer the peer to ask
 * @param key the key to ask for
 * @param key_size the size of the key
 * @param buffer where to put the results
 * @param buffer_length the size of the buffer
 * @returns true(1) on success
 */
int ipfs_routing_online_get_peer_value(ipfs_routing* routing, const struct Libp2pPeer* peer, const unsigned char* key, size_t key_size, void** buffer, size_t *buffer_size) {
	// build message
	struct Libp2pMessage* msg = libp2p_message_new();
	msg->key_size = key_size;
	msg->key = malloc(msg->key_size);
	memcpy(msg->key, key, msg->key_size);
	msg->message_type = MESSAGE_TYPE_GET_VALUE;

	// send message and receive results
	struct Libp2pMessage* ret_msg = ipfs_routing_online_send_receive_message(peer->sessionContext, msg);
	libp2p_message_free(msg);

	if (ret_msg == NULL)
		return 0;
	if (ret_msg->record == NULL || ret_msg->record->value_size <= 0) {
		libp2p_message_free(ret_msg);
		return 0;
	}
	// put message results into buffer
	*buffer_size = ret_msg->record->value_size;
	*buffer = malloc(*buffer_size);
	if (*buffer == NULL) {
		*buffer_size = 0;
		libp2p_message_free(ret_msg);
		return 0;
	}
	memcpy(*buffer, ret_msg->record->value, *buffer_size);
	libp2p_message_free(ret_msg);
	return 1;
}

/**
 * Retrieve a value from the dht
 * @param routing the context
 * @param key the key
 * @param key_size the size of the key
 * @param buffer where to put the results
 * @param buffer_size the length of the buffer
 */
int ipfs_routing_online_get_value (ipfs_routing* routing, const unsigned char *key, size_t key_size, void **buffer, size_t *buffer_size)
{
	struct Libp2pVector *peers = NULL;
	int retVal = 0;

	// find a provider
	routing->FindProviders(routing, key, key_size, &peers);
	if (peers == NULL) {
		libp2p_logger_debug("online", "online_get_value returned no providers\n");
		goto exit;
	}

	libp2p_logger_debug("online", "FindProviders returned %d providers\n", peers->total);

	for(int i = 0; i < peers->total; i++) {
		struct Libp2pPeer* current_peer = libp2p_peerstore_get_or_add_peer(routing->local_node->peerstore, libp2p_utils_vector_get(peers, i));
		if (current_peer->is_local) {
			// it's a local fetch. Retrieve it
			if (ipfs_routing_generic_get_value(routing, key, key_size, buffer, buffer_size) == 0) {
				retVal = 1;
				break;
			}
		} else if (libp2p_peer_is_connected(current_peer)) {
			// ask a connected peer for the file. If unsuccessful, continue in the loop.
			if (ipfs_routing_online_get_peer_value(routing, current_peer, key, key_size, buffer, buffer_size)) {
				retVal = 1;
				goto exit;
			}
		}
	}
	if (!retVal) {
		// we didn't get the file. Try to connect to the peers we're not connected to, and ask for the file
		for(int i = 0; i < peers->total; i++) {
			struct Libp2pPeer* current_peer = (struct Libp2pPeer*)libp2p_utils_vector_get(peers, i);
			if (current_peer->is_local) {
				// we tried this once, it didn't work. Skip it.
				continue;
			}
			if (!libp2p_peer_is_connected(current_peer)) {
				// attempt to connect. If unsuccessful, continue in the loop.
				libp2p_logger_debug("online", "Attempting to connect to peer to retrieve file\n");
				if (libp2p_peer_connect(&routing->local_node->identity->private_key, current_peer, routing->local_node->peerstore, 5)) {
					libp2p_logger_debug("online", "Peer connected\n");
					if (ipfs_routing_online_get_peer_value(routing, current_peer, key, key_size, buffer, buffer_size)) {
						libp2p_logger_debug("online", "Retrieved a value\n");
						retVal = 1;
						goto exit;
					} else {
						libp2p_logger_debug("online", "Did not retrieve a value\n");
					}
				}
			}
		}
	}

	retVal = 1;
	exit:
	if (peers != NULL) {
		/* Free the vector, not the items
		for (int i = 0; i < peers->total; i++) {
			struct Libp2pPeer* current = libp2p_utils_vector_get(peers, i);
			libp2p_peer_free(current);
		}
		*/
		libp2p_utils_vector_free(peers);
	}
    return retVal;
}


/**
 * Connects to swarm
 * @param routing the routing struct
 * @returns 0 on success, otherwise error code
 */
int ipfs_routing_online_bootstrap(struct IpfsRouting* routing) {
	char* peer_id = NULL;
	int peer_id_size = 0;
	struct MultiAddress* address = NULL;
	struct Libp2pPeer *peer = NULL;

	// for each address in our bootstrap list, add info into the peerstore
	struct Libp2pVector* bootstrap_peers = routing->local_node->repo->config->bootstrap_peers;
	for(int i = 0; i < bootstrap_peers->total; i++) {
		address = (struct MultiAddress*)libp2p_utils_vector_get(bootstrap_peers, i);
		// attempt to get the peer ID
		peer_id = multiaddress_get_peer_id(address);
		if (peer_id != NULL) {
			peer_id_size = strlen(peer_id);
			peer = libp2p_peer_new();
			peer->id_size = peer_id_size;
			peer->id = malloc(peer->id_size + 1);
			if (peer->id == NULL) { // out of memory?
				libp2p_peer_free(peer);
				free(peer_id);
				return -1;
			}
			memcpy(peer->id, peer_id, peer->id_size);
			peer->id[peer->id_size] = 0;
			peer->addr_head = libp2p_utils_linked_list_new();
			if (peer->addr_head == NULL) { // out of memory?
				libp2p_peer_free(peer);
				free(peer_id);
				return -1;
			}
			peer->addr_head->item = multiaddress_copy(address);
			libp2p_peerstore_add_peer(routing->local_node->peerstore, peer);
			libp2p_peer_free(peer);
			// now find it and attempt to connect
			peer = libp2p_peerstore_get_peer(routing->local_node->peerstore, (const unsigned char*)peer_id, peer_id_size);
			free(peer_id);
			if (peer == NULL) {
				return -1; // this should never happen
			}
			if (peer->sessionContext == NULL) { // should always be true unless we added it twice (TODO: we should prevent that earlier)
				libp2p_peer_connect(&routing->local_node->identity->private_key, peer, routing->local_node->peerstore, 5);
			}
		}
	}

	return 0;
}

/**
 * Create a new ipfs_routing struct for online clients
 * @param fs_repo the repo
 * @param private_key the local private key
 * @reurns the ipfs_routing struct that handles messages
 */
ipfs_routing* ipfs_routing_new_online (struct IpfsNode* local_node, struct RsaPrivateKey *private_key) {
    ipfs_routing *onlineRouting = malloc (sizeof(ipfs_routing));

    if (onlineRouting) {
        onlineRouting->local_node     = local_node;
        onlineRouting->sk            = private_key;

        onlineRouting->PutValue      = ipfs_routing_generic_put_value;
        onlineRouting->GetValue      = ipfs_routing_online_get_value;
        onlineRouting->FindProviders = ipfs_routing_online_find_providers;
        onlineRouting->FindPeer      = ipfs_routing_online_find_peer;
        onlineRouting->Provide       = ipfs_routing_online_provide;
        onlineRouting->Ping          = ipfs_routing_online_ping;
        onlineRouting->Bootstrap     = ipfs_routing_online_bootstrap;
        onlineRouting->Listen        = ipfs_null_listen;
        onlineRouting->Shutdown      = ipfs_null_shutdown;
    }

    onlineRouting->local_node->mode = MODE_ONLINE;

    onlineRouting->Bootstrap(onlineRouting);

    return onlineRouting;
}

int ipfs_routing_online_free(ipfs_routing* incoming) {
	free(incoming);
	return 1;
}

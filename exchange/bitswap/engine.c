#include <unistd.h>
#include "libp2p/utils/logger.h"
#include "ipfs/core/null.h"
#include "ipfs/exchange/bitswap/engine.h"
#include "ipfs/exchange/bitswap/wantlist_queue.h"
#include "ipfs/exchange/bitswap/peer_request_queue.h"

/***
 * Implementation of the bitswap engine
 */

/***
 * Allocate resources for a BitswapEngine
 * @returns a new struct BitswapEngine
 */
struct BitswapEngine* ipfs_bitswap_engine_new() {
	struct BitswapEngine* engine = (struct BitswapEngine*) malloc(sizeof(struct BitswapEngine));
	if (engine != NULL) {
		engine->shutting_down = 0;
	}
	return engine;
}

/***
 * Deallocate resources from struct BitswapEngine
 * @param engine the engine to free
 * @returns true(1)
 */
int ipfs_bitswap_engine_free(struct BitswapEngine* engine) {
	free(engine);
	return 1;
}

/***
 * A separate thread that processes the queue of local requests
 * @param context the context
 */
void* ipfs_bitswap_engine_wantlist_processor_start(void* ctx) {
	struct BitswapContext* context = (struct BitswapContext*)ctx;
	// the loop
	while (!context->bitswap_engine->shutting_down) {
		struct WantListQueueEntry* item = ipfs_bitswap_wantlist_queue_pop(context->localWantlist);
		if (item != NULL) {
			// if there is something on the queue process it.
			ipfs_bitswap_wantlist_process_entry(context, item);
		} else {
			// if there is nothing on the queue, wait...
			sleep(2);
		}
	}
	return NULL;
}

/***
 * A separate thread that processes the queue of remote requests
 * @param context the context
 */
void* ipfs_bitswap_engine_peer_request_processor_start(void* ctx) {
	struct BitswapContext* context = (struct BitswapContext*)ctx;
	// the loop
	struct Libp2pLinkedList* current = context->ipfsNode->peerstore->head_entry;
	int did_some_processing = 0;
	while (1) {
		if (context->bitswap_engine->shutting_down) // system shutting down
			break;

		if (current == NULL) { // the PeerStore is empty
			libp2p_logger_debug("bitswap_engine", "Peerstore is empty. Pausing.\n");
			sleep(1);
			continue;
		}
		if (current->item == NULL) {
			// error
			libp2p_logger_error("bitswap_engine", "Peerstore has a null entry.\n");
			break;
		}
		// see if they want something
		struct Libp2pPeer* current_peer_entry = ((struct PeerEntry*)current->item)->peer;
		if (current_peer_entry == NULL) {
			// error
			libp2p_logger_error("bitswap_engine", "Peerstore has an item that is a null peer.\n");
			break;
		}
		if (current_peer_entry->connection_type == CONNECTION_TYPE_CONNECTED) {
			if (current_peer_entry->sessionContext == NULL || current_peer_entry->sessionContext->default_stream == NULL) {
				current_peer_entry->connection_type = CONNECTION_TYPE_NOT_CONNECTED;
			} else {
				libp2p_logger_debug("bitswap_engine", "We're connected to %s. Lets see if there is a message waiting for us.\n", current_peer_entry->id);
				int retVal = current_peer_entry->sessionContext->default_stream->peek(current_peer_entry->sessionContext);
				if (retVal < 0) {
					libp2p_logger_debug("bitswap_engine", "We thought we were connected, but Peek reported an error.\n");
					libp2p_peer_handle_connection_error(current_peer_entry);
				} else if (retVal > 0) {
					libp2p_logger_debug("bitswap_engine", "%d bytes waiting on network for peer %s.\n", retVal, current_peer_entry->id);
					unsigned char* buffer = NULL;
					size_t buffer_len = 0;
					if (current_peer_entry->sessionContext->default_stream->read(current_peer_entry->sessionContext, &buffer, &buffer_len, 1)) {
						// handle it
						int retVal = libp2p_protocol_marshal(buffer, buffer_len, current_peer_entry->sessionContext, context->ipfsNode->protocol_handlers);
						free(buffer);
						did_some_processing = 1;
						if (retVal == -1) {
							libp2p_logger_error("bitswap_engine", "protocol_marshal tried to handle the network traffic, but failed.\n");
							// there was a problem. Clean up
							libp2p_peer_handle_connection_error(current_peer_entry);
						}
					} else {
						libp2p_logger_error("bitswap_engine", "It was said that there was %d bytes to read, but there wasn't. Cleaning up connection.\n");
						libp2p_peer_handle_connection_error(current_peer_entry);
					}
				}
			}
		} else {
			if (current_peer_entry->is_local) {
				//libp2p_logger_debug("bitswap_engine", "Local peer %s. Skipping.\n", current_peer_entry->id);
			} else {
				//libp2p_logger_debug("bitswap_engine", "We are not connected to this peer %s.\n", current_peer_entry->id);
			}
		}
		// attempt to get queue and process
		struct PeerRequestEntry* entry = ipfs_bitswap_peer_request_queue_find_entry(context->peerRequestQueue, current_peer_entry);
		if (entry != NULL) {
			//libp2p_logger_debug("bitswap_engine", "Processing queue for peer %s.\n", current_peer_entry->id);
			// we have a queue. Do some queue processing
			struct PeerRequest* item = entry->current;
			if (item != NULL) {
				// if there is something on the queue process it.
				if (ipfs_bitswap_peer_request_process_entry(context, item))
					did_some_processing = 1;
			}
		}
		// get next peer (or reset to head entry)
		if (current->next == NULL) {
			current = context->ipfsNode->peerstore->head_entry;
			if (!did_some_processing) {
				// we did nothing in this run through the peerstore. sleep for a sec
				sleep(1);
			}
			did_some_processing = 0;
		}
		else {
			//libp2p_logger_debug("bitswap_engine", "Moving on to the next peer.\n");
			current = current->next;
		}
	}
	return NULL;
}

/**
 * Starts the bitswap engine that processes queue items. There
 * should only be one of these per ipfs instance.
 *
 * @param context the context
 * @returns true(1) on success, false(0) otherwise
 */
int ipfs_bitswap_engine_start(const struct BitswapContext* context) {
	context->bitswap_engine->shutting_down = 0;

	// fire off the threads
	if (pthread_create(&context->bitswap_engine->wantlist_processor_thread, NULL, ipfs_bitswap_engine_wantlist_processor_start, (void*)context)) {
		return 0;
	}
	if (pthread_create(&context->bitswap_engine->peer_request_processor_thread, NULL, ipfs_bitswap_engine_peer_request_processor_start, (void*)context)) {
		ipfs_bitswap_engine_stop(context);
		return 0;
	}
	return 1;
}

/***
 * Shut down the engine
 * @param context the context
 * @returns true(1) on success, false(0) otherwise
 */
int ipfs_bitswap_engine_stop(const struct BitswapContext* context) {
	context->bitswap_engine->shutting_down = 1;

	int error1 = pthread_join(context->bitswap_engine->wantlist_processor_thread, NULL);
	int error2 = pthread_join(context->bitswap_engine->peer_request_processor_thread, NULL);

	ipfs_bitswap_engine_free(context->bitswap_engine);

	return !error1 && !error2;
}

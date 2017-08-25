#include <stdlib.h>
#include "ipfs/exchange/bitswap/peer_request_queue.h"

/***
 * Create a queue, do some work, free the queue, make sure valgrind likes it.
 */
int test_bitswap_peer_request_queue_new() {
	int retVal = 0;
	struct PeerRequestQueue* queue = NULL;
	struct PeerRequest* request = NULL;

	// create a queue
	queue = ipfs_bitswap_peer_request_queue_new();
	if (queue == NULL)
		goto exit;

	// add a request
	request = ipfs_bitswap_peer_request_new();
	if (request == NULL)
		goto exit;
	ipfs_bitswap_peer_request_queue_add(queue, request);

	retVal = 1;
	exit:
	// clean up
	ipfs_bitswap_peer_request_queue_free(queue);
	return retVal;
}

int test_bitswap_peer_request_queue_find() {
	return 0;
	/*
	int retVal = 0;
	struct PeerRequestQueue* queue = NULL;
	struct PeerRequest* request1 = NULL;
	struct PeerRequest* request2 = NULL;
	struct PeerRequestEntry* foundRequestEntry = NULL;

	// create a queue
	queue = ipfs_bitswap_peer_request_queue_new();
	if (queue == NULL)
		goto exit;

	// add a request
	request1 = ipfs_bitswap_peer_request_new();
	if (request1 == NULL)
		goto exit;
	request1->peer_id = 1;
	ipfs_bitswap_peer_request_queue_add(queue, request1);

	// add a second request
	request2 = ipfs_bitswap_peer_request_new();
	if (request2 == NULL)
		goto exit;
	request2->peer_id = 2;
	ipfs_bitswap_peer_request_queue_add(queue, request2);

	// find the first
	foundRequestEntry = ipfs_bitswap_peer_request_queue_find_entry(queue, request1);
	if (foundRequestEntry == NULL)
		goto exit;
	if (foundRequestEntry->current->peer_id != 1)
		goto exit;

	// find the second
	foundRequestEntry = ipfs_bitswap_peer_request_queue_find_entry(queue, request2);
	if (foundRequestEntry == NULL)
		goto exit;
	if (foundRequestEntry->current->peer_id != 2)
		goto exit;

	retVal = 1;
	exit:
	// clean up
	ipfs_bitswap_peer_request_queue_free(queue);
	return retVal;
	*/
}

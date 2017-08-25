#ifndef addresses_h
#define addresses_h

#include "libp2p/utils/linked_list.h"
#include "swarm.h"

struct Addresses {
	// list of strings in format "/family/address/type/port"
	struct Libp2pLinkedList* swarm_head;
	// info for api connection
	char* api;
	// info for http gateway
	char* gateway;
};

/**
 * initialize the Addresses struct with data. Must add the SwarmAddresses later
 * @param addresses the struct
 * @returns true(1) on success, otherwise false(0)
 */
int repo_config_addresses_new(struct Addresses** addresses);

/**
 * clear any memory allocated by a address_new call
 * @param addresses the struct
 * @returns true(1)
 */
int repo_config_addresses_free(struct Addresses* addresses);

#endif /* addresses_h */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ipfs/repo/config/addresses.h"

char* alloc_and_copy(char* source) {
	unsigned long strLen = strlen(source);
	char* result = malloc(sizeof(char) * (strLen + 1));
	strncpy(result, source, strLen);
	result[strLen] = 0;
	return result;
}

int repo_config_addresses_new(struct Addresses** addresses) {
	*addresses = (struct Addresses*)malloc(sizeof(struct Addresses));
	if (*addresses == NULL)
		return 0;

	struct Addresses* addr = *addresses;

	// allocate memory to store api and gateway
	addr->api = NULL;
	addr->gateway = NULL;
	addr->swarm_head = NULL;

	return 1;
}

int repo_config_addresses_free(struct Addresses* addresses) {
	free(addresses->api);
	free(addresses->gateway);
	libp2p_utils_linked_list_free(addresses->swarm_head);
	free(addresses);
	return 1;
}

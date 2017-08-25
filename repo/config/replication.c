#include <stdlib.h>
#include "multiaddr/multiaddr.h"
#include "ipfs/repo/config/replication.h"

/***
 * allocate memory and initialize the replication struct
 * @param replication a pointer to the struct to be allocated
 * @returns true(1) on success, false(0) otherwise
 */
int repo_config_replication_new(struct Replication** replication) {
	*replication = (struct Replication*)malloc(sizeof(struct Replication));
	if (*replication == NULL)
		return 0;
	struct Replication* out = *replication;
	out->announce_minutes = 0;
	out->nodes = NULL;
	return 1;
}

/***
 * Frees memory of a replication struct
 * @param replication the replication struct
 * @returns true(1);
 */
int repo_config_replication_free(struct Replication* replication) {
	if (replication != NULL) {
		// free the vector
		if (replication->nodes != NULL) {
			for(int i = 0; i < replication->nodes->total; i++) {
				struct MultiAddress* currAddr = (struct MultiAddress*)libp2p_utils_vector_get(replication->nodes, i);
				multiaddress_free(currAddr);
			}
			libp2p_utils_vector_free(replication->nodes);
		}
		// free the struct
		free(replication);
	}
	return 1;
}

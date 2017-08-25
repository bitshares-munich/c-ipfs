#include "libp2p/utils/vector.h"

struct Replication {
	int announce_minutes;
	struct Libp2pVector* nodes;
};

/***
 * allocate memory and initialize the replication struct
 * @param replication a pointer to the struct to be allocated
 * @returns true(1) on success, false(0) otherwise
 */
int repo_config_replication_new(struct Replication** replication);

/***
 * Frees memory of a replication struct
 * @param replication the replication struct
 * @returns true(1);
 */
int repo_config_replication_free(struct Replication* replication);

#ifndef bootstrap_peer_h
#define bootstrap_peer_h

#include "libp2p/utils/vector.h"

/***
 * get a list of peers to use to bootstrap the instance
 * @param bootstrap_peers A vector of MultiAddress structs will be allocated by this function
 * @returns true(1) on success, otherwise false(0)
 */
int repo_config_bootstrap_peers_retrieve(struct Libp2pVector** bootstrap_peers);

/***
 * frees up memory caused by call to repo_config_bootstrap_peers_retrieve
 * @param list the list to free
 * @returns true(1)
 */
int repo_config_bootstrap_peers_free(struct Libp2pVector* list);

#endif /* bootstrap_peer_h */

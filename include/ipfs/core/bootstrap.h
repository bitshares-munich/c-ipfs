#pragma once

/**
 * Fires up the connection to peers
 */

/**
 * Connect to the peers in the config file
 * @param param a IpfsNode object
 * @returns nothing useful
 */
void *ipfs_bootstrap_swarm(void* param);

void *ipfs_bootstrap_routing(void* param);

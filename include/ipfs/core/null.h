#pragma once

#include "libp2p/conn/session.h"
#include "ipfs/core/ipfs_node.h"

void *ipfs_null_connection (void *ptr);
void *ipfs_null_listen (void *ptr);
int ipfs_null_shutdown();

/***
 * Handle the incoming request from a Multistream
 * @param incoming the incoming request
 * @param incoming_size the size of the request in bytes
 * @param session the session context
 * @param connection_param the connection parameters
 * @returns True(1) on success, False(0) on error
 */
int ipfs_multistream_marshal(const unsigned char* incoming, size_t incoming_size, struct SessionContext* session, struct IpfsNode* local_node);


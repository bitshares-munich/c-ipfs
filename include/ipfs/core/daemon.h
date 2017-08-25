#ifndef DAEMON_H
#define DAEMON_H

#include <stdint.h>
#include "ipfs/core/ipfs_node.h"

#define MAX 5
#define CONNECTIONS 50

struct null_connection_params {
	int file_descriptor;
	int *count;
	char* ip;
	int port;
	struct IpfsNode* local_node;
};

struct null_listen_params {
	uint32_t ipv4;
	uint16_t port;
};

struct IpfsNodeListenParams {
	uint32_t ipv4;
	uint16_t port;
	struct IpfsNode* local_node;
};

int ipfs_daemon (int argc, char **argv);
int ipfs_daemon_start(char* repo_path);
int ipfs_daemon_stop();
int ipfs_ping (int argc, char **argv);

#endif // DAEMON_H

#ifndef __COMMANDS_CONTEXT_H__
#define __COMMANDS_CONTEXT_H__

#include "ipfs/commands/req_log.h"
#include "ipfs/repo/config/config.h"
#include "ipfs/core/ipfs_node.h"

struct Context {
	int online;
	char* config_root;
	struct ReqLog req_log;
	struct RepoConfig config;
	int (*load_config)(char* path);
	struct IpfsNode node;
	int (*construct_node)(struct IpfsNode* node);
};

#endif /* context_h */

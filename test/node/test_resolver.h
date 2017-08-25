#include <pthread.h>

#include "ipfs/importer/resolver.h"
#include "libp2p/os/utils.h"
#include "multiaddr/multiaddr.h"
#include "ipfs/core/daemon.h"

int test_resolver_get() {
	int retVal = 0;
	char* home_dir = os_utils_get_homedir();
	char* test_dir = malloc(strlen(home_dir) + 10);
	struct FSRepo* fs_repo = NULL;
	struct HashtableNode* result = NULL;
	int argc = 6;
	char* argv[argc];
	const char* work_path = "/tmp";
	char ipfs_path[12];
	sprintf(&ipfs_path[0], "%s/%s", work_path, ".ipfs");

	os_utils_filepath_join(home_dir, "ipfstest", test_dir, strlen(home_dir) + 10);

	// clean out repository
	if (!drop_and_build_repository(ipfs_path, 4001, NULL, NULL))
		goto exit;

	argv[0] = "ipfs";
	argv[1] = "add";
	argv[2] = "-r";
	argv[3] = test_dir;
	argv[4] = "-c";
	argv[5] = (char*)work_path;

	ipfs_import_files(argc, (char**)argv);

	ipfs_repo_fsrepo_new(ipfs_path, NULL, &fs_repo);
	ipfs_repo_fsrepo_open(fs_repo);

	struct IpfsNode ipfs_node;
	ipfs_node.repo = fs_repo;

	// find something that is already in the repository
	result = ipfs_resolver_get("QmbMecmXESf96ZNry7hRuzaRkEBhjqXpoYfPCwgFzVGDzB", NULL, &ipfs_node);
	if (result == NULL) {
		goto exit;
	}

	// clean up to try something else
	ipfs_hashtable_node_free(result);
	result = NULL;

	// find something where path includes the local node
	char path[255];
	strcpy(path, "/ipfs/");
	strcat(path, fs_repo->config->identity->peer->id);
	strcat(path, "/QmbMecmXESf96ZNry7hRuzaRkEBhjqXpoYfPCwgFzVGDzB");
	result = ipfs_resolver_get(path, NULL, &ipfs_node);
	if (result == NULL) {
		goto exit;
	}

	// clean up to try something else
	ipfs_hashtable_node_free(result);
	result = NULL;

	// find something by path
	result = ipfs_resolver_get("QmZBvycPAYScBoPEzm35zXHt6gYYV5t9PyWmr4sksLPNFS/hello_world.txt", NULL, &ipfs_node);
	if (result == NULL) {
		goto exit;
	}

	retVal = 1;
	exit:
	free(test_dir);
	ipfs_repo_fsrepo_free(fs_repo);
	ipfs_hashtable_node_free(result);
	return retVal;
}

void* test_resolver_daemon_start(void* arg) {
	ipfs_daemon_start((char*)arg);
	return NULL;
}

int test_resolver_remote_get() {
	// clean out repository
	const char* ipfs_path = "/tmp";
	os_utils_setenv("IPFS_PATH", ipfs_path, 1);
	char remote_peer_id[255];
	char path[255];
	pthread_t thread;
	int thread_started = 0;
	int retVal = 0;
	struct FSRepo* fs_repo = NULL;
	struct HashtableNode* result = NULL;

	// this should point to a test directory with files and directories
	char* home_dir = os_utils_get_homedir();
	char* test_dir = malloc(strlen(home_dir) + 10);

	int argc = 4;
	char* argv[argc];
	argv[0] = "ipfs";
	argv[1] = "add";
	argv[2] = "-r";
	argv[3] = test_dir;

	drop_and_build_repository(ipfs_path, 4001, NULL, NULL);

	// start the daemon in a separate thread
	if (pthread_create(&thread, NULL, test_resolver_daemon_start, (void*)ipfs_path) < 0)
		goto exit;
	thread_started = 1;

	os_utils_filepath_join(home_dir, "ipfstest", test_dir, strlen(home_dir) + 10);

	ipfs_import_files(argc, (char**)argv);

	ipfs_repo_fsrepo_new(ipfs_path, NULL, &fs_repo);
	ipfs_repo_fsrepo_open(fs_repo);

	// put the server in the peer store and change our peer id so we think it is remote (hack for now)
	strcpy(remote_peer_id, fs_repo->config->identity->peer->id);
	char multiaddress_string[100];
	sprintf(multiaddress_string, "/ip4/127.0.0.1/tcp/4001/ipfs/%s", remote_peer_id);
	struct MultiAddress* remote_addr = multiaddress_new_from_string(multiaddress_string);
	struct Peerstore* peerstore = libp2p_peerstore_new(fs_repo->config->identity->peer);
	struct Libp2pPeer* peer = libp2p_peer_new_from_multiaddress(remote_addr);
	libp2p_peerstore_add_peer(peerstore, peer);
	strcpy(fs_repo->config->identity->peer->id, "QmABCD");

    struct IpfsNode local_node;
    local_node.mode = MODE_ONLINE;
    local_node.peerstore = peerstore;
    local_node.repo = fs_repo;
    local_node.identity = fs_repo->config->identity;

	// find something by remote path
	strcpy(path, "/ipfs/");
	strcat(path, remote_peer_id);
	strcat(path, "/QmZBvycPAYScBoPEzm35zXHt6gYYV5t9PyWmr4sksLPNFS/hello_world.txt");
	result = ipfs_resolver_get(path, NULL, &local_node);
	if (result == NULL) {
		goto exit;
	}

	// cleanup
	retVal = 1;
	exit:
	ipfs_daemon_stop();
	if (thread_started)
		pthread_join(thread, NULL);
	ipfs_hashtable_node_free(result);
	if (fs_repo != NULL)
		ipfs_repo_fsrepo_free(fs_repo);
	if (local_node.peerstore != NULL)
		libp2p_peerstore_free(local_node.peerstore);
	return retVal;

}

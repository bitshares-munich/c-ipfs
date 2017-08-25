#include "cid/test_cid.h"
#include "cmd/ipfs/test_init.h"
#include "exchange/test_bitswap.h"
#include "exchange/test_bitswap_request_queue.h"
#include "flatfs/test_flatfs.h"
#include "merkledag/test_merkledag.h"
#include "node/test_node.h"
#include "node/test_importer.h"
#include "node/test_resolver.h"
#include "repo/test_repo_bootstrap_peers.h"
#include "repo/test_repo_config.h"
#include "repo/test_repo_fsrepo.h"
#include "repo/test_repo_identity.h"
#include "routing/test_routing.h"
#include "routing/test_supernode.h"
#include "storage/test_ds_helper.h"
#include "storage/test_datastore.h"
#include "storage/test_blocks.h"
#include "storage/test_unixfs.h"
#include "core/test_ping.h"
#include "core/test_null.h"
#include "core/test_daemon.h"
#include "core/test_node.h"
#include "libp2p/utils/logger.h"
 		 
int testit(const char* name, int (*func)(void)) {
	printf("TESTING %s...\n", name);
	int retVal = func();
	if (retVal)
		printf("%s success!\n", name);
	else
		printf("** Uh oh! %s failed.**\n", name);
	return retVal == 0;
}

const char* names[] = {
		"test_bitswap_new_free",
		"test_bitswap_peer_request_queue_new",
		"test_bitswap_retrieve_file",
		"test_bitswap_retrieve_file_known_remote",
		"test_bitswap_retrieve_file_remote",
		"test_bitswap_retrieve_file_third_party",
		"test_cid_new_free",
		"test_cid_cast_multihash",
		"test_cid_cast_non_multihash",
		"test_cid_protobuf_encode_decode",
		"test_daemon_startup_shutdown",
		"test_repo_config_new",
		"test_repo_config_init",
		"test_repo_config_write",
		"test_repo_config_identity_new",
		"test_repo_config_identity_private_key",
		"test_repo_fsrepo_write_read_block",
		"test_repo_fsrepo_build",
		"test_routing_supernode_start",
		"test_get_init_command",
		"test_import_small_file",
		"test_import_large_file",
		"test_repo_fsrepo_open_config",
		"test_flatfs_get_directory",
		"test_flatfs_get_filename",
		"test_flatfs_get_full_filename",
		"test_ds_key_from_binary",
		"test_blocks_new",
		"test_repo_bootstrap_peers_init",
		"test_ipfs_datastore_put",
		"test_node",
		"test_node_link_encode_decode",
		"test_node_encode_decode",
		"test_node_peerstore",
		"test_merkledag_add_data",
		"test_merkledag_get_data",
		"test_merkledag_add_node",
		"test_merkledag_add_node_with_links",
		"test_resolver_get",
		"test_routing_find_peer",
		"test_routing_provide" /*,
		"test_routing_find_providers",
		"test_routing_supernode_get_value",
		"test_routing_supernode_get_remote_value",
		"test_routing_retrieve_file_third_party",
		"test_routing_retrieve_large_file",
		"test_unixfs_encode_decode",
		"test_unixfs_encode_smallfile",
		"test_ping",
		"test_ping_remote",
		"test_null_add_provider",
		"test_resolver_remote_get"
		*/
};

int (*funcs[])(void) = {
		test_bitswap_new_free,
		test_bitswap_peer_request_queue_new,
		test_bitswap_retrieve_file,
		test_bitswap_retrieve_file_known_remote,
		test_bitswap_retrieve_file_remote,
		test_bitswap_retrieve_file_third_party,
		test_cid_new_free,
		test_cid_cast_multihash,
		test_cid_cast_non_multihash,
		test_cid_protobuf_encode_decode,
		test_daemon_startup_shutdown,
		test_repo_config_new,
		test_repo_config_init,
		test_repo_config_write,
		test_repo_config_identity_new,
		test_repo_config_identity_private_key,
		test_repo_fsrepo_write_read_block,
		test_repo_fsrepo_build,
		test_routing_supernode_start,
		test_get_init_command,
		test_import_small_file,
		test_import_large_file,
		test_repo_fsrepo_open_config,
		test_flatfs_get_directory,
		test_flatfs_get_filename,
		test_flatfs_get_full_filename,
		test_ds_key_from_binary,
		test_blocks_new,
		test_repo_bootstrap_peers_init,
		test_ipfs_datastore_put,
		test_node,
		test_node_link_encode_decode,
		test_node_encode_decode,
		test_node_peerstore,
		test_merkledag_add_data,
		test_merkledag_get_data,
		test_merkledag_add_node,
		test_merkledag_add_node_with_links,
		test_resolver_get,
		test_routing_find_peer,
		test_routing_provide /*,
		test_routing_find_providers,
		test_routing_provide,
		test_routing_supernode_get_value,
		test_routing_supernode_get_remote_value,
		test_routing_retrieve_file_third_party,
		test_routing_retrieve_large_file,
		test_unixfs_encode_decode,
		test_unixfs_encode_smallfile,
		test_ping,
		test_ping_remote,
		test_null_add_provider,
		test_resolver_remote_get
		*/
};

/**
 * Pull the next test name from the command line
 * @param the count of arguments on the command line
 * @param argv the command line arguments
 * @param arg_number the current argument we want
 * @returns a null terminated string of the next test or NULL
 */
char* get_test(int argc, char** argv, int arg_number) {
	char* retVal = NULL;
	char* ptr = NULL;
	if (argc > arg_number) {
		ptr = argv[arg_number];
		if (ptr[0] == '\'')
			ptr++;
		retVal = malloc(strlen(ptr) + 1);
		strcpy(retVal, ptr);
		ptr = strchr(retVal, '\'');
		if (ptr != NULL)
			ptr[0] = 0;
	}
	return retVal;
}

/**
 * run certain tests or run all
 */
int main(int argc, char** argv) {
	int counter = 0;
	int tests_ran = 0;
	char* test_wanted = NULL;
	int certain_tests = 0;
	int current_test_arg = 1;
	if(argc > 1) {
		certain_tests = 1;
	}
	int array_length = sizeof(funcs) / sizeof(funcs[0]);
	int array2_length = sizeof(names) / sizeof(names[0]);
	if (array_length != array2_length) {
		printf("Test arrays are not of the same length. Funcs: %d, Names: %d\n", array_length, array2_length);
	}
	test_wanted = get_test(argc, argv, current_test_arg);
	while (!certain_tests || test_wanted != NULL) {
		for (int i = 0; i < array_length; i++) {
			if (certain_tests) {
				// get the test we currently want from the command line
				const char* currName = names[i];
				if (strcmp(currName, test_wanted) == 0) {
					tests_ran++;
					counter += testit(names[i], funcs[i]);
				}
			}
			else
				if (!certain_tests) {
					tests_ran++;
					counter += testit(names[i], funcs[i]);
				}
		}
		if (!certain_tests) // we did them all, not certain ones
			break;
		free(test_wanted);
		test_wanted = get_test(argc, argv, ++current_test_arg);
	}
	if (tests_ran == 0)
		printf("***** No tests found *****\n");
	else {
		if (counter > 0) {
			printf("***** There were %d failed (out of %d) test(s) *****\n", counter, tests_ran);
		} else {
			printf("All %d tests passed\n", tests_ran);
		}
	}
	libp2p_logger_free();
	fclose(stdin);
	fclose(stdout);
	fclose(stderr);
	return 1;
}

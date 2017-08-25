#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

#include "libp2p/os/utils.h"
#include "ipfs/repo/config/config.h"
#include "ipfs/repo/fsrepo/fs_repo.h"

/**
 * The basic functions for initializing an IPFS repo
 */

/**
 * Get the correct repo home directory. This first looks at the
 * command line, then the IPFS_PATH environment variable,
 * then the user's home directory. This is where the .ipfs directory
 * is or will be.
 * @param argc number of command line parameters
 * @param argv command line parameters
 * @returns the repo home directory
 */
char* ipfs_repo_get_home_directory(int argc, char** argv) {
	char *result = NULL;
	// first check the command line
	for(int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config") == 0) {
			if (i + 1 < argc) {
				result = argv[i+1];
				break;
			}
		}
	}
	if (result == NULL) { // we didn't pass it on the command line
		// check IPFS_PATH
		result = os_utils_getenv("IPFS_PATH");
	}
	if (result == NULL) { // not on command line nor environment var.
		result = os_utils_get_homedir();
	}
	return result;
}

/**
 * Get the correct repo directory. Looks in all the appropriate places
 * for the ipfs directory.
 * @param argc number of command line arguments
 * @param argv command line arguments
 * @param repo_dir the results. This will point to the [IPFS_PATH]/.ipfs directory
 * @returns true(1) if the directory is there, false(0) if it is not.
 */
int ipfs_repo_get_directory(int argc, char** argv, char** repo_dir) {
	char* home = ipfs_repo_get_home_directory(argc, argv);
	int dir_len = strlen(home) + 7;
	*repo_dir = malloc(dir_len);
	os_utils_filepath_join(home, ".ipfs", *repo_dir, dir_len);
	return os_utils_directory_exists(*repo_dir);
}

/**
 * Make an IPFS directory at the passed in path
 * @param path the path
 * @param swarm_port the port that the swarm will run on
 * @param bootstrap_peers a Vector of MultiAddress of fellow peers
 * @param peer_id the peer id generated
 * @returns true(1) on success, false(0) on failure
 */
int make_ipfs_repository(const char* path, int swarm_port, struct Libp2pVector* bootstrap_peers, char **peer_id) {
	int retVal = 0;
	char currDirectory[1024];
	struct RepoConfig* repo_config = NULL;
	struct FSRepo* fs_repo = NULL;

	printf("initializing ipfs node at %s\n", path);
	// build a default repo config
	if (!ipfs_repo_config_new(&repo_config))
		goto exit;
	printf("generating 2048-bit RSA keypair...");
	if (!ipfs_repo_config_init(repo_config, 2048, path, swarm_port, bootstrap_peers)) {
		fprintf(stderr, "Unable to initialize repository at %s\n", path);
		goto exit;
	}
	printf("done\n");
	// now the fs_repo
	if (!ipfs_repo_fsrepo_new(path, repo_config, &fs_repo))
		goto exit;
	// this builds a new repo
	if (!ipfs_repo_fsrepo_init(fs_repo))
		goto exit;

	// give some results to the user
	printf("peer identity: %s\n", fs_repo->config->identity->peer->id);
	if (peer_id != NULL) {
		*peer_id = malloc(fs_repo->config->identity->peer->id_size + 1);
		strcpy(*peer_id, fs_repo->config->identity->peer->id);
	}

	// make sure the repository exists
	if (!os_utils_filepath_join(path, "config", currDirectory, 1024))
		goto exit;

	if (!os_utils_file_exists(currDirectory))
		goto exit;

	// cleanup
	retVal = 1;
	exit:
	if (fs_repo != NULL)
		ipfs_repo_fsrepo_free(fs_repo);

	return retVal;
}

/**
 * Initialize a repository
 * @param argc number of command line arguments
 * @param argv command line arguments
 * @returns true(1) on succes, false(0) otherwise
 */
int ipfs_repo_init(int argc, char** argv) {
	char* repo_directory = NULL;
	if (ipfs_repo_get_directory(argc, argv, &repo_directory)) {
		printf("Directory already exists: %s\n", repo_directory);
		return 0;
	}
	// make the directory
#ifdef __MINGW32__
	if (mkdir(repo_directory) == -1) {
#else
	if (mkdir(repo_directory, S_IRWXU) == -1) {
#endif
		printf("Unable to create the directory: %s\n", repo_directory);
		return 0;
	}
	// make the repository
	return make_ipfs_repository(repo_directory, 4001, NULL, NULL);
}

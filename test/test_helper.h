/**
 * Helpers for testing
 */

/***
 * Drop a repository by deleting the directory
 * @param path the path of the repository
 * @returns true(1) on success, false(0) otherwise
 */
int drop_repository(const char* path);

/**
 * Create a new repository in the directory, erasing old one
 * NOTE: base directory must already exist
 */
int drop_and_build_repository(const char* dir, int swarm_port, struct Libp2pVector* bootstrap_peers, char** peer_id);

int drop_build_and_open_repo(const char* path, struct FSRepo** fs_repo);

/***
 * Helper to create a test file in the OS
 * @pram fileName the resultant file name
 * @param bytes the data that goes in the file
 * @param num_bytes the size of the buffer
 */
int create_file(const char* fileName, unsigned char* bytes, size_t num_bytes);

/***
 * Create a buffer with some bytes
 * @param buffer where to put the bytes
 * @param num_bytes how much to fill of the buffer
 */
int create_bytes(unsigned char* buffer, size_t num_bytes);

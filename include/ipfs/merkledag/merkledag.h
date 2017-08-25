/***
 * Merkledag methods
 */
#ifndef __IPFS_MERKLEDAG_H__
#define __IPFS_MERKLEDAG_H__

#include "ipfs/merkledag/node.h"
#include "ipfs/repo/fsrepo/fs_repo.h"

/***
 * Adds a node to the dagService and blockService
 * @param node the node to add
 * @param fs_repo the repo to add to
 * @param bytes_written the number of bytes written
 * @returns true(1) on success
 */
int ipfs_merkledag_add(struct HashtableNode* node, struct FSRepo* fs_repo, size_t* bytes_written);

/***
 * Retrieves a node from the datastore based on the cid
 * @param cid the key to look for
 * @param node the node to be created
 * @param fs_repo the repository
 * @returns true(1) on success
 */
int ipfs_merkledag_get(const unsigned char* hash, size_t hash_size, struct HashtableNode** node, const struct FSRepo* fs_repo);

/***
 * Retrieves a node from the datastore based on the multihash
 * @param multihash the base58 encoded multihash (should start with Qm) as a null terminated string
 * @param node the node to be created
 * @param fs_repo the repository
 * @returns true(1) on success
 */
int ipfs_merkledag_get_by_multihash(const unsigned char* multihash, size_t multihash_length, struct HashtableNode** node, const struct FSRepo* fs_repo);

#endif

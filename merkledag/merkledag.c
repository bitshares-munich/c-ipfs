/**
 * A basic storage building block of the IPFS system
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libp2p/crypto/sha256.h"
#include "mh/multihash.h"
#include "mh/hashes.h"
#include "ipfs/merkledag/merkledag.h"
#include "ipfs/unixfs/unixfs.h"


/***
 * Adds a node to the dagService and blockService
 * @param node the node to add
 * @param fs_repo the repo to add to
 * @param bytes_written the number of bytes written
 * @returns true(1) on success
 */
int ipfs_merkledag_add(struct HashtableNode* node, struct FSRepo* fs_repo, size_t* bytes_written) {
	// taken from merkledag.go line 59
	int retVal = 0;

	// compute the hash if necessary
	if (node->hash == NULL) {
		size_t protobuf_size = ipfs_hashtable_node_protobuf_encode_size(node);
		unsigned char protobuf[protobuf_size];
		size_t bytes_encoded;
		retVal = ipfs_hashtable_node_protobuf_encode(node, protobuf, protobuf_size, &bytes_encoded);

		node->hash_size = 32;
		node->hash = (unsigned char*)malloc(node->hash_size);
		if (node->hash == NULL) {
			return 0;
		}
		if (libp2p_crypto_hashing_sha256(protobuf, bytes_encoded, &node->hash[0]) == 0) {
			free(node->hash);
			return 0;
		}
	}

	// write to block store & datastore
	retVal = ipfs_repo_fsrepo_node_write(node, fs_repo, bytes_written);
	if (retVal == 0) {
		return 0;
	}

	// TODO: call HasBlock (unsure why as yet)
	return 1;
}

/***
 * Retrieves a node from the datastore based on the hash
 * @param hash the key to look for
 * @param hash_size the length of the key
 * @param node the node to be created
 * @param fs_repo the repository
 * @returns true(1) on success
 */
int ipfs_merkledag_get(const unsigned char* hash, size_t hash_size, struct HashtableNode** node, const struct FSRepo* fs_repo) {
	int retVal = 1;
	size_t key_length = 100;
	unsigned char key[key_length];

	// look for the node in the datastore. If it is not there, it is not a node.
	// If it exists, it is only a block.
	retVal = fs_repo->config->datastore->datastore_get((char*)hash, hash_size, key, key_length, &key_length, fs_repo->config->datastore);
	if (retVal == 0)
		return 0;

	// we have the record from the db. Go get the node from the blockstore
	retVal = ipfs_repo_fsrepo_node_read(hash, hash_size, node, fs_repo);
	if (retVal == 0) {
		return 0;
	}

	// set the hash
	ipfs_hashtable_node_set_hash(*node, hash, hash_size);

	return 1;
}

int ipfs_merkledag_get_by_multihash(const unsigned char* multihash, size_t multihash_length, struct HashtableNode** node, const struct FSRepo* fs_repo) {
	// convert to hash
	size_t hash_size = 0;
	unsigned char* hash = NULL;
	if (mh_multihash_digest(multihash, multihash_length, &hash, &hash_size) < 0) {
		return 0;
	}
	return ipfs_merkledag_get(hash, hash_size, node, fs_repo);
}

#include <stdio.h>
#include <string.h>

#include "ipfs/cid/cid.h"
#include "ipfs/merkledag/merkledag.h"
#include "ipfs/merkledag/node.h"
#include "ipfs/repo/fsrepo/fs_repo.h"
#include "ipfs/repo/init.h"
#include "ipfs/core/ipfs_node.h"
#include "libp2p/utils/logger.h"

/**
 * pull objects from ipfs
 */

/***
 * Helper method to retrieve a protobuf'd Node from the router
 * @param local_node the context
 * @param hash the hash to retrieve
 * @param hash_size the length of the hash
 * @param result a place to store the Node
 * @returns true(1) on success, otherwise false(0)
 */
int ipfs_exporter_get_node(struct IpfsNode* local_node, const unsigned char* hash, const size_t hash_size,
		struct HashtableNode** result) {
	unsigned char *buffer = NULL;
	size_t buffer_size = 0;
	int retVal = 0;
	struct Libp2pMessage* msg = NULL;

	if (local_node->routing->GetValue(local_node->routing, hash, hash_size, (void**)&buffer, &buffer_size)) {
		libp2p_logger_debug("exporter", "get_node got a value. Converting it to a HashtableNode\n");
		// unprotobuf
		if (ipfs_hashtable_node_protobuf_decode(buffer, buffer_size, result)) {
			libp2p_logger_debug("exporter", "Conversion to HashtableNode successful\n");
		}
	} else {
		libp2p_logger_debug("exporter", "get_node got no value. Returning false.\n");
		goto exit;
	}

	// copy in the hash
	(*result)->hash_size = hash_size;
	(*result)->hash = malloc(hash_size);
	memcpy((*result)->hash, hash, hash_size);

	retVal = 1;
	exit:
	if (buffer != NULL)
		free(buffer);
	if (msg != NULL)
		libp2p_message_free(msg);

	return retVal;
}

/***
 * Get a file by its hash, and write the data to a filestream
 * @param hash the base58 multihash of the cid
 * @param file_descriptor where to write
 * @param local_node the context
 */
int ipfs_exporter_to_filestream(const unsigned char* hash, FILE* file_descriptor, struct IpfsNode* local_node) {

	// convert hash to cid
	struct Cid* cid = NULL;
	if ( ipfs_cid_decode_hash_from_base58(hash, strlen((char*)hash), &cid) == 0) {
		return 0;
	}

	// find block
	struct HashtableNode* read_node = NULL;
	if (!ipfs_exporter_get_node(local_node, cid->hash, cid->hash_length, &read_node)) {
		ipfs_cid_free(cid);
		return 0;
	}

	// no longer need the cid
	ipfs_cid_free(cid);

	if (read_node->head_link == NULL) {
		// convert the node's data into a UnixFS data block
		struct UnixFS* unix_fs;
		ipfs_unixfs_protobuf_decode(read_node->data, read_node->data_size, &unix_fs);
		size_t bytes_written = fwrite(unix_fs->bytes, 1, unix_fs->bytes_size, file_descriptor);
		if (bytes_written != unix_fs->bytes_size) {
			ipfs_hashtable_node_free(read_node);
			ipfs_unixfs_free(unix_fs);
			return 0;
		}
		ipfs_unixfs_free(unix_fs);
	} else {
		struct NodeLink* link = read_node->head_link;
		struct HashtableNode* link_node = NULL;
		while (link != NULL) {
			if ( !ipfs_exporter_get_node(local_node, link->hash, link->hash_size, &link_node)) {
				ipfs_hashtable_node_free(read_node);
				return 0;
			}
			struct UnixFS* unix_fs;
			ipfs_unixfs_protobuf_decode(link_node->data, link_node->data_size, &unix_fs);
			size_t bytes_written = fwrite(unix_fs->bytes, 1, unix_fs->bytes_size, file_descriptor);
			if (bytes_written != unix_fs->bytes_size) {
				ipfs_hashtable_node_free(link_node);
				ipfs_hashtable_node_free(read_node);
				ipfs_unixfs_free(unix_fs);
				return 0;
			}
			ipfs_hashtable_node_free(link_node);
			ipfs_unixfs_free(unix_fs);
			link = link->next;
		}
	}

	if (read_node != NULL)
		ipfs_hashtable_node_free(read_node);

	return 1;
}


/**
 * get a file by its hash, and write the data to a file
 * @param hash the base58 multihash of the cid
 * @param file_name the file name to write to
 * @returns true(1) on success
 */
int ipfs_exporter_to_file(const unsigned char* hash, const char* file_name, struct IpfsNode *local_node) {
	// process blocks
	FILE* file = fopen(file_name, "wb");
	if (file == NULL) {
		return 0;
	}
	int retVal = ipfs_exporter_to_filestream(hash, file, local_node);
	fclose(file);
	return retVal;
}

/**
 * get a file by its hash, and write the data to a file
 * @param hash the base58 multihash of the cid
 * @param file_name the file name to write to
 * @returns true(1) on success
 */
int ipfs_exporter_to_console(const unsigned char* hash, struct IpfsNode *local_node) {
	// convert hash to cid
	struct Cid* cid = NULL;
	if ( ipfs_cid_decode_hash_from_base58(hash, strlen((char*)hash), &cid) == 0) {
		return 0;
	}

	// find block
	struct HashtableNode* read_node = NULL;
	if (!ipfs_exporter_get_node(local_node, cid->hash, cid->hash_length, &read_node)) {
		ipfs_cid_free(cid);
		return 0;
	}

	// no longer need the cid
	ipfs_cid_free(cid);

	// process blocks
	struct NodeLink* link = read_node->head_link;
	printf("{Links:[");
	while (link != NULL) {
		unsigned char b58[100];
		ipfs_cid_hash_to_base58(link->hash, link->hash_size, b58, 100);
		printf("{\"Name\":\"%s\",\"Hash\":\"%s\",\"Size\":%lu}", (link->name != NULL ? link->name : ""), (char*)b58, link->t_size);
		link = link->next;
	}
	printf("],\"Data\":\"");
	for(size_t i = 0LU; i < read_node->data_size; i++) {
		printf("%02x", read_node->data[i]);
	}
	printf("\"}\n");

	if (read_node != NULL)
		ipfs_hashtable_node_free(read_node);

	return 1;
}

/***
 * Called from the command line with ipfs object get [hash].
 * Retrieves the object pointed to by hash, and displays
 * its block data (links and data elements)
 * @param argc number of arguments
 * @param argv arguments
 * @returns true(1) on success
 */
int ipfs_exporter_object_get(int argc, char** argv) {
	char* repo_path = NULL;

	if (!ipfs_repo_get_directory(argc, argv, &repo_path)) {
		fprintf(stderr, "Unable to open repository: %s\n", repo_path);
		return 0;
	}

	struct IpfsNode* local_node = NULL;
	if (!ipfs_node_online_new(repo_path, &local_node))
		return 0;

	// find hash
	int retVal = ipfs_exporter_to_console((unsigned char*)argv[3], local_node);

	ipfs_node_free(local_node);

	return retVal;
}

int ipfs_exporter_cat_node(struct HashtableNode* node, struct IpfsNode* local_node, FILE *file) {
	// process this node, then move on to the links

	// build the unixfs
	struct UnixFS* unix_fs;
	ipfs_unixfs_protobuf_decode(node->data, node->data_size, &unix_fs);
	for(size_t i = 0LU; i < unix_fs->bytes_size; i++) {
		fprintf(file, "%c", unix_fs->bytes[i]);
	}
	ipfs_unixfs_free(unix_fs);
	// process links
	struct NodeLink* current = node->head_link;
	while (current != NULL) {
		// find the node
		struct HashtableNode* child_node = NULL;
		if (!ipfs_exporter_get_node(local_node, current->hash, current->hash_size, &child_node)) {
			return 0;
		}
		ipfs_exporter_cat_node(child_node, local_node, file);
		ipfs_hashtable_node_free(child_node);
		current = current->next;
	}

	return 1;
}

int ipfs_exporter_object_cat_to_file(struct IpfsNode *local_node, unsigned char* hash, int hash_size, FILE* file) {
	struct HashtableNode* read_node = NULL;

	// find block
	if (!ipfs_exporter_get_node(local_node, hash, hash_size, &read_node)) {
		return 0;
	}

	int retVal = ipfs_exporter_cat_node(read_node, local_node, file);
	ipfs_hashtable_node_free(read_node);
	return retVal;
}

/***
 * Called from the command line with ipfs cat [hash]. Retrieves the object
 * pointed to by hash, and displays its raw block data to the console
 * @param argc number of arguments
 * @param argv arguments
 * @returns true(1) on success
 */
int ipfs_exporter_object_cat(int argc, char** argv) {
	struct IpfsNode *local_node = NULL;
	char* repo_dir = NULL;

	if (!ipfs_repo_get_directory(argc, argv, &repo_dir)) {
		fprintf(stderr, "Unable to open repo: %s\n", repo_dir);
		return 0;
	}

	if (!ipfs_node_online_new(repo_dir, &local_node))
		return 0;

	// find hash
	// convert hash to cid
	struct Cid* cid = NULL;
	if ( ipfs_cid_decode_hash_from_base58((unsigned char*)argv[2], strlen(argv[2]), &cid) == 0) {
		return 0;
	}

	int retVal = ipfs_exporter_object_cat_to_file(local_node, cid->hash, cid->hash_length, stdout);
	ipfs_cid_free(cid);

	return retVal;

}

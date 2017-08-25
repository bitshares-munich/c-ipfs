#pragma once

#include "ipfs/core/ipfs_node.h"

/**
 * Pull bytes from the hashtable
 */

/**
 * get a file by its hash, and write the data to a file
 * @param hash the base58 multihash of the cid
 * @param file_name the file name to write to
 * @returns true(1) on success
 */
int ipfs_exporter_to_file(const unsigned char* hash, const char* file_name, struct IpfsNode* local_node);

/***
 * Retrieve a protobuf'd Node from the router
 * @param local_node the context
 * @param hash the hash to retrieve
 * @param hash_size the length of the hash
 * @param result a place to store the Node
 * @returns true(1) on success, otherwise false(0)
 */
int ipfs_exporter_get_node(struct IpfsNode* local_node, const unsigned char* hash, const size_t hash_size, struct HashtableNode** result);

int ipfs_exporter_object_get(int argc, char** argv);

/***
 * Called from the command line with ipfs cat [hash]. Retrieves the object pointed to by hash, and displays its block data (links and data elements)
 * @param argc number of arguments
 * @param argv arguments
 * @returns true(1) on success
 */
int ipfs_exporter_object_cat(int argc, char** argv);

/**
 * Retrieves the object pointed to by hash and displays the raw data
 * @param local_node the local node
 * @param hash the hash to use
 * @param hash_size the length of the hash
 * @param file the file descrptor to write to
 * @returns true(1) on success, false(0) otherwise
 */
int ipfs_exporter_object_cat_to_file(struct IpfsNode *local_node, unsigned char* hash, int hash_size, FILE* file);

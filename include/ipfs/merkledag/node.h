/**
 * An implementation of an IPFS node
 * Copying the go-ipfs-node project
 */
#ifndef IPFS_NODE_H
#define IPFS_NODE_H

#include "ipfs/cid/cid.h"

/*====================================================================================
 *
 * Structures
 *
 *===================================================================================*/

struct NodeLink
{
	size_t hash_size;
	unsigned char* hash;
	char* name;
	size_t t_size;
	struct NodeLink* next;
};

struct HashtableNode
{
	// saved in protobuf
	size_t data_size;
	unsigned char* data;
	struct NodeLink* head_link;
	// not saved in protobuf
	unsigned char* encoded;
	// a base32 representation of the multihash
	unsigned char* hash;
	size_t hash_size;
};

/*====================================================================================
 *
 * Functions
 *
 *===================================================================================*/

/*====================================================================================
 * Link Functions
 *===================================================================================*/

/* Create_Link
 * @Param name: The name of the link (char *)
 * @Param ahash: An Qmhash
 * @param hash_size the size of the hash
 * @param node_link a pointer to the new struct NodeLink
 * @returns true(1) on success
 */
int ipfs_node_link_create(char * name, unsigned char * ahash, size_t hash_size, struct NodeLink** node_link);

/****
 * Allocate memory for a new NodeLink
 * @param node_link a pointer to the newly allocated memory
 * @returns true(1) on success
 */
int ipfs_node_link_new(struct NodeLink** node_link);

/* ipfs_node_link_free
 * @param L: Free the link you have allocated.
 */
int ipfs_node_link_free(struct NodeLink * node_link);

/***
 * Node protobuf functions
 */

/***
 * Get the approximate size needed to protobuf encode this link
 * @param link the link to examine
 * @returns the maximum size that should be needed
 */
size_t ipfs_node_link_protobuf_encode_size(const struct NodeLink* link);

/***
 * Encode a NodeLink into protobuf format
 * @param link the link
 * @param buffer where to put the encoded results
 * @param max_buffer_length the max size that should be put in buffer
 * @pram bytes_written the amount of the buffer used
 * @returns true(1) on success
 */
int ipfs_node_link_protobuf_encode(const struct NodeLink* link, unsigned char* buffer, size_t max_buffer_length, size_t* bytes_written);

/****
 * Decode from a byte array into a NodeLink
 * @param buffer the byte array
 * @param buffer_length the length of the byte array
 * @param link the pointer to the new NodeLink (NOTE: Will be allocated in this function)
 * @param bytes_read the amount of bytes read by this function
 * @returns true(1) on success
 */
int ipfs_node_link_protobuf_decode(unsigned char* buffer, size_t buffer_length, struct NodeLink** link);


/***
 * return an approximate size of the encoded node
 * @param node the node to examine
 * @returns the max size of an encoded stream of bytes, if it were encoded
 */
size_t ipfs_hashtable_node_protobuf_encode_size(const struct HashtableNode* node);

/***
 * Encode a node into a protobuf byte stream
 * @param node the node to encode
 * @param buffer where to put it
 * @param max_buffer_length the length of buffer
 * @param bytes_written how much of buffer was used
 * @returns true(1) on success
 */
int ipfs_hashtable_node_protobuf_encode(const struct HashtableNode* node, unsigned char* buffer, size_t max_buffer_length, size_t* bytes_written);

/***
 * Decode a stream of bytes into a Node structure
 * @param buffer where to get the bytes from
 * @param buffer_length the length of buffer
 * @param node pointer to the Node to be created
 * @returns true(1) on success
 */
int ipfs_hashtable_node_protobuf_decode(unsigned char* buffer, size_t buffer_length, struct HashtableNode** node);

/*====================================================================================
 * Node Functions
 *===================================================================================*/

/****
 * Creates an empty node, allocates the required memory
 * @param node the pointer to the memory allocated
 * @returns true(1) on success, otherwise false(0)
 */
int ipfs_hashtable_node_new(struct HashtableNode** node);

/***
 * Allocates memory for a node, and sets the data section to indicate
 * that this node is a directory
 * @param node the node to initialize
 * @returns true(1) on success, otherwise false(0)
 */
int ipfs_hashtable_node_create_directory(struct HashtableNode** node);

/***
 * Determine if this node is actually a directory
 * @param node the node to examine
 * @returns true(1) if this node is a directory. Otherwise, false(0)
 */
int ipfs_hashtable_node_is_directory(struct HashtableNode* node);

/**
 * sets the Cid into the struct element titled cached
 * @param node the node to work with
 * @param cid the cid
 * @returns true(1) on success
 */
int ipfs_hashtable_node_set_hash(struct HashtableNode* node, const unsigned char* hash, size_t hash_size);

/*ipfs_node_set_data
 * Sets the data of a node
 * @param Node: The node which you want to set data in.
 * @param Data, the data you want to assign to the node
 * Sets pointers of encoded & cached to NULL /following go method
 * returns 1 on success 0 on failure
 */
int ipfs_hashtable_node_set_data(struct HashtableNode * N, unsigned char * Data, size_t data_size);

/*ipfs_node_set_encoded
 * @param NODE: the node you wish to alter (struct Node *)
 * @param Data: The data you wish to set in encoded.(unsigned char *)
 * returns 1 on success 0 on failure
 */
int ipfs_hashtable_node_set_encoded(struct HashtableNode * N, unsigned char * Data);

/*ipfs_node_get_data
 * Gets data from a node
 * @param Node: = The node you want to get data from. (unsigned char *)
 * Returns data of node.
 */
unsigned char * ipfs_hashtable_node_get_data(struct HashtableNode * N);

/*ipfs_node_free
 * Once you are finished using a node, always delete it using this.
 * It will take care of the links inside it.
 * @param N: the node you want to free. (struct Node *)
 */
int ipfs_hashtable_node_free(struct HashtableNode * N);

/*ipfs_node_get_link_by_name
 * Returns a copy of the link with given name
 * @param Name: (char * name) searches for link with this name
 * Returns the link struct if it's found otherwise returns NULL
 */
struct NodeLink * ipfs_hashtable_node_get_link_by_name(struct HashtableNode * N, char * Name);

/*ipfs_node_remove_link_by_name
 * Removes a link from node if found by name.
 * @param name: Name of link (char * name)
 * returns 1 on success, 0 on failure.
 */
int ipfs_hashtable_node_remove_link_by_name(char * Name, struct HashtableNode * mynode);

/* ipfs_node_add_link
 * Adds a link to your node
 * @param mynode: &yournode
 * @param mylink: the CID you want to create a node from
 * @param linksz: sizeof(your cid here)
 * Returns your node with the newly added link
 */
int ipfs_hashtable_node_add_link(struct HashtableNode * mynode, struct NodeLink * mylink);

/*ipfs_node_new_from_link
 * Create a node from a link
 * @param mylink: the link you want to create it from. (struct Cid *)
 * @param node the pointer to the new node
 * @returns true(1) on success
 */
int ipfs_hashtable_node_new_from_link(struct NodeLink * mylink, struct HashtableNode** node);

/*ipfs_node_new_from_data
 * @param data: bytes buffer you want to create the node from
 * @param data_size the size of the data
 * @param node the pointer to the new node
 * @returns true(1) on success
 */
int ipfs_hashtable_node_new_from_data(unsigned char * data, size_t data_size, struct HashtableNode** node);

/***
 * create a Node struct from encoded data
 * @param data: encoded bytes buffer you want to create the node from. Note: this copies the pointer, not a memcpy
 * @param node a pointer to the node that will be created
 * @returns true(1) on success
 */
int ipfs_hashtable_node_new_from_encoded(unsigned char * data, struct HashtableNode** node);

/*Node_Resolve_Max_Size
 * !!!This shouldn't concern you!
 *Gets the ammount of words that will be returned by Node_Resolve
 *@Param1: The string that will be processed (eg: char * sentence = "foo/bar/bin")
 *Returns either -1 if something went wrong or the ammount of words that would be processed.
*/
int Node_Resolve_Max_Size(char * input1);

/*Node_Resolve Basically stores everything in a pointer array eg: char * bla[Max_Words_]
 * !!!This shouldn't concern you!!!
 *@param1: Pointer array(char * foo[x], X=Whatever ammount there is. should be used with the helper function Node_Resolve_Max_Size)
 *@param2: Sentence to gather words/paths from (Eg: char * meh = "foo/bar/bin")
 *@Returns 1 or 0, 0 if something went wrong, 1 if everything went smoothly.
*/
int Node_Resolve(char ** result, char * input1);

/**************************************************************************************************************************************
 *|||||||||||||||||||||||||||||||||||||||| !!!! IMPORTANT !!! ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||*
 **************************************************************************************************************************************
 * Not sure where this is used, I'm making something to easen it up for all of you.
 * This in itself will get all the links for you in a link[] array inside Link_Proc
 * the memory allocation for storage will be noted in the ammount of links.
 * After being done with this, you have to free the array for which you will have a function specially made for you.
 *
 * What this does:
 * It searches for links using a path like /foo/bar/bin/, if links with those names are found in the node you specify, it stores them
 * in a custom struct, Link_Proc; which is what Node_Resolve_Link returns.
 * Notes:
 * Use it, free it, it's all already laid out for you.
 * There will also be a tutorial demonstrating it in the same folder here so everyone can understand this.
*/
struct Link_Proc
{
	char * remaining_links; // Not your concern.
	int ammount; //This will store the ammount of links, so you know what to process.
	struct NodeLink * links[]; // Link array
};

/*Node_Resolve_Links
 * Processes a path returning all links.
 * @param N: The node you want to get links from
 * @param path: The "foo/bar/bin" path
 */
struct Link_Proc * Node_Resolve_Links(struct HashtableNode * N, char * path);

/*Free_link_Proc
 * frees the Link_Proc struct you created.
 * @param1: Link_Proc struct (struct Link_Proc *)
 */
void Free_Link_Proc(struct Link_Proc * LPRC);

/*Node_Tree() Basically a unix-like ls
 *@Param1: Result char * foo[strlen(sentence)]
 *@Param2: char sentence[] = foo/bar/bin/whatever
 *Return: 0 if failure, 1 if success
*/
int Node_Tree(char * result, char * input1); //I don't know where you use this but here it is.

#endif

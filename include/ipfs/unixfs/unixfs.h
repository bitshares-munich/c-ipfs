/***
 * A unix-like file system over IPFS blocks
 */

#pragma once

/**
 * The protobuf info:
 * message Data {
 *  enum DataType {
 *		Raw = 0;
 *		Directory = 1;
 *		File = 2;
 *		Metadata = 3;
 *		Symlink = 4;
 *	}
 *
 *	required DataType Type = 1;
 *	optional bytes Data = 2;
 *	optional uint64 filesize = 3;
 *	repeated uint64 blocksizes = 4;
 * }
 *
 * message Metadata {
 *	optional string MimeType = 1;
 * }
 */

enum UnixFSDataType {
	UNIXFS_RAW,
	UNIXFS_DIRECTORY,
	UNIXFS_FILE,
	UNIXFS_METADATA,
	UNIXFS_SYMLINK
};

struct UnixFSBlockSizeNode {
	size_t block_size;
	struct UnixFSBlockSizeNode* next;
};

struct UnixFS {
	enum UnixFSDataType data_type;
	size_t bytes_size; // the size of the bytes array
	unsigned char* bytes; // an array of bytes
	size_t file_size; // when saving files that have been chunked
	struct UnixFSBlockSizeNode* block_size_head; // a linked list of block sizes
	unsigned char* hash; // not saved
	size_t hash_length; // not saved
};

struct UnixFSMetaData {
	char* mime_type;
};

/**
 * Allocate memory for a new UnixFS struct
 * @param obj the pointer to the new object
 * @returns true(1) on success
 */
int ipfs_unixfs_new(struct UnixFS** obj);

/***
 * Free the resources used by a UnixFS struct
 * @param obj the struct to free
 * @returns true(1)
 */
int ipfs_unixfs_free(struct UnixFS* obj);

/***
 * Write data to data section of a UnixFS stuct. NOTE: this also calculates a sha256 hash
 * @param data the data to write
 * @param data_length the length of the data
 * @param unix_fs the struct to add to
 * @returns true(1) on success
 */
int ipfs_unixfs_add_data(unsigned char* data, size_t data_length, struct UnixFS* unix_fs);

int ipfs_unixfs_add_blocksize(const struct UnixFSBlockSizeNode* blocksize, struct UnixFS* unix_fs);

/**
 * Protobuf functions
 */

/**
 * Calculate the max size of the protobuf before encoding
 * @param obj what will be encoded
 * @returns the size of the buffer necessary to encode the object
 */
size_t ipfs_unixfs_protobuf_encode_size(const struct UnixFS* obj);

/***
 * Encode a UnixFS object into protobuf format
 * @param incoming the incoming object
 * @param outgoing where the bytes will be placed
 * @param max_buffer_size the size of the outgoing buffer
 * @param bytes_written how many bytes were written in the buffer
 * @returns true(1) on success
 */
int ipfs_unixfs_protobuf_encode(const struct UnixFS* incoming, unsigned char* outgoing, size_t max_buffer_size, size_t* bytes_written);

/***
 * Decodes a protobuf array of bytes into a UnixFS object
 * @param incoming the array of bytes
 * @param incoming_size the length of the array
 * @param outgoing the UnixFS object
 */
int ipfs_unixfs_protobuf_decode(unsigned char* incoming, size_t incoming_size, struct UnixFS** outgoing);

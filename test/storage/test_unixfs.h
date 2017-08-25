#include "ipfs/unixfs/unixfs.h"

int test_unixfs_encode_decode() {
	struct UnixFS* unixfs = NULL;
	int retVal;

	// a directory
	retVal = ipfs_unixfs_new(&unixfs);
	unixfs->data_type = UNIXFS_DIRECTORY;

	// serialize
	size_t buffer_size = ipfs_unixfs_protobuf_encode_size(unixfs);
	unsigned char buffer[buffer_size];
	size_t bytes_written = 0;

	retVal = ipfs_unixfs_protobuf_encode(unixfs, buffer, buffer_size, &bytes_written);
	if (retVal == 0) {
		ipfs_unixfs_free(unixfs);
		return 0;
	}

	// unserialize
	struct UnixFS* results = NULL;
	retVal = ipfs_unixfs_protobuf_decode(buffer, bytes_written, &results);
	if (retVal == 0) {
		ipfs_unixfs_free(unixfs);
		return 0;
	}

	// compare
	if (results->data_type != unixfs->data_type) {
		ipfs_unixfs_free(unixfs);
		ipfs_unixfs_free(results);
		return 0;
	}

	if (results->block_size_head != unixfs->block_size_head) {
		ipfs_unixfs_free(unixfs);
		ipfs_unixfs_free(results);
		return 0;
	}

	ipfs_unixfs_free(unixfs);
	ipfs_unixfs_free(results);
	return 1;
}

int test_unixfs_encode_smallfile() {
	struct UnixFS* unixfs = NULL;
	ipfs_unixfs_new(&unixfs);

	unsigned char bytes[] = {
			0x54, 0x68, 0x69, 0x73, 0x20,
			0x69, 0x73, 0x20, 0x74, 0x65,
			0x78, 0x74, 0x20, 0x77, 0x69,
			0x74, 0x68, 0x69, 0x6e, 0x20,
			0x48, 0x65, 0x6c, 0x6c, 0x6f,
			0x57, 0x65, 0x72, 0x6c, 0x64,
			0x2e, 0x74, 0x78, 0x74, 0x0a };
	unsigned char expected_results[] = {
			0x08, 0x02, 0x12, 0x23,
			0x54, 0x68, 0x69, 0x73, 0x20,
			0x69, 0x73, 0x20, 0x74, 0x65,
			0x78, 0x74, 0x20, 0x77, 0x69,
			0x74, 0x68, 0x69, 0x6e, 0x20,
			0x48, 0x65, 0x6c, 0x6c, 0x6f,
			0x57, 0x65, 0x72, 0x6c, 0x64,
			0x2e, 0x74, 0x78, 0x74, 0x0a
	};

	unixfs->bytes = (unsigned char*)malloc(35);
	memcpy(unixfs->bytes, bytes, 35);
	unixfs->bytes_size = 35;
	unixfs->data_type = UNIXFS_FILE;

	size_t protobuf_size = 43;
	unsigned char protobuf[protobuf_size];
	size_t bytes_written;
	ipfs_unixfs_protobuf_encode(unixfs, protobuf, protobuf_size, &bytes_written);

	int retVal = 1;

	if (bytes_written != 39) {
		printf("Length should be %lu, but is %lu\n", 41LU, bytes_written);
		retVal = 0;
	}

	for(int i = 0; i < bytes_written; i++) {
		if (expected_results[i] != protobuf[i]) {
			printf("Byte at position %d should be %02x but is %02x\n", i, expected_results[i], protobuf[i]);
			retVal = 0;
		}
	}

	ipfs_unixfs_free(unixfs);

	return retVal;
}

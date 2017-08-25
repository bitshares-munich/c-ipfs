#include "ipfs/blocks/block.h"

int test_blocks_new() {
	const unsigned char* input = (const unsigned char*)"Hello, World!";
	int retVal = 0;
	struct Block* block = ipfs_block_new();
	if (block == NULL)
		return 0;

	retVal = ipfs_blocks_block_add_data(input, strlen((const char*)input) + 1, block);
	if (retVal == 0) {
		ipfs_block_free(block);
		return 0;
	}

	// now examine the block
	if (strcmp((const char*)block->data, (const char*)input) != 0) {
		ipfs_block_free(block);
		return 0;
	}

	if (block->data_length != strlen((const char*)input) + 1) {
		ipfs_block_free(block);
		return 0;
	}

	if (block->cid->codec != CID_PROTOBUF) {
		ipfs_block_free(block);
		return 0;
	}

	if (block->cid->version != 0) {
		ipfs_block_free(block);
		return 0;
	}

	if (block->cid->hash_length != 32) {
		ipfs_block_free(block);
		return 0;
	}

	unsigned char result_hash[32] = {33, 153, 66, 187, 124, 250, 87, 12, 12, 73, 43, 247, 175, 153, 10, 51, 192, 195, 218, 69, 220, 170, 105, 179, 195, 0, 203, 213, 172, 3, 244, 10 };
	for(int i = 0; i < 32; i++) {
		if (block->cid->hash[i] != result_hash[i]) {
			ipfs_block_free(block);
			return 0;
		}
	}

	retVal = ipfs_block_free(block);

	return 1;
}

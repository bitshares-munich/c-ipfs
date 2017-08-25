enum UnixFSFormatType { RAW, UNIXFS_FILE, DIRECTORY, METADATA, SYMLINK };

struct UnixFSData {
	enum UnixFSFormatType type;
	unsigned char* data;
	size_t data_length;
};

struct FSNode {
	unsigned char* data;
	size_t data_size;
	size_t* blocksizes;
	size_t subtotal;
	enum UnixFSFormatType type;
};

/**
 * Copy data into a new struct
 * @param type the type of data
 * @param data the acutal data
 * @param data_length the length of the data array
 * @param result where the struct will be stored
 * @returns true(1) on success
 */
int ipfs_unixfs_format_new(enum UnixFSFormatType type, const unsigned char* data, size_t data_length, struct UnixFSData** result);

/**
 * Free memory allocated by _new
 * @param data the struct
 * @returns true(1) on success
 */
int ipfs_unixfs_format_free(struct UnixFSData* data);

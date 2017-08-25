#include <string.h>

#include "mh/hashes.h"
#include "mh/multihash.h"

#include "ipfs/cid/cid.h"
#include "ipfs/multibase/multibase.h"

#include "libp2p/crypto/sha256.h"

int test_cid_new_free() {

	unsigned char* hash = (unsigned char*)"ABC123";

	struct Cid* cid = ipfs_cid_new(0, hash, strlen((char*)hash), CID_PROTOBUF);

	if (cid == NULL)
		return 0;

	if (cid->version != 0)
		return 0;

	if (cid->codec != CID_PROTOBUF)
		return 0;

	if (cid->hash_length != strlen((char*)hash))
		return 0;

	if (strncmp((char*)cid->hash, (char*)hash, 6) != 0)
		return 0;

	return ipfs_cid_free(cid);
}

/***
 * Test sending a multibase encoded multihash into cid_cast method
 * that should return a Cid struct
 */
int test_cid_cast_multihash() {
	// first, build a multihash
	char* string_to_hash = "Hello, World!";
	unsigned char hashed[32];
	memset(hashed, 0, 32);
	// hash the string
	libp2p_crypto_hashing_sha256((unsigned char*)string_to_hash, strlen(string_to_hash), hashed);
	size_t multihash_size = mh_new_length(MH_H_SHA2_256, 32);
	unsigned char multihash[multihash_size];
	memset(multihash, 0, multihash_size);
	unsigned char* ptr = multihash;

	int retVal = mh_new(ptr, MH_H_SHA2_256, hashed, 32);
	if (retVal < 0)
		return 0;

	// now call cast
	struct Cid cid;
	retVal = ipfs_cid_cast(multihash, multihash_size, &cid);
	if (retVal == 0)
		return 0;
	// check results
	if (cid.version != 0)
		return 0;
	if (cid.hash_length != 32)
		return 0;
	if (cid.codec != CID_PROTOBUF)
		return 0;
	if (strncmp((char*)hashed, (char*)cid.hash, 32) != 0)
		return 0;

	return 1;
}

int test_cid_cast_non_multihash() {
	// first, build a hash
	char* string_to_hash = "Hello, World!";
	unsigned char hashed[32];
	memset(hashed, 0, 32);
	// hash the string
	libp2p_crypto_hashing_sha256((unsigned char*)string_to_hash, strlen(string_to_hash), hashed);

	// now make it a hash with a version and codec embedded in varints before the hash
	size_t array_size = 34; // 32 for the hash, 2 for the 2 varints
	unsigned char array[array_size];
	memset(array, 0, array_size);
	// first the version
	array[0] = 0;
	// then the codec
	array[1] = CID_PROTOBUF;
	// then the hash
	memcpy(&array[2], hashed, 32);

	// now call cast
	struct Cid cid;
	int retVal = ipfs_cid_cast(array, array_size, &cid);
	if (retVal == 0)
		return 0;
	// check results
	if (cid.version != 0)
		return 0;
	if (cid.hash_length != 32)
		return 0;
	if (cid.codec != CID_PROTOBUF)
		return 0;
	if (strncmp((char*)hashed, (char*)cid.hash, 32) != 0)
		return 0;

	return 1;
}

int test_cid_protobuf_encode_decode() {
	struct Cid tester;
	tester.version = 1;
	tester.codec = CID_ETHEREUM_BLOCK;
	tester.hash = (unsigned char*)"ABC123";
	tester.hash_length = 6;
	size_t bytes_written_to_buffer;

	// encode
	size_t buffer_length = ipfs_cid_protobuf_encode_size(&tester);
	unsigned char buffer[buffer_length];
	ipfs_cid_protobuf_encode(&tester, buffer, buffer_length, &bytes_written_to_buffer);

	// decode
	struct Cid* results;
	ipfs_cid_protobuf_decode(buffer, bytes_written_to_buffer, &results);

	// compare
	if (tester.version != results->version) {
		printf("Version %d does not match version %d\n", tester.version, results->version);
		ipfs_cid_free(results);
		return 0;
	}

	if (tester.codec != results->codec) {
		printf("Codec %02x does not match %02x\n", tester.codec, results->codec);
		ipfs_cid_free(results);
		return 0;
	}

	if (tester.hash_length != results->hash_length) {
		printf("Hash length %d does not match %d\n", (int)tester.hash_length, (int)results->hash_length);
		ipfs_cid_free(results);
		return 0;
	}

	for(int i = 0; i < 6; i++) {
		if (tester.hash[i] != results->hash[i]) {
			printf("Hash character %c does not match %c at position %d", tester.hash[i], results->hash[i], i);
			ipfs_cid_free(results);
			return 0;
		}
	}

	ipfs_cid_free(results);
	return 1;
}

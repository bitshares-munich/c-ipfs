#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define __USE_ISOC11
#include <time.h>
#include "ipfs/namesys/routing.h"
#include "ipfs/util/time.h"
#include "mh/multihash.h"
#include "ipfs/namesys/pb.h"
#include "ipfs/namesys/namesys.h"
#include "ipfs/cid/cid.h"
#include "ipfs/path/path.h"
#include "libp2p/crypto/encoding/base58.h"

char* ipfs_routing_cache_get (char *key, struct ipns_entry *ientry)
{
    int i;
    struct routingResolver *cache;
    struct timespec now;

    if (key && ientry) {
        cache = ientry->cache;
        if (cache) {
            timespec_get (&now, TIME_UTC);
            for (i = 0 ; i < cache->next ; i++) {
                if (((now.tv_sec < cache->data[i]->eol.tv_sec ||
                     (now.tv_sec == cache->data[i]->eol.tv_sec && now.tv_nsec < cache->data[i]->eol.tv_nsec))) &&
                    strcmp(cache->data[i]->key, key) == 0) {
                    return cache->data[i]->value;
                }
            }
        }
    }
    return NULL;
}

void ipfs_routing_cache_set (char *key, char *value, struct ipns_entry *ientry)
{
    struct cacheEntry *n;
    struct routingResolver *cache;

    if (key && value && ientry) {
        cache = ientry->cache;
        if (cache && cache->next < cache->cachesize) {
            n = malloc(sizeof (struct cacheEntry));
            if (n) {
                n->key = key;
                n->value = value;
                timespec_get (&n->eol, TIME_UTC); // now
                n->eol.tv_sec += DefaultResolverCacheTTL; // sum TTL seconds to time seconds.
                cache->data[cache->next++] = n;
            }
        }
    }
}

// NewRoutingResolver constructs a name resolver using the IPFS Routing system
// to implement SFS-like naming on top.
// cachesize is the limit of the number of entries in the lru cache. Setting it
// to '0' will disable caching.
struct routingResolver* ipfs_namesys_new_routing_resolver (struct libp2p_routing_value_store *route, int cachesize)
{
    struct routingResolver *ret;

    if (!route) {
        fprintf(stderr, "attempt to create resolver with NULL routing system\n");
        exit (1);
    }

    ret = calloc (1, sizeof (struct routingResolver));

    if (!ret) {
        return NULL;
    }

    ret->data = calloc(cachesize, sizeof(void*));
    if (!ret) {
        free (ret);
        return NULL;
    }

    ret->cachesize = cachesize;
    return ret;
}

// ipfs_namesys_routing_resolve implements Resolver.
int ipfs_namesys_routing_resolve (char **path, char *name, struct namesys_pb *pb)
{
    return ipfs_namesys_routing_resolve_n(path, name, DefaultDepthLimit, pb);
}

// ipfs_namesys_routing_resolve_n implements Resolver.
int ipfs_namesys_routing_resolve_n (char **path, char *name, int depth, struct namesys_pb *pb)
{
    return ipfs_namesys_routing_resolve_once (path, name, depth, "/ipns/", pb);
}

/**
 * convert bytes to a hex string representation
 * @param bytes the bytes to convert
 * @param bytes_size the length of the bytes array
 * @param buffer where to put the results
 * @param buffer_length the length of the buffer
 * @returns 0 on success, otherwise an error code
 */
int ipfs_namesys_bytes_to_hex_string(const unsigned char* bytes, size_t bytes_size, char* buffer, size_t buffer_length) {
	if (bytes_size * 2 > buffer_length) {
		return ErrInvalidParam;
	}
	for(size_t i = 0; i < bytes_size; i++) {
		sprintf(&buffer[i*2], "%02x", bytes[i]);
	}
	return 0;
}

/***
 * Convert a hex string to an array of bytes
 * @param hex a null terminated string of bytes in 2 digit hex format
 * @param buffer where to put the results. NOTE: this allocates memory
 * @param buffer_length the size of the buffer that was allocated
 * @returns 0 on success, otherwise an error code
 */
int ipfs_namesys_hex_string_to_bytes(const unsigned char* hex, unsigned char** buffer, size_t* buffer_length) {
	size_t hex_size = strlen((char*)hex);
	char* pos = (char*)hex;

	// sanity check
	if (hex_size % 2 != 0)
		return ErrInvalidParam;

	// allocate memory
	*buffer = (unsigned char*)malloc( hex_size / 2 );
	unsigned char* ptr = *buffer;

	// convert string
	for(size_t i = 0; i < hex_size; i++) {
		sscanf(pos, "%2hhx", &ptr[i]);
		pos += 2;
	}

	return 0;
}

/***
 * Implements resolver. Uses the IPFS routing system to resolve SFS-like names.
 *
 * @param path the path
 * @param name the name (b58 encoded)
 * @param depth
 * @param depth
 * @param prefix
 * @param pb
 * @returns 0 on success, otherwise error code
 */
int ipfs_namesys_routing_resolve_once (char **path, char *name, int depth, char *prefix, struct namesys_pb *pb)
{
    int err, l, s, ok;
    unsigned char* multihash = NULL;
    size_t multihash_size = 0;
    char *h, *string, val[8];
    char pubkey[60];

    if (!path || !name || !prefix) {
        return ErrInvalidParam;
    }
    // log.Debugf("RoutingResolve: '%s'", name)
    *path = ipfs_routing_cache_get (name, pb->IpnsEntry);
    if (*path) {
        return 0; // cached
    }

    if (memcmp(name, prefix, strlen(prefix)) == 0) {
        name += strlen (prefix); // trim prefix.
    }

    // turn the b58 encoded name into a multihash
    err = libp2p_crypto_encoding_base58_decode((unsigned char*)name, strlen(name), &multihash, &multihash_size);
    if (!err) {
        // name should be a multihash. if it isn't, error out here.
        //log.Warningf("RoutingResolve: bad input hash: [%s]\n", name)
    	if (multihash != NULL)
    		free(multihash);
        return ErrInvalidParam;
    }

    // use the routing system to get the name.
    // /ipns/<name>
    l = strlen(prefix);
    s = (multihash_size * 2) + 1;
    h = malloc(l + s); // alloc to fit prefix + hexhash + null terminator
    if (!h) {
   		free(multihash);
        return ErrAllocFailed;
    }
    memcpy(h, prefix, l); // copy prefix
    if (ipfs_namesys_bytes_to_hex_string(multihash, multihash_size, h+l, s)) { // hexstring just after prefix.
   		free(multihash);
   		free(h);
        return ErrUnknow;
    }

    err = ipfs_namesys_routing_get_value (val, h);
	free(h); // no longer needed
    if (err) {
        //log.Warning("RoutingResolve get failed.")
   		free(multihash);
        return err;
    }


    //err = protobuf decode (val, pb.IpnsEntry);
    //if (err) {
    //    return err;
    //}

    // name should be a public key retrievable from ipfs
    err = ipfs_namesys_routing_getpublic_key (pubkey, multihash, multihash_size);
	free(multihash); // done with multihash for now
	multihash = NULL;
	multihash_size = 0;
    if (err) {
        return err;
    }

    // TODO: implement libp2p_crypto_verify
    // check sig with pk
    err = libp2p_crypto_verify(ipns_entry_data_for_sig(pb->IpnsEntry), pb->IpnsEntry->signature, &ok);
    if (err || !ok) {
        char buf[500];
        snprintf(buf, sizeof(buf), Err[ErrInvalidSignatureFmt], pubkey);
        l = strlen(buf) + 1;
        Err[ErrInvalidSignature] = malloc(l);
        if (!Err[ErrInvalidSignature]) {
            return ErrAllocFailed;
        }
        memcpy(Err[ErrInvalidSignature], buf, l);
        return ErrInvalidSignature;
    }
    // ok sig checks out. this is a valid name.

    // check for old style record:
    err = ipfs_namesys_pb_get_value (&string, pb->IpnsEntry);
    if (err) {
   		free(multihash);
        return err;
    }
    err = ipfs_namesys_hex_string_to_bytes((unsigned char*)string, &multihash, &multihash_size);
    if (err) {
    	if (multihash != NULL)
    		free(multihash);
    	return ErrInvalidParam;
    }
    err = mh_multihash_hash(multihash, multihash_size);
    if (err < 0) {
        // Not a multihash, probably a new record
        err = ipfs_path_parse(*path, string);
        if (err) {
        	free(multihash);
            return err;
        }
    } else {
        // Its an old style multihash record
        //log.Warning("Detected old style multihash record")
        struct Cid *cid = ipfs_cid_new(0, multihash, multihash_size, CID_PROTOBUF);
        if (cid == NULL) {
            free(multihash);
            return 0;
        }

        err = ipfs_path_parse_from_cid (*path, (char*)cid->hash);
        if (err) {
            free(multihash);
        	ipfs_cid_free(cid);
            return err;
        }
        ipfs_cid_free(cid);
    }
    free(multihash);

    ipfs_routing_cache_set (name, *path, pb->IpnsEntry);

    return 0;
}

int ipfs_namesys_routing_check_EOL (struct timespec *ts, struct namesys_pb *pb)
{
    int err;

    if (*(pb->IpnsEntry->validityType) == IpnsEntry_EOL) {
        err = ipfs_util_time_parse_RFC3339 (ts, pb->IpnsEntry->validity);
        if (!err) {
            return 1;
        }
    }
    return 0;
}

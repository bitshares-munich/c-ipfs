#include <stdlib.h>
#include <string.h>

#include "ipfs/repo/fsrepo/fs_repo.h"

#define IPFS_PIN_C
#include "ipfs/pin/pin.h"

#include "ipfs/cid/cid.h"
#include "ipfs/datastore/key.h"
#include "ipfs/merkledag/merkledag.h"
#include "ipfs/util/errs.h"

// package pin implements structures and methods to keep track of
// which objects a user wants to keep stored locally.

#define PIN_DATASTOREKEY_SIZE 100
char *pinDatastoreKey = NULL;
size_t pinDatastoreKeySize = 0;

struct Cid *emptyKey = NULL;

int ipfs_pin_init ()
{
    int err;
    unsigned char *empty_hash = (unsigned char*) "QmdfTbBqBPQ7VNxZEYEj14VmRuZBkqFbiwReogJgS1zR1n";

    if (!pinDatastoreKey) { // initialize just one time.
        pinDatastoreKey = malloc(PIN_DATASTOREKEY_SIZE);
        if (!pinDatastoreKey) {
            return ErrAllocFailed;
        }
        err = ipfs_datastore_key_new("/local/pins", pinDatastoreKey, PIN_DATASTOREKEY_SIZE, &pinDatastoreKeySize);
        if (err) {
            free (pinDatastoreKey);
            pinDatastoreKey = NULL;
            return err;
        }

        if (!ipfs_cid_protobuf_decode(empty_hash, strlen ((char*)empty_hash), &emptyKey)) {
            return ErrCidDecodeFailed;
        }
    }

    return 0;
}

// Return pointer to string or NULL if invalid.
char *ipfs_pin_mode_to_string (PinMode mode)
{
    if (mode < 0 || mode >= (sizeof (ipfs_pin_linkmap) / sizeof (void*))) {
        return NULL;
    }
    return (char*)ipfs_pin_linkmap[mode];
}

// Return array index or -1 if fail.
PinMode ipfs_string_to_pin_mode (char *str)
{
    PinMode pm;

    for (pm = 0 ; pm < (sizeof (ipfs_pin_linkmap) / sizeof (void*)) ; pm++) {
        if (strcmp(ipfs_pin_linkmap[pm], str) == 0) {
            return pm;
        }
    }
    return -1; // not found.
}

int ipfs_pin_is_pinned (struct Pinned *p)
{
    return (p->Mode != NotPinned);
}

char *ipfs_pin_pinned_msg (struct Pinned *p)
{
    char *ret, *ptr, *msg;

    switch (p->Mode) {
        case NotPinned:
            msg = "not pinned";
            ret = malloc(strlen (msg) + 1);
            if (!ret) {
                return NULL;
            }
            strcpy(ret, msg);
            break;
        case Indirect:
            msg = "pinned via ";
            ret = malloc(strlen (msg) + p->Via->hash_length + 1);
            if (!ret) {
                return NULL;
            }
            ptr = ret;
            memcpy(ptr, msg, strlen(msg));
            ptr += strlen(msg);
            memcpy(ptr, p->Via->hash, p->Via->hash_length);
            ptr += p->Via->hash_length;
            *ptr = '\0';
            break;
        default:
            ptr = ipfs_pin_mode_to_string (p->Mode);
            if (!ptr) {
                return NULL;
            }
            msg = "pinned: ";
            ret = malloc(strlen (msg) + strlen (ptr) + 1);
            if (!ret) {
                return NULL;
            }
            strcpy(ret, msg);
            strcat(ret, ptr);
    }
    return ret;
}

// Find out if the child is in the hash.
int ipfs_pin_has_child (struct FSRepo *ds,
                        unsigned char *hash,  size_t hash_size,
                        unsigned char *child, size_t child_size)
{
    struct HashtableNode *node;
    struct NodeLink *node_link;

    if (ipfs_merkledag_get (hash, hash_size, &node, ds)) {
        if (node) {
            if ((node->hash_size == child_size) &&
                (memcmp (node->hash, child, child_size) == 0)) {
                return 1;
            }
            // browse the node links.
            for (node_link = node->head_link ; node_link ; node_link = node_link->next) {
                if ((node_link->hash_size == child_size) &&
                    (memcmp (node_link->hash, child, child_size) != 0) &&
                     ipfs_pin_has_child (ds, node_link->hash, node_link->hash_size,
                                             child, child_size)) {
                    return 1;
                }
                if ((node_link->hash_size == child_size) &&
                    (memcmp (node_link->hash, child, child_size) == 0)) {
                    return 1; // child is a child of the hash node.
                }
            }
        }
    }
    return 0;
}

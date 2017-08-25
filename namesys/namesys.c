#include <string.h>
#include <stdlib.h>
#ifndef __USE_ISOC11
#define __USE_ISOC11
#endif
#include <time.h>
#include "ipfs/cid/cid.h"
#include "ipfs/path/path.h"
#include "ipfs/namesys/namesys.h"
#include "ipfs/dnslink/dnslink.h"

/* mpns (a multi-protocol NameSystem) implements generic IPFS naming.
 *
 * Uses several Resolvers:
 * (a) IPFS routing naming: SFS-like PKI names.
 * (b) dns domains: resolves using links in DNS TXT records
 * (c) proquints: interprets string as the raw byte data.
 *
 * It can only publish to: (a) IPFS routing naming.
*/

mpns **ns;
// NewNameSystem will construct the IPFS naming system based on Routing
/*
func NewNameSystem(r routing.ValueStore, ds ds.Datastore, cachesize int) NameSystem {
	return &mpns{
		resolvers: map[string]resolver{
			"dns":      newDNSResolver(),
			"proquint": new(ProquintResolver),
			"dht":      NewRoutingResolver(r, cachesize),
		},
		publishers: map[string]Publisher{
			"/ipns/": NewRoutingPublisher(r, ds),
		},
	}
}*/

const int DefaultResolverCacheTTL = 60;

// ipfs_namesys_resolve implements Resolver.
int ipfs_namesys_resolve(char **path, char *name)
{
    return ipfs_namesys_resolve_n(path, name, DefaultDepthLimit);
}

// ipfs_namesys_resolve_n implements Resolver.
int ipfs_namesys_resolve_n(char **path, char *name, int depth)
{
    char ipfs_prefix[] = "/ipfs/";
    char p[500];
    int err = 0;;

    if (memcmp(name, ipfs_prefix, strlen(ipfs_prefix)) == 0) {
        ipfs_path_parse(p, name);
        *path = malloc(strlen(p) + 1);
        if (*p) {
            memcpy(*path, p, strlen(p) + 1);
        } else {
            err = ErrAllocFailed;
        }
        return err;
    }

    if (*name == '/') {
        int err, l;
        char *str;

        l = sizeof(ipfs_prefix) + strlen(name);
        str = malloc(l);
        if (!str) {
            return ErrAllocFailed;
        }
        str[--l] = '\0';
        strncpy(str, ipfs_prefix, l);
        strncat(str, name+1, l -  strlen (str)); // ignore inital / from name, because ipfs_prefix already has it.
        err = ipfs_path_parse(p, str); // save return value.
        free (str);              // so we can free allocated memory before return.
        *path = malloc(strlen(p) + 1);
        if (*p) {
            memcpy(*path, p, strlen(p) + 1);
        } else {
            err = ErrAllocFailed;
        }
        return err;
    }

    return ipfs_namesys_resolve(path, name);
}

// ipfs_namesys_resolve_once implements resolver.
int ipfs_namesys_resolve_once (char **path, char *name)
{
    char ipns_prefix[] = "/ipns/";
    char *ptr = NULL;
    char **segs;
    int i, err = 0;

    if (!name) { // NULL pointer.
        return ErrNULLPointer;
    }

    if (memcmp (name, ipns_prefix, strlen(ipns_prefix)) == 0) { // prefix missing.
        i = strlen(name) + sizeof(ipns_prefix);
        ptr = malloc(i);
        if (!ptr) { // allocation fail.
            return ErrAllocFailed;
        }
        ptr[--i] = '\0';
        strncpy(ptr, ipns_prefix, i);
        strncat(ptr, name, i - strlen(ptr));
        segs = ipfs_path_split_segments(ptr);
        free (ptr);
    } else {
        segs = ipfs_path_split_segments(name);
    }

    if (!segs || ipfs_path_segments_length(segs) < 2) {
        //log.Warningf("Invalid name syntax for %s", name);
        return ErrResolveFailed;
    }

    for (i = 0 ; ns[i] ; i++) {
        char *p;
        //log.Debugf("Attempting to resolve %s with %s", segments[1], ns[i]->resolver->protocol);
        err = ns[i]->resolver->func(&p, segs[1]);
        if (!err) {
            if (ipfs_path_segments_length(segs) > 2) {
                *path = ipfs_path_from_segments(p, segs+2);
            } else {
                *path = p;
            }
            return 0;
        }
    }
    //log.Warningf("No resolver found for %s", name);
    return ErrResolveFailed;
}

// ipfs_namesys_publish implements Publisher
int ipfs_namesys_publish (char *proto, ciPrivKey name, char *value)
{
    int i;

    for (i = 0 ; ns[i] ; i++) {
        if (strcmp(ns[i]->publisher->protocol, proto)==0) {
            return ns[i]->publisher->func(name, value);
        }
    }
    return ErrPublishFailed;
}

int ipfs_namesys_publish_with_eol (char *proto, ciPrivKey name, char *value, time_t eol)
{
    int i;

    for (i = 0 ; ns[i] ; i++) {
        if (strcmp(ns[i]->publisher->protocol, proto)==0) {
            return ns[i]->publisher->func_eol(name, value, eol);
        }
    }
    return ErrPublishFailed;
}

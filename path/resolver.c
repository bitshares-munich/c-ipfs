#include "ipfs/cid/cid.h"
#include "ipfs/path/path.h"

Resolver* ipfs_path_new_basic_resolver (DAGService *ds)
{
    Resolver *ret = malloc(sizeof(Resolver));
    if (!ret) return NULL;
    ret->DAG = ds;
    ret->ResolveOnce = ipfs_path_resolve_single;
    return ret;
}

// ipfs_path_split_abs_path clean up and split fpath. It extracts the first component (which
// must be a Multihash) and return it separately.
int ipfs_path_split_abs_path (struct Cid* cid, char ***parts, char *fpath)
{
    *parts = ipfs_path_split_segments(fpath);

    if (strcmp (**parts, "ipfs") == 0) *parts++;

    // if nothing, bail.
    if (!**parts) return ErrNoComponents;

    // first element in the path is a cid
    cid_decode_from_string(**parts, strlen(**parts), cid);
    return 0;
}

// ipfs_path_resolve_path fetches the node for given path. It returns the last item
// returned by ipfs_path_resolve_path_components.
int ipfs_path_resolve_path(Node **nd, Context ctx, Resolver *s, char *fpath)
{
    int err = IsValid(fpath);
    Node **ndd;

    if (err) {
        return err;
    }
    err = ipfs_path_resolve_path_components(&ndd, ctx, s, fpath);
    if (err) {
        return err;
    }
    if (ndd == NULL) {
        return ErrBadPath;
    }
    while(*ndd) {
        *nd = *ndd;
        ndd++;
    }
    return 0;
}

int ipfs_path_resolve_single(NodeLink **lnk, Context ctx, DAGService *ds, Node **nd, char *name)
{
    return ipfs_path_resolve_link(lnk, name);
}

// ipfs_path_resolve_path_components fetches the nodes for each segment of the given path.
// It uses the first path component as a hash (key) of the first node, then
// resolves all other components walking the links, with ipfs_path_resolve_links.
int ipfs_path_resolve_path_components(Node ***nd, Context ctx, Resolver *s, char *fpath)
{
    int err;
    struct Cid h;
    char **parts;

    err = ipfs_path_split_abs_path(&h, &parts, fpath);
    if (err) {
        return err;
    }

    //log.Debug("resolve dag get");
    //*nd = s->DAG.Get(ctx, h);
    //if (nd == DAG_ERR_VAL) {
    //   return DAG_ERR_VAL;
    //}

    return ipfs_path_resolve_links(ctx, *nd, parts);
}

// ipfs_path_resolve_links iteratively resolves names by walking the link hierarchy.
// Every node is fetched from the DAGService, resolving the next name.
// Returns the list of nodes forming the path, starting with ndd. This list is
// guaranteed never to be empty.
//
// ipfs_path_resolve_links(nd, []string{"foo", "bar", "baz"})
// would retrieve "baz" in ("bar" in ("foo" in nd.Links).Links).Links
int ipfs_path_resolve_links(Node ***result, Context ctx, Node *ndd, char **names)
{
    int err, idx = 0, l;
    NodeLink *lnk;
    Node *nd;

    *result = calloc (sizeof(Node*), ipfs_path_segments_length(names) + 1);
    if (!*result) {
        return -1;
    }
    memset (*result, NULL, sizeof(Node*) * (ipfs_path_segments_length(names)+1));

    *result[idx++] = ndd;
    nd = ndd; // dup arg workaround

    while (*names) {
        //TODO
        //var cancel context.CancelFunc
        //ctx, cancel = context.WithTimeout(ctx, time.Minute)
        //defer cancel()

        // for each of the path components
        err = ipfs_path_resolve_link(&lnk, *names);
        if (err) {
            char msg[51];
            *result[idx] = NULL;
            snprintf(msg, sizeof(msg), ErrPath[ErrNoLinkFmt], *names, nd->Cid);
            if (ErrPath[ErrNoLink]) {
                free(ErrPath[ErrNoLink]);
            }
            l = strlen(msg) + 1;
            ErrPath[ErrNoLink] = malloc(l);
            if (ErrPath[ErrNoLink]) {
                memcpy(ErrPath[ErrNoLink], msg, l);
            }
            free (*result);
            return ErrNoLink;
        }
        names++;
    }
    return 0;
}

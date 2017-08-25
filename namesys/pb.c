#include <string.h>
#include <stdlib.h>
#include "ipfs/namesys/routing.h"
#include "ipfs/namesys/pb.h"

int IpnsEntry_ValidityType_value (char *s)
{
    int r;

    if (!s) {
        return -1; // invalid.
    }

    for (r = 0 ; IpnsEntry_ValidityType_name[r] ; r++) {
        if (strcmp (IpnsEntry_ValidityType_name[r], s) == 0) {
            return r; // found
        }
    }

    return -1; // not found.
}

struct ipns_entry* ipfs_namesys_pb_new_ipns_entry ()
{
    return calloc(1, sizeof (struct ipns_entry));
}

void ipfs_namesys_ipnsentry_reset (struct ipns_entry *m)
{
    if (m) {
        // ipns_entry is an struct of pointers,
        // so we can access as an array of pointers.
        char **a = (char **)m;
        int i, l = (sizeof (struct ipns_entry) / sizeof (void*)) - 2; // avoid last 2 pointers, cache and eol.
        for (i = 0 ; i < l ; i++) {
            if (a[i]) {
                free (a[i]); // free allocated pointers,
                a[i] = NULL; // and mark as free.
            }
        }
    }
}

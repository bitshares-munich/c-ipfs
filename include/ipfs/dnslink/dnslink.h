#ifndef DNSLINK_H
   #define DNSLINK_H

   #include "ipfs/util/errs.h"

   // DefaultDepthLimit controls how many dns links to resolve through before
   // returning. Users can override this default.
   #ifndef DefaultDepthLimit
      #define DefaultDepthLimit 16
   #endif
   // MaximumDepthLimit governs the max number of recursive resolutions.
   #ifndef MaximumDepthLimit
      #define MaximumDepthLimit 256
   #endif

   #ifndef IPFS_DNSLINK_C
      extern int (*ipfs_dnslink_lookup_txt)(char ***, char *);
   #endif // IPFS_DNSLINK_C

   int ipfs_dns (int argc, char **argv);
   int ipfs_dnslink_resolve (char **p, char *domain);
   int ipfs_dnslink_resolve_n (char **p, char *d, int depth);
   int ipfs_dnslink_resolv_lookupTXT(char ***txt, char *domain);
   int ipfs_dnslink_resolve_once (char ***p, char *domain);
   int ipfs_dnslink_parse_txt (char **path, char *txt);
   int ipfs_dnslink_parse_link_domain (char **domain, char**rest, char *txt);
#endif // DNSLINK_H

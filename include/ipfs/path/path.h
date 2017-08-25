#ifndef IPFS_PATH_H
   #define IPFS_PATH_H

   #include "ipfs/util/errs.h"

   char* ipfs_path_from_cid (struct Cid *c);
   char** ipfs_path_split_n (char *p, char *delim, int n);
   char** ipfs_path_split_segments (char *p);
   int ipfs_path_segments_length (char **s);
   void ipfs_path_free_segments (char ***s);
   char *ipfs_path_clean_path(char *str);
   int ipfs_path_is_just_a_key (char *p);
   int ipfs_path_pop_last_segment (char **str, char *p);
   char *ipfs_path_from_segments(char *prefix, char **seg);
   int ipfs_path_parse_from_cid (char *dst, char *txt);
   int ipfs_path_parse (char *dst, char *txt);
   int ipfs_path_is_valid (char *p);
#endif // IPFS_PATH_H

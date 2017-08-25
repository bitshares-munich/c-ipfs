#include <string.h>
#include <stdlib.h>
#include <ipfs/cid/cid.h>
#include <ipfs/path/path.h>

#include <arpa/inet.h>

// FromCid safely converts a cid.Cid type to a Path type
char* ipfs_path_from_cid (struct Cid *c)
{
   const char prefix[] = "/ipfs/";
   char *rpath, *cidstr = (char*)c->hash;
   int l;

   l = sizeof(prefix) + strlen(cidstr);
   rpath = malloc(l);
   if (!rpath) return NULL;
   rpath[--l] = '\0';
   strncpy(rpath, prefix, l);
   strncat(rpath, cidstr, l - strlen(rpath));
   return rpath;
}

char** ipfs_path_split_n (char *p, char *delim, int n)
{
   char *c, **r, *rbuf;
   int i, dlen = strlen(delim);

   if (n == 0) {
      return NULL; // no split?
   }

   if (n < 0) { // negative, count all delimiters + 1.
      for (c = p , n = 0 ; c ; n++) {
         c = strstr(c, delim);
         if (c) {
            c += dlen;
         }
      }
   } else {
      n++; // increment param value.
   }

   rbuf = malloc(strlen(p) + 1);
   if (!rbuf) {
      return NULL;
   }

   r = calloc(sizeof(char*), n + 1); // splits plus NULL pointer termination
   if (!r) {
      free(rbuf);
      return NULL;
   }

   memcpy(rbuf, p, strlen(p) + 1); // keep original
   for (c = rbuf, i = 0 ; i < n && c ; i++) {
      r[i] = c;
      c = strstr(c, delim);
      if (c) {
         *c = '\0';
         c += dlen;
      }
   }
   r[i] = NULL;

   return r;
}

char** ipfs_path_split_segments (char *p)
{
   if (*p == '/') p++; // Ignore leading slash

   return ipfs_path_split_n (p, "/", -1);
}

// Count segments
int ipfs_path_segments_length (char **s)
{
   int r = 0;

   if (s) {
      while (s[r]) r++;
   }

   return r;
}

// free memory allocated by ipfs_path_split_segments
void ipfs_path_free_segments (char ***s)
{
   if (*s && **s) {
      free(**s); // free string buffer
      free(*s); // free array
      *s = NULL;
   }
}

// ipfs_path_clean_path returns the shortest path name equivalent to path
// by purely lexical processing. It applies the following rules
// iteratively until no further processing can be done:
//
//	1. Replace multiple slashes with a single slash.
//	2. Eliminate each . path name element (the current directory).
//	3. Eliminate each inner .. path name element (the parent directory)
//	   along with the non-.. element that precedes it.
//	4. Eliminate .. elements that begin a rooted path:
//	   that is, replace "/.." by "/" at the beginning of a path.
//
// The returned path ends in a slash only if it is the root "/".
//
// If the result of this process is an empty string, Clean
// returns the string ".".
//
// See also Rob Pike, ``Lexical File Names in Plan 9 or
// Getting Dot-Dot Right,''
// https://9p.io/sys/doc/lexnames.html
char *ipfs_path_clean_path(char *str)
{
    char *buf;
    char **path, **p, *r, *s;
    int l;

    l = strlen(str)+3;
    r = buf = malloc(l);
    *r = '\0';
    if (!r) {
        return NULL;
    }
    buf[--l] = '\0';

    path = ipfs_path_split_n(str, "/", -1);
    if (!path) {
        free(r);
        return NULL;
    }
    p = path;

    for (p = path; *p ; p++) {
        if (**p == '\0' && r[0] == '\0') {
            strncpy(r, "/", l);
        } else {
            if (strcmp(*p, "..") == 0) {
                s = strrchr(r, '/');
                if (s && s > r) {
                    *s = '\0';
                }
            } else if (**p != '\0' && **p != '/' && strcmp(*p, ".")) {
                if (*r == '\0') {
                    strncat(r, "./", l - strlen(r));
                } else if (r[strlen(r)-1] != '/') {
                    strncat(r, "/", l - strlen(r));
                }
                strncat(r, *p, l - strlen(r));
            }
        }
    }

    ipfs_path_free_segments(&path);

    l = strlen(buf)+1;
    r = malloc(l);
    if (!r) {
        free(buf);
        return NULL;
    }
    strncpy(r, buf, l);
    free(buf);

    return r;
}

// ipfs_path_is_just_a_key returns true if the path is of the form <key> or /ipfs/<key>.
int ipfs_path_is_just_a_key (char *p)
{
   char **parts;
   int ret = 0;
   parts = ipfs_path_split_segments (p);
   if (parts) {
      if (ipfs_path_segments_length (parts) == 2 && strcmp (parts[0], "ipfs") == 0) ret++;
      ipfs_path_free_segments(&parts);
   }
   return ret;
}

// ipfs_path_pop_last_segment returns a new Path without its final segment, and the final
// segment, separately. If there is no more to pop (the path is just a key),
// the original path is returned.
int ipfs_path_pop_last_segment (char **str, char *p)
{
   if (ipfs_path_is_just_a_key(p)) return 0;
   *str = strrchr(p, '/');
   if (!*str) return ErrBadPath; // error
   **str = '\0';
   (*str)++;
   return 0;
}

char *ipfs_path_from_segments(char *prefix, char **seg)
{
   int retlen, i;
   char *ret;

   if (!prefix || !seg) return NULL;

   retlen = strlen(prefix);
   for (i = 0 ; seg[i] ; i++) {
      retlen += strlen(seg[i]) + 1; // count each segment length + /.
   }

   ret = malloc(retlen + 1); // allocate final string size + null terminator.
   if (!ret) return NULL;

   ret[retlen] = '\0';
   strncpy(ret, prefix, retlen);
   for (i = 0 ; seg[i] ; i++) {
      strncat(ret, "/", retlen - strlen(ret));
      strncat(ret, seg[i], retlen - strlen(ret));
   }
   return ret;
}

int ipfs_path_parse_from_cid (char *dst, char *txt)
{
   struct Cid *c = NULL;
   char *r;

   if (!txt || txt[0] == '\0') return ErrNoComponents;

   //c = cidDecode(txt);

   if (!c) {
      return ErrCidDecode;
   }

   r = ipfs_path_from_cid(c);

   if (!r) {
      return ErrCidDecode;
   }
   memcpy (dst, r, strlen(r) + 1);
   free (r);
   return 0;
}

int ipfs_path_parse (char *dst, char *txt)
{
   int err, i;
   char *c;
   const char prefix[] = "/ipfs/";
   const int plen = strlen(prefix);

   if (!txt || txt[0] == '\0') return ErrNoComponents;

   if (*txt != '/' || strchr (txt+1, '/') == NULL) {
      if (*txt == '/') {
         txt++;
      }
      err = ipfs_path_parse_from_cid (dst+plen, txt);
      if (err == 0) { // only change dst if ipfs_path_parse_from_cid returned success.
         // Use memcpy instead of strncpy to avoid overwriting
         // result of ipfs_path_parse_from_cid with a null terminator.
         memcpy (dst, prefix, plen);
      }
      return err;
   }

   c = txt;
   for (i = 0 ; (c = strchr(c, '/')) ; i++) c++;
   if (i < 3) return ErrBadPath;

   if (strcmp (txt, prefix) == 0) {
      char *buf;
      i = strlen(txt+6);
      buf = malloc(i + 1);
      if (!buf) {
          return ErrAllocFailed;
      }
      buf[i] = '\0';
      strncpy (buf, txt+6, i); // copy to temp buffer.
      c = strchr(buf, '/');
      if (c) {
         *c = '\0';
      }
      err = ipfs_path_parse_from_cid(dst, buf);
      free (buf);
      return err;
   } else if (strcmp (txt, "/ipns/") != 0) {
      return ErrBadPath;
   }
   return 0;
}

int ipfs_path_is_valid (char *p)
{
   char buf[4096];
   return ipfs_path_parse(buf, p);
}

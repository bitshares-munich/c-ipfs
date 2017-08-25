#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "ipfs/namesys/namesys.h"
#include "ipfs/namesys/isdomain.h"

void ipfs_isdomain_to_upper(char *dst, char *src)
{
    while(*src) {
        *dst++ = toupper(*src++);
    }
    *dst = '\0';
}

int ipfs_isdomain_has_suffix (char *s, char *suf)
{
    char *p;

    p = s + strlen(s) - strlen(suf);
    return strcmp(p, suf) == 0;
}

int ipfs_isdomain_is_at_array(tlds *a, char *s)
{
    char str[strlen(s)+1];

    ipfs_isdomain_to_upper(str, s);
    while(a->str) {
        if (strcmp(a->str, str) == 0) {
            return a->condition;
        }
        a++;
    }
    return 0;
}

int ipfs_isdomain_match_string (char *d)
{
    char str[strlen(d)+1], *p = str, *l;

    ipfs_isdomain_to_upper(str, d);

    // l point to last two chars.
    l = p + strlen(p) - 2;

    // can't start with a dot
    if (*p == '.') {
        return 0; // invalid
    }

    // last 2 chars can't be a dot or a number.
    if ((*l >= 'A' && *l <= 'Z') && (l[1] >= 'A' && l[1] <= 'Z')) {
        while (*p) {
            if ((*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9') || *p == '.' || *p == '-') {
                p++;
            } else {
                return 0; // invalid
            }
        }
    } else {
        return 0; // invalid
    }

    return 1; // valid
}

// ipfs_isdomain_is_icann_tld returns whether the given string is a TLD (Top Level Domain),
// according to ICANN. Well, really according to the TLDs listed in this
// package.
int ipfs_isdomain_is_icann_tld(char *s)
{
    return ipfs_isdomain_is_at_array (TLDs, s);
}

// ipfs_isdomain_is_extended_tld returns whether the given string is a TLD (Top Level Domain),
// extended with a few other "TLDs", .bit, .onion
int ipfs_isdomain_is_extended_tld (char *s)
{
    return ipfs_isdomain_is_at_array (ExtendedTLDs, s);
}

// ipfs_isdomain_is_tld returns whether the given string is a TLD (according to ICANN, or
// in the set of ExtendedTLDs listed in this package.
int ipfs_isdomain_is_tld (char *s)
{
    return ipfs_isdomain_is_icann_tld (s) || ipfs_isdomain_is_extended_tld(s);
}

// ipfs_isdomain_is_domain returns whether given string is a domain.
// It first checks the TLD, and then uses a regular expression.
int ipfs_isdomain_is_domain (char *s)
{
    int err;
    char *str, *tld;

    str = malloc(strlen(s) + 1);
    if (!str) {
        return ErrAllocFailed;
    }
    memcpy(str, s, strlen(s) + 1);
    s = str; // work with local copy.

    if (ipfs_isdomain_has_suffix (s, ".")) {
        s[strlen(s) - 1] = '\0';
    }

    tld = strrchr(s, '.');

    if (!tld) { // don't have a dot.
        return 0;
    }

    tld++; // ignore last dot

    if (!ipfs_isdomain_is_tld (tld)) {
        return 0;
    }

    err = ipfs_isdomain_match_string(s);
    free (s);
    return err;
}

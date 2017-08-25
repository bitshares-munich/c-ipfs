#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "ipfs/namesys/namesys.h"
#include "ipfs/cid/cid.h"
#include "ipfs/path/path.h"

const uint8_t  conse[] = {'b', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm', 'n', 'p', 'r', 's', 't', 'v', 'z'};
const uint8_t  vowse[] = {'a', 'i', 'o', 'u'};

// Find decoded number from the encoded consonant.
static inline int consd(char c)
{
    int i;

    for (i = 0 ; i < sizeof(conse) ; i++) {
        if (c == conse[i]) {
            return i;
        }
    }

    return 0;
}

// Find decoded number of encoded vowel.
static inline int vowsd(char c)
{
    int i;

    for (i = 0 ; i < sizeof(vowse) ; i++) {
        if (c == vowse[i]) {
            return i;
        }
    }

    return 0;
}

/**
* Tests if a given string is a Proquint identifier
*
* @param {string} str The candidate string.
*
* @return {bool} Whether or not it qualifies.
* @return {error} Error
*/
int ipfs_proquint_is_proquint(char *str)
{
    int i, c, l = strlen(str);

    // if str is null, or length is invalid
    if (!str || ((l+1) % 6)) {
        return 0; // it's not a proquint
    }

    // run every position
    for (i = 0 ; i < l ; i++) {
        if (((i+1) % 6) == 0) {      // After each 5 characters
            if (str[i] != '-') { // need a -
                return 0;        // or it's not a proquint
            }
        } else {
            switch ((i+1) % 2) { // i + 1 to avoid zero division
                case 0:
                    // compare with vowse array
                    c = vowsd(str[i]);
                    if (str[i] != vowse[c]) {

                        return 0; // it's not a proquint
                    }
                    break;
                default:
                    // compare with conse array
                    c = consd(str[i]);
                    if (str[i] != conse[c]) {
                        return 0; // it's not a proquint
                    }
            }
        }
    }

    return 1; // passed on every value.
}

/**
* Encodes an arbitrary byte slice into an identifier.
*
* @param {[]byte} buf Slice of bytes to encode.
*
* @return {string} The given byte slice as an identifier.
*/
char *ipfs_proquint_encode(char *buf, int size)
{
    char *ret;
    int i, c;
    uint16_t n;

    if (size % 2) {
        return NULL; // not multiple of 2
    }

    if (!buf) {
        return NULL;
    }

    // Each word (2 bytes) uses 5 ascii characters
    // and one - or a NULL terminator.
    ret = malloc(size * 3);
    if (!ret) {
        return NULL;
    }

    for (i = 0, c = 0; i < size; i += 2) {
        n = ((buf[i] & 0xff) << 8) | (buf[i + 1] & 0xff);

        ret[c++] = conse[(n >> 12) & 0x0f];
        ret[c++] = vowse[(n >> 10) & 0x03];
        ret[c++] = conse[(n >> 6)  & 0x0f];
        ret[c++] = vowse[(n >> 4)  & 0x03];
        ret[c++] = conse[n         & 0x0f];
        ret[c++] = '-';
    }
    ret[--c] = '\0';

    return ret;
}

/**
* Decodes an identifier into its corresponding byte slice.
*
* @param {string} str Identifier to convert.
*
* @return {[]byte} The identifier as a byte slice.
*/
char *ipfs_proquint_decode(char *str)
{
    char *ret;
    int i, c, l = strlen(str);
    uint16_t x;

    // make sure its a valid Proquint string.
    if (!ipfs_proquint_is_proquint(str) && ((l+1) % 3)==0) {
        return NULL;
    }

    ret = malloc((l+1)/3);
    if (!ret) {
        return NULL;
    }

    for (i = 0, c = 0 ; i < l ; i += 6) {
        x =(consd(str[i + 0]) << 12) | \
           (vowsd(str[i + 1]) << 10) | \
           (consd(str[i + 2]) <<  6) | \
           (vowsd(str[i + 3]) <<  4) | \
           (consd(str[i + 4]) <<  0);

        ret[c++] = x >> 8;
        ret[c++] = x & 0xff;
    }

    return ret;
}

// resolveOnce implements resolver. Decodes the proquint string.
int ipfs_proquint_resolve_once (char **p, char *name)
{
    int err = ipfs_proquint_is_proquint(name);
    char buf[500];

    if (err) {
        *p = NULL;
        err = ErrInvalidProquint;
    } else {
        err = ipfs_path_parse(buf, ipfs_proquint_decode(name));
        if (!err) {
            *p = malloc (strlen(buf) + 1);
            if (p) {
                memcpy(*p, buf, strlen(buf) + 1);
            }
        }
    }
    return err;
}

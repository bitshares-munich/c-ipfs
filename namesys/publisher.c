#include <stdlib.h>
#include <string.h>
#include "ipfs/util/errs.h"
#include "ipfs/util/time.h"
#include "ipfs/namesys/pb.h"
#include "ipfs/namesys/publisher.h"

char* ipns_entry_data_for_sig (struct ipns_entry *entry)
{
    char *ret;

    if (!entry || !entry->value || !entry->validity) {
        return NULL;
    }
    ret = calloc (1, strlen(entry->value) + strlen (entry->validity) + sizeof(IpnsEntry_ValidityType) + 1);
    if (ret) {
        strcpy(ret, entry->value);
        strcat(ret, entry->validity);
        if (entry->validityType) {
            memcpy(ret+strlen(entry->value)+strlen(entry->validity), entry->validityType, sizeof(IpnsEntry_ValidityType));
        } else {
            memcpy(ret+strlen(entry->value)+strlen(entry->validity), &IpnsEntry_EOL, sizeof(IpnsEntry_ValidityType));
        }
    }
    return ret;
}

int ipns_selector_func (int *idx, struct ipns_entry ***recs, char *k, char **vals)
{
    int err = 0, i, c;

    if (!idx || !recs || !k || !vals) {
        return ErrInvalidParam;
    }

    for (c = 0 ; vals[c] ; c++); // count array

    *recs = calloc(c+1, sizeof (void*)); // allocate return array.
    if (!*recs) {
        return ErrAllocFailed;
    }
    for (i = 0 ; i < c ; i++) {
        *recs[i] = calloc(1, sizeof (struct ipns_entry)); // alloc every record
        if (!*recs[i]) {
            return ErrAllocFailed;
        }
        //err = proto.Unmarshal(vals[i], *recs[i]); // and decode.
        if (err) {
            ipfs_namesys_ipnsentry_reset (*recs[i]); // make sure record is empty.
        }
    }
    return ipns_select_record(idx, *recs, vals);
}

int ipns_select_record (int *idx, struct ipns_entry **recs, char **vals)
{
    int err, i, best_i = -1, best_seq = 0;
    struct timespec rt, bestt;

    if (!idx || !recs || !vals) {
        return ErrInvalidParam;
    }

    for (i = 0 ; recs[i] ; i++) {
        if (!(recs[i]->sequence) || *(recs[i]->sequence) < best_seq) {
            continue;
        }

        if (best_i == -1 || *(recs[i]->sequence) > best_seq) {
            best_seq = *(recs[i]->sequence);
            best_i = i;
        } else if (*(recs[i]->sequence) == best_seq) {
            err = ipfs_util_time_parse_RFC3339 (&rt, ipfs_namesys_pb_get_validity (recs[i]));
            if (err) {
                continue;
            }
            err = ipfs_util_time_parse_RFC3339 (&bestt, ipfs_namesys_pb_get_validity (recs[best_i]));
            if (err) {
                continue;
            }
            if (rt.tv_sec > bestt.tv_sec || (rt.tv_sec == bestt.tv_sec && rt.tv_nsec > bestt.tv_nsec)) {
                best_i = i;
            } else if (rt.tv_sec == bestt.tv_sec && rt.tv_nsec == bestt.tv_nsec) {
                if (memcmp(vals[i], vals[best_i], strlen(vals[best_i])) > 0) { // FIXME: strlen?
                    best_i = i;
                }
            }
        }
    }
    if (best_i == -1) {
        return ErrNoRecord;
    }
    *idx = best_i;
    return 0;
}

// ipns_validate_ipns_record implements ValidatorFunc and verifies that the
// given 'val' is an IpnsEntry and that that entry is valid.
int ipns_validate_ipns_record (char *k, char *val)
{
    int err = 0;
    struct ipns_entry *entry = ipfs_namesys_pb_new_ipns_entry();
    struct timespec ts, now;

    if (!entry) {
        return ErrAllocFailed;
    }
    //err = proto.Unmarshal(val, entry);
    if (err) {
        return err;
    }
    if (ipfs_namesys_pb_get_validity_type (entry) == IpnsEntry_EOL) {
        err = ipfs_util_time_parse_RFC3339 (&ts, ipfs_namesys_pb_get_validity (entry));
        if (err) {
            //log.Debug("failed parsing time for ipns record EOL")
            return err;
        }
        timespec_get (&now, TIME_UTC);
        if (now.tv_nsec > ts.tv_nsec || (now.tv_nsec == ts.tv_nsec && now.tv_nsec > ts.tv_nsec)) {
            return ErrExpiredRecord;
        }
    } else {
        return ErrUnrecognizedValidity;
    }
   return 0;
}

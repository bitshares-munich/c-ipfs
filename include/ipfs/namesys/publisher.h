#ifndef IPFS_PUBLISHER_H
    #define IPFS_PUBLISHER_H
    char* ipns_entry_data_for_sig (struct ipns_entry *entry);
    int ipns_selector_func (int *idx, struct ipns_entry ***recs, char *k, char **vals);
    int ipns_select_record (int *idx, struct ipns_entry **recs, char **vals);
    // ipns_validate_ipns_record implements ValidatorFunc and verifies that the
    // given 'val' is an IpnsEntry and that that entry is valid.
    int ipns_validate_ipns_record (char *k, char *val);
#endif // IPFS_PUBLISHER_H

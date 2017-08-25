#ifndef IPFS_TIME_H
    #define IPFS_TIME_H
    #ifndef __USE_XOPEN
        #define __USE_XOPEN
    #endif // __USE_XOPEN

    #ifndef __USE_ISOC11
        #define __USE_ISOC11
    #endif // __USE_ISOC11

    #include <time.h>

    int ipfs_util_time_parse_RFC3339 (struct timespec *ts, char *s);
    char *ipfs_util_time_format_RFC3339 (struct timespec *ts);
#endif // IPFS_TIME_H

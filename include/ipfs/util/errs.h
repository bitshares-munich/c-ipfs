#ifndef IPFS_ERRS_H
    #define IPFS_ERRS_H

    extern char *Err[];

    enum {
        ErrAllocFailed = 1,
        ErrNULLPointer,
        ErrUnknow,
        ErrPipe,
        ErrPoll,
        ErrPublishFailed,
        ErrResolveFailed,
        ErrResolveRecursion,
        ErrExpiredRecord,
        ErrUnrecognizedValidity,
        ErrInvalidProquint,
        ErrInvalidDomain,
        ErrInvalidDNSLink,
        ErrBadPath,
        ErrNoComponents,
        ErrCidDecode,
        ErrNoLink,
        ErrNoLinkFmt,
        ErrInvalidParam,
        ErrResolveLimit,
        ErrInvalidSignature,
        ErrInvalidSignatureFmt,
        ErrNoRecord,
        ErrCidDecodeFailed,
        ErrOffline
    } ErrsIdx;
#endif // IPFS_ERRS_H

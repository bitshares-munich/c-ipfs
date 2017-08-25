#include <stdio.h>
#include <string.h>

#include "libp2p/utils/logger.h"
#include "ipfs/repo/init.h"
#include "ipfs/importer/importer.h"
#include "ipfs/importer/exporter.h"
#include "ipfs/dnslink/dnslink.h"
#include "ipfs/core/daemon.h"

#ifdef __MINGW32__
    void bzero(void *s, size_t n)
    {
        memset (s, '\0', n);
    }

    char *strtok_r(char *str, const char *delim, char **save)
    {
        char *res, *last;

        if( !save )
            return strtok(str, delim);
        if( !str && !(str = *save) )
            return NULL;
        last = str + strlen(str);
        if( (*save = res = strtok(str, delim)) )
        {
            *save += strlen(res);
            if( *save < last )
                (*save)++;
            else
                *save = NULL;
        }
        return res;
    }
#endif // MINGW

void stripit(int argc, char** argv) {
	char tmp[strlen(argv[argc])];
	strcpy(tmp, &argv[argc][1]);
	tmp[strlen(tmp)-1] = 0;
	strcpy(argv[argc], tmp);
	return;
}

void strip_quotes(int argc, char** argv) {
	for(int i = 0; i < argc; i++) {
		if (argv[i][0] == '\'' && argv[i][strlen(argv[i])-1] == '\'') {
			stripit(i, argv);
		}
	}
}

#define INIT 1
#define ADD 2
#define OBJECT_GET 3
#define DNS 4
#define CAT 5
#define DAEMON 6
#define PING 7
#define GET 8

/***
 * Basic parsing of command line arguments to figure out where the user wants to go
 */
int parse_arguments(int argc, char** argv) {
	if (argc == 1) {
		printf("No parameters passed.\n");
		return 0;
	}
	if (strcmp("init", argv[1]) == 0) {
		return INIT;
	}
	if (strcmp("add", argv[1]) == 0) {
		return ADD;
	}
	if (strcmp("object", argv[1]) == 0 && argc > 2 && strcmp("get", argv[2]) == 0) {
		return OBJECT_GET;
	}
	if (strcmp("cat", argv[1]) == 0) {
		return CAT;
	}
	if (strcmp("dns", argv[1]) == 0) {
		return DNS;
	}
	if (strcmp("daemon", argv[1]) == 0) {
		return DAEMON;
	}
	if (strcmp("ping", argv[1]) == 0) {
		return PING;
	}
	if (strcmp("get", argv[1]) == 0) {
		return GET;
	}
	return -1;
}

/***
 * The beginning
 */
int main(int argc, char** argv) {
	// for debugging
	libp2p_logger_add_class("null");
	libp2p_logger_add_class("bitswap");
	libp2p_logger_add_class("secio");
	libp2p_logger_add_class("peer_request_queue");
	libp2p_logger_add_class("bitswap_engine");
	libp2p_logger_add_class("peerstore");
	libp2p_logger_add_class("dht_protocol");
	libp2p_logger_add_class("peer");

	strip_quotes(argc, argv);
	int retVal = parse_arguments(argc, argv);
	switch (retVal) {
	case (INIT):
		return ipfs_repo_init(argc, argv);
		break;
	case (ADD):
		ipfs_import_files(argc, argv);
		break;
	case (OBJECT_GET):
		ipfs_exporter_object_get(argc, argv);
		break;
	case(GET):
		//ipfs_exporter_get(argc, argv);
		//break;
	case (CAT):
		ipfs_exporter_object_cat(argc, argv);
		break;
	case (DNS):
		ipfs_dns(argc, argv);
		break;
	case (DAEMON):
		ipfs_daemon(argc, argv);
		break;
	case (PING):
		ipfs_ping(argc, argv);
		break;
	}
	libp2p_logger_free();
}

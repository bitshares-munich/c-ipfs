#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

#include "ipfs/repo/init.h"
#include "ipfs/repo/fsrepo/fs_repo.h"
#include "libp2p/os/utils.h"

/***
 * Helper to create a test file in the OS
 */
int create_file(const char* fileName, unsigned char* bytes, size_t num_bytes) {
	FILE* file = fopen(fileName, "wb");
	fwrite(bytes, num_bytes, 1, file);
	fclose(file);
	return 1;
}

/***
 * Create a buffer with some data
 */
int create_bytes(unsigned char* buffer, size_t num_bytes) {
	int counter = 0;

	for(int i = 0; i < num_bytes; i++) {
		buffer[i] = counter++;
		if (counter > 15)
			counter = 0;
	}
	return 1;
}

/***
 * Remove a directory and everything in it
 * @param path the directory to remove
 * @returns true(1) on success, otherwise false(0)
 */
int remove_directory(const char *path)
{
   DIR *d = opendir(path);
   size_t path_len = strlen(path);
   int r = -1;

   if (d)
   {
      struct dirent *p;

      r = 1;

      while (r && (p=readdir(d)))
      {
          int r2 = -1;
          char *buf;
          size_t len;

          /* Skip the names "." and ".." as we don't want to recurse on them. */
          if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
          {
             continue;
          }

          len = path_len + strlen(p->d_name) + 2;
          buf = malloc(len);

          if (buf)
          {
             struct stat statbuf;

             snprintf(buf, len, "%s/%s", path, p->d_name);

             if (!stat(buf, &statbuf))
             {
                if (S_ISDIR(statbuf.st_mode))
                {
                   r2 = remove_directory(buf);
                }
                else
                {
                   r2 = !unlink(buf);
                }
             }

             free(buf);
          }

          r = r2;
      }

      closedir(d);
   }

   if (r)
   {
      r = !rmdir(path);
   }

   return r;
}

/***
 * Drop a repository by removing the directory
 */
int drop_repository(const char* path) {

	if (os_utils_file_exists(path)) {
		return remove_directory(path);
		/*
		// the config file
		if (!os_utils_filepath_join(path, "config", currDirectory, 1024))
			return 0;
		unlink(currDirectory);

		// the datastore directory
		if (!os_utils_filepath_join(path, "datastore", currDirectory, 1024))
			return 0;
		if (!remove_directory(currDirectory))
			return 0;

		// the blockstore directory
		if (!os_utils_filepath_join(path, "blockstore", currDirectory, 1024))
			return 0;
		if (!remove_directory(currDirectory))
			return 0;

		return remove_directory(path);
		*/
	}

	return 1;
}

/**
 * drops and builds a repository at the specified path
 * @param path the path
 * @param swarm_port the port that the swarm should run on
 * @param bootstrap_peers a vector of fellow peers as MultiAddresses, can be NULL
 * @param peer_id a place to store the generated peer id
 * @returns true(1) on success, otherwise false(0)
 */
int drop_and_build_repository(const char* path, int swarm_port, struct Libp2pVector* bootstrap_peers, char **peer_id) {

	if (os_utils_file_exists(path)) {
		if (!drop_repository(path)) {
			return 0;
		}
	}
	mkdir(path, S_IRWXU);

	return make_ipfs_repository(path, swarm_port, bootstrap_peers, peer_id);
}


int drop_build_and_open_repo(const char* path, struct FSRepo** fs_repo) {

	if (!drop_and_build_repository(path, 4001, NULL, NULL))
		return 0;

	if (!ipfs_repo_fsrepo_new(path, NULL, fs_repo))
		return 0;

	if (!ipfs_repo_fsrepo_open(*fs_repo)) {
		free(*fs_repo);
		*fs_repo = NULL;
		return 0;
	}
	return 1;
}


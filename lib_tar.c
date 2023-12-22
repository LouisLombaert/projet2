#include "lib_tar.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/**
 * Checks whether the archive is valid.
 *
 * Each non-null header of a valid archive has:
 *  - a magic value of "ustar" and a null,
 *  - a version value of "00" and no null,
 *  - a correct checksum
 *
 * @param tar_fd A file descriptor pointing to the start of a file supposed to contain a tar archive.
 *
 * @return a zero or positive value if the archive is valid, representing the number of non-null headers in the archive,
 *         -1 if the archive contains a header with an invalid magic value,
 *         -2 if the archive contains a header with an invalid version value,
 *         -3 if the archive contains a header with an invalid checksum value
 */
int check_archive(int tar_fd) {
    lseek(tar_fd, 0, SEEK_SET);
    char buffer[sizeof(tar_header_t)];
    int counter = 0;
    
    while (1) {
        if(read(tar_fd, buffer, sizeof(tar_header_t)) != sizeof(tar_header_t)) return -1; // read error 

        tar_header_t* tar_header = (tar_header_t*)buffer;

        if(strncmp(tar_header->magic, TMAGIC, TMAGLEN) != 0) return -1; // magic error
        if(strncmp(tar_header->version, TVERSION, TVERSLEN) != 0) return -2; // version error

        int sum = 0;
        for (int i = 0; i < sizeof(tar_header_t); i++) {
            sum += (i >= 148 && i < 156) ? ' ' : buffer[i];
        }

        if(TAR_INT(tar_header->chksum) != sum) return -3; // checksum error

        int skip = (TAR_INT(tar_header->size) + sizeof(tar_header_t) - 1) / sizeof(tar_header_t);
        lseek(tar_fd,skip*sizeof(tar_header_t),SEEK_CUR);

        counter++;

        // Check end
        char check_end[2*sizeof(tar_header_t)];
        if(read(tar_fd, check_end, 2*sizeof(tar_header_t)) != 2*sizeof(tar_header_t)) return -1; // read error

        int stop = 0;
        for (int i = 0; i < 2*sizeof(tar_header_t); ++i) {
            stop += check_end[i];
        }

        lseek(tar_fd, -2*sizeof(tar_header_t), SEEK_CUR);
        if(stop == 0) break;
    }

    return counter;
}

/**
 * Checks whether an entry exists in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive,
 *         any other value otherwise.
 */
int exists(int tar_fd, char *path) {
    tar_header_t* buffer = malloc(sizeof(tar_header_t));
    lseek(tar_fd, 0, SEEK_SET); 

    while (read(tar_fd, buffer, sizeof(tar_header_t)) > 0) {
        if(strcmp(path, buffer->name) == 0) { free(buffer); return 1; } // File exists
    }
     
    free(buffer);
    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a directory.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a directory,
 *         any other value otherwise.
 */
int is_dir(int tar_fd, char *path) {
    tar_header_t* buffer = malloc(sizeof(tar_header_t));
    lseek(tar_fd, 0, SEEK_SET); 

    while (read(tar_fd, buffer, sizeof(tar_header_t)) > 0) {
        if(strcmp(path, buffer->name) == 0 && buffer->typeflag == DIRTYPE) { free(buffer); return 1; } 
    }
     
    free(buffer);
    return 0;
}


/**
 * Checks whether an entry exists in the archive and is a file.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a file,
 *         any other value otherwise.
 */
int is_file(int tar_fd, char *path) {
    tar_header_t* buffer = malloc(sizeof(tar_header_t));
    lseek(tar_fd, 0, SEEK_SET); 

    while (read(tar_fd, buffer, sizeof(tar_header_t)) > 0) {
        if(strcmp(path, buffer->name) == 0 && buffer->typeflag == REGTYPE) { free(buffer); return 1; } 
    }
     
    free(buffer);
    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a symlink.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 * @return zero if no entry at the given path exists in the archive or the entry is not symlink,
 *         any other value otherwise.
 */
int is_symlink(int tar_fd, char *path) {
    tar_header_t* buffer = malloc(sizeof(tar_header_t));
    lseek(tar_fd, 0, SEEK_SET); 

    while (read(tar_fd, buffer, sizeof(tar_header_t)) > 0) {
        if(strcmp(path, buffer->name) == 0 && buffer->typeflag == SYMTYPE) { free(buffer); return 1; } 
    }
     
    free(buffer);
    return 0;
}


/**
 * Lists the entries at a given path in the archive.
 * list() does not recurse into the directories listed at the given path.
 *
 * Example:
 *  dir/          list(..., "dir/", ...) lists "dir/a", "dir/b", "dir/c/" and "dir/e/"
 *   ├── a
 *   ├── b
 *   ├── c/
 *   │   └── d
 *   └── e/
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive. If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param entries An array of char arrays, each one is long enough to contain a tar entry path.
 * @param no_entries An in-out argument.
 *                   The caller set it to the number of entries in `entries`.
 *                   The callee set it to the number of entries listed.
 *
 * @return zero if no directory at the given path exists in the archive,
 *         any other value otherwise.
 */
int checkEnd(int fd){
    char data[1024];
    read(fd,data,1024);

    unsigned int sum = 0;
    for (int i = 0; i < 1024; ++i) {
        sum += data[i];
    }
    lseek(fd,-1024,SEEK_CUR);

    return sum == 0;
}

int list(int tar_fd, char *path, char **entries, size_t *no_entries) {
    lseek(tar_fd, 0, SEEK_SET);
    char buffer[sizeof(tar_header_t)];
    long skip = 0;
    int initBack = 0;
    int c = -1;
    
    while (1) {
        if(read(tar_fd, buffer, sizeof(tar_header_t)) < 0) return -1; // read error
        
        tar_header_t* tar_header = (tar_header_t*)buffer;

        char* name = tar_header->name;
        if(tar_header->typeflag == SYMTYPE) strcat(name, "/"); 

        if(strncmp(name, path, strlen(path)) == 0) {
            if(tar_header->typeflag == SYMTYPE && c == -1) return list(tar_fd, tar_header->linkname, entries, no_entries);

            int lastBack = 0;
            for (int i = 0; i < strlen(tar_header->name); ++i) {
                if(tar_header->name[i] == '/') lastBack = i;
            }

            if (c == -1) {
                initBack = lastBack;
                c++;
            } else {
                if (c < *no_entries && (lastBack <= initBack || tar_header->name[lastBack + 1] == '\0')) {
                    memcpy(entries[c], name, strlen(name));
                    c++;
                }
            }
        }

        skip += (TAR_INT(tar_header->size) + sizeof(tar_header_t) - 1) / sizeof(tar_header_t) + 1;
        lseek(tar_fd, skip * sizeof(tar_header_t), SEEK_SET);

        // Check for the end of the file
        char check_end[2 * sizeof(tar_header_t)];
        if (read(tar_fd, check_end, 2 * sizeof(tar_header_t)) != 2 * sizeof(tar_header_t)) return -1; // read error

        int stop = 0;
        for (int i = 0; i < 2 * sizeof(tar_header_t); ++i) {
            stop += check_end[i];
        }

        lseek(tar_fd, -2 * sizeof(tar_header_t), SEEK_CUR);
        if(stop == 0) break;
    }
    *no_entries = (c == -1) ? 0 : c;
    
    return *no_entries;
}

void resolve_symlink(int tar_fd, tar_header_t* buffer, char* resolved_path, char **entries, size_t *no_entries, size_t max_no_entries) {
    tar_header_t* linked_entry = malloc(sizeof(tar_header_t));
    int counter = 0;
    
    lseek(tar_fd, 0, SEEK_SET);

    ssize_t len = readlink(buffer->name, resolved_path, sizeof(resolved_path) - 1);
    resolved_path[len] = '\0';
    strcat(resolved_path, "/");

    while (read(tar_fd, linked_entry, sizeof(tar_header_t)) > 0) {
        if (strncmp(resolved_path, linked_entry->name, strlen(resolved_path)) == 0) {
            if (strcmp(resolved_path, linked_entry->name) != 0){
                if(counter < max_no_entries) { //max 10 entréés => counter va de 0 à 9 
                    entries[counter] = malloc(strlen(linked_entry->name) + 1);
                    strcpy(entries[counter], linked_entry->name);
                    counter++;
                }
		        *no_entries += 1;
	        } else if(linked_entry->typeflag != DIRTYPE){
                //vérifier, si l'entrée est la directory elle-même, que c'est bien une directory
                free(linked_entry);
                printf("not a directory.\n");
                return;
            }
            continue;
            
        }

        lseek(tar_fd, ((TAR_INT(linked_entry->size) / sizeof(tar_header_t)) + 1) * sizeof(tar_header_t), SEEK_CUR);
   }
   free(linked_entry);

}

int liste(int tar_fd, char *path, char **entries, size_t *no_entries) {
    tar_header_t* buffer= malloc(sizeof(tar_header_t));
    int counter = 0;
    size_t max_no_entries = *no_entries;
    *no_entries = 0;
    lseek(tar_fd, 0, SEEK_SET);


    while(read(tar_fd, buffer, sizeof(tar_header_t)) > 0){
        tar_header_t* tar_header = (tar_header_t*) buffer;
        size_t length = strlen(path);

        if(buffer->typeflag == SYMTYPE) {
            char resolved_path[30];
            resolve_symlink(tar_fd, buffer, resolved_path, entries, no_entries, max_no_entries);
            return (*no_entries > 0) ? 1 : 0;
        } else if(strncmp(path, buffer->name, length) == 0) {
            // vérifier que l'entrée n'est pas la directory elle même
            if (strcmp(path, buffer->name) != 0){
                if(counter < max_no_entries) { //max 10 entréés => counter va de 0 à 9 
                    entries[counter] = malloc(strlen(buffer->name) + 1);
                    strcpy(entries[counter], buffer->name);
                    counter++;
                }
		        *no_entries += 1;
	        }
            else if(buffer->typeflag != DIRTYPE){
                //vérifier, si l'entrée est la directory elle-même, que c'est bien une directory
                    free(buffer);
                    printf("not a directory.\n");
                	return 0;
            }
            continue;
            
        }

        lseek(tar_fd, ((TAR_INT(tar_header->size) / sizeof(tar_header_t)) + 1) * sizeof(tar_header_t), SEEK_CUR); // Skip to the next header block
    }
    
    free(buffer);
    return (*no_entries > 0) ? 1 : 0;
}

/**
 * Reads a file at a given path in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive to read from.  If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param offset An offset in the file from which to start reading from, zero indicates the start of the file.
 * @param dest A destination buffer to read the given file into.
 * @param len An in-out argument.
 *            The caller set it to the size of dest.
 *            The callee set it to the number of bytes written to dest.
 *
 * @return -1 if no entry at the given path exists in the archive or the entry is not a file,
 *         -2 if the offset is outside the file total length,
 *         zero if the file was read in its entirety into the destination buffer,
 *         a positive value if the file was partially read, representing the remaining bytes left to be read to reach
 *         the end of the file.
 *
 */
ssize_t read_file(int tar_fd, char *path, size_t offset, uint8_t *dest, size_t *len) {
    lseek(tar_fd,0,SEEK_SET);
    char buffer[sizeof(tar_header_t)];

    while(1){
        if(read(tar_fd,buffer,sizeof(tar_header_t)) < 0) return -1; // read error

        tar_header_t* tar_header = (tar_header_t*) buffer;

        if(strncmp(tar_header->name,path, strlen(path)) == 0){
            if(tar_header->typeflag == DIRTYPE) return -1;
            
            if(tar_header->typeflag == SYMTYPE){
                return read_file(tar_fd,tar_header->linkname,offset,dest,len); // recurrsif call for the symlink
            } else {

                if(!(tar_header->typeflag == REGTYPE || tar_header->typeflag == AREGTYPE)) return -1;

                ssize_t size = TAR_INT(tar_header->size);
                if(offset > size) return -2;

                lseek(tar_fd, offset, SEEK_CUR);

                if((size - offset) < *len) *len = size - offset;

                if(read(tar_fd, dest, *len) < 0) return -1; // read error

                return (size - offset) - *len;
            }
        }

        lseek(tar_fd,(TAR_INT(tar_header->size) + sizeof(tar_header_t) - 1) / sizeof(tar_header_t)*sizeof(tar_header_t),SEEK_CUR);

        // Check for the end of the file
        char check_end[2 * sizeof(tar_header_t)];
        if (read(tar_fd, check_end, 2 * sizeof(tar_header_t)) != 2 * sizeof(tar_header_t)) return -1; // read error

        int stop = 0;
        for (int i = 0; i < 2 * sizeof(tar_header_t); ++i) {
            stop += check_end[i];
        }

        lseek(tar_fd, -2 * sizeof(tar_header_t), SEEK_CUR);
        if(stop == 0) break;
    }
    return -1;
}
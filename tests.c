#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "lib_tar.h"

/**
 * You are free to use this file to write tests for your implementation
 */

void debug_dump(const uint8_t *bytes, size_t len) {
    for (int i = 0; i < len;) {
        printf("%04x:  ", (int) i);

        for (int j = 0; j < 16 && i + j < len; j++) {
            printf("%02x ", bytes[i + j]);
        }
        printf("\t");
        for (int j = 0; j < 16 && i < len; j++, i++) {
            printf("%c ", bytes[i]);
        }
        printf("\n");
    }
}


int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s tar_file\n", argv[0]);
        return -1;
    }

    int fd = open(argv[1] , O_RDONLY);
    if (fd == -1) {
        perror("open(tar_file)");
        return -1;
    }

    int ret = check_archive(fd);
    printf("===========================\n|| check_archive() tests ||\n===========================\n\n");
    printf("check_archive should return 6 and returned: %d\n", ret);

    // Exists() tests
    int exists_make = exists(fd, "Makefile");
    int exists_tests = exists(fd, "tests.c");
    int exists_nope = exists(fd, "nope.txt");
    int exists_dir = exists(fd, "testDir/");

    printf("\n\n====================\n|| exists() tests ||\n====================\n\n");
    printf("Exists Makefile should return 1 and returnd:%d\n", exists_make);
    printf("Exists tests.c should return 1 and returnd:%d\n", exists_tests);
    printf("Exists nope.txt should return 0 and returnd:%d\n", exists_nope);
    printf("Exists testDir should return 1 and returnd:%d\n", exists_dir);

    // is_dir() tests
    int is_dir_make = is_dir(fd, "Makefile");
    int is_dir_dir = is_dir(fd, "testDir/");

    printf("\n\n====================\n|| is_dir() tests ||\n====================\n\n");
    printf("Is_dir Makefile should return 0 and returnd:%d\n", is_dir_make);
    printf("Is_dir dir should return 1 and returnd:%d\n", is_dir_dir);

    // is_file() tests
    int is_file_make = is_file(fd,"Makefile");
    int is_file_target_file = is_file(fd, "target_file.txt");
    int is_file_nope = is_file(fd, "nope.txt");
    int is_file_file1 = is_file(fd, "file1.txt");

    printf("\n\n=====================\n|| is_file() tests ||\n=====================\n\n");
    printf("Is_file Makefile should return 1 and returnd:%d\n", is_file_make);
    printf("Is_file target_file.txt should return 1 and returnd:%d\n", is_file_target_file);
    printf("Is_file nope.txt should return 0 and returnd:%d\n", is_file_nope);
    printf("Is_file file1.txt should return 1 and returnd:%d\n", is_file_file1);

    // is_symlink() tests
    int is_symlink_link = is_symlink(fd, "symbolic_link.txt");
    int is_symlink_nope = is_symlink(fd, "nope.txt");

    printf("\n\n========================\n|| is_symlink() tests ||\n========================\n\n");
    printf("Is_symlink symbolic_link.txt should return 1 and returnd:%d\n", is_symlink_link);
    printf("Is_symlink nope.txt should return 0 and returnd:%d\n", is_symlink_nope);

    // read_file() test
    size_t len = 512;
    uint8_t buffer[512];
    ssize_t result = read_file(fd, "testDir/file1.txt", 0, buffer, &len);

    printf("\n\n=======================\n|| read_file() tests ||\n=======================\n\n");
    printf("read_file should return 0 and returned:%ld\nShould show 'file to read.' and read: %zu bytes from file1.txt and print:\n%s\n",result, len, (char *)buffer);
    

    //list() tests
    char **entries;
    entries = (char **)malloc(10 * sizeof(char *));
    for (int i=0; i < 10; i++){
    	entries[i] = (char *) malloc(30*sizeof(char));
    }
    size_t *no_entries;
    no_entries = (size_t *)malloc(sizeof(size_t));
    *no_entries = 10;
    
    int list_test = list(fd, "testDir/", entries, no_entries);
    printf("\nlist_test results: testDir(1): %d, avec %ld entrées(2).\n", list_test, *no_entries);

    for (int i=0; i<*no_entries; i++){
        printf("%s\n", entries[i]);
        free(entries[i]);
    }
    free(no_entries);
    
    close(fd);

    return 0;
}
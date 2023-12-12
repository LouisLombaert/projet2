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
    printf("check_archive returned %d\n", ret);

    // Exists() tests
    int exists_make = exists(fd, "Makefile");
    int exists_tests = exists(fd, "tests.c");
    int exists_nope = exists(fd, "nope");

    printf("Exists results: Makefile(1): %d, tests.c(1):%d, nope(0):%d\n", exists_make, exists_tests, exists_nope);

    // is_dir() tests
    int is_dir_make = is_dir(fd, "Makefile");
    int is_dir_dir = is_dir(fd, "dir");

    printf("is_dir results: Makefile(0): %d, dir(0):%d\n", is_dir_make, is_dir_dir);

    // is_file() tests
    int is_file_make = is_file(fd,"Makefile");
    int is_file_tests = is_file(fd, "tests.c");
    int is_file_nope = is_file(fd, "nope");

    printf("is_file results: Makefile(1): %d, tests.c(1):%d nope:%d\n", is_file_make, is_file_tests, is_file_nope);

    // is_symlink() tests
    int is_symlink_make = is_symlink(fd, "Makefile");
    printf("is_symlink results: Makefile(0): %d\n", is_symlink_make);

    return 0;
}
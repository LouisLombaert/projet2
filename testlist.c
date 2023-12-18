#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define TAR_BLOCK_SIZE 512
#define TAR_NAME_SIZE 100

typedef struct {
    char name[TAR_NAME_SIZE];
} tar_header;

bool is_directory(const char *name) {
    size_t len = strlen(name);
    return len > 0 && name[len - 1] == '/';
}

bool starts_with(const char *pre, const char *str) {
    return strncmp(pre, str, strlen(pre)) == 0;
}

int list(int tar_fd, const char *path, char entries[][TAR_NAME_SIZE], int *no_entries) {
    tar_header header;
    int listed_count = 0;
    int max_entries = *no_entries;
    *no_entries = 0;

    while (read(tar_fd, &header, sizeof(tar_header)) > 0) {
        if (starts_with(path, header.name)) {
            if (listed_count < max_entries) {
                strcpy(entries[listed_count++], header.name);
            }
            *no_entries += 1;
        }
        lseek(tar_fd, TAR_BLOCK_SIZE - sizeof(tar_header), SEEK_CUR);
    }

    return (*no_entries > 0) ? 1 : 0;
}

int main(int argc, char **argv){
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
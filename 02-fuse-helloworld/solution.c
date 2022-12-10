#include "solution.h"
#define FUSE_USE_VERSION 31
#include <fuse.h>

static const struct fuse_operations hellofs_ops = {
	/* implement me */
};
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

void report_error (const char* msg, char* path) {
    char* message = calloc(strlen(path) + strlen(msg) + 1, 1);
    strcpy(message, msg);
    strcat(message, path);
    perror(message);
    free(message);
}

int check_dir(int dirfd, char** full_path, int* buff_len, int path_len) {
    errno = 0;
    DIR *current_dir = fdopendir(dirfd);
    if (current_dir == NULL) {
        report_error("failed to open directory ", *full_path);
        return -1;
    }
    while (1) {
        errno = 0;
        struct dirent *entry = readdir(current_dir);
        if (entry == NULL) {
            if (errno != 0) {
                report_error("failed to read directory ", *full_path);
                closedir(current_dir);
                return -1;
            }
            else {
                if (closedir(current_dir) == -1) {
                    report_error("failed to close directory ", *full_path);
                    return -1;
                }
                return 0;
            }
        }
        else {
            int new_len = strlen(*full_path) + strlen(entry->d_name) + 1;
            while (new_len + 1 > *buff_len) {
                *buff_len = 2 * (*buff_len);
                char* new_path = calloc(*buff_len, 1);
                strcpy(new_path, *full_path);
                free(*full_path);
                *full_path = new_path;
            }
            strcat(*full_path, "/");
            strcat(*full_path, entry->d_name);

            if (entry->d_type == DT_DIR && strcmp(entry->d_name, "..") != 0 && strcmp(entry->d_name, ".") != 0) {
                int newdirfd = openat(dirfd, entry->d_name, O_RDONLY);
                if (newdirfd == -1) {
                    report_error("failed to open directory ", *full_path);
                    closedir(current_dir);
                    return -1;
                }
                int res = check_dir(newdirfd, full_path, buff_len, new_len);
                if (res == -1) {
                    closedir(current_dir);
                    return -1;
                }
            }

            if (entry->d_type == DT_REG) {
                int fd = openat(dirfd, entry->d_name, O_RDONLY);
                if (fd == -1) {
                    report_error("failed to open file ", *full_path);
                    closedir(current_dir);
                    return -1;
                }
                struct stat* buf = calloc(1, sizeof(struct stat));
                int res = fstat(fd, buf);
                if (res == -1) {
                    report_error("failed to read file permissions ", *full_path);
                    closedir(current_dir);
                    close(fd);
                    return -1;
                }
                int mode = buf->st_mode;
                free(buf);
                res = fchmod(fd, mode | S_IWUSR | S_IWGRP | S_IWOTH);
                if (res == -1) {
                    report_error("failed to change mode ", *full_path);
                    closedir(current_dir);
                    close(fd);
                    return -1;
                }
                res = close(fd);
                if (res == -1) {
                    report_error("failed to close file ", *full_path);
                    closedir(current_dir);
                    return -1;
                }
                printf("changed mode in %s\n", *full_path);
            }
            (*full_path)[path_len] = '\0';
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Wrong arguments' count. Only working directory path is needed");
        return -1;
    }
    char* full_path = calloc(strlen(argv[1]) + 1, 1);
    strcpy(full_path, argv[1]);
    int buff_len = strlen(argv[1]) + 1;

    int newdirfd = open(full_path, O_RDONLY);
    if (newdirfd == -1) {
        report_error("failed to open directory ", full_path);
        free(full_path);
        return -1;
    }
    int res = check_dir(newdirfd, &full_path, &buff_len, strlen(argv[1]));
    free(full_path);
    if (res == -1) {
        return -1;
    }
    return 0;
}

int helloworld(const char *mntp)
{
	char *argv[] = {"exercise", "-f", (char *)mntp, NULL};
	return fuse_main(3, argv, &hellofs_ops, NULL);
}

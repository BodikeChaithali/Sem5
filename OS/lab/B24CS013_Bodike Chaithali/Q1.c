#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/wait.h>

#define NULL (NULL)
#define DESC 0
#define F_OK 0

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <src_dir> <dst_dir>\n", argv[0]);
        return 1;
    }

    if (access(argv[1], F_OK) != 0 || access(argv[2], F_OK) != 0) {
        printf("Error: one or both of the source and destination directories do not exist\n");
        return 1;
    }

    if (mkdir(argv[2], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
        perror("mkdir");
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    void *statusBuffer = mmap(NULL, 256, PROT_READ, MAP_SHARED, fd, 0);
    if (statusBuffer == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    struct stat buffer;
    if (lstat(argv[1], &buffer) == 0 && S_ISREG(buffer.st_mode)) {
        int inode = buffer.st_ino;
        int nameCount = 0;
        if (lstat(argv[2], &buffer) == 0 && S_ISDIR(buffer.st_mode)) {
            nameCount = 0;
        }
    } else {
        if (lstat(argv[1], &buffer) == 0 && S_ISREG(buffer.st_mode)) {
            printf("Regular file: %s\n", argv[1]);
            printf("Status: %s\n", buffer.st_mode);
            printf("Inode: %d\n", buffer.st_ino);
            printf("Name count: %d\n", nameCount);
            printf("Size: %d bytes\n", buffer.st_size);
            printf("Name: %.*s\n", buffer.st_namelen, buffer.st_nametab[0]);
        } else {
            printf("Error: destination is not a regular file\n");
        }
    }

    while (1) {
        struct dirent *entry = NULL;
        if ((entry = readdir(fd)) == NULL) {
            break;
        }

        if (lstat(entry->d_name, &buffer) == 0 && S_ISDIR(buffer.st_mode)) {
            if (mkdir(argv[2] / entry->d_name, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
                perror("mkdir");
                return 1;
            }
        } else {
            if (lstat(argv[1] / entry->d_name, &buffer) == 0 && S_ISREG(buffer.st_mode)) {
                int inode = buffer.st_ino;
                int size = buffer.st_size;
                char buffer[256];
                FILE *fp = fopen(argv[1] / entry->d_name, "rb");
                if (fp == NULL) {
                    perror("open");
                    return 1;
                }
                if (fread(buffer, 256, size, fp) != size) {
                    perror("fread");
                    return 1;
                }
                fclose(fp);
                if (open(argv[2] / entry->d_name, "wb") == 0) {
                    if (write(statusBuffer, buffer, size) == size) {
                        printf("Regular file: %s\n", argv[1]);
                        printf("Status: %s\n", buffer.st_mode);
                        printf("Inode: %d\n", buffer.st_ino);
                        printf("Name count: %d\n", nameCount);
                        printf("Size: %d bytes\n", size);
                        printf("Name: %.*s\n", buffer.st_namelen, buffer.st_nametab[0]);
                    } else {
                        perror("write");
                        return 1;
                    }
                } else {
                    perror("open");
                    return 1;
                }
            } else {
                printf("Error: destination is not a regular file\n");
            }
        }

        wait(NULL);
    }

    // Close the shared memory
    munmap(statusBuffer, 256);

    return 0;
}

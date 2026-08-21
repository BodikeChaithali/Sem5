#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

int main(int argc, char *argv[]) {
    int fd;
    char buffer[1024];
    ssize_t bytes_read;

    long lines = 0;
    long words = 0;
    long bytes = 0;

    int in_word = 0;
    int i;

    if (argc != 2) {
        printf("Usage: ./fileinfo <filename>\n");
        return 1;
    }

    fd = open(argv[1], O_RDONLY);

    if (fd == -1) {
        printf("Error: Cannot open file\n");
        return 1;
    }

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {

        bytes += bytes_read;

        for (i = 0; i < bytes_read; i++) {

            if (buffer[i] == '\n') {
                lines++;
            }

            if (isspace((unsigned char)buffer[i])) {
                in_word = 0;
            }
            else if (in_word == 0) {
                words++;
                in_word = 1;
            }
        }
    }

    if (bytes_read == -1) {
        printf("Error: Cannot read file\n");
        close(fd);
        return 1;
    }

    close(fd);

    printf("File: %s\n", argv[1]);
    printf("Lines: %ld\n", lines);
    printf("Words: %ld\n", words);
    printf("Bytes: %ld\n", bytes);

    if (access(argv[1], R_OK) == 0) {
        printf("Readable: Yes\n");
    }
    else {
        printf("Readable: No\n");
    }

    if (access(argv[1], W_OK) == 0) {
        printf("Writable: Yes\n");
    }
    else {
        printf("Writable: No\n");
    }

    return 0;
}

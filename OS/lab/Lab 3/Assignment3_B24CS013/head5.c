#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int fd;
    char buffer[1024];
    ssize_t bytesRead;
    int lines = 0;
    int i;

    if (argc != 2) {
        write(STDERR_FILENO, "Usage: ./head5 <filename>\n", 26);
        return 1;
    }

    fd = open(argv[1], O_RDONLY);

    if (fd == -1) {
        write(STDERR_FILENO, "Error: Cannot open file\n", 24);
        return 1;
    }

    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {

        for (i = 0; i < bytesRead; i++) {

            write(STDOUT_FILENO, &buffer[i], 1);

            if (buffer[i] == '\n') {
                lines++;

                if (lines == 5) {
                    close(fd);
                    return 0;
                }
            }
        }
    }

    if (bytesRead == -1) {
        write(STDERR_FILENO, "Error: Cannot read file\n", 24);
        close(fd);
        return 1;
    }

    close(fd);

    return 0;
}

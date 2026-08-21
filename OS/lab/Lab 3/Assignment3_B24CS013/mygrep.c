#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int fd;
    char buffer[1024];
    char line[4096];
    int line_pos = 0;
    int line_number = 1;
    ssize_t bytes_read;
    int i;

    if (argc != 3) {
        printf("Usage: ./mygrep <search_word> <filename>\n");
        return 1;
    }

    fd = open(argv[2], O_RDONLY);

    if (fd == -1) {
        printf("Error: Cannot open file\n");
        return 1;
    }

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {

        for (i = 0; i < bytes_read; i++) {

            if (buffer[i] == '\n') {
                line[line_pos] = '\0';

                if (strstr(line, argv[1]) != NULL) {
                    printf("%d: %s\n", line_number, line);
                }

                line_number++;
                line_pos = 0;
            }
            else {
                if (line_pos < sizeof(line) - 1) {
                    line[line_pos] = buffer[i];
                    line_pos++;
                }
            }
        }
    }

    if (line_pos > 0) {
        line[line_pos] = '\0';

        if (strstr(line, argv[1]) != NULL) {
            printf("%d: %s\n", line_number, line);
        }
    }

    if (bytes_read == -1) {
        printf("Error: Cannot read file\n");
        close(fd);
        return 1;
    }

    close(fd);

    return 0;
}

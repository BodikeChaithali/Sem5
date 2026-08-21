#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int source_fd, destination_fd;
    char buffer[1024];
    ssize_t bytes_read, bytes_written;

    if (argc != 3) {
        write(STDERR_FILENO, "Usage: ./mycp <source> <destination>\n", 38);
        return 1;
    }

    source_fd = open(argv[1], O_RDONLY);

    if (source_fd == -1) {
        write(STDERR_FILENO, "Error: Cannot open source file\n", 31);
        return 1;
    }

    destination_fd = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, 0644);

    if (destination_fd == -1) {
        write(STDERR_FILENO, "Error: Cannot open destination file\n", 35);
        close(source_fd);
        return 1;
    }

    while ((bytes_read = read(source_fd, buffer, sizeof(buffer))) > 0) {

        bytes_written = write(destination_fd, buffer, bytes_read);

        if (bytes_written == -1) {
            write(STDERR_FILENO, "Error: Cannot write to destination file\n", 41);
            close(source_fd);
            close(destination_fd);
            return 1;
        }
    }

    if (bytes_read == -1) {
        write(STDERR_FILENO, "Error: Cannot read source file\n", 32);
        close(source_fd);
        close(destination_fd);
        return 1;
    }

    close(source_fd);
    close(destination_fd);

    return 0;
}

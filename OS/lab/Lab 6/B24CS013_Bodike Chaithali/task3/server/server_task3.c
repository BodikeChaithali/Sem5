#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define MAX_FILENAME 255
#define SOURCE_FILE "source_task3.c"
#define EXECUTABLE_FILE "program_task3"
#define COMPILE_ERROR_FILE "compile_error_task3.txt"
#define MAX_RESULT_SIZE (1024 * 1024)

static int send_all(int sock, const void *buf, size_t len) {
    const char *p = (const char *)buf;
    size_t total = 0;
    while (total < len) {
        ssize_t n = send(sock, p + total, len - total, 0);
        if (n < 0) { if (errno == EINTR) continue; return -1; }
        if (n == 0) return -1;
        total += (size_t)n;
    }
    return 0;
}

static int recv_all(int sock, void *buf, size_t len) {
    char *p = (char *)buf;
    size_t total = 0;
    while (total < len) {
        ssize_t n = recv(sock, p + total, len - total, 0);
        if (n < 0) { if (errno == EINTR) continue; return -1; }
        if (n == 0) return -1;
        total += (size_t)n;
    }
    return 0;
}

static void cleanup_files(void) {
    remove(SOURCE_FILE);
    remove(EXECUTABLE_FILE);
    remove(COMPILE_ERROR_FILE);
}

static int safe_filename(const char *name) {
    return name[0] != '\0' && strstr(name, "..") == NULL &&
           strchr(name, '/') == NULL && strchr(name, '\\') == NULL;
}

int main(void) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(server_fd); return 1;
    }
    if (listen(server_fd, 5) < 0) {
        perror("listen"); close(server_fd); return 1;
    }

    printf("Remote Execution Server listening on port %d...\n", PORT);

    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) { perror("accept"); continue; }

        printf("Client connected.\n");
        cleanup_files();

        uint32_t net_name_len;
        if (recv_all(client_fd, &net_name_len, sizeof(net_name_len)) < 0) { close(client_fd); continue; }
        uint32_t name_len = ntohl(net_name_len);
        if (name_len == 0 || name_len > MAX_FILENAME) { close(client_fd); continue; }

        char filename[MAX_FILENAME + 1];
        if (recv_all(client_fd, filename, name_len) < 0) { close(client_fd); continue; }
        filename[name_len] = '\0';
        if (!safe_filename(filename)) { close(client_fd); continue; }

        uint64_t file_size;
        if (recv_all(client_fd, &file_size, sizeof(file_size)) < 0) { close(client_fd); continue; }

        FILE *source = fopen(SOURCE_FILE, "wb");
        if (!source) { perror("fopen"); close(client_fd); continue; }

        char buffer[BUFFER_SIZE];
        uint64_t received = 0;
        int receive_ok = 1;
        while (received < file_size) {
            size_t want = (file_size - received > BUFFER_SIZE) ? BUFFER_SIZE : (size_t)(file_size - received);
            ssize_t n = recv(client_fd, buffer, want, 0);
            if (n <= 0) { receive_ok = 0; break; }
            if (fwrite(buffer, 1, (size_t)n, source) != (size_t)n) { receive_ok = 0; break; }
            received += (uint64_t)n;
        }
        fclose(source);

        if (!receive_ok) {
            cleanup_files();
            close(client_fd);
            continue;
        }

        printf("Received source file: %s (%llu bytes)\n", filename, (unsigned long long)file_size);
        printf("Compiling...\n");

        char compile_cmd[256];
        snprintf(compile_cmd, sizeof(compile_cmd),
                 "gcc %s -o %s 2> %s", SOURCE_FILE, EXECUTABLE_FILE, COMPILE_ERROR_FILE);
        int status = system(compile_cmd);

        if (status != 0) {
            FILE *err = fopen(COMPILE_ERROR_FILE, "rb");
            char *errors = calloc(1, MAX_RESULT_SIZE);
            size_t len = 0;
            if (errors && err) {
                len = fread(errors, 1, MAX_RESULT_SIZE - 1, err);
                fclose(err);
            } else if (err) {
                fclose(err);
            }

            uint32_t result_type = htonl(0); /* compile error */
            uint64_t net_len = (uint64_t)len;
            if (send_all(client_fd, &result_type, sizeof(result_type)) < 0 ||
                send_all(client_fd, &net_len, sizeof(net_len)) < 0 ||
                send_all(client_fd, errors ? errors : "Compilation failed.\n", errors ? len : strlen("Compilation failed.\n")) < 0) {
                free(errors);
                cleanup_files();
                close(client_fd);
                continue;
            }

            printf("Compilation failed; error sent to client.\n");
            free(errors);
            cleanup_files();
            close(client_fd);
            printf("Client disconnected.\n");
            continue;
        }

        printf("Compilation successful. Executing program with fork(), exec(), and pipe()...\n");

        int pipe_fd[2];
        if (pipe(pipe_fd) < 0) {
            perror("pipe");
            cleanup_files(); close(client_fd); continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            close(pipe_fd[0]); close(pipe_fd[1]);
            cleanup_files(); close(client_fd); continue;
        }

        if (pid == 0) {
            close(pipe_fd[0]);
            dup2(pipe_fd[1], STDOUT_FILENO);
            dup2(pipe_fd[1], STDERR_FILENO);
            close(pipe_fd[1]);
            execl("./" EXECUTABLE_FILE, "./" EXECUTABLE_FILE, (char *)NULL);
            _exit(127);
        }

        close(pipe_fd[1]);
        char *output = malloc(MAX_RESULT_SIZE);
        size_t output_len = 0;
        if (output) {
            while (output_len < MAX_RESULT_SIZE - 1) {
                ssize_t n = read(pipe_fd[0], output + output_len, MAX_RESULT_SIZE - 1 - output_len);
                if (n > 0) output_len += (size_t)n;
                else if (n == 0) break;
                else if (errno == EINTR) continue;
                else break;
            }
        }
        close(pipe_fd[0]);
        waitpid(pid, NULL, 0);

        uint32_t result_type = htonl(1); /* execution output */
        uint64_t net_len = (uint64_t)output_len;
        if (send_all(client_fd, &result_type, sizeof(result_type)) < 0 ||
            send_all(client_fd, &net_len, sizeof(net_len)) < 0 ||
            (output_len > 0 && send_all(client_fd, output, output_len) < 0)) {
            free(output);
            cleanup_files();
            close(client_fd);
            continue;
        }

        printf("Execution output sent directly to client (%zu bytes).\n", output_len);
        free(output);
        cleanup_files();
        close(client_fd);
        printf("Client disconnected.\n");
    }

    close(server_fd);
    return 0;
}

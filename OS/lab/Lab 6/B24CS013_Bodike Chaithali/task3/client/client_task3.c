#include <arpa/inet.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define MAX_FILENAME 255
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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <server_ip>\n", argv[0]);
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) != 1) {
        fprintf(stderr, "Invalid server IP address.\n"); close(sock); return 1;
    }
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect"); close(sock); return 1;
    }

    printf("Connected to server.\n");
    printf("Enter the C source filename (example: add.c).\n");
    printf("C source filename: ");
    fflush(stdout);

    char filename[MAX_FILENAME + 1];
    if (!fgets(filename, sizeof(filename), stdin)) { close(sock); return 1; }
    filename[strcspn(filename, "\r\n")] = '\0';
    if (filename[0] == '\0' || strstr(filename, "..") || strchr(filename, '/') || strchr(filename, '\\')) {
        printf("Invalid filename.\n"); close(sock); return 1;
    }

    FILE *fp = fopen(filename, "rb");
    if (!fp) { perror("fopen"); close(sock); return 1; }
    if (fseek(fp, 0, SEEK_END) != 0) { perror("fseek"); fclose(fp); close(sock); return 1; }
    long long size = ftell(fp);
    if (size < 0) { perror("ftell"); fclose(fp); close(sock); return 1; }
    rewind(fp);
    uint64_t file_size = (uint64_t)size;

    uint32_t name_len = htonl((uint32_t)strlen(filename));
    if (send_all(sock, &name_len, sizeof(name_len)) < 0 ||
        send_all(sock, filename, strlen(filename)) < 0 ||
        send_all(sock, &file_size, sizeof(file_size)) < 0) {
        perror("send"); fclose(fp); close(sock); return 1;
    }

    printf("Sending file...\n");
    char buffer[BUFFER_SIZE];
    uint64_t sent = 0;
    while (sent < file_size) {
        size_t n = fread(buffer, 1, BUFFER_SIZE, fp);
        if (n == 0) {
            printf("Error reading source file.\n");
            fclose(fp); close(sock); return 1;
        }
        if (send_all(sock, buffer, n) < 0) {
            perror("send"); fclose(fp); close(sock); return 1;
        }
        sent += (uint64_t)n;
    }
    fclose(fp);

    printf("File sent.\n");
    printf("Waiting for execution result...\n\n");

    uint32_t net_type;
    uint64_t result_len;
    if (recv_all(sock, &net_type, sizeof(net_type)) < 0 ||
        recv_all(sock, &result_len, sizeof(result_len)) < 0) {
        printf("Failed to receive result from server.\n");
        close(sock); return 1;
    }

    if (result_len >= MAX_RESULT_SIZE) {
        printf("Result too large.\n");
        close(sock); return 1;
    }

    char *result = calloc(1, (size_t)result_len + 1);
    if (!result) { perror("calloc"); close(sock); return 1; }

    if (result_len > 0 && recv_all(sock, result, (size_t)result_len) < 0) {
        printf("Failed to receive full result.\n");
        free(result); close(sock); return 1;
    }
    result[result_len] = '\0';

    uint32_t type = ntohl(net_type);
    if (type == 0) {
        printf("Compilation failed:\n");
        printf("--------------------------------\n");
        printf("%s", result);
        printf("--------------------------------\n");
    } else if (type == 1) {
        printf("Program Output:\n");
        printf("--------------------------------\n");
        printf("%s", result);
        printf("--------------------------------\n");
    } else {
        printf("Invalid result received from server.\n");
    }

    free(result);
    close(sock);
    printf("Connection closed.\n");
    return 0;
}


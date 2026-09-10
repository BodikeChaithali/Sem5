/*
 * client.c - Simple TCP Client (cross-machine version)
 * Compile: gcc client.c -o client
 * Run:     ./client <server_ip>
 * Example: ./client 192.168.1.25
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int sock_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE] = {0};
    const char *message = "Hello from client!";

    // Server IP must be passed as a command-line argument
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <server_ip>\n", argv[0]);
        fprintf(stderr, "Example: %s 192.168.1.25\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    const char *server_ip = argv[1];

    // 1. Create socket (IPv4, TCP)
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // 2. Convert IP address from text to binary form
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("invalid address / address not supported");
        exit(EXIT_FAILURE);
    }

    // 3. Connect to the server
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server at %s:%d\n", server_ip, PORT);

    // 4. Send message to server
    send(sock_fd, message, strlen(message), 0);
    printf("Message sent: %s\n", message);

    // 5. Read server's response
    memset(buffer, 0, BUFFER_SIZE);
    ssize_t bytes_read = read(sock_fd, buffer, BUFFER_SIZE - 1);
    if (bytes_read > 0) {
        printf("Server response: %s\n", buffer);
    }

    // 6. Close the socket
    close(sock_fd);
    return 0;
}

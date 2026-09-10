/*
 * server.c - Simple TCP Server
 * Compile: gcc server.c -o server
 * Run:     ./server
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    const char *response = "Hello from server!";

    // 1. Create socket (IPv4, TCP)
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Allow reuse of address (avoids "Address already in use" on restart)
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;   // bind to all interfaces
    address.sin_port = htons(PORT);

    // 2. Bind socket to IP/port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 3. Listen for incoming connections (backlog of 3)
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        // 4. Accept a client connection (blocking call)
        if ((client_fd = accept(server_fd, (struct sockaddr *)&address,
                                 (socklen_t *)&addrlen)) < 0) {
            perror("accept failed");
            continue;
        }

        printf("Client connected: %s:%d\n",
               inet_ntoa(address.sin_addr), ntohs(address.sin_port));

        // 5. Read data from client
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read > 0) {
            printf("Received from client: %s\n", buffer);
        }

        // 6. Send response back to client
        send(client_fd, response, strlen(response), 0);
        printf("Response sent to client\n");

        // 7. Close this client connection, go back to accepting new ones
        close(client_fd);
    }

    close(server_fd);
    return 0;
}

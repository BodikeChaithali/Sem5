#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in server_addr;

    char buffer[1024];

    // Check IP argument
    if (argc != 2)
    {
        printf("Usage: %s <server_ip>\n", argv[0]);
        return 1;
    }

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    // Server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Convert IP address
    if (inet_pton(AF_INET,
                  argv[1],
                  &server_addr.sin_addr) <= 0)
    {
        perror("Invalid server IP");
        close(sock);
        return 1;
    }

    // Connect
    if (connect(sock,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    printf("Connected to server.\n");
    while (1)
    {
        printf("kv>");
        char input[1000];

        Scanf("[^\n]",&input);
        // Send request
        snprintf(buffer,sizeof(buffer),input);
        send(sock,
             buffer,
             strlen(buffer),
             0);

        // Receive response
        memset(buffer, 0, sizeof(buffer));

        int bytes = read(sock,
                         buffer,
                         sizeof(buffer) - 1);

        if (bytes > 0)
        {
            buffer[bytes] = '\0';

            printf("\nServer: %s", buffer);
        }
        else
        {
            printf("Server disconnected.\n");
            break;
        }
    }

    // Close connection
    close(sock);

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    char buffer[1024];

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    printf("Socket created successfully.\n");

    // Allow reuse of port
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
               &opt, sizeof(opt));

    // Server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind
    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(1);
    }

    printf("Bind successful.\n");

    // Listen
    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        exit(1);
    }

    printf("Server listening on port %d...\n", PORT);

    // Keep accepting new clients
    while (1)
    {
        // Accept client
        client_fd = accept(
            server_fd,
            (struct sockaddr *)&client_addr,
            &addr_len
        );

        if (client_fd < 0)
        {
            perror("Accept failed");
            continue;
        }

        printf("\nClient connected: %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        // Handle multiple requests from same client
        while (1)
        {
            memset(buffer, 0, sizeof(buffer));

            int bytes = read(client_fd,
                             buffer,
                             sizeof(buffer) - 1);

            if (bytes <= 0)
            {
                printf("Client disconnected.\n");
                break;
            }

            buffer[bytes] = '\0';

            printf("Received: %s\n", buffer);

            // Check EXIT
            if (strcmp(buffer, "EXIT") == 0)
            {
                printf("Client requested exit.\n");
                break;
            }

            // Variables for request
            char operation[20];
            double value;
            double result;

            // Parse request
            if (sscanf(buffer, "%19s %lf",
                       operation, &value) != 2)
            {
                char *error = "Invalid request format\n";

                send(client_fd,
                     error,
                     strlen(error),
                     0);

                continue;
            }

            // CM_TO_M
            if (strcmp(operation, "CM_TO_M") == 0)
            {
                result = value / 100.0;

                char response[1024];

                snprintf(response,
                         sizeof(response),
                         "%.2f cm = %.2f m\n",
                         value,
                         result);

                send(client_fd,
                     response,
                     strlen(response),
                     0);

                printf("Sent: %s", response);
            }

            // M_TO_KM
            else if (strcmp(operation, "M_TO_KM") == 0)
            {
                result = value / 1000.0;

                char response[1024];

                snprintf(response,
                         sizeof(response),
                         "%.2f m = %.2f km\n",
                         value,
                         result);

                send(client_fd,
                     response,
                     strlen(response),
                     0);

                printf("Sent: %s", response);
            }

            // C_TO_F
            else if (strcmp(operation, "C_TO_F") == 0)
            {
                result = (value * 9.0 / 5.0) + 32.0;

                char response[1024];

                snprintf(response,
                         sizeof(response),
                         "%.2f C = %.2f F\n",
                         value,
                         result);

                send(client_fd,
                     response,
                     strlen(response),
                     0);

                printf("Sent: %s", response);
            }

            // KG_TO_G
            else if (strcmp(operation, "KG_TO_G") == 0)
            {
                result = value * 1000.0;

                char response[1024];

                snprintf(response,
                         sizeof(response),
                         "%.2f kg = %.2f g\n",
                         value,
                         result);

                send(client_fd,
                     response,
                     strlen(response),
                     0);

                printf("Sent: %s", response);
            }

            // Unknown operation
            else
            {
                char *error = "Invalid conversion operation\n";

                send(client_fd,
                     error,
                     strlen(error),
                     0);

                printf("Sent: %s", error);
            }
        }

        // Close only the client connection
        close(client_fd);

        printf("Client connection closed.\n");
    }

    // Close server socket
    close(server_fd);

    return 0;
}

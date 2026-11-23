#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080

struct pair {
  char key[1000];
  char value[1000];
}

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    struct P = pair;
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

        // Communication loop
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

            printf("Received: %s", buffer);

            // Check for EXIT
            if (strncmp(buffer, "EXIT", 4) == 0)
            {
                printf("Client requested exit.\n");
                break;
            }

            // Receive:
            // operation number1 number2
            //
            // Example:
            // ADD 10 20
            // SUB 50 20
            // MUL 5 6
            char operation[256];
            double num1, num2, result;
            char op[3],city[100],V[100];
	    sscanf(buffer,"%s",op,city,V);
	    // Prepare response
            char response[1024];
	    if(op == 'S') {
	      //find Key
	      P.value = V;
	      send(client_fd,
                     "OK",
                     strlen(2),
                     0);
                     snprintf(response,
                     sizeof(response),
                     "Set city %s %s gives OK %s\n",
                    city,V);
            }
            if(op == 'G') {
              //file key
              send(client_fd,
                     P.value,
                     strlen(p.valye),
                     0);
                     snprintf(response,
                     sizeof(response),
                     "Get city gives VALUE %s %s\n",
                     city,V);
              }
	      
            if (sscanf(buffer, "%s",
                       op,
                       city,
                       V) != 3)
            {
                char *error = "Invalid request\n";

                send(client_fd,
                     error,
                     strlen(error),
                     0);

                continue;
            }
            

            // Send result
            send(client_fd,
                 response,
                 strlen(response),
                 0);

            printf("Sent: %s", response);
        }

        close(client_fd);

        printf("Client connection closed.\n");
    }

    close(server_fd);

    return 0;
}

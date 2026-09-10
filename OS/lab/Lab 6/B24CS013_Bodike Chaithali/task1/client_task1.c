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

    // Convert server IP address
    if (inet_pton(AF_INET,
                  argv[1],
                  &server_addr.sin_addr) <= 0)
    {
        perror("Invalid server IP");
        close(sock);
        return 1;
    }

    // Connect to server
    if (connect(sock,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    printf("Connected to unit converter server.\n");

    // Conversion loop
    while (1)
    {
        int choice;
        double value;

        printf("\n");
        printf("========== UNIT CONVERTER ==========\n");
        printf("1. Centimeter to Meter\n");
        printf("2. Meter to Kilometer\n");
        printf("3. Celsius to Fahrenheit\n");
        printf("4. Kilogram to Gram\n");
        printf("5. Exit\n");
        printf("====================================\n");

        printf("Enter your choice: ");
        scanf("%d", &choice);

        // Exit
        if (choice == 5)
        {
            strcpy(buffer, "EXIT");

            send(sock,
                 buffer,
                 strlen(buffer),
                 0);

            printf("Exiting unit converter...\n");
            break;
        }

        // Validate choice
        if (choice < 1 || choice > 5)
        {
            printf("Invalid choice!\n");
            continue;
        }

        // Get value
        printf("Enter value: ");
        scanf("%lf", &value);

        // Prepare request
        if (choice == 1)
        {
            snprintf(buffer,
                     sizeof(buffer),
                     "CM_TO_M %.2f",
                     value);
        }
        else if (choice == 2)
        {
            snprintf(buffer,
                     sizeof(buffer),
                     "M_TO_KM %.2f",
                     value);
        }
        else if (choice == 3)
        {
            snprintf(buffer,
                     sizeof(buffer),
                     "C_TO_F %.2f",
                     value);
        }
        else if (choice == 4)
        {
            snprintf(buffer,
                     sizeof(buffer),
                     "KG_TO_G %.2f",
                     value);
        }

        // Send request
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

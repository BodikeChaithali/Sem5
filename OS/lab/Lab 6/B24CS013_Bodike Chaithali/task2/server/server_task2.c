#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define COMMAND_SIZE 256

/*
 * Receive exactly size bytes
 */
int receive_all(int sock, void *buffer, size_t size)
{
    size_t total = 0;
    char *ptr = (char *)buffer;

    while (total < size)
    {
        int bytes = recv(sock,
                         ptr + total,
                         size - total,
                         0);

        if (bytes <= 0)
        {
            return -1;
        }

        total += bytes;
    }

    return 0;
}

/*
 * Send exactly size bytes
 */
int send_all(int sock, const void *buffer, size_t size)
{
    size_t total = 0;
    const char *ptr = (const char *)buffer;

    while (total < size)
    {
        int bytes = send(sock,
                         ptr + total,
                         size - total,
                         0);

        if (bytes <= 0)
        {
            return -1;
        }

        total += bytes;
    }

    return 0;
}

int main()
{
    int server_fd, client_fd;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    socklen_t addr_len = sizeof(client_addr);

    char command[COMMAND_SIZE];
    char buffer[BUFFER_SIZE];

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

    setsockopt(server_fd,
               SOL_SOCKET,
               SO_REUSEADDR,
               &opt,
               sizeof(opt));

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

    // Accept clients continuously
    while (1)
    {
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

        // Persistent connection
        while (1)
        {
            /*
             * Receive exactly 256 bytes
             * containing the command.
             */
            memset(command, 0, sizeof(command));

            if (receive_all(client_fd,
                            command,
                            COMMAND_SIZE) < 0)
            {
                printf("Client disconnected.\n");
                break;
            }

            printf("Received command: %s\n", command);

            /*
             * EXIT
             */
            if (strcmp(command, "EXIT") == 0)
            {
                printf("Client requested exit.\n");
                break;
            }

            /*
             * UPLOAD
             */
            if (strncmp(command, "UPLOAD ", 7) == 0)
            {
                char filename[256];
                long long file_size;

                memset(filename, 0, sizeof(filename));

                if (sscanf(command + 7,
                           "%255s",
                           filename) != 1)
                {
                    char *error = "Invalid upload request\n";

                    send(client_fd,
                         error,
                         strlen(error),
                         0);

                    continue;
                }

                /*
                 * Receive file size.
                 */
                if (receive_all(client_fd,
                                &file_size,
                                sizeof(file_size)) < 0)
                {
                    printf("Failed to receive file size.\n");
                    break;
                }

                printf("File size: %lld bytes\n",
                       file_size);

                /*
                 * Validate filename.
                 */
                if (strstr(filename, "..") != NULL ||
                    strchr(filename, '/') != NULL ||
                    strchr(filename, '\\') != NULL)
                {
                    char *error = "Invalid filename\n";

                    send(client_fd,
                         error,
                         strlen(error),
                         0);

                    continue;
                }

                /*
                 * Validate file size.
                 */
                if (file_size < 0)
                {
                    char *error = "Invalid file size\n";

                    send(client_fd,
                         error,
                         strlen(error),
                         0);

                    continue;
                }

                /*
                 * Open file for writing.
                 */
                FILE *fp = fopen(filename, "wb");

                if (fp == NULL)
                {
                    char *error =
                        "Cannot create file on server\n";

                    send(client_fd,
                         error,
                         strlen(error),
                         0);

                    continue;
                }

                /*
                 * Receive file data.
                 */
                long long remaining = file_size;

                while (remaining > 0)
                {
                    int to_receive;

                    if (remaining > BUFFER_SIZE)
                    {
                        to_receive = BUFFER_SIZE;
                    }
                    else
                    {
                        to_receive = (int)remaining;
                    }

                    int received = recv(client_fd,
                                        buffer,
                                        to_receive,
                                        0);

                    if (received <= 0)
                    {
                        printf("File transfer interrupted.\n");

                        fclose(fp);
                        remove(filename);

                        close(client_fd);

                        return 1;
                    }

                    fwrite(buffer,
                           1,
                           received,
                           fp);

                    remaining -= received;
                }

                fclose(fp);

                printf("File uploaded successfully: %s\n",
                       filename);

                /*
                 * Send confirmation.
                 */
                char response[COMMAND_SIZE];

                memset(response, 0, sizeof(response));

                strcpy(response,
                       "UPLOAD SUCCESS\n");

                send_all(client_fd,
                         response,
                         COMMAND_SIZE);
            }

            /*
             * DOWNLOAD
             */
            else if (strncmp(command, "DOWNLOAD ", 9) == 0)
            {
                char filename[256];

                memset(filename, 0, sizeof(filename));

                if (sscanf(command + 9,
                           "%255s",
                           filename) != 1)
                {
                    long long error_size = -1;

                    send_all(client_fd,
                             &error_size,
                             sizeof(error_size));

                    continue;
                }

                /*
                 * Validate filename.
                 */
                if (strstr(filename, "..") != NULL ||
                    strchr(filename, '/') != NULL ||
                    strchr(filename, '\\') != NULL)
                {
                    long long error_size = -1;

                    send_all(client_fd,
                             &error_size,
                             sizeof(error_size));

                    continue;
                }

                /*
                 * Open file.
                 */
                FILE *fp = fopen(filename, "rb");

                if (fp == NULL)
                {
                    printf("File not found: %s\n",
                           filename);

                    long long file_size = -1;

                    send_all(client_fd,
                             &file_size,
                             sizeof(file_size));

                    continue;
                }

                /*
                 * Find file size.
                 */
                fseek(fp, 0, SEEK_END);

                long long file_size = ftell(fp);

                fseek(fp, 0, SEEK_SET);

                printf("Sending file: %s (%lld bytes)\n",
                       filename,
                       file_size);

                /*
                 * Send file size.
                 */
                if (send_all(client_fd,
                             &file_size,
                             sizeof(file_size)) < 0)
                {
                    fclose(fp);
                    break;
                }

                /*
                 * Send file contents.
                 */
                long long remaining = file_size;

                while (remaining > 0)
                {
                    size_t chunk_size;

                    if (remaining > BUFFER_SIZE)
                    {
                        chunk_size = BUFFER_SIZE;
                    }
                    else
                    {
                        chunk_size = (size_t)remaining;
                    }

                    size_t bytes_read = fread(
                        buffer,
                        1,
                        chunk_size,
                        fp
                    );

                    if (bytes_read <= 0)
                    {
                        printf("File reading error.\n");
                        break;
                    }

                    if (send_all(client_fd,
                                 buffer,
                                 bytes_read) < 0)
                    {
                        printf("File sending failed.\n");
                        break;
                    }

                    remaining -= bytes_read;
                }

                fclose(fp);

                printf("File sent successfully: %s\n",
                       filename);
            }

            /*
             * Invalid command
             */
            else
            {
                char *error = "Invalid command\n";

                send(client_fd,
                     error,
                     strlen(error),
                     0);

                printf("Invalid command received.\n");
            }
        }

        close(client_fd);

        printf("Client connection closed.\n");
    }

    close(server_fd);

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define COMMAND_SIZE 256
#define FILENAME_SIZE 240

/*
 * Send exactly size bytes
 */
int send_all(int sock, const void *buffer, size_t size)
{
    size_t total = 0;
    const char *ptr = (const char *)buffer;

    while (total < size)
    {
        ssize_t bytes = send(sock,
                             ptr + total,
                             size - total,
                             0);

        if (bytes <= 0)
        {
            return -1;
        }

        total += (size_t)bytes;
    }

    return 0;
}

/*
 * Receive exactly size bytes
 */
int receive_all(int sock, void *buffer, size_t size)
{
    size_t total = 0;
    char *ptr = (char *)buffer;

    while (total < size)
    {
        ssize_t bytes = recv(sock,
                             ptr + total,
                             size - total,
                             0);

        if (bytes <= 0)
        {
            return -1;
        }

        total += (size_t)bytes;
    }

    return 0;
}

/*
 * Remove newline from string
 */
void remove_newline(char *str)
{
    str[strcspn(str, "\n")] = '\0';
}

/*
 * Clear remaining characters from input
 */
void clear_input_buffer()
{
    int ch;

    while ((ch = getchar()) != '\n' && ch != EOF)
    {
        /* Clear input buffer */
    }
}

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in server_addr;

    char command[COMMAND_SIZE];
    char buffer[BUFFER_SIZE];

    /*
     * Check command-line argument
     */
    if (argc != 2)
    {
        printf("Usage: %s <server_ip>\n", argv[0]);
        return 1;
    }

    /*
     * Create TCP socket
     */
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    /*
     * Server address
     */
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    /*
     * Convert server IP address
     */
    if (inet_pton(AF_INET,
                  argv[1],
                  &server_addr.sin_addr) <= 0)
    {
        printf("Invalid server IP address.\n");
        close(sock);
        return 1;
    }

    /*
     * Connect to server
     */
    if (connect(sock,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    printf("Connected to file transfer server.\n");

    /*
     * Persistent connection
     */
    while (1)
    {
        int choice;

        printf("\n");
        printf("====================================\n");
        printf("       FILE TRANSFER SYSTEM\n");
        printf("====================================\n");
        printf("1. Upload File\n");
        printf("2. Download File\n");
        printf("3. Exit\n");
        printf("====================================\n");

        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1)
        {
            printf("Invalid choice! Enter 1, 2, or 3.\n");
            clear_input_buffer();
            continue;
        }

        clear_input_buffer();

        /*
         * EXIT
         */
        if (choice == 3)
        {
            memset(command, 0, sizeof(command));

            strcpy(command, "EXIT");

            if (send_all(sock,
                         command,
                         COMMAND_SIZE) < 0)
            {
                printf("Failed to send exit request.\n");
            }
            else
            {
                printf("Exiting file transfer system...\n");
            }

            break;
        }

        /*
         * UPLOAD
         */
        if (choice == 1)
        {
            char filename[FILENAME_SIZE];

            printf("Enter filename to upload: ");

            if (fgets(filename,
                      sizeof(filename),
                      stdin) == NULL)
            {
                printf("Invalid filename.\n");
                continue;
            }

            remove_newline(filename);

            if (strlen(filename) == 0)
            {
                printf("Filename cannot be empty.\n");
                continue;
            }

            /*
             * Open file
             */
            FILE *fp = fopen(filename, "rb");

            if (fp == NULL)
            {
                printf("File not found: %s\n",
                       filename);
                continue;
            }

            /*
             * Find file size
             */
            if (fseek(fp, 0, SEEK_END) != 0)
            {
                printf("Unable to determine file size.\n");
                fclose(fp);
                continue;
            }

            long long file_size = ftell(fp);

            if (file_size < 0)
            {
                printf("Unable to determine file size.\n");
                fclose(fp);
                continue;
            }

            if (fseek(fp, 0, SEEK_SET) != 0)
            {
                printf("Unable to read file.\n");
                fclose(fp);
                continue;
            }

            /*
             * Prepare command
             */
            memset(command, 0, sizeof(command));

            snprintf(command,
                     sizeof(command),
                     "UPLOAD %s",
                     filename);

            /*
             * Send command
             */
            if (send_all(sock,
                         command,
                         COMMAND_SIZE) < 0)
            {
                printf("Failed to send upload request.\n");
                fclose(fp);
                break;
            }

            /*
             * Send file size
             */
            if (send_all(sock,
                         &file_size,
                         sizeof(file_size)) < 0)
            {
                printf("Failed to send file size.\n");
                fclose(fp);
                break;
            }

            printf("Uploading %s (%lld bytes)...\n",
                   filename,
                   file_size);

            /*
             * Send file contents
             * using 1024-byte buffer
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

                size_t bytes_read = fread(buffer,
                                          1,
                                          chunk_size,
                                          fp);

                if (bytes_read == 0)
                {
                    printf("File reading error.\n");
                    fclose(fp);
                    break;
                }

                if (send_all(sock,
                             buffer,
                             bytes_read) < 0)
                {
                    printf("File sending failed.\n");
                    fclose(fp);
                    close(sock);
                    return 1;
                }

                remaining -= (long long)bytes_read;
            }

            fclose(fp);

            /*
             * Receive server response
             */
            memset(command, 0, sizeof(command));

            if (receive_all(sock,
                            command,
                            COMMAND_SIZE) < 0)
            {
                printf("Server disconnected.\n");
                break;
            }

            printf("Server: %s",
                   command);
        }

        /*
         * DOWNLOAD
         */
        else if (choice == 2)
        {
            char filename[FILENAME_SIZE];

            printf("Enter filename to download: ");

            if (fgets(filename,
                      sizeof(filename),
                      stdin) == NULL)
            {
                printf("Invalid filename.\n");
                continue;
            }

            remove_newline(filename);

            if (strlen(filename) == 0)
            {
                printf("Filename cannot be empty.\n");
                continue;
            }

            /*
             * Open local file before sending request.
             * This prevents protocol problems if the
             * local file cannot be created.
             */
            FILE *fp = fopen(filename, "wb");

            if (fp == NULL)
            {
                printf("Cannot create local file: %s\n",
                       filename);
                continue;
            }

            /*
             * Prepare download command
             */
            memset(command, 0, sizeof(command));

            snprintf(command,
                     sizeof(command),
                     "DOWNLOAD %s",
                     filename);

            /*
             * Send command
             */
            if (send_all(sock,
                         command,
                         COMMAND_SIZE) < 0)
            {
                printf("Failed to send download request.\n");
                fclose(fp);
                break;
            }

            /*
             * Receive file size
             */
            long long file_size;

            if (receive_all(sock,
                            &file_size,
                            sizeof(file_size)) < 0)
            {
                printf("Failed to receive file information.\n");
                fclose(fp);
                break;
            }

            /*
             * File not found on server
             */
            if (file_size < 0)
            {
                printf("Server: File not found.\n");
                fclose(fp);
                remove(filename);
                continue;
            }

            printf("Downloading %s (%lld bytes)...\n",
                   filename,
                   file_size);

            /*
             * Receive file contents
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

                ssize_t received = recv(sock,
                                        buffer,
                                        chunk_size,
                                        0);

                if (received <= 0)
                {
                    printf("File transfer interrupted.\n");

                    fclose(fp);
                    remove(filename);
                    close(sock);

                    return 1;
                }

                size_t written = fwrite(buffer,
                                        1,
                                        (size_t)received,
                                        fp);

                if (written != (size_t)received)
                {
                    printf("Error writing downloaded file.\n");

                    fclose(fp);
                    remove(filename);
                    close(sock);

                    return 1;
                }

                remaining -= (long long)received;
            }

            fclose(fp);

            printf("File downloaded successfully: %s\n",
                   filename);
        }

        /*
         * Invalid menu choice
         */
        else
        {
            printf("Invalid choice! Enter 1, 2, or 3.\n");
        }
    }

    /*
     * Close socket
     */
    close(sock);

    return 0;
}

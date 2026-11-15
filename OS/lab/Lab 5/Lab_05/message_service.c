#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define SIZE 200

int main() {
    int pipe1[2];
    int pipe2[2];
    pid_t pid;

    char message[SIZE];
    char acknowledgement[SIZE];

    if (pipe(pipe1) == -1) {
        perror("pipe");
        return 1;
    }

    if (pipe(pipe2) == -1) {
        perror("pipe");
        return 1;
    }

    pid = fork();

    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        close(pipe1[1]);
        close(pipe2[0]);

        while (1) {
            memset(message, 0, SIZE);

            if (read(pipe1[0], message, SIZE) <= 0) {
                break;
            }

            if (strcmp(message, "exit") == 0) {
                break;
            }

            printf("Child received: %s\n", message);

            strcpy(acknowledgement, "Message received successfully");

            write(pipe2[1], acknowledgement, SIZE);
        }

        close(pipe1[0]);
        close(pipe2[1]);

        exit(0);
    } else {
        close(pipe1[0]);
        close(pipe2[1]);

        while (1) {
            printf("\nEnter message (type exit to stop): ");
            fflush(stdout);

            fgets(message, SIZE, stdin);
            message[strcspn(message, "\n")] = '\0';

            write(pipe1[1], message, SIZE);

            if (strcmp(message, "exit") == 0) {
                break;
            }

            memset(acknowledgement, 0, SIZE);

            read(pipe2[0], acknowledgement, SIZE);

            printf("Parent received acknowledgement: %s\n",
                   acknowledgement);
        }

        close(pipe1[1]);
        close(pipe2[0]);

        wait(NULL);

        printf("Communication terminated.\n");
    }

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    pid_t pid;
    int status;

    /* Check command-line arguments */
    if (argc != 3)
    {
        printf("Usage: %s <command> <argument>\n", argv[0]);
        printf("Example: %s sleep 10\n", argv[0]);
        return 1;
    }

    /* Create child process */
    pid = fork();

    if (pid < 0)
    {
        perror("fork failed");
        return 1;
    }

    /* Child process */
    if (pid == 0)
    {
        printf("Child: executing %s %s\n", argv[1], argv[2]);

        execvp(argv[1], &argv[1]);

        /* This executes only if execvp fails */
        perror("exec failed");
        exit(1);
    }

    /* Parent process */
    else
    {
        printf("Parent: waiting for child...\n");

        if (waitpid(pid, &status, 0) == -1)
        {
            perror("waitpid failed");
            return 1;
        }

        /* Check whether child exited normally */
        if (WIFEXITED(status))
        {
            if (WEXITSTATUS(status) == 0)
            {
                printf("Child completed successfully.\n");
            }
            else
            {
                printf("Child exited with status %d.\n", WEXITSTATUS(status));
            }
        }
        else
        {
            printf("Child did not exit normally.\n");
        }
    }

    return 0;
}

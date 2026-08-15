#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

/* Tokenize input */
char **tokenize(char *line)
{
    char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
    char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));

    int i, tokenIndex = 0, tokenNo = 0;

    for (i = 0; i < strlen(line); i++)
    {
        char c = line[i];

        if (c == ' ' || c == '\t' || c == '\n')
        {
            token[tokenIndex] = '\0';

            if (tokenIndex != 0)
            {
                tokens[tokenNo] = (char *)malloc(MAX_TOKEN_SIZE);
                strcpy(tokens[tokenNo], token);
                tokenNo++;
                tokenIndex = 0;
            }
        }
        else
        {
            token[tokenIndex++] = c;
        }
    }

    free(token);
    tokens[tokenNo] = NULL;

    return tokens;
}

/* Reap terminated background processes */
void reap_background_processes()
{
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        printf("Shell: Background process finished\n");
    }
}

int main()
{
    char line[MAX_INPUT_SIZE];
    char cwd[256];
    char **tokens;

    while (1)
    {
        /*
         * Reap any background processes that have already
         * finished. WNOHANG ensures that the shell does not wait.
         */
        reap_background_processes();

        /* Display current working directory */
        getcwd(cwd, sizeof(cwd));
        printf("%s $ ", cwd);
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL)
            break;

        /* Ignore empty command */
        if (strcmp(line, "\n") == 0)
            continue;

        tokens = tokenize(line);

        if (tokens[0] == NULL)
        {
            free(tokens);
            continue;
        }

        /*
         * Check whether command is a background command.
         * The last token must be "&".
         */
        int background = 0;
        int tokenCount = 0;

        while (tokens[tokenCount] != NULL)
        {
            tokenCount++;
        }

        if (tokenCount > 1 &&
            strcmp(tokens[tokenCount - 1], "&") == 0)
        {
            background = 1;

            /* Remove "&" from arguments passed to execvp */
            free(tokens[tokenCount - 1]);
            tokens[tokenCount - 1] = NULL;
        }

        /* Built-in: cd */
        if (strcmp(tokens[0], "cd") == 0)
        {
            if (background)
            {
                printf("cd cannot be executed in background\n");
            }
            else if (tokens[1] == NULL || tokens[2] != NULL)
            {
                printf("Usage: cd <directory>\n");
            }
            else
            {
                if (chdir(tokens[1]) != 0)
                    perror("cd");
            }

            for (int i = 0; tokens[i] != NULL; i++)
                free(tokens[i]);

            free(tokens);

            continue;
        }

        /* Create child process */
        pid_t pid = fork();

        if (pid < 0)
        {
            perror("fork");

            for (int i = 0; tokens[i] != NULL; i++)
                free(tokens[i]);

            free(tokens);

            continue;
        }

        /* Child process */
        if (pid == 0)
        {
            execvp(tokens[0], tokens);

            /* exec failed */
            perror(tokens[0]);
            exit(1);
        }

        /* Parent process */

        if (background)
        {
            /*
             * Background process:
             * Do NOT wait for the child.
             * Immediately return to the prompt.
             */
            printf("Shell: Background process started (PID = %d)\n", pid);
        }
        else
        {
            /*
             * Foreground process:
             * Wait specifically for this child.
             *
             * This is important because waitpid(pid, ...)
             * cannot accidentally reap a background child.
             */
            int status;

            if (waitpid(pid, &status, 0) == -1)
            {
                perror("waitpid");
            }
            else if (WIFEXITED(status))
            {
                printf("EXITSTATUS: %d\n", WEXITSTATUS(status));
            }
        }

        /* Free memory */
        for (int i = 0; tokens[i] != NULL; i++)
            free(tokens[i]);

        free(tokens);

        /*
         * Reap background processes that may have finished
         * while the foreground command was running.
         */
        reap_background_processes();
    }

    /*
     * Final cleanup: reap any children that have already
     * terminated without blocking.
     */
    reap_background_processes();

    return 0;
}

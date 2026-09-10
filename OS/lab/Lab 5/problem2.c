#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

typedef struct
{
    int addition;
    int subtraction;
    int multiplication;
} Results;

int main()
{
    int pipe1[2];
    int pipe2[2];

    pid_t pid;

    int num1, num2;
    Results result;
    if (pipe(pipe1) == -1)
    {
        perror("pipe1");
        exit(1);
    }
    if (pipe(pipe2) == -1)
    {
        perror("pipe2");
        exit(1);
    }
    pid = fork();

    if (pid < 0)
    {
        perror("fork");
        exit(1);
    }
    if (pid > 0)
    {
        close(pipe1[0]);
        close(pipe2[1]);
        printf("Enter first number: ");
        scanf("%d", &num1);

        printf("Enter second number: ");
        scanf("%d", &num2);
        write(pipe1[1], &num1, sizeof(num1));
        write(pipe1[1], &num2, sizeof(num2));
        close(pipe1[1]);
        read(pipe2[0], &result, sizeof(result));
        printf("\nResults received from child:\n");

        printf("Addition       = %d\n",
               result.addition);

        printf("Subtraction    = %d\n",
               result.subtraction);

        printf("Multiplication = %d\n",
               result.multiplication);
        close(pipe2[0]);
        wait(NULL);
    }
    else {
        close(pipe1[1]);
        close(pipe2[0]);
        read(pipe1[0], &num1, sizeof(num1));
        read(pipe1[0], &num2, sizeof(num2));
        close(pipe1[0]);
        result.addition = num1 + num2;

        result.subtraction = num1 - num2;

        result.multiplication = num1 * num2;
        write(pipe2[1], &result, sizeof(result));
        close(pipe2[1]);
    }

    return 0;
}

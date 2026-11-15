#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

struct Numbers {
    int num1;
    int num2;
};

struct Results {
    int addition;
    int subtraction;
    int multiplication;
};

int main() {
    int pipe1[2];
    int pipe2[2];

    pid_t pid;

    struct Numbers numbers;
    struct Results results;

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

    /*
       CHILD PROCESS
    */
    if (pid == 0) {

        close(pipe1[1]);
        close(pipe2[0]);

        read(pipe1[0], &numbers, sizeof(numbers));

        printf("\nChild received numbers:\n");
        printf("Number 1: %d\n", numbers.num1);
        printf("Number 2: %d\n", numbers.num2);

        results.addition =
            numbers.num1 + numbers.num2;

        results.subtraction =
            numbers.num1 - numbers.num2;

        results.multiplication =
            numbers.num1 * numbers.num2;

        printf("\nChild calculated results.\n");

        write(pipe2[1], &results, sizeof(results));

        close(pipe1[0]);
        close(pipe2[1]);

        exit(0);
    }

    /*
       PARENT PROCESS
    */
    else {

        close(pipe1[0]);
        close(pipe2[1]);

        printf("Enter first number: ");
        scanf("%d", &numbers.num1);

        printf("Enter second number: ");
        scanf("%d", &numbers.num2);

        write(pipe1[1], &numbers, sizeof(numbers));

        printf("\nParent sent numbers to child.\n");

        read(pipe2[0], &results, sizeof(results));

        printf("\nResults received from child:\n");
        printf("Addition       : %d\n",
               results.addition);

        printf("Subtraction    : %d\n",
               results.subtraction);

        printf("Multiplication : %d\n",
               results.multiplication);

        close(pipe1[1]);
        close(pipe2[0]);

        wait(NULL);

        printf("\nParent-child communication completed.\n");
    }

    return 0;
}

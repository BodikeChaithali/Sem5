#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define SIZE 100

struct Job {
    int job_id;
    char operation[20];
    int value;
};

struct Result {
    int job_id;
    long long result;
};

long long factorial(int n) {
    long long fact = 1;

    for (int i = 1; i <= n; i++) {
        fact = fact * i;
    }

    return fact;
}

int main() {
    int pipe1[2];
    int pipe2[2];
    pid_t pid;

    struct Job job;
    struct Result result;

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
            if (read(pipe1[0], &job, sizeof(job)) <= 0) {
                break;
            }

            if (job.job_id == -1) {
                break;
            }

            result.job_id = job.job_id;

            if (strcmp(job.operation, "square") == 0) {
                result.result = (long long)job.value * job.value;
            }
            else if (strcmp(job.operation, "cube") == 0) {
                result.result = (long long)job.value *
                                job.value *
                                job.value;
            }
            else if (strcmp(job.operation, "double") == 0) {
                result.result = (long long)2 * job.value;
            }
            else if (strcmp(job.operation, "factorial") == 0) {
                result.result = factorial(job.value);
            }
            else {
                result.result = -1;
            }

            write(pipe2[1], &result, sizeof(result));
        }

        close(pipe1[0]);
        close(pipe2[1]);

        exit(0);
    }
    else {
        close(pipe1[0]);
        close(pipe2[1]);

        while (1) {
            printf("\nEnter Job ID (-1 to stop): ");
            scanf("%d", &job.job_id);

            if (job.job_id == -1) {
                write(pipe1[1], &job, sizeof(job));
                break;
            }

            printf("Enter Operation: ");
            scanf("%s", job.operation);

            printf("Enter Input Value: ");
            scanf("%d", &job.value);

            write(pipe1[1], &job, sizeof(job));

            read(pipe2[0], &result, sizeof(result));

            if (result.result == -1) {
                printf("Invalid operation.\n");
            } else {
                printf("Job ID: %d\n", result.job_id);
                printf("Result: %lld\n", result.result);
            }
        }

        close(pipe1[1]);
        close(pipe2[0]);

        wait(NULL);

        printf("Job processing terminated.\n");
    }

    return 0;
}

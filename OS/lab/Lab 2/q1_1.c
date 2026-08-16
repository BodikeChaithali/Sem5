#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(void)
{
    pid_t pid;

    pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid == 0)
    {
        printf("I am child. PID = %d\n", getpid());
    }
    else
    {
        printf("I am parent. PID = %d\n", getpid());
    }

    return 0;
}

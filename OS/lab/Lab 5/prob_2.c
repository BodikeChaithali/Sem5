#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int fd1[2],fd2[2];

    if(pipe(fd1) == -1 || pipe(fd2) == -1) {
        perror("pipe");
        exit(1);
    }

    pid_t pid = fork();

    int a , b;
    if(pid < 0) {
        perror("pid");
        exit(1);
    }
    else if(pid == 0) {
        close(fd1[1]);
        close(fd2[0]);
        read(fd1[0],&a,sizeof(int));
        read(fd1[0],&b,sizeof(int));
        printf("Child: Received a = %d, b = %d\n",a,b);
        int sum = a + b;
        write(fd2[1],&sum,sizeof(int));
        int sub = a - b;
        write(fd2[1],&sub,sizeof(int));
        int mul = a * b;
        write(fd2[1],&mul,sizeof(int));
        close(fd1[0]);
        close(fd2[1]);
    }
    else {
        close(fd1[0]);
        close(fd2[1]);
        printf("Parent: Enter two integers: ");
        scanf("%d %d",&a,&b);
        write(fd1[1],&a,sizeof(int));
        write(fd1[1],&b,sizeof(int));
        int sum,sub,mul;
        read(fd2[0],&sum,sizeof(int));
        read(fd2[0],&sub,sizeof(int));
        read(fd2[0],&mul,sizeof(int));
        printf("Parent: Received sum = %d, sub = %d, mul = %d\n",sum,sub,mul);
        close(fd1[1]);
        close(fd2[0]);
        wait(NULL);
    }
    return 0;
}
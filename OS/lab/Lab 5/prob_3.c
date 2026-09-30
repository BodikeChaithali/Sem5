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

    char msg[100],acknowledgement[100];

    if(pid < 0) {
        perror("pid");
        exit(1);
    }
    else if(pid == 0) {
        close(fd1[1]);
        close(fd2[0]);
        while(1) {
            read(fd1[0],msg,sizeof(msg));
            if(strcmp(msg, "exit") == 0)
                    break;
            printf("Child: Received message: %s\n",msg);
            write(fd2[1],"Message received",strlen("Message received") + 1);
        }
        close(fd1[0]);
        close(fd2[1]); 
        printf("Child: Communication terminated.\n");
        exit(0);

    }
    else {
        close(fd1[0]);
        close(fd2[1]);
        while(1) {
            fgets(msg,100,stdin);
            msg[strcspn(msg, "\n")] = '\0';
            if(strcmp(msg, "exit") == 0) {
                write(fd1[1], msg, strlen(msg) + 1);
                break;
            }
            write(fd1[1],msg,strlen(msg) + 1);
            read(fd2[0],acknowledgement,sizeof(acknowledgement));
            printf("Parent: Acknowledgement received: %s\n",acknowledgement);
        }
        close(fd1[1]);
        close(fd2[0]);
        wait(NULL);
        printf("Parent: Communication terminated.\n");
    }

    return 0;
}
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

    int i;
    pid_t pid = fork();
    if(pid < 0) {
        perror("pid");
        exit(1);    
    }
    else if(pid > 0) {
        close(fd1[0]);
        close(fd2[1]);
        int job_id;
        int op;
        int val;
        int res;
        while(1) {
            printf("\nEnter Job ID (-1 to stop): ");
            scanf("%d", &job_id);

            if (job_id == -1) {
                // Send -1 to child to indicate termination
                write(fd1[1], &job_id, sizeof(int));
                break;
            }
        printf("Parent: Enter job ID, operation (1 for square,2 for cube) and value:");
        scanf("%d %d %d",&job_id,&op,&val);
        write(fd1[1],&job_id,sizeof(int));
        write(fd1[1],&op,sizeof(int));
        write(fd1[1],&val,sizeof(int)); 
        read(fd2[0],&res,sizeof(int));
        printf("Parent: Job %d completed.\n", job_id);
        printf("Parent: Result = %d\n",res);
        }
        close(fd1[1]);
        close(fd2[0]);
        wait(NULL);
        printf("\nParent: All jobs completed.\n");
    }
    else {
        close(fd1[1]);
        close(fd2[0]);
        int job_id;
        int op;
        int val;
        int result;

        while (1) {
            read(fd1[0], &job_id, sizeof(int));
            if (job_id == -1) 
                break;
            read(fd1[0],&op,sizeof(int));
            read(fd1[0],&val,sizeof(int));
            printf("Child: Received Job ID = %d, Operation = %d, Value = %d\n",job_id, op, val);
            int result;
            if(op == 1) 
                result = val * val;
            else if(op == 2) 
                result = val * val * val;
            else if(op == 3) 
                result = val * 3;
            else if(op == 4) {
                result = 1;
                for(int i = 1;i <= val;i++)
                    result *= i;
            }
            else {
                printf("Child: Invalid operation.\n");
                result = -1;
            }
            write(fd2[1],&result,sizeof(int));
        }
        close(fd1[0]);
        close(fd2[1]);
        printf("Child: No more jobs. Exiting.\n");
        exit(0);
    }


    return 0;
}
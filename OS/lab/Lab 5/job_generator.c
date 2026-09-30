#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define SHM_NAME "/my_shared_buffer"

typedef struct {
    int id;
    int priority;
    int burst_time;
} Job;

typedef struct{
    Job job[BUFFER_SIZE];
    int count;
} SharedMemory;

int main() {
    int shm_fd;
    shm_fd = shm_open(SHM_NAME,O_CREAT | O_RDWR,0666);
    if(shm_fd ==  -1) {
        perror("shm_open");
        exit(1);
    }

    ftruncate(shm_fd,sizeof(SharedMemory));

    SharedMemory *shm = mmap(NULL,sizeof(SharedMemory),PROT_READ | PROT_WRITE,MAP_SHARED,shm_fd,0);
    if(shm == -1 ) {
        perror("mmap");
        exit(1);
    }

    shm->count = 0;
    for(int i = 0;i < BUFFER_SIZE;i++) {
        while(shm->count == BUFFER_SIZE) {
            printf("Buffer full. Producer waiting...\n");
            sleep(1);
        }
        shm->job[i].id = i+1;
        shm->job[i].priority = rand() % 10 + 1; // Random priority between 1 and 10
        shm->job[i].burst_time = rand() % 10 + 1; // Random burst time between 1 and 10
        shm->count++;
    }

    printf("Produced %d jobs.\n", BUFFER_SIZE);
    munmap(shm,sizeof(SharedMemory));
    close(shm_fd);
    return 0;
}
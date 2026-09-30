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
    int shm_fd = shm_open(SHM_NAME,O_CREAT | O_RDWR,0666);

    if(shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    SharedMemory *shm = mmap(NULL,sizeof(SharedMemory),PROT_READ | PROT_WRITE,MAP_SHARED,shm_fd,0);
    if(shm == -1) {
        perror("mmap");
        exit(1);
    }

    shm->count = 0;
    while(1) {
        while(shm->count == 0) {
            printf("Buffer empty. Consumer waiting...\n");
            sleep(1);
        }
        int highest_priority_index = 0;
        for(int i = 1; i < shm->count; i++) {
            if(shm->job[i].priority > shm->job[highest_priority_index].priority)
                highest_priority_index = i;
        }
        Job curr = shm->job[highest_priority_index];
        printf("Consumed Job ID: %d, Priority: %d, Burst Time: %d\n", curr.id, curr.priority, curr.burst_time);
        for(int i = highest_priority_index; i < shm->count - 1; i++) 
            shm->job[i] = shm->job[i + 1];
        shm->count--;

        printf("Executing Job %d...\n", curr.id);

        sleep(curr.burst_time);

        printf("Job %d completed.\n", curr.id);
        printf("Jobs remaining in buffer: %d\n", shm->count);
    }

    munmap(shm,sizeof(SharedMemory));
    close(shm_fd);
    shm_unlink(SHM_NAME);
    return 0;
}
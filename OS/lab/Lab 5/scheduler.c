#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>

#define MAX_JOBS 5

#define SHM_NAME "/job_shared_memory"
#define EMPTY_SEM "/job_empty"
#define FULL_SEM "/job_full"
#define MUTEX_SEM "/job_mutex"

typedef struct
{
    int job_id;
    int priority;
    int burst_time;
} Job;

typedef struct
{
    Job jobs[MAX_JOBS];
    int count;
} SharedMemory;

int main()
{
    int total_jobs;
    int shm_fd;

    SharedMemory *shm;

    sem_t *empty;
    sem_t *full;
    sem_t *mutex;
    printf("Enter total number of jobs: ");
    scanf("%d", &total_jobs);

    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);

    if (shm_fd == -1)
    {
        perror("shm_open");
        exit(1);
    }
    shm = mmap(NULL,
               sizeof(SharedMemory),
               PROT_READ | PROT_WRITE,
               MAP_SHARED,
               shm_fd,
               0);

    if (shm == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }

    empty = sem_open(EMPTY_SEM, 0);

    if (empty == SEM_FAILED)
    {
        perror("sem_open empty");
        exit(1);
    }

    full = sem_open(FULL_SEM, 0);

    if (full == SEM_FAILED)
    {
        perror("sem_open full");
        exit(1);
    }

    mutex = sem_open(MUTEX_SEM, 0);

    if (mutex == SEM_FAILED)
    {
        perror("sem_open mutex");
        exit(1);
    }

    printf("\nScheduler started.\n");

    for (int completed = 0;
         completed < total_jobs;
         completed++)
    {
        int highest_index = 0;
        sem_wait(full);
        sem_wait(mutex);
        for (int i = 1; i < shm->count; i++)
        {
            if (shm->jobs[i].priority >
                shm->jobs[highest_index].priority)
            {
                highest_index = i;
            }
        }

        Job selected = shm->jobs[highest_index];
        for (int i = highest_index;
             i < shm->count - 1;
             i++)
        {
            shm->jobs[i] = shm->jobs[i + 1];
        }

        shm->count--;
        sem_post(mutex);
        sem_post(empty);
        printf("\n--------------------------------\n");
        printf("Executing Job %d\n", selected.job_id);
        printf("Priority    : %d\n", selected.priority);
        printf("CPU Burst   : %d seconds\n",
               selected.burst_time);
        printf("--------------------------------\n");

        sleep(selected.burst_time);

        printf("Job %d completed.\n",
               selected.job_id);
    }

    printf("\nAll jobs completed.\n");
    printf("Scheduler finished.\n");

    munmap(shm, sizeof(SharedMemory));
    close(shm_fd);
    sem_close(empty);
    sem_close(full);
    sem_close(mutex);
    shm_unlink(SHM_NAME);

    sem_unlink(EMPTY_SEM);
    sem_unlink(FULL_SEM);
    sem_unlink(MUTEX_SEM);

    return 0;
}

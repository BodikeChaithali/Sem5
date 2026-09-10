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

    if (total_jobs <= 0)
    {
        printf("Invalid number of jobs.\n");
        return 1;
    }

    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);

    if (shm_fd == -1)
    {
        perror("shm_open");
        exit(1);
    }

    if (ftruncate(shm_fd, sizeof(SharedMemory)) == -1)
    {
        perror("ftruncate");
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


    shm->count = 0;

    empty = sem_open(EMPTY_SEM, O_CREAT, 0666, MAX_JOBS);

    if (empty == SEM_FAILED)
    {
        perror("sem_open empty");
        exit(1);
    }

    full = sem_open(FULL_SEM, O_CREAT, 0666, 0);

    if (full == SEM_FAILED)
    {
        perror("sem_open full");
        exit(1);
    }

    mutex = sem_open(MUTEX_SEM, O_CREAT, 0666, 1);

    if (mutex == SEM_FAILED)
    {
        perror("sem_open mutex");
        exit(1);
    }

    for (int i = 1; i <= total_jobs; i++)
    {
        Job job;

        job.job_id = i;

        printf("\nEnter priority for Job %d: ", i);
        scanf("%d", &job.priority);

        printf("Enter CPU burst time for Job %d: ", i);
        scanf("%d", &job.burst_time);

        sem_wait(empty);

        sem_wait(mutex);

        shm->jobs[shm->count] = job;
        shm->count++;

        printf("Job %d added to shared memory.\n",
               job.job_id);

        printf("Priority = %d, Burst Time = %d\n",
               job.priority,
               job.burst_time);

        printf("Jobs currently in shared memory: %d\n",
               shm->count);

        sem_post(mutex);

        sem_post(full);
    }

    printf("\nJob Generator finished.\n");

    munmap(shm, sizeof(SharedMemory));
    close(shm_fd);

    sem_close(empty);
    sem_close(full);
    sem_close(mutex);

    return 0;
}

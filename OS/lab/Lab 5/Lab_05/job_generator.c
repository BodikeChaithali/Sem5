#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_JOBS 5
#define KEY_FILE "job_key"

struct Job {
    int job_id;
    int priority;
    int burst_time;
};

struct SharedData {
    int total_jobs;
    struct Job jobs[MAX_JOBS];
    int ready;
};

int main() {
    key_t key;
    int shmid;
    struct SharedData *shared;
    int n;

    FILE *fp = fopen(KEY_FILE, "a");
    if (fp == NULL) {
        perror("fopen");
        return 1;
    }
    fclose(fp);

    key = ftok(KEY_FILE, 'J');

    if (key == -1) {
        perror("ftok");
        return 1;
    }

    shmid = shmget(key, sizeof(struct SharedData),
                   0666 | IPC_CREAT);

    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    shared = (struct SharedData *)shmat(shmid, NULL, 0);

    if (shared == (void *)-1) {
        perror("shmat");
        return 1;
    }

    printf("Enter total number of jobs (maximum 5): ");
    scanf("%d", &n);

    if (n < 1 || n > MAX_JOBS) {
        printf("Invalid number of jobs.\n");
        shmdt(shared);
        return 1;
    }

    shared->total_jobs = n;

    for (int i = 0; i < n; i++) {
        printf("\nEnter details for Job %d\n", i + 1);

        printf("Job ID: ");
        scanf("%d", &shared->jobs[i].job_id);

        printf("Priority: ");
        scanf("%d", &shared->jobs[i].priority);

        printf("CPU Burst Time: ");
        scanf("%d", &shared->jobs[i].burst_time);
    }

    shared->ready = 1;

    printf("\nJobs successfully placed in shared memory.\n");
    printf("Waiting for Scheduler...\n");

    shmdt(shared);

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

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
    struct Job temp;

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

    printf("Scheduler started.\n");
    printf("Waiting for jobs from Generator...\n");

    while (shared->ready != 1) {
        sleep(1);
    }

    printf("\nJobs received from shared memory.\n");

    /*
       Sort jobs according to priority.
       Higher priority number comes first.
    */
    for (int i = 0; i < shared->total_jobs - 1; i++) {
        for (int j = i + 1; j < shared->total_jobs; j++) {

            if (shared->jobs[j].priority >
                shared->jobs[i].priority) {

                temp = shared->jobs[i];
                shared->jobs[i] = shared->jobs[j];
                shared->jobs[j] = temp;
            }
        }
    }

    printf("\nPriority Scheduling Order:\n");

    for (int i = 0; i < shared->total_jobs; i++) {

        printf("\nExecuting Job ID: %d\n",
               shared->jobs[i].job_id);

        printf("Priority: %d\n",
               shared->jobs[i].priority);

        printf("CPU Burst Time: %d\n",
               shared->jobs[i].burst_time);

        sleep(shared->jobs[i].burst_time);

        printf("Job %d completed.\n",
               shared->jobs[i].job_id);
    }

    printf("\nAll jobs completed.\n");

    shmdt(shared);

    /*
       Remove shared memory segment.
    */
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        return 1;
    }

    printf("Shared memory removed.\n");

    return 0;
}

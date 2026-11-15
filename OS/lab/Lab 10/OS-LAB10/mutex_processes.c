#define _DEFAULT_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#define ITERATIONS 100000
#define CS_DELAY_US 0  /* Time spent inside the critical section */

/* Compile with: gcc mutex_processes.c -o mutex -pthread */
typedef struct {
    volatile int counter;
    pthread_mutex_t mutex;  /* A waiting process SLEEPS, it does not spin */
} SharedData;

static void worker(SharedData *sh) {
    for (int i = 0; i < ITERATIONS; ++i) {
        pthread_mutex_lock(&sh->mutex);    /* Entry section */

        int temp = sh->counter;            /* Critical section */
        if (CS_DELAY_US > 0)
            usleep(CS_DELAY_US);
        sh->counter = temp + 1;

        pthread_mutex_unlock(&sh->mutex);  /* Exit section */
    }
}

int main(void) {
    /* Shared memory, filled with zeros */
    SharedData *sh = mmap(NULL, sizeof(*sh), PROT_READ | PROT_WRITE,
                          MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sh == MAP_FAILED) return 1;

    pthread_mutexattr_t attr;  /* Make the mutex work between processes */
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&sh->mutex, &attr);

    if (fork() == 0) {
        worker(sh);
        _exit(0);
    }
    worker(sh);
    wait(NULL);

    printf("Expected counter: %d\n", 2 * ITERATIONS);
    printf("Observed counter: %d\n", sh->counter);
    return 0;
}

#define _DEFAULT_SOURCE
#include <sched.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#define NPROC 3
#define ITERATIONS 10000

typedef struct {
    volatile int counter;
    volatile int choosing[NPROC];
    volatile int number[NPROC];
} SharedData;

static void lock(SharedData *sh, int i) {
    /* Doorway: take a ticket one bigger than the largest */
    sh->choosing[i] = 1;
    __sync_synchronize();  /* Memory barrier: keep reads/writes in order */
    int max = 0;
    for (int j = 0; j < NPROC; ++j)
        if (sh->number[j] > max)
            max = sh->number[j];
    sh->number[i] = max + 1;
    sh->choosing[i] = 0;
    __sync_synchronize();

    for (int p = 0; p < NPROC; ++p) {
        while (sh->choosing[p])  /* Wait while p is taking a ticket */
            sched_yield();
        while (sh->number[p] != 0 &&  /* Wait while (number[p], p) < (number[i], i) */
               (sh->number[p] < sh->number[i] ||
                (sh->number[p] == sh->number[i] && p < i)))
            sched_yield();
    }
}

static void unlock(SharedData *sh, int i) {
    sh->number[i] = 0;
}

int main(void) {
    /* Shared memory, filled with zeros */
    SharedData *sh = mmap(NULL, sizeof(*sh), PROT_READ | PROT_WRITE,
                          MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sh == MAP_FAILED) return 1;

    for (int me = 0; me < NPROC; ++me) {
        if (fork() == 0) {
            for (int k = 0; k < ITERATIONS; ++k) {
                lock(sh, me);     /* Entry section */
                sh->counter++;    /* Critical section */
                unlock(sh, me);   /* Exit section */
            }
            _exit(0);
        }
    }
    while (wait(NULL) > 0) { }  /* Wait for all processes */

    printf("Expected counter: %d\n", NPROC * ITERATIONS);
    printf("Observed counter: %d\n", sh->counter);
    return 0;
}

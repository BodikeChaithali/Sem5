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
    sh->choosing[i] = 1;
    __sync_synchronize();
    int max = 0;
    for (int j = 0; j < NPROC; ++j)
        if (sh->number[j] > max) max = sh->number[j];
    sh->number[i] = max + 1;
    sh->choosing[i] = 0;
    __sync_synchronize();

    for (int p = 0; p < NPROC; ++p) {
        while (sh->choosing[p]) sched_yield();
        while (sh->number[p] != 0 &&
               (sh->number[p] < sh->number[i] ||
                (sh->number[p] == sh->number[i] && p < i)))
            sched_yield();
    }
}

static void unlock(SharedData *sh, int i) {
    sh->number[i] = 0;
}

int main(void) {
    SharedData *sh = mmap(NULL, sizeof(*sh), PROT_READ | PROT_WRITE,
                          MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sh == MAP_FAILED) { perror("mmap"); return 1; }

    for (int me = 0; me < NPROC; ++me) {
        pid_t pid = fork();
        if (pid < 0) { perror("fork"); return 1; }
        if (pid == 0) {
            for (int k = 0; k < ITERATIONS; ++k) {
                lock(sh, me);
                sh->counter++;
                unlock(sh, me);
            }
            _exit(0);
        }
    }
    while (wait(NULL) > 0) { }
    printf("Expected counter: %d\n", NPROC * ITERATIONS);
    printf("Observed counter: %d\n", sh->counter);
    return 0;
}

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <sched.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define NPROC 3
#define ITERATIONS 100

typedef struct {
    volatile int counter;
    volatile int choosing[NPROC];
    volatile int number[NPROC];
} SharedData;

void lock(SharedData *sh, int i) {
    sh->choosing[i] = 1;
    __sync_synchronize();

    int max = 0;
    for (int j = 0; j < NPROC; j++)
        if (sh->number[j] > max)
            max = sh->number[j];

    sh->number[i] = max + 1;
    sh->choosing[i] = 0;
    __sync_synchronize();

    for (int p = 0; p < NPROC; p++) {
        /* Intentionally removed the choosing wait. */

        while (sh->number[p] != 0 &&
              (sh->number[p] < sh->number[i] ||
              (sh->number[p] == sh->number[i] && p < i)))
            sched_yield();
    }
}

void unlock(SharedData *sh, int i) {
    sh->number[i] = 0;
}

int main(void) {
    SharedData *sh = mmap(NULL, sizeof(SharedData),
        PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (sh == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    sh->counter = 0;

    for (int i = 0; i < NPROC; i++) {
        sh->choosing[i] = 0;
        sh->number[i] = 0;
    }

    for (int i = 0; i < NPROC; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            return 1;
        }

        if (pid == 0) {
            for (int j = 0; j < ITERATIONS; j++) {
                lock(sh, i);
                sh->counter++;
                unlock(sh, i);
            }
            _exit(0);
        }
    }

    while (wait(NULL) > 0) {
    }

    printf("Expected counter: %d\n",
           NPROC * ITERATIONS);
    printf("Observed counter: %d\n", sh->counter);

    munmap(sh, sizeof(SharedData));
    return 0;
}

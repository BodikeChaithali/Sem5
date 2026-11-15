#define _DEFAULT_SOURCE
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>  /* rand() */
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#define N 5      /* Philosophers */
#define MEALS 5  /* Meals per philosopher */

#define LEFT(i)  (((i) + N - 1) % N)
#define RIGHT(i) (((i) + 1) % N)
#define down(s)  sem_wait(s)  /* Names used in the lecture */
#define up(s)    sem_post(s)

enum { THINKING, HUNGRY, EATING };

/* Compile with: gcc dining_philosophers.c -o dining -pthread */
typedef struct {
    int state[N];
    int meals;       /* Meals eaten in total */
    int eating_now;  /* Philosophers eating right now */
    sem_t mutex;     /* Protects state[], starts at 1 */
    sem_t s[N];      /* One per philosopher, start at 0 */
} SharedData;

static void test(SharedData *sh, int i) {
    if (sh->state[i] == HUNGRY &&
        sh->state[LEFT(i)] != EATING && sh->state[RIGHT(i)] != EATING) {
        sh->state[i] = EATING;
        up(&sh->s[i]);
    }
}

static void take_forks(SharedData *sh, int i) {
    down(&sh->mutex);
    sh->state[i] = HUNGRY;
    test(sh, i);
    up(&sh->mutex);
    down(&sh->s[i]);  /* Blocks if the forks were not acquired */
}

static void put_forks(SharedData *sh, int i) {
    down(&sh->mutex);
    sh->state[i] = THINKING;
    test(sh, LEFT(i));
    test(sh, RIGHT(i));
    up(&sh->mutex);
}

static void philosopher(SharedData *sh, int i) {
    for (int m = 0; m < MEALS; ++m) {
        usleep(1000);                /* think() */
        take_forks(sh, i);           /* Entry section */

        int now = __sync_add_and_fetch(&sh->eating_now, 1);  /* eat() */
        printf("Philosopher %d eats, eating now: %d\n", i + 1, now);
        usleep(1000);
        __sync_sub_and_fetch(&sh->eating_now, 1);
        __sync_add_and_fetch(&sh->meals, 1);

        put_forks(sh, i);            /* Exit section */
    }
}

int main(void) {
    setbuf(stdout, NULL);  /* Print immediately */

    /* Shared memory, filled with zeros (so every state[i] = THINKING) */
    SharedData *sh = mmap(NULL, sizeof(*sh), PROT_READ | PROT_WRITE,
                          MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sh == MAP_FAILED) return 1;

    sem_init(&sh->mutex, 1, 1);  /* 1 = shared between processes */
    for (int i = 0; i < N; ++i)
        sem_init(&sh->s[i], 1, 0);

    for (int i = 0; i < N; ++i) {
        if (fork() == 0) {
            philosopher(sh, i);
            _exit(0);
        }
    }
    while (wait(NULL) > 0) { }  /* Wait for all philosophers */

    printf("Expected meals: %d\n", N * MEALS);
    printf("Observed meals: %d\n", sh->meals);
    return 0;
}

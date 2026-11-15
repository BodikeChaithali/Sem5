#define _DEFAULT_SOURCE
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define N 5
#define MEALS 20

#define LEFT(i)  (((i) + N - 1) % N)
#define RIGHT(i) (((i) + 1) % N)

enum { THINKING, HUNGRY, EATING };

typedef struct {
    int state[N];
    int meals;
    int eating_now;
    int max_eating;
    sem_t mutex;
    sem_t self[N];
} SharedData;

void test(SharedData *sh, int i) {
    if (sh->state[i] == HUNGRY &&
        sh->state[LEFT(i)] != EATING &&
        sh->state[RIGHT(i)] != EATING) {

        sh->state[i] = EATING;
        sem_post(&sh->self[i]);
    }
}

void take_forks(SharedData *sh, int i) {
    sem_wait(&sh->mutex);

    sh->state[i] = HUNGRY;
    test(sh, i);

    sem_post(&sh->mutex);
    sem_wait(&sh->self[i]);
}

void put_forks(SharedData *sh, int i) {
    sem_wait(&sh->mutex);

    sh->state[i] = THINKING;

    test(sh, LEFT(i));
    test(sh, RIGHT(i));

    sem_post(&sh->mutex);
}

void philosopher(SharedData *sh, int i) {
    for (int m = 0; m < MEALS; m++) {
        usleep(1000);

        take_forks(sh, i);

        int now = __sync_add_and_fetch(
            &sh->eating_now, 1
        );

        int oldmax;

        do {
            oldmax = sh->max_eating;

            if (now <= oldmax)
                break;

        } while (!__sync_bool_compare_and_swap(
            &sh->max_eating, oldmax, now
        ));

        printf("Philosopher %d eats, eating now: %d\n",
               i + 1, now);

        usleep(1000);

        __sync_sub_and_fetch(&sh->eating_now, 1);
        __sync_add_and_fetch(&sh->meals, 1);

        put_forks(sh, i);
    }
}

int main(void) {
    setbuf(stdout, NULL);

    SharedData *sh = mmap(
        NULL, sizeof(SharedData),
        PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0
    );

    if (sh == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    sh->meals = 0;
    sh->eating_now = 0;
    sh->max_eating = 0;

    for (int i = 0; i < N; i++)
        sh->state[i] = THINKING;

    if (sem_init(&sh->mutex, 1, 1) != 0) {
        perror("sem_init");
        return 1;
    }

    for (int i = 0; i < N; i++) {
        if (sem_init(&sh->self[i], 1, 0) != 0) {
            perror("sem_init");
            return 1;
        }
    }

    for (int i = 0; i < N; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            return 1;
        }

        if (pid == 0) {
            philosopher(sh, i);
            _exit(0);
        }
    }

    while (wait(NULL) > 0) {
    }

    printf("Expected meals: %d\n", N * MEALS);
    printf("Observed meals: %d\n", sh->meals);
    printf("Largest eating now: %d\n", sh->max_eating);

    for (int i = 0; i < N; i++)
        sem_destroy(&sh->self[i]);

    sem_destroy(&sh->mutex);
    munmap(sh, sizeof(SharedData));

    return 0;
}

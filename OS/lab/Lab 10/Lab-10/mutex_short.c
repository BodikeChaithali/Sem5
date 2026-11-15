#define _DEFAULT_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#define ITERATIONS 100000
#define CS_DELAY_US 0

typedef struct {
    int counter;
    pthread_mutex_t mutex;
} SharedData;

void worker(SharedData *sh) {
    for (int i = 0; i < ITERATIONS; i++) {
        pthread_mutex_lock(&sh->mutex);

        int temp = sh->counter;

        if (CS_DELAY_US > 0)
            usleep(CS_DELAY_US);

        sh->counter = temp + 1;

        pthread_mutex_unlock(&sh->mutex);
    }
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

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(
        &attr, PTHREAD_PROCESS_SHARED
    );

    if (pthread_mutex_init(&sh->mutex, &attr) != 0) {
        fprintf(stderr, "Mutex initialization failed\n");
        return 1;
    }

    pthread_mutexattr_destroy(&attr);

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        worker(sh);
        _exit(0);
    }

    worker(sh);
    wait(NULL);

    printf("Expected counter: %d\n", 2 * ITERATIONS);
    printf("Observed counter: %d\n", sh->counter);

    pthread_mutex_destroy(&sh->mutex);
    munmap(sh, sizeof(SharedData));

    return 0;
}

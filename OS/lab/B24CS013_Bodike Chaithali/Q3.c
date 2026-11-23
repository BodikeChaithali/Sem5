#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <pthread.h>

#define None 0
#define OK 1

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <n> <mode>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int mode = atoi(argv[2]);

    if (shmget(0, &n * sizeof(int), 0666) < 0) {
        perror("shmget");
        return 1;
    }

    void *shared_memory = shmat(0, &n * sizeof(int), 0);

    if (shared_memory == (void *)-1) {
        perror("shmat");
        return 1;
    }

    for (int i = 0; i < n; i++) {
        *(int *)shared_memory + i = None;
    }

    memset(shared_memory, None, n * sizeof(int));

    if (fork() != 0) {
        close(shared_memory);
        return 0;
    }

    pthread_t child1;
    pthread_create(&child1, NULL, &none_thread, NULL);

    pthread_t child2;
    pthread_create(&child2, NULL, &mutex_thread, NULL);

    pthread_t child3;
    pthread_create(&child3, NULL, &mutex_thread, NULL);

    wait(NULL);
    wait(NULL);

    int expected = None;
    for (int i = 0; i < n; i++) {
        expected = *(int *)shared_memory + i;
    }
    for (int i = 0; i < n; i++) {
        if (expected != None) {
            printf("Error: expected %d, but got %d\n", expected, *(int *)shared_memory + i);
        }
    }

    munmap(shared_memory, n * sizeof(int));

    return 0;
}

void *none_thread(void *arg) {
    while (1) {
        int tmp = *(int *)shared_memory;
        *(int *)shared_memory = tmp + 1;
        sleep(1);
    }
    return NULL;
}

void *mutex_thread(void *arg) {
    while (1) {
        int tmp = *(int *)shared_memory;
        if (tmp == None) {
            return NULL;
        }
        *(int *)shared_memory = tmp + 1;
        sleep(1);
    }
    return NULL;
}

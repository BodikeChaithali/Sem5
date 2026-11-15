#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define KEY_FILE "barber_key"
#define CHAIRS 3

struct BarberShop {
    int queue[CHAIRS];
    int front;
    int rear;
    int count;
    int barber_busy;
    int stop;
};

int main() {
    key_t key;
    int shmid;
    struct BarberShop *shop;

    FILE *fp = fopen(KEY_FILE, "a");
    if (fp == NULL) {
        perror("fopen");
        return 1;
    }
    fclose(fp);

    key = ftok(KEY_FILE, 'B');
    if (key == -1) {
        perror("ftok");
        return 1;
    }

    shmid = shmget(key, sizeof(struct BarberShop), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    shop = (struct BarberShop *)shmat(shmid, NULL, 0);
    if (shop == (void *)-1) {
        perror("shmat");
        return 1;
    }

    shop->front = 0;
    shop->rear = 0;
    shop->count = 0;
    shop->barber_busy = 0;
    shop->stop = 0;

    printf("Barber started.\n");

    while (1) {
        if (shop->count > 0) {
            int customer_id;

            shop->barber_busy = 1;

            customer_id = shop->queue[shop->front];
            shop->front = (shop->front + 1) % CHAIRS;
            shop->count--;

            printf("Barber is serving Customer %d\n", customer_id);

            sleep(3);

            printf("Barber finished Customer %d\n", customer_id);

            shop->barber_busy = 0;
        } else {
            if (shop->stop == 1) {
                break;
            }

            if (shop->barber_busy == 0) {
                printf("Barber is sleeping...\n");
            }

            sleep(1);
        }
    }

    printf("No more customers. Barber is closing.\n");

    if (shmdt(shop) == -1) {
        perror("shmdt");
    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
    }

    return 0;
}

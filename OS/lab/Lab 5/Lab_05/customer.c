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
    int customer_id;

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

    printf("Customer process started.\n");

    while (1) {
        printf("\nEnter Customer ID (-1 to stop): ");
        scanf("%d", &customer_id);

        if (customer_id == -1) {
            shop->stop = 1;
            printf("Customer generation stopped.\n");
            break;
        }

        if (shop->count < CHAIRS) {
            shop->queue[shop->rear] = customer_id;
            shop->rear = (shop->rear + 1) % CHAIRS;
            shop->count++;

            if (shop->barber_busy) {
                printf("Customer %d is waiting in a chair.\n", customer_id);
            } else {
                printf("Customer %d arrives and will be served.\n",
                       customer_id);
            }
        } else {
            printf("All chairs are occupied. Customer %d leaves.\n",
                   customer_id);
        }
    }

    if (shmdt(shop) == -1) {
        perror("shmdt");
    }

    return 0;
}

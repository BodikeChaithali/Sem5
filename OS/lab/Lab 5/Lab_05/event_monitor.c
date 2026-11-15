#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define KEY_FILE "event_key"
#define MAX_TYPE 50
#define MAX_MESSAGE 200

struct Event {
    int event_id;
    char event_type[MAX_TYPE];
    char event_message[MAX_MESSAGE];
    int status;
};

int main() {
    key_t key;
    int shmid;
    struct Event *event;

    FILE *fp = fopen(KEY_FILE, "a");
    if (fp == NULL) {
        perror("fopen");
        return 1;
    }
    fclose(fp);

    key = ftok(KEY_FILE, 'E');
    if (key == -1) {
        perror("ftok");
        return 1;
    }

    shmid = shmget(key, sizeof(struct Event), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    event = (struct Event *)shmat(shmid, NULL, 0);
    if (event == (void *)-1) {
        perror("shmat");
        return 1;
    }

    printf("Event Monitor Started...\n");
    printf("Waiting for events...\n");

    while (1) {
        if (event->status == 1) {
            printf("\n----- New Event -----\n");
            printf("Event ID      : %d\n", event->event_id);
            printf("Event Type    : %s\n", event->event_type);
            printf("Event Message : %s\n", event->event_message);
            printf("---------------------\n");

            event->status = 0;

            if (strcmp(event->event_type, "EXIT") == 0) {
                break;
            }

            printf("Waiting for events...\n");
        }

        usleep(100000);
    }

    if (shmdt(event) == -1) {
        perror("shmdt");
    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
    }

    printf("Event Monitor terminated.\n");
    return 0;
}

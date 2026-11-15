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
    int event_id = 1;

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

    if (event->status != 0 && event->status != 1) {
        event->status = 0;
    }

    printf("Event Generator Started.\n");

    while (1) {
        char type[MAX_TYPE];
        char message[MAX_MESSAGE];

        while (event->status != 0) {
            usleep(100000);
        }

        printf("\nEnter Event Type (or EXIT to stop): ");
        fflush(stdout);
        fgets(type, sizeof(type), stdin);
        type[strcspn(type, "\n")] = '\0';

        if (strcmp(type, "EXIT") == 0) {
            event->event_id = event_id++;
            strcpy(event->event_type, "EXIT");
            strcpy(event->event_message, "Event Generator is terminating");
            event->status = 1;
            break;
        }

        printf("Enter Event Message: ");
        fflush(stdout);
        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = '\0';

        event->event_id = event_id++;
        strcpy(event->event_type, type);
        strcpy(event->event_message, message);

        event->status = 1;
    }

    if (shmdt(event) == -1) {
        perror("shmdt");
    }

    printf("Event Generator terminated.\n");
    return 0;
}

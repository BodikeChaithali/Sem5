#include <stdio.h>
#include <string.h>

#define MAX 100
#define NAME_LEN 32

typedef struct {
    char name[NAME_LEN];
    int size;
} Song;

int findSong(Song songs[], int n, char name[]) {
    for (int i = 0; i < n; i++) {
        if (strcmp(songs[i].name, name) == 0)
            return i;
    }

    return -1;
}

int chooseVictim(int stored[],
                 int n,
                 int firstTime[],
                 int lastUsed[],
                 int useLRU) {

    int victim = -1;

    for (int i = 0; i < n; i++) {

        if (!stored[i])
            continue;

        if (victim == -1) {
            victim = i;
        } else if (useLRU) {

            if (lastUsed[i] < lastUsed[victim])
                victim = i;

        } else {

            if (firstTime[i] < firstTime[victim])
                victim = i;
        }
    }

    return victim;
}

void simulate(
    int capacity,
    Song songs[],
    int S,
    char plays[][NAME_LEN],
    int P,
    int useLRU,
    int *hits,
    int *misses,
    int *downloaded
) {
    int stored[MAX] = {0};
    int firstTime[MAX];
    int lastUsed[MAX];

    int usedSpace = 0;

    *hits = 0;
    *misses = 0;
    *downloaded = 0;

    for (int i = 0; i < S; i++) {
        firstTime[i] = -1;
        lastUsed[i] = -1;
    }

    for (int i = 0; i < P; i++) {

        int id = findSong(songs, S, plays[i]);

        /*
            Assume input song names are valid library songs.
        */
        if (id == -1)
            continue;

        /*
            HIT
        */
        if (stored[id]) {

            (*hits)++;

            lastUsed[id] = i;

            continue;
        }

        /*
            MISS
        */
        (*misses)++;

        /*
            Song cannot fit even in completely empty storage.
        */
        if (songs[id].size > capacity) {
            continue;
        }

        /*
            Delete songs until enough space exists.
        */
        while (usedSpace + songs[id].size > capacity) {

            int victim = chooseVictim(
                stored,
                S,
                firstTime,
                lastUsed,
                useLRU
            );

            stored[victim] = 0;

            usedSpace -= songs[victim].size;
        }

        /*
            Download and store song.
        */
        stored[id] = 1;

        usedSpace += songs[id].size;

        *downloaded += songs[id].size;

        firstTime[id] = i;
        lastUsed[id] = i;
    }

    /*
        Print stored songs in deletion order.
    */
    int printed[MAX] = {0};

    printf("stored:");

    for (int count = 0; count < S; count++) {

        int best = -1;

        for (int i = 0; i < S; i++) {

            if (!stored[i] || printed[i])
                continue;

            if (best == -1) {
                best = i;
            } else if (useLRU) {

                if (lastUsed[i] < lastUsed[best])
                    best = i;

            } else {

                if (firstTime[i] < firstTime[best])
                    best = i;
            }
        }

        if (best == -1)
            break;

        printf(" %s", songs[best].name);

        printed[best] = 1;
    }

    printf("\n");
}

int main(void) {

    int C, S, P;

    Song songs[MAX];
    char plays[MAX][NAME_LEN];

    scanf("%d", &C);
    scanf("%d", &S);

    for (int i = 0; i < S; i++) {
        scanf("%s %d",
              songs[i].name,
              &songs[i].size);
    }

    scanf("%d", &P);

    for (int i = 0; i < P; i++)
        scanf("%s", plays[i]);

    int lruHits, lruMisses, lruDownloaded;
    int fifoHits, fifoMisses, fifoDownloaded;

    /*
        LRU
    */
    printf("LRU: ");

    /*
        We need the statistics before printing stored,
        so simulate first using a temporary approach.
    */

    /*
        LRU simulation
    */
    {
        int stored[MAX] = {0};
        int firstTime[MAX];
        int lastUsed[MAX];
        int usedSpace = 0;

        lruHits = 0;
        lruMisses = 0;
        lruDownloaded = 0;

        for (int i = 0; i < S; i++) {
            firstTime[i] = -1;
            lastUsed[i] = -1;
        }

        for (int i = 0; i < P; i++) {

            int id = findSong(songs, S, plays[i]);

            if (stored[id]) {
                lruHits++;
                lastUsed[id] = i;
                continue;
            }

            lruMisses++;

            if (songs[id].size > C)
                continue;

            while (usedSpace + songs[id].size > C) {

                int victim = chooseVictim(
                    stored,
                    S,
                    firstTime,
                    lastUsed,
                    1
                );

                stored[victim] = 0;
                usedSpace -= songs[victim].size;
            }

            stored[id] = 1;
            usedSpace += songs[id].size;

            lruDownloaded += songs[id].size;

            firstTime[id] = i;
            lastUsed[id] = i;
        }

        printf("hits %d, misses %d, downloaded %d MB, ",
               lruHits,
               lruMisses,
               lruDownloaded);

        int printed[MAX] = {0};

        printf("stored:");

        for (int count = 0; count < S; count++) {

            int best = -1;

            for (int i = 0; i < S; i++) {

                if (!stored[i] || printed[i])
                    continue;

                if (best == -1 ||
                    lastUsed[i] < lastUsed[best]) {

                    best = i;
                }
            }

            if (best == -1)
                break;

            printf(" %s", songs[best].name);
            printed[best] = 1;
        }

        printf("\n");
    }

    /*
        FIFO simulation
    */
    printf("FIFO: ");

    {
        int stored[MAX] = {0};
        int firstTime[MAX];
        int lastUsed[MAX];
        int usedSpace = 0;

        fifoHits = 0;
        fifoMisses = 0;
        fifoDownloaded = 0;

        for (int i = 0; i < S; i++) {
            firstTime[i] = -1;
            lastUsed[i] = -1;
        }

        for (int i = 0; i < P; i++) {

            int id = findSong(songs, S, plays[i]);

            if (stored[id]) {
                fifoHits++;
                continue;
            }

            fifoMisses++;

            if (songs[id].size > C)
                continue;

            while (usedSpace + songs[id].size > C) {

                int victim = chooseVictim(
                    stored,
                    S,
                    firstTime,
                    lastUsed,
                    0
                );

                stored[victim] = 0;
                usedSpace -= songs[victim].size;
            }

            stored[id] = 1;

            usedSpace += songs[id].size;

            fifoDownloaded += songs[id].size;

            firstTime[id] = i;
            lastUsed[id] = i;
        }

        printf("hits %d, misses %d, downloaded %d MB, ",
               fifoHits,
               fifoMisses,
               fifoDownloaded);

        int printed[MAX] = {0};

        printf("stored:");

        for (int count = 0; count < S; count++) {

            int best = -1;

            for (int i = 0; i < S; i++) {

                if (!stored[i] || printed[i])
                    continue;

                if (best == -1 ||
                    firstTime[i] < firstTime[best]) {

                    best = i;
                }
            }

            if (best == -1)
                break;

            printf(" %s", songs[best].name);
            printed[best] = 1;
        }

        printf("\n");
    }

    /*
        Compare downloaded data.
    */
    if (lruDownloaded < fifoDownloaded)
        printf("Better policy: LRU\n");
    else if (fifoDownloaded < lruDownloaded)
        printf("Better policy: FIFO\n");
    else
        printf("Better policy: Both equal\n");

    return 0;
}

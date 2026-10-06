#include <stdio.h>
#include <string.h>

#define MAX 100
#define NAME_LEN 32

int findTool(char belt[][NAME_LEN],
             int count,
             char tool[]) {

    for (int i = 0; i < count; i++) {

        if (strcmp(belt[i], tool) == 0)
            return i;
    }

    return -1;
}


/*
    Optimal replacement strategy.

    Choose the tool whose next use is farthest away.

    If several tools will never be used again,
    return the one that was put on the belt earliest.
*/
int simulateOptimal(int K,
                    int N,
                    char tools[][NAME_LEN],
                    int printTrips) {

    char belt[MAX][NAME_LEN];
    int addedAt[MAX];

    int count = 0;
    int trips = 0;

    for (int i = 0; i < N; i++) {

        int pos =
            findTool(belt, count, tools[i]);

        /* Tool already on belt */
        if (pos != -1)
            continue;

        trips++;

        /* Free space available */
        if (count < K) {

            strcpy(belt[count], tools[i]);

            addedAt[count] = i;

            if (printTrips) {

                printf("Trip %d: take %s\n",
                       trips, tools[i]);
            }

            count++;
            continue;
        }

        /*
            Belt is full.

            Find the tool whose next use is farthest.
        */
        int victim = -1;
        int farthest = -1;

        for (int j = 0; j < K; j++) {

            int nextUse = N;

            for (int k = i + 1; k < N; k++) {

                if (strcmp(tools[k], belt[j]) == 0) {

                    nextUse = k;
                    break;
                }
            }

            /*
                Choose:
                1. farther next use
                2. if both are never used again,
                   choose the one added earlier
            */
            if (victim == -1 ||
                nextUse > farthest ||
                (nextUse == farthest &&
                 nextUse == N &&
                 addedAt[j] < addedAt[victim])) {

                victim = j;
                farthest = nextUse;
            }
        }

        if (printTrips) {

            printf("Trip %d: take %s, return %s\n",
                   trips,
                   tools[i],
                   belt[victim]);
        }

        strcpy(belt[victim], tools[i]);

        addedAt[victim] = i;
    }

    return trips;
}


/*
    LRU replacement strategy.

    Return the tool that has not been used
    for the longest time.
*/
int simulateLRU(int K,
                int N,
                char tools[][NAME_LEN]) {

    char belt[MAX][NAME_LEN];
    int lastUsed[MAX];

    int count = 0;
    int trips = 0;

    for (int i = 0; i < K; i++)
        lastUsed[i] = -1;

    for (int i = 0; i < N; i++) {

        int pos =
            findTool(belt, count, tools[i]);

        /* Tool already on belt */
        if (pos != -1) {

            lastUsed[pos] = i;
            continue;
        }

        trips++;

        /* Free space available */
        if (count < K) {

            strcpy(belt[count], tools[i]);

            lastUsed[count] = i;

            count++;

        } else {

            /*
                Find least recently used tool.
            */
            int victim = 0;

            for (int j = 1; j < K; j++) {

                if (lastUsed[j] < lastUsed[victim])
                    victim = j;
            }

            strcpy(belt[victim], tools[i]);

            lastUsed[victim] = i;
        }
    }

    return trips;
}


int main(void) {

    int K, N;

    char tools[MAX][NAME_LEN];

    scanf("%d", &K);
    scanf("%d", &N);

    for (int i = 0; i < N; i++)
        scanf("%31s", tools[i]);

    int optimalTrips =
        simulateOptimal(K, N, tools, 1);

    int lruTrips =
        simulateLRU(K, N, tools);

    printf("Total trips: %d\n",
           optimalTrips);

    printf("Trips if returning least recently used tool: %d\n",
           lruTrips);

    return 0;
}

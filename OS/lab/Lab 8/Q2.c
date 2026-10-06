#include <stdio.h>
#include <string.h>

#define MAX 100
#define NAME_LEN 32

int findApp(char ram[][NAME_LEN], int count, char app[]) {
    for (int i = 0; i < count; i++) {
        if (strcmp(ram[i], app) == 0)
            return i;
    }

    return -1;
}

int main(void) {
    int K, N;
    char ram[MAX][NAME_LEN];
    int lastUsed[MAX];

    char killed[MAX][NAME_LEN];
    int killedCount = 0;

    int count = 0;
    int totalTime = 0;

    scanf("%d", &K);
    scanf("%d", &N);

    for (int i = 0; i < K; i++)
        lastUsed[i] = -1;

    for (int i = 0; i < N; i++) {
        char app[NAME_LEN];

        scanf("%s", app);

        int pos = findApp(ram, count, app);

        if (pos != -1) {
            // Warm start
            totalTime += 1;

            lastUsed[pos] = i;
        } else {
            // Cold start
            totalTime += 4;

            if (count < K) {
                strcpy(ram[count], app);
                lastUsed[count] = i;
                count++;
            } else {
                // Find least recently used app
                int victim = 0;

                for (int j = 1; j < K; j++) {
                    if (lastUsed[j] < lastUsed[victim])
                        victim = j;
                }

                strcpy(killed[killedCount], ram[victim]);
                killedCount++;

                strcpy(ram[victim], app);
                lastUsed[victim] = i;
            }
        }
    }

    printf("Total time: %d seconds\n", totalTime);

    printf("Apps killed:");

    if (killedCount == 0) {
        printf(" none");
    } else {
        for (int i = 0; i < killedCount; i++)
            printf(" %s", killed[i]);
    }

    printf("\n");

    /*
       Print most recently opened first.
       Find the maximum lastUsed value repeatedly.
    */
    printf("Recent apps:");

    int used[MAX] = {0};

    for (int pos = 0; pos < count; pos++) {
        int best = -1;

        for (int j = 0; j < count; j++) {
            if (!used[j]) {
                if (best == -1 ||
                    lastUsed[j] > lastUsed[best]) {
                    best = j;
                }
            }
        }

        printf(" %s", ram[best]);
        used[best] = 1;
    }

    printf("\n");

    return 0;
}

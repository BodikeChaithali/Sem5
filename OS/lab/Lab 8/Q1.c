#include <stdio.h>

#define MAX 1000

int simulateFIFO(int ref[], int n, int k,
                 int finalMemory[], int *finalCount) {

    int memory[MAX];
    int count = 0;
    int next = 0;
    int time = 0;

    for (int i = 0; i < k; i++)
        memory[i] = -1;

    for (int i = 0; i < n; i++) {

        int page = ref[i];
        int hit = 0;

        for (int j = 0; j < count; j++) {
            if (memory[j] == page) {
                hit = 1;
                break;
            }
        }

        if (hit) {
            time += 1;
        } else {
            time += 5;

            if (count < k) {
                memory[count] = page;
                count++;
            } else {
                memory[next] = page;
                next = (next + 1) % k;
            }
        }
    }

    *finalCount = 0;

    /*
       If memory was never full, the items are already
       in earliest-stored order.
    */
    if (count < k) {

        for (int i = 0; i < count; i++)
            finalMemory[(*finalCount)++] = memory[i];

    } else {

        /*
           next points to the oldest item.
           Print from next circularly.
        */
        for (int i = 0; i < k; i++) {

            int pos = (next + i) % k;

            finalMemory[(*finalCount)++] = memory[pos];
        }
    }

    return time;
}

int main(void) {

    int K, N, T;
    int ref[MAX];
    int finalMemory[MAX];
    int finalCount;

    scanf("%d", &K);
    scanf("%d", &N);

    for (int i = 0; i < N; i++)
        scanf("%d", &ref[i]);

    scanf("%d", &T);

    int totalTime =
        simulateFIFO(ref, N, K, finalMemory, &finalCount);

    printf("Total time with %d slots: %d seconds\n",
           K, totalTime);

    printf("Memory at end:");

    for (int i = 0; i < finalCount; i++)
        printf(" %d", finalMemory[i]);

    printf("\n");

    int minimumSlots = -1;

    /*
       N slots are enough to hold every distinct vehicle
       that can occur in the sequence.
    */
    for (int slots = 1; slots <= N; slots++) {

        int temp[MAX];
        int tempCount;

        int time =
            simulateFIFO(ref, N, slots, temp, &tempCount);

        if (time <= T) {
            minimumSlots = slots;
            break;
        }
    }

    if (minimumSlots == -1) {

        printf("Minimum slots for time <= %d: Not possible\n",
               T);

    } else {

        printf("Minimum slots for time <= %d: %d\n",
               T, minimumSlots);
    }

    return 0;
}

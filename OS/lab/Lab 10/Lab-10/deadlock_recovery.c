#include <stdio.h>

#define NPROC 3
#define NRES 4

int A[NRES] = {2, 1, 0, 0};

int C[NPROC][NRES] = {
    {0, 0, 1, 0},
    {2, 0, 0, 1},
    {0, 1, 2, 0}
};

int R[NPROC][NRES] = {
    {2, 0, 0, 1},
    {1, 0, 1, 0},
    {2, 1, 1, 0}
};

/* Remember which process was terminated during recovery. */
int killed[NPROC] = {0, 0, 0};

int detect(int finished[NPROC]) {
    int work[NRES];

    for (int j = 0; j < NRES; j++)
        work[j] = A[j];

    for (int i = 0; i < NPROC; i++)
        finished[i] = killed[i];

    int progress = 1;

    while (progress) {
        progress = 0;

        for (int i = 0; i < NPROC; i++) {
            if (finished[i])
                continue;

            int possible = 1;

            for (int j = 0; j < NRES; j++) {
                if (R[i][j] > work[j]) {
                    possible = 0;
                    break;
                }
            }

            if (possible) {
                /* Process finishes; return its allocation. */
                for (int j = 0; j < NRES; j++)
                    work[j] += C[i][j];

                finished[i] = 1;
                progress = 1;

                printf("P%d finishes, A =", i + 1);

                for (int j = 0; j < NRES; j++)
                    printf(" %d", work[j]);

                printf("\n");
            }
        }
    }

    int deadlocked = 0;

    for (int i = 0; i < NPROC; i++) {
        if (!finished[i])
            deadlocked++;
    }

    return deadlocked;
}

void print_deadlocked(int finished[NPROC]) {
    printf("DEADLOCK, deadlocked processes:");

    for (int i = 0; i < NPROC; i++) {
        if (!finished[i])
            printf(" P%d", i + 1);
    }

    printf("\n");
}

int main(void) {
    int finished[NPROC];

    int count = detect(finished);

    if (count == 0) {
        printf("No deadlock\n");
        return 0;
    }

    print_deadlocked(finished);

    /* Choose the unfinished process holding fewest resources. */
    int victim = -1;
    int fewest = 2147483647;

    for (int i = 0; i < NPROC; i++) {
        if (finished[i])
            continue;

        int held = 0;

        for (int j = 0; j < NRES; j++)
            held += C[i][j];

        if (held < fewest) {
            fewest = held;
            victim = i;
        }
    }

    if (victim == -1)
        return 0;

    printf("Killing P%d (holds %d resources)\n",
           victim + 1, fewest);

    /* Return the victim's allocation and clear its rows. */
    for (int j = 0; j < NRES; j++) {
        A[j] += C[victim][j];
        C[victim][j] = 0;
        R[victim][j] = 0;
    }

    /* Mark it as terminated, not normally finished. */
    killed[victim] = 1;

    printf("Available resources after recovery:");

    for (int j = 0; j < NRES; j++)
        printf(" %d", A[j]);

    printf("\n");

    /* Detect again, excluding the terminated process. */
    int finished_again[NPROC];
    int remaining = detect(finished_again);

    if (remaining == 0) {
        printf("No deadlock after recovery\n");
    } else {
        print_deadlocked(finished_again);
    }

    return 0;
}

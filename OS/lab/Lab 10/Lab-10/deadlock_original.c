#include <stdio.h>

#define NPROC 3
#define NRES 4

/* Available resources: A, B, C, D */
int A[NRES] = {2, 1, 0, 0};

/* Currently allocated resources */
int C[NPROC][NRES] = {
    {0, 0, 1, 0},
    {2, 0, 0, 1},
    {0, 1, 2, 0}
};

/* Outstanding resource requests */
int R[NPROC][NRES] = {
    {2, 0, 0, 1},
    {1, 0, 1, 0},
    {2, 1, 0, 0}
};

int detect(int finished[NPROC]) {
    int work[NRES];

    for (int j = 0; j < NRES; j++)
        work[j] = A[j];

    for (int i = 0; i < NPROC; i++)
        finished[i] = 0;

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

int main(void) {
    int finished[NPROC];

    int count = detect(finished);

    if (count == 0) {
        printf("No deadlock\n");
    } else {
        printf("DEADLOCK, deadlocked processes:");

        for (int i = 0; i < NPROC; i++) {
            if (!finished[i])
                printf(" P%d", i + 1);
        }

        printf("\n");
    }

    return 0;
}

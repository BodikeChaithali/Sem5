#include <stdio.h>

#define NPROC 7
#define NRES 6

/*
Resource order: R, S, T, U, V, W

Processes:
P1 = A
P2 = B
P3 = C
P4 = D
P5 = E
P6 = F
P7 = G
*/

/* Available resources: S is free initially. */
int A[NRES] = {0, 1, 0, 0, 0, 0};

/* Current allocations. */
int C[NPROC][NRES] = {
    {1, 0, 0, 0, 0, 0},  /* A holds R */
    {0, 0, 0, 0, 0, 0},  /* B */
    {0, 0, 0, 0, 0, 0},  /* C */
    {0, 0, 0, 1, 0, 0},  /* D holds U */
    {0, 0, 1, 0, 0, 0},  /* E holds T */
    {0, 0, 0, 0, 0, 1},  /* F holds W */
    {0, 0, 0, 0, 1, 0}   /* G holds V */
};

/* Outstanding requests. */
int R[NPROC][NRES] = {
    {0, 1, 0, 0, 0, 0},  /* A wants S */
    {0, 0, 1, 0, 0, 0},  /* B wants T */
    {0, 1, 0, 0, 0, 0},  /* C wants S */
    {0, 1, 1, 0, 0, 0},  /* D wants S and T */
    {0, 0, 0, 0, 1, 0},  /* E wants V */
    {0, 1, 0, 0, 0, 0},  /* F wants S */
    {0, 0, 0, 1, 0, 0}   /* G wants U */
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

    for (int i = 0; i < NPROC; i++)
        if (!finished[i])
            deadlocked++;

    return deadlocked;
}

int main(void) {
    int finished[NPROC];

    int count = detect(finished);

    if (count == 0) {
        printf("No deadlock\n");
    } else {
        printf("DEADLOCK, deadlocked processes:");

        for (int i = 0; i < NPROC; i++)
            if (!finished[i])
                printf(" P%d", i + 1);

        printf("\n");
    }

    return 0;
}

#include <stdio.h>

#define NPROC 3
#define NRES 4

/* Slides 21-24. Resources: tape drives, plotters, scanners, CD-ROMs */
int A[NRES] = { 2, 1, 0, 0 };  /* Available */

int C[NPROC][NRES] = {         /* Current allocation: who has what */
    { 0, 0, 1, 0 },            /* P1 */
    { 2, 0, 0, 1 },            /* P2 */
    { 0, 1, 2, 0 },            /* P3 */
};

int R[NPROC][NRES] = {         /* Requests: who is waiting for what */
    { 2, 0, 0, 1 },            /* P1 */
    { 1, 0, 1, 0 },            /* P2 */
    { 2, 1, 0, 0 },            /* P3 */
};

/* Returns the number of deadlocked processes; finished[i] = 1 if Pi can finish */
int detect(int finished[NPROC]) {
    int work[NRES];
    for (int j = 0; j < NRES; ++j)
        work[j] = A[j];
    for (int i = 0; i < NPROC; ++i)
        finished[i] = 0;

    int progress = 1;
    while (progress) {
        progress = 0;
        for (int i = 0; i < NPROC; ++i) {
            if (finished[i])
                continue;
            int ok = 1;  /* Can the request of Pi be met? */
            for (int j = 0; j < NRES; ++j)
                if (R[i][j] > work[j])
                    ok = 0;
            if (ok) {    /* Pi runs to completion and returns everything */
                for (int j = 0; j < NRES; ++j)
                    work[j] += C[i][j];
                finished[i] = 1;
                progress = 1;
                printf("P%d finishes, A =", i + 1);
                for (int j = 0; j < NRES; ++j)
                    printf(" %d", work[j]);
                printf("\n");
            }
        }
    }

    int deadlocked = 0;
    for (int i = 0; i < NPROC; ++i)
        if (!finished[i])
            deadlocked++;
    return deadlocked;
}

int main(void) {
    int finished[NPROC];

    if (detect(finished) == 0) {
        printf("No deadlock\n");
    } else {
        printf("DEADLOCK, deadlocked processes:");
        for (int i = 0; i < NPROC; ++i)
            if (!finished[i])
                printf(" P%d", i + 1);
        printf("\n");
    }
    return 0;
}

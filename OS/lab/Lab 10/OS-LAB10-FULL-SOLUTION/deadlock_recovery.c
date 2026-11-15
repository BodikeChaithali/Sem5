#include <stdio.h>
#define NPROC 3
#define NRES 4
int A[NRES] = {2,1,0,0};
int C[NPROC][NRES] = {{{0,0,1,0}, {2,0,0,1}, {0,1,2,0}}};
int R[NPROC][NRES] = {{{2,0,0,1}, {1,0,1,0}, {2,1,1,0}}};

int detect(int finished[NPROC]) {
    int work[NRES];
    for (int j = 0; j < NRES; ++j) work[j] = A[j];
    for (int i = 0; i < NPROC; ++i) finished[i] = 0;
    int progress = 1;
    while (progress) {
        progress = 0;
        for (int i = 0; i < NPROC; ++i) {
            if (finished[i]) continue;
            int ok = 1;
            for (int j = 0; j < NRES; ++j)
                if (R[i][j] > work[j]) ok = 0;
            if (ok) {
                for (int j = 0; j < NRES; ++j) work[j] += C[i][j];
                finished[i] = 1;
                progress = 1;
                printf("P%d finishes, A =", i + 1);
                for (int j = 0; j < NRES; ++j) printf(" %d", work[j]);
                printf("\\n");
            }
        }
    }
    int deadlocked = 0;
    for (int i = 0; i < NPROC; ++i) if (!finished[i]) ++deadlocked;
    return deadlocked;
}

static void print_deadlocked(const int finished[NPROC]) {
    printf("DEADLOCK, deadlocked processes:");
    for (int i = 0; i < NPROC; ++i)
        if (!finished[i]) printf(" P%d", i + 1);
    printf("\\n");
}

int main(void) {
    int finished[NPROC];
    int count = detect(finished);
    if (count == 0) { printf("No deadlock\n"); return 0; }
    print_deadlocked(finished);

    int victim = -1, fewest = 2147483647;
    for (int i = 0; i < NPROC; ++i) {
        if (finished[i]) continue;
        int held = 0;
        for (int j = 0; j < NRES; ++j) held += C[i][j];
        if (held < fewest) { fewest = held; victim = i; }
    }
    if (victim >= 0) {
        printf("Killing P%d (holds %d resources)\n", victim + 1, fewest);
        for (int j = 0; j < NRES; ++j) {
            A[j] += C[victim][j];
            C[victim][j] = 0;
            R[victim][j] = 0;
        }
        int again[NPROC];
        if (detect(again) == 0) printf("No deadlock after recovery\n");
        else print_deadlocked(again);
    }
    return 0;
}

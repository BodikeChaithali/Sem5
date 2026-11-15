#include <stdio.h>
#define NPROC 3
#define NRES 1

int Max[NPROC][NRES] = {{{9}, {4}, {7}}};
int Alloc[NPROC][NRES] = {{{3}, {2}, {2}}};
int Avail[NRES] = {3};
int who = 1;
int Request[NRES] = {2};

static int is_safe(void) {
    int work[NRES], done[NPROC] = {0}, count = 0;
    for (int j = 0; j < NRES; ++j) work[j] = Avail[j];
    printf(" ");
    int progress = 1;
    while (progress) {
        progress = 0;
        for (int i = 0; i < NPROC; ++i) {
            if (done[i]) continue;
            int ok = 1;
            for (int j = 0; j < NRES; ++j)
                if (Max[i][j] - Alloc[i][j] > work[j]) ok = 0;
            if (ok) {
                for (int j = 0; j < NRES; ++j) work[j] += Alloc[i][j];
                done[i] = 1;
                ++count;
                progress = 1;
                printf("%c ", 'A' + i);
            }
        }
    }
    return count == NPROC;
}

int main(void) {
    printf("Current state, finish order:");
    int initial_safe = is_safe();
    printf(" -> %s\\n", initial_safe ? "SAFE" : "UNSAFE");

    int valid = 1;
    for (int j = 0; j < NRES; ++j) {
        if (Request[j] > Avail[j] ||
            Request[j] > Max[who][j] - Alloc[who][j] ||
            Request[j] < 0) valid = 0;
    }
    if (!valid) {
        printf("After %c's request: request INVALID / POSTPONED\\n", 'A' + who);
        return 0;
    }

    for (int j = 0; j < NRES; ++j) {
        Avail[j] -= Request[j];
        Alloc[who][j] += Request[j];
    }
    printf("After %c's request, finish order:", 'A' + who);
    if (is_safe()) {
        printf(" -> SAFE, request GRANTED\\n");
    } else {
        printf(" -> UNSAFE, request POSTPONED\\n");
        for (int j = 0; j < NRES; ++j) {
            Avail[j] += Request[j];
            Alloc[who][j] -= Request[j];
        }
    }
    return 0;
}

#include <stdio.h>

#define NPROC 3
#define NRES 1

int Max[NPROC][NRES]   = { {9}, {4}, {7} };  /* Maximum claim */
int Alloc[NPROC][NRES] = { {3}, {2}, {2} };  /* Has */
int Avail[NRES]        = { 3 };              /* Free units */

int who = 1;                 /* The request comes from B (0 = A, 1 = B, ...) */
int Request[NRES] = { 2 };   /* B asks for 2 units */

/* Safe if there is an order in which every process can get its Max and finish */
int is_safe(void) {
    int work[NRES], done[NPROC] = { 0 }, count = 0;
    for (int j = 0; j < NRES; ++j)
        work[j] = Avail[j];

    int progress = 1;
    while (progress) {
        progress = 0;
        for (int i = 0; i < NPROC; ++i) {
            if (done[i])
                continue;
            int ok = 1;  /* Need = Max - Alloc must fit in work */
            for (int j = 0; j < NRES; ++j)
                if (Max[i][j] - Alloc[i][j] > work[j])
                    ok = 0;
            if (ok) {    /* i finishes and returns what it has */
                for (int j = 0; j < NRES; ++j)
                    work[j] += Alloc[i][j];
                done[i] = 1;
                count++;
                progress = 1;
                printf(" %c", 'A' + i);
            }
        }
    }
    return count == NPROC;
}

int main(void) {
    printf("Current state, finish order:");
    printf(is_safe() ? "  -> SAFE\n" : "  -> UNSAFE\n");

    /* Pretend to grant the request, then check the new state */
    for (int j = 0; j < NRES; ++j) {
        Avail[j] -= Request[j];
        Alloc[who][j] += Request[j];
    }
    printf("After %c's request, finish order:", 'A' + who);
    if (is_safe()) {
        printf("  -> SAFE, request GRANTED\n");
    } else {
        printf("  -> UNSAFE, request POSTPONED\n");
        for (int j = 0; j < NRES; ++j) {  /* Undo */
            Avail[j] += Request[j];
            Alloc[who][j] -= Request[j];
        }
    }
    return 0;
}

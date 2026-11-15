#include <stdio.h>
#include <stdbool.h>

#define P 5
#define R 3

int available[R]     = {3, 3, 2};
int max[P][R]        = {{7,5,3},{3,2,2},{9,0,2},{2,2,2},{4,3,3}};
int allocation[P][R] = {{0,1,0},{2,0,0},{3,0,2},{2,1,1},{0,0,2}};
int need[P][R];

bool is_safe(int seq[]) {
    int work[R];
    bool finish[P] = {false};
    for (int j = 0; j < R; j++) work[j] = available[j];

    int count = 0;
    while (count < P) {
        bool found = false;
        for (int i = 0; i < P; i++) {
            if (finish[i]) continue;
            int j;
            for (j = 0; j < R; j++)
                if (need[i][j] > work[j]) break;
            if (j == R) {
                for (int k = 0; k < R; k++) work[k] += allocation[i][k];
                finish[i] = true;
                seq[count++] = i;
                found = true;
            }
        }
        if (!found) return false;
    }
    return true;
}

bool request_resources(int p, int req[]) {
    /* Step 1: request must not exceed the declared maximum claim */
    for (int j = 0; j < R; j++) {
        if (req[j] > need[p][j]) {
            printf("  ERROR: P%d asked for more than its maximum claim\n", p);
            return false;
        }
    }

    /* Step 2: enough free resources right now? */
    for (int j = 0; j < R; j++) {
        if (req[j] > available[j]) {
            printf("  WAIT: not enough resources available now\n");
            return false;
        }
    }

    /* Step 3: pretend to allocate */
    for (int j = 0; j < R; j++) {
        available[j]     -= req[j];
        allocation[p][j] += req[j];
        need[p][j]       -= req[j];
    }

    /* Step 4: safety check */
    int seq[P];
    if (is_safe(seq)) {
        printf("  GRANTED: new state is safe, sequence");
        for (int i = 0; i < P; i++) printf(" P%d", seq[i]);
        printf("\n");
        return true;
    }

    /* unsafe: roll back */
    for (int j = 0; j < R; j++) {
        available[j]     += req[j];
        allocation[p][j] -= req[j];
        need[p][j]       += req[j];
    }
    printf("  DENIED: granting would leave the system unsafe, P%d must wait\n", p);
    return false;
}

int main(void) {
    for (int i = 0; i < P; i++)
        for (int j = 0; j < R; j++)
            need[i][j] = max[i][j] - allocation[i][j];

    int r1[R] = {1, 0, 2};
    int r2[R] = {3, 3, 0};
    int r3[R] = {0, 2, 0};
    int r4[R] = {1, 0, 0};

    printf("Request 1: P1 asks (1,0,2)\n"); request_resources(1, r1);
    printf("Request 2: P4 asks (3,3,0)\n"); request_resources(4, r2);
    printf("Request 3: P0 asks (0,2,0)\n"); request_resources(0, r3);
    printf("Request 4: P3 asks (1,0,0)\n"); request_resources(3, r4);
    printf("Final Available: %d %d %d\n", available[0], available[1], available[2]);
    return 0;
}

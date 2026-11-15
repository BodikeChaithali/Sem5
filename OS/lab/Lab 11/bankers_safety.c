#include <stdio.h>
#include <stdbool.h>

#define P 5   /* processes      */
#define R 3   /* resource types */

int available[R]     = {3, 3, 2};
int max[P][R]        = {{7,5,3},{3,2,2},{9,0,2},{2,2,2},{4,3,3}};
int allocation[P][R] = {{0,1,0},{2,0,0},{3,0,2},{2,1,1},{0,0,2}};
int need[P][R];

void compute_need(void) {
    for (int i = 0; i < P; i++)
        for (int j = 0; j < R; j++)
            need[i][j] = max[i][j] - allocation[i][j];
}

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
            if (j == R) {                       /* Need_i <= Work */
                for (int k = 0; k < R; k++)
                    work[k] += allocation[i][k]; /* Pi finishes, releases */
                finish[i] = true;
                seq[count++] = i;
                found = true;
            }
        }
        if (!found) return false;               /* nobody can proceed */
    }
    return true;
}

int main(void) {
    compute_need();
    printf("Process  Alloc   Max     Need\n");
    for (int i = 0; i < P; i++)
        printf("P%d       %d %d %d   %d %d %d   %d %d %d\n", i,
               allocation[i][0], allocation[i][1], allocation[i][2],
               max[i][0], max[i][1], max[i][2],
               need[i][0], need[i][1], need[i][2]);
    printf("Available: %d %d %d\n\n", available[0], available[1], available[2]);

    int seq[P];
    if (is_safe(seq)) {
        printf("System is in a SAFE state.\nSafe sequence: ");
        for (int i = 0; i < P; i++) printf("P%d%s", seq[i], i < P - 1 ? " -> " : "\n");
    } else {
        printf("System is NOT in a safe state.\n");
    }
    return 0;
}

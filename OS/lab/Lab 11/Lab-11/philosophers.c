#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/wait.h>

#define N 5

void philosopher(int id, int fixed) {
    int left = id, right = (id + 1) % N;
    int first = left, second = right;

    if (fixed && first > second) {          /* fix mode: lower-numbered fork first */
        int tmp = first; first = second; second = tmp;
    }

    char path[64];
    sprintf(path, "/tmp/fork%d.lock", first);
    int f1 = open(path, O_RDWR | O_CREAT, 0644);
    sprintf(path, "/tmp/fork%d.lock", second);
    int f2 = open(path, O_RDWR | O_CREAT, 0644);

    for (int meal = 1; meal <= 3; meal++) {
        usleep(100000);                     /* thinking */

        flock(f1, LOCK_EX);
        printf("P%d picked fork %d\n", id, first);
        fflush(stdout);

        usleep(300000);                     /* widen the deadlock window */

        flock(f2, LOCK_EX);
        printf("P%d picked fork %d -> EATING meal %d\n", id, second, meal);
        fflush(stdout);

        usleep(100000);

        flock(f2, LOCK_UN);
        flock(f1, LOCK_UN);
    }
    close(f1);
    close(f2);
    exit(0);
}

int main(int argc, char *argv[]) {
    int fixed = (argc > 1 && strcmp(argv[1], "fix") == 0);
    pid_t pid[N];

    printf("Mode: %s\n", fixed ? "FIXED (lower-numbered fork first)"
                               : "DEADLOCK (left fork, then right fork)");
    fflush(stdout);

    for (int i = 0; i < N; i++) {
        pid[i] = fork();
        if (pid[i] == 0) philosopher(i, fixed);
    }

    int finished = 0;                       /* watchdog: 8 seconds */
    for (int t = 0; t < 80 && finished < N; t++) {
        if (waitpid(-1, NULL, WNOHANG) > 0) finished++;
        else usleep(100000);
    }

    if (finished < N) {
        printf("Watchdog: %d of %d philosophers finished -> DEADLOCK\n", finished, N);
        printf("Recovery: killing all philosophers\n");
        for (int i = 0; i < N; i++) kill(pid[i], SIGKILL);
        while (wait(NULL) > 0) ;            /* reap them */
        return 1;
    }

    printf("All %d philosophers ate 3 meals. No deadlock.\n", N);
    return 0;
}

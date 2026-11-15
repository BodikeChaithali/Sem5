#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/wait.h>

/* Each resource is a lock file. flock() gives one process exclusive use. */
#define RES_A "/tmp/resA.lock"
#define RES_B "/tmp/resB.lock"

void use_resources(const char *name, const char *first, const char *second) {
    /* open inside the child so each process has its own lock */
    int f1 = open(first,  O_RDWR | O_CREAT, 0644);
    int f2 = open(second, O_RDWR | O_CREAT, 0644);

    flock(f1, LOCK_EX);                          /* acquire first resource */
    printf("%s (PID %d): holds %s, wants %s\n", name, getpid(),
           strrchr(first, '/') + 1, strrchr(second, '/') + 1);
    fflush(stdout);
    sleep(1);                                    /* let the other process grab its first */

    flock(f2, LOCK_EX);                          /* acquire second resource */
    printf("%s (PID %d): got both, working\n", name, getpid());
    fflush(stdout);

    flock(f2, LOCK_UN);                          /* release both */
    flock(f1, LOCK_UN);
    close(f1);
    close(f2);
    exit(0);
}

int main(int argc, char *argv[]) {
    int fixed = (argc > 1 && strcmp(argv[1], "fix") == 0);
    printf("Mode: %s\n", fixed ? "FIXED (same order A then B)"
                                : "DEADLOCK (P1: A then B, P2: B then A)");
    fflush(stdout);

    pid_t p1 = fork();
    if (p1 == 0) use_resources("P1", RES_A, RES_B);

    pid_t p2 = fork();
    if (p2 == 0) {
        if (fixed) use_resources("P2", RES_A, RES_B);
        else       use_resources("P2", RES_B, RES_A);
    }

    /* Watchdog: give the children 4 seconds to finish */
    int finished = 0;
    for (int t = 0; t < 40 && finished < 2; t++) {
        if (waitpid(-1, NULL, WNOHANG) > 0) finished++;   /* non-blocking check */
        else usleep(100000);
    }

    if (finished < 2) {
        printf("Watchdog: P1 and P2 still blocked after 4 s -> DEADLOCK\n");
        printf("Recovery: killing both processes\n");
        kill(p1, SIGKILL);
        kill(p2, SIGKILL);
        while (wait(NULL) > 0) ;                  /* reap them */
        return 1;
    }
    printf("Both processes finished. No deadlock.\n");
    return 0;
}

/*
 * Race Condition Demo — Two Separate Processes via fork() + shared memory
 *
 * Same structure as peterson_processes.c BUT without Peterson's Algorithm.
 * No flag[], no turn — both processes charge into the Critical Section
 * simultaneously, causing a race condition on shared_counter.
 *
 * Compile:  gcc -o race_condition race_condition.c
 * Run:      ./race_condition
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define ITERATIONS 10

/* ─── Shared memory structure ─── */
typedef struct {
    volatile int shared_counter;
} Shared;

/* ═══════════════════════════════════════════════
 *  NO LOCK — process barges straight into the CS
 * ═══════════════════════════════════════════════ */

/* ═══════════════════════════════════
 *  PROCESS BODY
 * ═══════════════════════════════════ */
void run_process(Shared *sh, int me)
{
    for (int i = 0; i < ITERATIONS; i++) {

        /* ── Remainder Section ── */
        printf("[PID %d | P%d] Remainder section (iteration %d)\n",
               getpid(), me, i + 1);
        fflush(stdout);
        usleep(50000);

        /* ── NO Entry Section — no lock acquired ── */
        printf("[PID %d | P%d] Wants to enter Critical Section (NO LOCK)...\n",
               getpid(), me);
        fflush(stdout);

        /* ══ CRITICAL SECTION — UNPROTECTED ════════════════ */
        printf("[PID %d | P%d] >>> ENTERED CS | counter = %d\n",
               getpid(), me, sh->shared_counter);
        fflush(stdout);

        /*
         * RACE HERE:
         *   Step 1: read  shared_counter into a local temp
         *   Step 2: sleep (other process can read the SAME old value!)
         *   Step 3: write temp+1 back — one increment is LOST
         */
        int temp = sh->shared_counter;  /* both processes may read same value */
        usleep(20000);                  /* simulate work — widens the race window */
        sh->shared_counter = temp + 1; /* both may write the same result back  */

        printf("[PID %d | P%d] <<< LEAVING CS  | counter = %d\n",
               getpid(), me, sh->shared_counter);
        fflush(stdout);
        /* ═══════════════════════════════════════════════════ */

        /* ── NO Exit Section — no lock released ── */

        usleep(10000);
    }

    printf("[PID %d | P%d] Finished all %d iterations.\n",
           getpid(), me, ITERATIONS);
    fflush(stdout);
}

/* ─── Main ─── */
int main(void)
{
    /* Allocate shared memory visible to both parent and child */
    Shared *sh = mmap(NULL, sizeof(Shared),
                      PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS,
                      -1, 0);
    if (sh == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    sh->shared_counter = 0;

    printf("==============================================\n");
    printf("  Race Condition Demo — Two Separate Processes\n");
    printf("==============================================\n");
    printf("Shared memory at : %p\n", (void *)sh);
    printf("Each process runs : %d iterations\n", ITERATIONS);
    printf("Expected counter  : %d  (if no race)\n", ITERATIONS * 2);
    printf("WARNING: No synchronization — race condition expected!\n\n");

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        munmap(sh, sizeof(Shared));
        return 1;
    }

    if (pid == 0) {
        /* ── Child process → acts as P1 ── */
        run_process(sh, 1);
        munmap(sh, sizeof(Shared));
        exit(0);
    } else {
        /* ── Parent process → acts as P0 ── */
        run_process(sh, 0);

        int status;
        waitpid(pid, &status, 0);

        int expected = ITERATIONS * 2;
        int actual   = sh->shared_counter;
        int lost     = expected - actual;

        printf("\n==============================================\n");
        printf("Parent PID : %d  (P0)\n", getpid());
        printf("Child  PID : %d  (P1)\n", pid);
        printf("----------------------------------------------\n");
        printf("Expected counter  : %d\n", expected);
        printf("Actual   counter  : %d\n", actual);
        printf("Lost increments   : %d  <-- race condition!\n", lost);
        printf("----------------------------------------------\n");

        if (actual != expected)
            printf("✗ RACE CONDITION CONFIRMED — %d update(s) lost!\n", lost);
        else
            printf("~ No visible corruption this run (race is non-deterministic).\n"
                   "  Try running again — it will appear.\n");

        printf("==============================================\n");

        munmap(sh, sizeof(Shared));
    }

    return 0;
}

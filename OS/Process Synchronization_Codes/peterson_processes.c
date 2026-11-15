/*
 * Peterson's Algorithm — Two SEPARATE PROCESSES via fork() + shared memory
 *
 * Uses POSIX shared memory (mmap with MAP_SHARED) so both processes
 * share the same flag[], turn, and counter variables.
 *
 * Compile:  gcc -o peterson_proc peterson_processes.c
 * Run:      ./peterson_proc
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define ITERATIONS 5

/* ─── Shared memory structure ─── */
typedef struct {
    volatile int flag[2];       /* flag[i]=1 means process i wants CS */
    volatile int turn;          /* whose turn when both want in        */
    volatile int shared_counter;
} Shared;

/* ═══════════════════════════════════
 *  ENTRY SECTION — Peterson's lock
 * ═══════════════════════════════════ */
void peterson_lock(Shared *sh, int me, int other)
{
    sh->flag[me] = 1;       /* Announce: I want to enter            */
    sh->turn     = other;   /* Yield priority to the other process  */

    /* Busy-wait: spin while other wants in AND it's their turn */
    while (sh->flag[other] == 1 && sh->turn == other)
        ;
}

/* ═══════════════════════════════════
 *  EXIT SECTION — Peterson's unlock
 * ═══════════════════════════════════ */
void peterson_unlock(Shared *sh, int me)
{
    sh->flag[me] = 0;   /* I am done — release interest */
}

/* ═══════════════════════════════════
 *  PROCESS BODY
 * ═══════════════════════════════════ */
void run_process(Shared *sh, int me)
{
    int other = 1 - me;

    for (int i = 0; i < ITERATIONS; i++) {

        /* ── Remainder Section ── */
        printf("[PID %d | P%d] Remainder section (iteration %d)\n",
               getpid(), me, i + 1);
        fflush(stdout);
        usleep(100000);

        /* ── Entry Section ── */
        printf("[PID %d | P%d] Wants to enter Critical Section...\n",
               getpid(), me);
        fflush(stdout);
        peterson_lock(sh, me, other);

        /* ══ CRITICAL SECTION ═══════════════════════════════ */
        printf("[PID %d | P%d] >>> ENTERED CS | counter = %d\n",
               getpid(), me, sh->shared_counter);
        fflush(stdout);

        /* Read → modify → write (would race without Peterson's) */
        int temp = sh->shared_counter;
        usleep(20000);              /* simulate work inside CS */
        sh->shared_counter = temp + 1;

        printf("[PID %d | P%d] <<< LEAVING CS  | counter = %d\n",
               getpid(), me, sh->shared_counter);
        fflush(stdout);
        /* ═══════════════════════════════════════════════════ */

        /* ── Exit Section ── */
        peterson_unlock(sh, me);

        usleep(30000);
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

    /* Initialise shared state */
    sh->flag[0]      = 0;
    sh->flag[1]      = 0;
    sh->turn         = 0;
    sh->shared_counter = 0;

    printf("==============================================\n");
    printf("  Peterson's Algorithm — Two Separate Processes\n");
    printf("==============================================\n");
    printf("Shared memory at: %p\n", (void *)sh);
    printf("Each process runs %d iterations.\n", ITERATIONS);
    printf("Expected final counter: %d\n\n", ITERATIONS * 2);

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

        /* Wait for child to finish */
        int status;
        waitpid(pid, &status, 0);

        /* Final verification */
        printf("\n==============================================\n");
        printf("Parent PID : %d  (P0)\n", getpid());
        printf("Child  PID : %d  (P1)\n", pid);
        printf("Final shared_counter = %d  (expected: %d)\n",
               sh->shared_counter, ITERATIONS * 2);

        if (sh->shared_counter == ITERATIONS * 2)
            printf("✓ No race condition — Peterson's Algorithm works!\n");
        else
            printf("✗ Race condition detected — counter mismatch!\n");

        printf("==============================================\n");

        munmap(sh, sizeof(Shared));
    }

    return 0;
}

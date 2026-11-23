#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t keep_running = 1;   
volatile sig_atomic_t usr1_count = 0;
void handle_sigint(int sig) {
    printf("\n[Handler] Caught SIGINT (signal %d) - Ctrl+C detected.\n", sig);
    printf("[Handler] Press Ctrl+C again within 3 seconds to confirm exit...\n");
    signal(SIGINT, handle_sigint);
}
void handle_sigterm(int sig) {
    printf("\n[Handler] Caught SIGTERM (signal %d). Shutting down gracefully...\n", sig);
    keep_running = 0;  
}

void handle_sigusr1(int sig) {
    usr1_count++;
    printf("\n[Handler] Caught SIGUSR1 (signal %d). Count so far: %d\n", sig, usr1_count);
}
void handle_sigquit(int sig) {
    printf("\n[Handler] Caught SIGQUIT (signal %d) - Ctrl+\\ detected.\n", sig);
    printf("[Handler] Ignoring it this time (demo only).\n");
}

int main() {
    printf("PID of this process: %d\n", getpid());
    printf("---------------------------------------------\n");
    printf("Try in another terminal:\n");
    printf("  kill -SIGUSR1 %d\n", getpid());
    printf("  kill -SIGTERM %d\n", getpid());
    printf("Or press Ctrl+C / Ctrl+\\ here.\n");
    printf("---------------------------------------------\n\n");
    signal(SIGINT,  handle_sigint);
    signal(SIGTERM, handle_sigterm);
    signal(SIGUSR1, handle_sigusr1);
    signal(SIGQUIT, handle_sigquit);
    int n;
    int type,value;
    scanf("%d",n);
    scanf("%d %d",type,value);
    int seconds = 0;
    while(n-- > 0) {
    if(type == 2) {
      while (keep_running) {
          printf("Main loop running... (%d sec elapsed, usr1_count=%d)\n",
               seconds, usr1_count);
        sleep(1);
        seconds++;
      }
    }
    }

    printf("\nExiting main loop cleanly. Goodbye.\n");
    return 0;
}

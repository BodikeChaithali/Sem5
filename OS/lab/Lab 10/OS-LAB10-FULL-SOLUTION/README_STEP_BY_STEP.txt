OS LAB 10 — FULL SOURCE CODE AND EXECUTION GUIDE
================================================

Extract this folder. Open a terminal in this folder.

Compiler:
    sudo apt update
    sudo apt install build-essential

All five source families are provided as complete standalone C files.
Do not edit fragments: compile the exact file named in each step.

HANDS-ON 1: BAKERY
------------------
1) Original, NPROC=3, ITERATIONS=10000:
   gcc bakery_original.c -o bakery_original
   ./bakery_original
   Repeat ./bakery_original three times.
   Expected counter: 30000.

2) NPROC=5:
   gcc bakery_nproc5.c -o bakery_nproc5
   ./bakery_nproc5
   Expected counter: 50000.

3) NPROC=8:
   gcc bakery_nproc8.c -o bakery_nproc8
   ./bakery_nproc8
   Expected counter: 80000.

4) ITERATIONS=100 (NPROC=3):
   gcc bakery_iterations100.c -o bakery_iterations100
   ./bakery_iterations100
   Expected counter: 300.

5) Without choosing wait:
   gcc bakery_no_choosing.c -o bakery_no_choosing
   ./bakery_no_choosing
   Repeat several times. This intentionally removes the choosing[] wait;
   output may still be correct in a particular run. Record every observed value.

HANDS-ON 2: MUTEX
-----------------
1) Short critical section:
   gcc mutex_short.c -o mutex_short -pthread
   ./mutex_short
   time ./mutex_short
   Expected counter: 200000.

2) Long critical section (ITERATIONS=1000, CS_DELAY_US=1000):
   gcc mutex_long.c -o mutex_long -pthread
   ./mutex_long
   time ./mutex_long
   Expected counter: 2000. Record real/user/sys times from your computer.

HANDS-ON 3: DINING PHILOSOPHERS
-------------------------------
1) Five meals each:
   gcc dining_meals5.c -o dining_meals5 -pthread
   ./dining_meals5
   Expected meals: 25.

2) Twenty meals each:
   gcc dining_meals20.c -o dining_meals20 -pthread
   ./dining_meals20
   Expected meals: 100.
   Record the largest 'eating now' value shown. It must not exceed 2.

HANDS-ON 4: DEADLOCK DETECTION
------------------------------
1) Original:
   gcc deadlock_original.c -o deadlock_original
   ./deadlock_original
   Expected: P3, P2, P1 finish; no deadlock.

2) P3 requests one extra scanner:
   gcc deadlock_p3_scanner.c -o deadlock_p3_scanner
   ./deadlock_p3_scanner
   Expected deadlocked set: P1 P2 P3.

3) Recovery:
   gcc deadlock_recovery.c -o deadlock_recovery
   ./deadlock_recovery
   It selects the unfinished process holding the fewest resources, kills it,
   releases its allocation, clears its request/allocation rows, then detects again.

4) Seven-process graph:
   gcc deadlock_graph7.c -o deadlock_graph7
   ./deadlock_graph7
   Expected deadlocked set: P2 P4 P5 P7. P1, P3 and P6 can finish.

HANDS-ON 5: BANKER'S ALGORITHM
------------------------------
1) Original state, B asks for 2:
   gcc banker_original.c -o banker_original
   ./banker_original
   Initial state safe; B's request granted.

2) A allocation=4, Free=2:
   gcc banker_a4_free2.c -o banker_a4_free2
   ./banker_a4_free2
   Initial state unsafe; request is postponed.

3) Original state, A asks for 1:
   gcc banker_a_requests1.c -o banker_a_requests1
   ./banker_a_requests1
   Initial state safe; A's request postponed because tentative state is unsafe.

4) Multi-resource initial state:
   gcc banker_multi_initial.c -o banker_multi_initial
   ./banker_multi_initial

5) B requests (0,0,1,0):
   gcc banker_multi_b_request.c -o banker_multi_b_request
   ./banker_multi_b_request

6) After B's request is granted, E requests (0,0,1,0):
   gcc banker_multi_e_request.c -o banker_multi_e_request
   ./banker_multi_e_request

NOTES
-----
- Scheduler order and timing vary. Do not copy timing values from another machine.
- Record the exact observed output from your terminal in the PDF tables.
- In the Bakery algorithm, removing choosing[] wait does not force every run to fail.
- Banker's algorithm uses Need = Max - Allocation and checks a tentative grant
  by temporarily updating Available and Allocation, then rolling back if unsafe.

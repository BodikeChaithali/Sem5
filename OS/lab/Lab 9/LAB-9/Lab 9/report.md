## Hands-on 1 — Run and Investigate the Race-Condition Program

### Observation Table

| Run | Iterations/process | Expected total | Observed total |
|---|---:|---:|---:|
| Original run 1 | 1000 | 2000 | 1001 |
| Original run 2 | 1000 | 2000 | 1000 |
| Original run 3 | 1000 | 2000 | 1001 |
| 100 iterations | 100 | 200 | 100 |
| No-delay run 1 | 100 | 200 | 200 |
| No-delay run 2 | 100 | 200 | 200 |
| No-delay run 3 | 100 | 200 | 200 |

### Does one correct run prove that the program is race-free?

No, one correct run does not prove that the program is race-free because concurrent processes may execute in different interleavings. A correct result in one execution may occur by chance even though unsynchronized access to the shared counter can cause lost updates.

### Analysis

The original program produces incorrect counter values because both processes can read the same counter value and overwrite each other's updates. Reducing the number of iterations to 100 still produces a lost update in the recorded run. After removing the delay, all three recorded runs produced the expected value of 200, but this does not prove that the race condition has been eliminated.

---

## Hands-on 2 — Implement and Trace Three Approaches

### Version A: Strict Alternation

**Entry section:**
```c
while (sh->turn != me) {
    /* Busy waiting */
}
```

**Critical section:**
```c
sh->counter++;
```

**Exit section:**
```c
sh->turn = other;
```

**Recorded result:** Expected counter = 20000; observed counter = 20000.

**Analysis:** Strict alternation uses the shared `turn` variable to decide which process can enter the critical section. It enforces turn-based access in this teaching example, but it can violate progress if the process whose turn it is does not need to enter.

### Version B: Check-Then-Set Flags

**Entry section:**
```c
while (sh->flag[other]) {
    /* Busy waiting */
}
sh->flag[me] = 1;
```

**Critical section:**
```c
sh->counter++;
```

**Exit section:**
```c
sh->flag[me] = 0;
```

**Recorded result:** Expected counter = 20000; observed counter = 20000 in the displayed runs.

**Analysis:** Both processes can check the other process's flag before either sets its own flag. Therefore, both can enter the critical section, and mutual exclusion is not guaranteed. A successful execution does not prove that the algorithm is correct.

### Version C: Set-Then-Wait Flags

**Entry section:**
```c
sh->flag[me] = 1;

while (sh->flag[other]) {
    /* Busy waiting */
}
```

**Critical section:**
```c
sh->counter++;
```

**Exit section:**
```c
sh->flag[me] = 0;
```

**Recorded result:** Expected counter = 20000; observed counter = 20000; exit status = 0.

**Deadlock trace:**

1. Process 0 sets `flag[0] = 1`.
2. Process 1 sets `flag[1] = 1`.
3. Process 0 waits because `flag[1] == 1`.
4. Process 1 waits because `flag[0] == 1`.
5. Neither process can reach the exit section to reset its flag, so both may wait indefinitely.

**Analysis:** The recorded execution completed successfully, but the algorithm can deadlock when both processes set their flags before checking the other process's flag. The successful run does not disprove the possibility of deadlock.

### Hands-on 2 Results Table

| Version | Entry section | Exit section |
|---|---|---|
| A: Strict alternation | `while (sh->turn != me)` | `sh->turn = other;` |
| B: Check-then-set flags | Check other flag, then set own flag | `sh->flag[me] = 0;` |
| C: Set-then-wait flags | Set own flag, then wait for other flag | `sh->flag[me] = 0;` |

---

## Hands-on 3 — Modify, Run, and Compare

### Peterson's Algorithm

**Entry section:**
```c
sh->flag[me] = 1;
sh->turn = other;

while (sh->flag[other] == 1 && sh->turn == other) {
    /* Busy waiting */
}
```

**Critical section:**
```c
int temp = sh->counter;
sh->counter = temp + 1;
```

**Exit section:**
```c
sh->flag[me] = 0;
```

### Recorded Results

| Experiment | Iterations/process | Expected | Observed | Notes |
|---|---:|---:|---:|---|
| Race condition, run 1 | 1000 | 2000 | 1001 | Lost updates |
| Race condition, run 2 | 1000 | 2000 | 1000 | Lost updates |
| Strict alternation | 10000 | 20000 | 20000 | Turn-based access |
| Check-then-set flags | 10000 | 20000 | 20000 | Mutual exclusion not guaranteed |
| Set-then-wait flags | 10000 | 20000 | 20000 | Completed; deadlock remains possible |
| Peterson | 10000 | 20000 | 20000 | Waiting condition enabled |
| Peterson, 100 iterations | 100 | 200 | 200 | Reduced workload |
| Peterson without wait, run 1 | 10000 | 20000 | 20000 | Waiting condition removed |
| Peterson without wait, run 2 | 10000 | 20000 | 20000 | Waiting condition removed |
| Peterson without wait, run 3 | 10000 | 20000 | 20000 | Waiting condition removed |
| Race condition, 10000 iterations | 10000 | 20000 | Pending | Additional comparison required |

### Why is output alone not a complete correctness proof?

Output alone is not a complete correctness proof because a program may produce the expected counter in one execution while still allowing race conditions in other execution interleavings. Correct synchronization requires reasoning about mutual exclusion, progress, and the algorithm's behavior, rather than relying only on observed counter values.

---

## Conclusion

The experiments demonstrate that unsynchronized access to a shared counter can cause lost updates. Strict alternation enforces turn-based entry but can restrict progress. Check-then-set flags do not guarantee mutual exclusion, while set-then-wait flags can deadlock. Peterson's algorithm introduces a waiting condition intended to coordinate two processes, whereas removing that condition removes its intended mutual-exclusion protection.

The observed results are experimental evidence for the executions performed, not a complete proof of algorithm correctness.

#include <stdio.h>
#include <stdbool.h>

#define READ      (1 << 0)   // 001
#define WRITE     (1 << 1)   // 010
#define EXEC      (1 << 2)   // 100
#define READ_EXEC (READ | EXEC)

typedef struct {
    unsigned int base;
    unsigned int limit;
    unsigned char perms;
    int min_ring;
} ProtectedSegment;

/*
    Ring 0 = Kernel
    Ring 1
    Ring 2
    Ring 3 = User
*/

ProtectedSegment table[4] = {
    {1000, 500, READ | EXEC, 0},  // Seg 0: Kernel Code
    {2000, 500, READ | EXEC, 1},  // Seg 1
    {3000, 800, READ | WRITE, 2}, // Seg 2
    {4000, 800, READ, 3}          // Seg 3
};

bool access_segment(int seg,
                    unsigned int offset,
                    unsigned char req_perm,
                    int current_cpu_ring)
{
    if (seg < 0 || seg >= 4) {
        printf("[TRAP] Invalid Segment!\n");
        return false;
    }

    // Permission check
    if ((table[seg].perms & req_perm) != req_perm) {
        printf("[TRAP] Protection Violation!\n");
        return false;
    }

    // Ring privilege check
    if (current_cpu_ring > table[seg].min_ring) {
        printf("[TRAP] Privilege Violation!\n");
        return false;
    }

    // Bounds check
    if (offset >= table[seg].limit) {
        printf("[TRAP] Segmentation Fault!\n");
        return false;
    }

    printf("SUCCESS -> Physical Address: %u\n",
           table[seg].base + offset);

    return true;
}

int main()
{
    printf("=== Multi-Ring Hardware Privilege Checker ===\n\n");

    access_segment(0, 0, READ, 3);

    access_segment(0, 0, READ, 0);

    access_segment(3, 1, READ, 1);

    access_segment(2, 1, WRITE, 2);

    access_segment(1, 0, READ_EXEC, 3);

    return 0;
}

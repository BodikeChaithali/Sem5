#include <stdio.h>
#include <stdbool.h>

#define MAX_BLOCKS 10

typedef struct {
    int start;
    int size;
    bool is_free;
} Block;

Block memory[MAX_BLOCKS] = {
    {0,   200, false},   // Occupied
    {200, 300, true},    // Hole A
    {500, 200, false},   // Occupied
    {700, 200, true}     // Hole B
};

int block_count = 4;

int worst_fit_allocate(int req_size)
{
    int worst_index = -1;

    // Find the largest free hole that can satisfy req_size
    for (int i = 0; i < block_count; i++) {

        if (memory[i].is_free &&
            memory[i].size >= req_size) {

            if (worst_index == -1 ||
                memory[i].size > memory[worst_index].size) {

                worst_index = i;
            }
        }
    }

    // No suitable hole found
    if (worst_index == -1) {
        printf("[FAILED] No suitable free hole found.\n");
        return -1;
    }

    int allocated_base = memory[worst_index].start;
    int leftover_size = memory[worst_index].size - req_size;

    // Split the block if there is leftover space
    if (leftover_size > 0) {

        if (block_count >= MAX_BLOCKS) {
            printf("[FAILED] No space for new block entry.\n");
            return -1;
        }

        // Shift blocks to create a new leftover block
        for (int i = block_count; i > worst_index + 1; i--) {
            memory[i] = memory[i - 1];
        }

        // Allocated part
        memory[worst_index].size = req_size;
        memory[worst_index].is_free = false;

        // Leftover free part
        memory[worst_index + 1].start =
            allocated_base + req_size;

        memory[worst_index + 1].size =
            leftover_size;

        memory[worst_index + 1].is_free = true;

        block_count++;
    }
    else {
        memory[worst_index].is_free = false;
    }

    printf("[ALLOC] Requested Size: %d\n", req_size);
    printf("[ALLOC] Chosen Hole Base: %d\n", allocated_base);

    if (leftover_size > 0) {
        printf("[SPLIT] New Free Hole -> Base: %d, Size: %d\n",
               allocated_base + req_size,
               leftover_size);
    }

    return allocated_base;
}

void print_memory()
{
    printf("\nCurrent Memory Blocks:\n");

    for (int i = 0; i < block_count; i++) {

        printf("Block %d: Base=%d Size=%d %s\n",
               i,
               memory[i].start,
               memory[i].size,
               memory[i].is_free ? "FREE" : "ALLOCATED");
    }
}

int main()
{
    printf("=== Worst-Fit Allocation ===\n\n");

    worst_fit_allocate(150);

    print_memory();

    return 0;
}

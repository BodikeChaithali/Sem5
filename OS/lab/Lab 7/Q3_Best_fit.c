#include <stdio.h>
#include <stdbool.h>

#define MAX_BLOCKS 10

typedef struct {
    int start;
    int size;
    bool is_free;
} Block;

Block memory[MAX_BLOCKS] = {
    {0,   200, false},
    {200, 300, true},   // Hole A
    {500, 200, false},
    {700, 200, true}    // Hole B
};

int block_count = 4;

int best_fit_allocate(int req_size)
{
    int best_index = -1;

    // Find the smallest free hole that can fit req_size
    for (int i = 0; i < block_count; i++) {

        if (memory[i].is_free &&
            memory[i].size >= req_size) {

            if (best_index == -1 ||
                memory[i].size < memory[best_index].size) {

                best_index = i;
            }
        }
    }

    if (best_index == -1) {
        printf("[FAILED] No suitable free hole found.\n");
        return -1;
    }

    int allocated_base = memory[best_index].start;

    int leftover_size =
        memory[best_index].size - req_size;

    // If there is leftover space, create a new block
    if (leftover_size > 0) {

        if (block_count >= MAX_BLOCKS) {
            printf("[FAILED] No space for new block entry.\n");
            return -1;
        }

        // Move blocks to make space for new leftover block
        for (int i = block_count; i > best_index + 1; i--) {
            memory[i] = memory[i - 1];
        }

        memory[best_index].size = req_size;
        memory[best_index].is_free = false;

        memory[best_index + 1].start =
            allocated_base + req_size;

        memory[best_index + 1].size =
            leftover_size;

        memory[best_index + 1].is_free = true;

        block_count++;
    }
    else {
        memory[best_index].is_free = false;
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
    printf("=== Best-Fit Allocation ===\n\n");

    best_fit_allocate(150);

    print_memory();

    return 0;
}

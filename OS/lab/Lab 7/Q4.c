#include <stdio.h>
#include <string.h>

#define RAM_SIZE 1000

char RAM[RAM_SIZE];

typedef struct {
    int id;
    int base;
    int size;
} ActiveSegment;

ActiveSegment segs[2] = {
    {0, 200, 6},   // "HELLO" + '\0'
    {1, 600, 6}    // "WORLD" + '\0'
};

int count = 2;

void compact_memory()
{
    int current_base = 0;

    printf("=== Starting Memory Compaction ===\n\n");

    for (int i = 0; i < count; i++) {

        int old_base = segs[i].base;
        int new_base = current_base;

        printf("Relocating Seg %d: Old Base = %d -> New Base = %d\n",
               segs[i].id,
               old_base,
               new_base);

        // Move actual bytes
        memmove(&RAM[new_base],
                &RAM[old_base],
                segs[i].size);

        // Clear old location if it is different
        if (old_base != new_base) {
            memset(&RAM[old_base], 0, segs[i].size);
        }

        // Update segment base
        segs[i].base = new_base;

        // Move to next free location
        current_base += segs[i].size;
    }

    printf("\nCompaction Complete.\n");
    printf("Free Space Starts at Base: %d\n", current_base);
}

int main()
{
    // Put initial data into simulated RAM
    strcpy(&RAM[200], "HELLO");
    strcpy(&RAM[600], "WORLD");

    printf("Before Compaction:\n");
    printf("RAM[200] = %s\n", &RAM[200]);
    printf("RAM[600] = %s\n\n", &RAM[600]);

    compact_memory();

    printf("\nAfter Compaction:\n");
    printf("Seg 0 Base = %d\n", segs[0].base);
    printf("Seg 1 Base = %d\n", segs[1].base);

    printf("RAM[0] = %s\n", &RAM[0]);
    printf("RAM[6] = %s\n", &RAM[6]);

    return 0;
}

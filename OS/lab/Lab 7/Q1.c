#include <stdio.h>
#include <stdbool.h>

typedef struct {
    int segment_id;
    unsigned int base;
    unsigned int limit;
    int downward;
} Segment;

#define MAX_SEGMENTS 3

Segment segment_table[MAX_SEGMENTS] = {
    {0, 1400, 1000, 0},  // Code
    {1, 6300, 400,  0},  // Data
    {2, 4700, 400,  1}   // Stack (Downward)
};

bool translate(int seg_num, unsigned int offset, unsigned int *phys_addr)
{
    if (seg_num < 0 || seg_num >= MAX_SEGMENTS)
        return false;

    // Bounds check
    if (offset >= segment_table[seg_num].limit)
        return false;

    if (segment_table[seg_num].downward)
        *phys_addr = segment_table[seg_num].base - offset;
    else
        *phys_addr = segment_table[seg_num].base + offset;

    return true;
}

int main()
{
    unsigned int phys;

    // Valid downward-growing stack access
    if (translate(2, 100, &phys))
        printf("Seg 2, Offset 100 -> Physical Address: %u\n", phys);
    else
        printf("Seg 2, Offset 100 -> SEGMENTATION FAULT\n");

    // Invalid access
    if (translate(2, 500, &phys))
        printf("Seg 2, Offset 500 -> Physical Address: %u\n", phys);
    else
        printf("Seg 2, Offset 500 -> SEGMENTATION FAULT\n");

    return 0;
}

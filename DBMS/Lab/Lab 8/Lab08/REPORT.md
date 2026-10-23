# Lab 7a — Heap File with Slotted Pages

## 1. Why are slot numbers kept stable after deletion?

Slot numbers are kept stable because indexes may store a record's RID, which contains the page ID and slot number. If deleting a record shifted the other slots, existing index entries could point to the wrong records. A tombstone marks a deleted slot without changing the positions of other slots.

## 2. Why does deletion use tombstones instead of immediately reclaiming space?

Deletion replaces the slot entry with `(0, 0)`. This is a small metadata update and does not require moving other records. The deleted record's bytes remain in the page until compaction, so deletion can preserve the RIDs of the remaining records.

## 3. What does page compaction do?

Page compaction moves live records together to remove unused space left by deleted records. It updates the physical offsets in their slot entries while preserving their slot numbers. Tombstones remain unchanged, and the reclaimed space can be used for new insertions.

## 4. Why is `HeapFile.scan()` implemented as a generator?

A generator yields records one at a time instead of storing every record in memory. The scan reads each page and visits its live slots in page-then-slot order, skipping deleted records. This makes sequential scanning more memory-efficient for large heap files.

## Test Results

### 1. Automated Test Cases

Command used:

```bash
python3 -m pytest test_heapfile.py -q
```

Output:

```text
........                                                                 [100%]
8 passed in 0.04s
```

All 8 test cases passed successfully.

### 2. Manual Smoke Test

Command used:

```bash
python3 heapfile.py
```

Output:

```text
get r1: b'hello'
delete r1: True
scan: [(RID(page_id=0, slot_no=1), b'world')]
```

The manual test successfully verified record retrieval, deletion, and scanning of the remaining live record.
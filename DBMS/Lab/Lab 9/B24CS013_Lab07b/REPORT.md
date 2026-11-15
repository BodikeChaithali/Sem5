# Lab 7b Report — Records, File Organizations & the Catalog

## Question 1: Variable vs Fixed Length

The encoded Zhang row `(128, "Zhang", "Comp. Sci.", 102)` occupies 28 bytes. The actual values occupy 21 bytes: `id` (4 bytes), `name` (5 bytes), `dept_name` (10 bytes), and `tot_cred` (2 bytes). Therefore, the format overhead is **28 − 21 = 7 bytes**, consisting of a 2-byte field-count header, a 1-byte NULL bitmap, and a 4-byte offset array for the two VARCHAR fields.

The fixed-length version using `CHAR(20)` for both strings occupies 46 bytes, so our format saves 18 bytes for this row. Fixed-length records can be preferable for tables containing consistently sized fields, such as fixed-width numeric telemetry records, because field positions are predictable and no per-row VARCHAR offset array is needed.

## Question 2: Where Do NULLs Go?

Our record format reserves every fixed field's slot even when its value is NULL and marks NULLs in a bitmap. This keeps fixed-field positions constant and allows direct field access, but wastes the reserved bytes for NULL fixed values. PostgreSQL can omit NULL attributes from the tuple's data area, saving space when many attributes are NULL. However, accessing attributes can require processing the NULL bitmap and tuple layout. The trade-off is storage efficiency versus simpler, predictable field positioning.

## Question 3: Measured Costs

### Experiment 1 — 4,000 records

Command:

```bash
python3 compare.py
```

Output:

```text
N = 4000 records, 10 records/page

Organization    pages B   lookup reads   range reads   insert I/Os
Heap                400          210.7         400.0           2.0
Sorted              400            9.7          11.1          16.5
Hashed              579            1.0         579.0           2.3
```

### Experiment 2 — 20,000 records

Command:

```bash
python3 compare.py 20000 10
```

Output:

```text
N = 20000 records, 10 records/page

Organization    pages B   lookup reads   range reads   insert I/Os
Heap               2000         1002.6        2000.0           2.0
Sorted             2000           12.0          15.1          18.5
Hashed             2931            1.0        2931.0           2.2
```

### Comparison with Lecture 21

The theoretical point-lookup costs are approximately \(B/2\) reads for heap files, \(\log_2 B\) reads for sorted files, and one read for hashed files without overflow.

| Records | Organization | Theoretical lookup cost | Measured lookup cost |
|---:|---|---:|---:|
| 4,000 | Heap | 400 / 2 = 200 | 210.7 |
| 4,000 | Sorted | log₂(400) ≈ 8.64 | 9.7 |
| 4,000 | Hashed | Approximately 1 | 1.0 |
| 20,000 | Heap | 2,000 / 2 = 1,000 | 1,002.6 |
| 20,000 | Sorted | log₂(2,000) ≈ 10.97 | 12.0 |
| 20,000 | Hashed | Approximately 1 | 1.0 |

The measured lookup costs are close to the theoretical predictions. Heap lookup scans pages sequentially until the key is found, while sorted lookup uses binary search to locate the relevant primary page. Hash lookup accesses the appropriate bucket directly; the measured average is one read in both experiments.

Range queries behave differently: heap files scan every page, sorted files locate the starting page and scan the relevant pages, and hashed files must scan all buckets and overflow pages because neighbouring keys can reside in different buckets.

Sorted inserts are expensive because every inserted key is larger than all existing keys. Each insertion locates the last primary page, which is full, and follows or extends its overflow chain. This causes repeated page reads and writes as the chain grows. Real systems use techniques such as B-tree/B+ tree page splits, rebalancing, buffering, and overflow management to handle dynamic ordered data more efficiently.

## Question 4: Choosing an Organization

**(a) Look up a user by `user_id` on every login — Hashed.**

A hash organization supports exact-key lookups in approximately one page read when there is no overflow. This makes it suitable for frequent user-ID lookups, provided collisions and overflow are handled appropriately.

**(b) List all orders placed between two dates — Sorted.**

Keeping records or an index ordered by order date allows the database to seek to the lower bound and scan only the pages covering the requested range.

**(c) Append-only audit log that is only ever scanned in full — Heap.**

Appending records to a heap file is simple and inexpensive. Since the workload always scans the entire log, maintaining sorted order provides little benefit.

## Question 5: SQLite Under the Microscope

### C1. The File Header and the Catalog

The SQLite database header begins with the magic string `SQLite format 3\0`. The page size is 4096 bytes, and the database contains 21 pages, giving a file size of 86,016 bytes. The `sqlite_master` catalog lists ten tables and their ten corresponding primary-key indexes.

The `student` table has root page 10, and its primary-key index has root page 11. The `takes` table has root page 20 and contains 24 cells. The root-page information allows SQLite to locate the B-tree associated with each table or index.

### C2. A B-Tree Page Is a Slotted Page

Command:

```bash
python3 peek_sqlite.py university.db 10 --cells
```

Actual output:

```text
page 10 at byte offset 36864
  type                    : leaf table
  first_freeblock         : 0
  num_cells               : 15
  cell_content_start      : 3684
  fragmented_bytes        : 0
  header_len              : 8
  cell_pointer_array_end  : 38
  unallocated_gap         : 3646
  cell pointer array (slot -> offset of the cell in this page):
    slot   0 -> 4068
    slot   1 -> 4038
    slot   2 -> 4012
    slot   3 -> 3986
    slot   4 -> 3959
    slot   5 -> 3935
    slot   6 -> 3904
    slot   7 -> 3873
    slot   8 -> 3850
    slot   9 -> 3822
    slot  10 -> 3796
    slot  11 -> 3765
    slot  12 -> 3739
    slot  13 -> 3713
    slot  14 -> 3684
```

Page 10 is a leaf table page containing 15 cells. Its cell pointer array ends at byte 38, and the unallocated gap is 3646 bytes. The first cell pointer is 4068, near the end of the page, and subsequent pointers move toward lower offsets. This shows that SQLite packs cells from the end of the page toward the front while the pointer array grows forward.

This is equivalent to the slotted-page organization studied in Lab 7a: the cell pointer array corresponds to the slot directory, and the unallocated gap corresponds to contiguous free space.

### C3. Deletion Leaves Holes; VACUUM Compacts

Commands:

```bash
cp university.db scratch.db
python3 peek_sqlite.py scratch.db 20

sqlite3 scratch.db "DELETE FROM takes WHERE semester = 'Spring';"
python3 peek_sqlite.py scratch.db 20

sqlite3 scratch.db "VACUUM;"
python3 peek_sqlite.py scratch.db 20
```

**Before deletion:**

```text
page 20 at byte offset 77824
  type                    : leaf table
  first_freeblock         : 0
  num_cells               : 24
  cell_content_start      : 3382
  fragmented_bytes        : 0
  header_len              : 8
  cell_pointer_array_end  : 56
  unallocated_gap         : 3326
```

**After DELETE:**

```text
page 20 at byte offset 77824
  type                    : leaf table
  first_freeblock         : 3595
  num_cells               : 13
  cell_content_start      : 3503
  fragmented_bytes        : 0
  header_len              : 8
  cell_pointer_array_end  : 34
  unallocated_gap         : 3469
```

**After VACUUM:**

```text
page 20 at byte offset 77824
  type                    : leaf table
  first_freeblock         : 0
  num_cells               : 13
  cell_content_start      : 3717
  fragmented_bytes        : 0
  header_len              : 8
  cell_pointer_array_end  : 34
  unallocated_gap         : 3683
```

Deleting rows whose semester is `Spring` removes 11 rows, reducing the cell count from 24 to 13. The first freeblock becomes nonzero because deleted cells leave holes within the cell-content area. The contiguous unallocated gap grows only slightly, from 3326 to 3469 bytes, because deleted cell space is scattered through the page.

After `VACUUM`, SQLite rebuilds and compacts the database. The freeblock pointer returns to zero, and the contiguous gap grows to 3683 bytes. This resembles the `Page.compact` operation in Lab 7a. Dropping a table does not immediately shrink the database file because SQLite can mark its pages reusable on the freelist instead of truncating the file.

### C4. Dropped Tables and the Freelist

Commands:

```bash
sqlite3 scratch.db "DROP TABLE prereq;"
python3 peek_sqlite.py scratch.db | head -4

sqlite3 scratch.db "VACUUM; PRAGMA page_count;"
```

After `DROP TABLE prereq`, the database still contains 21 pages, and two pages are on the freelist. These correspond to the dropped table and its index. SQLite marks these pages as reusable rather than immediately shrinking the file.

After `VACUUM`, SQLite rebuilds the database and removes the unused pages. The page count decreases from 21 to 19, and the freelist is cleared.

### C5. Big Tables and Big Values

The following commands create a table containing 5000 rows and inspect its root page:

```bash
sqlite3 scratch.db "CREATE TABLE big(id INTEGER PRIMARY KEY, name TEXT);
WITH RECURSIVE c(i) AS (
  SELECT 1
  UNION ALL
  SELECT i + 1 FROM c WHERE i < 5000
)
INSERT INTO big
SELECT i, 'student_' || i FROM c;"

sqlite3 scratch.db "SELECT rootpage FROM sqlite_master WHERE name = 'big';"
python3 peek_sqlite.py scratch.db 20
```

The `big` table's root page is 20. It is an interior table page with 24 cells and rightmost child page 45. The 5000 rows cannot fit on one 4096-byte page, so SQLite uses a B-tree with an interior page pointing to leaf pages.

Next, create a table containing a large text value:

```bash
sqlite3 scratch.db "CREATE TABLE doc(id INTEGER PRIMARY KEY, body TEXT);
INSERT INTO doc VALUES (1, hex(randomblob(10000)));"

sqlite3 scratch.db "SELECT pagetype, count(*) FROM dbstat WHERE name = 'doc' GROUP BY pagetype;"
```

The hexadecimal representation of 10,000 random bytes contains 20,000 characters. Since this value cannot fit entirely on a 4096-byte page, SQLite stores part of the payload on the leaf page and the remainder on overflow pages. The observed `dbstat` result is one leaf page and four overflow pages, demonstrating how SQLite handles records with large values.

## Conclusion

This lab demonstrated variable-length record encoding, NULL handling, direct field access using offsets, and the I/O costs of heap, sorted, and hashed file organizations. The SQLite experiments connected these concepts to a real database by examining its file header, catalog, B-tree pages, freeblocks, freelist, compaction, and overflow pages.

The supplied autograder completed successfully with **21 passed tests**. The Zhang record encoded to exactly 28 bytes, and both comparison experiments produced the measured results documented above.

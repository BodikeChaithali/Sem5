# Lab 09 Report: Buffer Pool Manager

## Task 1: LRUReplacer
Uses an `OrderedDict` that holds only unpinned frames. `unpin` adds a frame (or moves it to the end if already present), `pin` removes it, `victim` pops the oldest entry (or returns `None` if empty), and `size` returns the count. All operations are O(1).

## Task 2: fetch_page
On a cache hit, it increments `hits` and `pin_count` and pins the frame in the replacer. On a miss, it increments `misses`, gets a frame from the free list or the replacer, writes the old page to disk if dirty, reads the new page, and sets `pin_count = 1`. It returns `None` if all frames are pinned.

## Task 3: unpin_page
Returns `False` if the page is not buffered or `pin_count <= 0`. The dirty flag is only ever set to True, never overwritten. When `pin_count` reaches 0, the frame is added to the replacer.

## Task 4: flush_page and flush_all
`flush_page` writes the page to disk only if it is dirty, then clears the dirty flag, without unpinning or evicting. `flush_all` calls `flush_page` on every page in the page table.

## Task 5: new_page and delete_page
`new_page` allocates a page on disk, evicts an old page (writing it back if dirty) if needed, and returns `(page_id, frame)` with `pin_count = 1`. `delete_page` returns `False` if the page is pinned; otherwise it removes the page from the page table and replacer, resets the frame, and returns it to the free list.

## How to Run
1. Place `buffer_pool.py`, `test_buffer_pool.py` and `REPORT.md` in the same folder.
2. Install pytest: `pip install pytest`
3. Run: `pytest test_buffer_pool.py -v`

Result: all 12 tests pass.
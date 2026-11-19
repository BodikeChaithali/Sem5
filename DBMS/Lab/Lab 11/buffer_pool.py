"""
CSL303 / MAL505 — Database Management Systems
Lab 09 [BUILD] — Buffer Pool Manager

Implement an in-memory LRU Buffer Pool Manager that coordinates transparent
paging between secondary block storage and memory frames.

You will implement the components marked with # TODO(student):
  1. LRUReplacer:
     - victim()
     - pin(frame_id)
     - unpin(frame_id)
     - size()
  2. BufferPoolManager:
     - fetch_page(page_id)
     - unpin_page(page_id, is_dirty)
     - flush_page(page_id)
     - flush_all()
     - new_page()
     - delete_page(page_id)

Do NOT modify DiskManager, Frame, or the method signatures.
Run the autograder:
    pytest test_buffer_pool.py -v
"""
from __future__ import annotations
import os
from collections import OrderedDict
from typing import Optional, Tuple

PAGE_SIZE = 4096


class DiskManager:
    """
    Manages persistent reading and writing of fixed-size (4096-byte) pages on disk.
    Every page k resides at byte offset k * PAGE_SIZE within the database file.
    """

    def __init__(self, db_filename: str):
        self.db_filename = db_filename
        self.num_reads = 0
        self.num_writes = 0
        if not os.path.exists(db_filename):
            with open(db_filename, "wb") as f:
                pass

    def num_pages(self) -> int:
        """Returns the total number of pages currently stored in the database file."""
        return os.path.getsize(self.db_filename) // PAGE_SIZE

    def read_page(self, page_id: int) -> bytearray:
        """Reads a 4096-byte page from disk at offset page_id * PAGE_SIZE."""
        self.num_reads += 1
        with open(self.db_filename, "rb") as f:
            f.seek(page_id * PAGE_SIZE)
            data = f.read(PAGE_SIZE)
            if len(data) < PAGE_SIZE:
                # Pad newly allocated or unwritten space to full page size
                return bytearray(data + b"\x00" * (PAGE_SIZE - len(data)))
            return bytearray(data)

    def write_page(self, page_id: int, page_data: bytes | bytearray) -> None:
        """Writes a 4096-byte page to disk at offset page_id * PAGE_SIZE."""
        assert len(page_data) == PAGE_SIZE, f"Page data must be exactly {PAGE_SIZE} bytes"
        self.num_writes += 1
        mode = "r+b" if os.path.exists(self.db_filename) and os.path.getsize(self.db_filename) > 0 else "wb"
        with open(self.db_filename, mode) as f:
            f.seek(page_id * PAGE_SIZE)
            f.write(page_data)

    def allocate_page(self) -> int:
        """Appends a new blank page to the end of the file and returns its page_id."""
        page_id = self.num_pages()
        with open(self.db_filename, "ab") as f:
            f.write(b"\x00" * PAGE_SIZE)
        return page_id

    def deallocate_page(self, page_id: int) -> None:
        """Deallocates a page on disk (mocked as a no-op for simple file storage)."""
        pass


class Frame:
    """
    Represents a physical frame in buffer pool RAM holding exactly one 4096-byte page.

    Attributes:
        frame_id:   Index of this frame in the buffer pool array (0 <= frame_id < pool_size).
        data:       The raw memory bytearray(4096) storing page bytes.
        page_id:    The disk page ID currently occupying this frame (-1 if unallocated).
        pin_count:  Number of concurrent threads / active operators currently reading or modifying this page.
                    INVARIANT: A frame with pin_count > 0 CANNOT be evicted!
        is_dirty:   Boolean flag indicating whether the in-memory page has been modified
                    since it was read from disk. If True, it MUST be flushed to disk before eviction.
    """

    def __init__(self, frame_id: int):
        self.frame_id: int = frame_id
        self.data: bytearray = bytearray(PAGE_SIZE)
        self.page_id: int = -1
        self.pin_count: int = 0
        self.is_dirty: bool = False

    def reset(self) -> None:
        """Resets the frame to an unallocated, clean state."""
        self.data = bytearray(PAGE_SIZE)
        self.page_id = -1
        self.pin_count = 0
        self.is_dirty = False


class LRUReplacer:
    """
    Tracks unpinned frames that are candidates for page eviction using the
    Least Recently Used (LRU) policy.

    Only frames with pin_count == 0 are tracked in the replacer.
    When a frame is pinned (pin_count > 0), it is removed from the replacer.
    When a frame is unpinned (pin_count becomes 0), it is added to the replacer.
    """

    def __init__(self, pool_size: int):
        self.pool_size = pool_size
        # Hint: An OrderedDict can store frame_ids in access order, allowing O(1) operations.
        self._cache: OrderedDict[int, bool] = OrderedDict()

    def victim(self) -> Optional[int]:
        """
        Finds the frame that was unpinned least recently, removes it from the replacer,
        and returns its frame_id.

        Returns:
            The victim frame_id, or None if the replacer contains no candidate frames.
        """
        # TODO(student): Implement LRU victim selection
        raise NotImplementedError

    def pin(self, frame_id: int) -> None:
        """
        Removes a frame from the replacer because it is now pinned (in use).
        If frame_id is not in the replacer, this operation has no effect.
        """
        # TODO(student): Implement pin
        raise NotImplementedError

    def unpin(self, frame_id: int) -> None:
        """
        Adds a frame to the replacer because its pin_count reached 0 (eligible for eviction).
        If frame_id is already in the replacer, moves it to the most-recently-unpinned position.
        """
        # TODO(student): Implement unpin
        raise NotImplementedError

    def size(self) -> int:
        """Returns the number of frames currently eligible for eviction."""
        # TODO(student): Implement size
        raise NotImplementedError


class BufferPoolManager:
    """
    Manages the buffer pool frames, page table, pin counts, dirty flags, and
    transparent I/O interaction with the DiskManager.
    """

    def __init__(self, pool_size: int, disk_manager: DiskManager):
        self.pool_size: int = pool_size
        self.disk_manager: DiskManager = disk_manager

        # Frame array: physical memory slots
        self.frames: list[Frame] = [Frame(i) for i in range(pool_size)]

        # Page Table: hash map from page_id to frame_id
        self.page_table: dict[int, int] = {}

        # Replacer: tracks unpinned frames
        self.replacer: LRUReplacer = LRUReplacer(pool_size)

        # Free List: list of frame_ids that have never held a page yet
        self.free_list: list[int] = list(range(pool_size))

        # Performance monitoring metrics
        self.hits: int = 0
        self.misses: int = 0

    @property
    def hit_ratio(self) -> float:
        """Returns the cache hit ratio (hits / total accesses)."""
        total = self.hits + self.misses
        return self.hits / total if total > 0 else 0.0

    def _find_available_frame(self) -> Optional[int]:
        """
        Internal helper: Finds a frame to use.
        1. If free_list has available frames, pop and return one.
        2. Otherwise, request a victim frame from self.replacer.
        Returns None if all frames in the buffer pool are currently pinned.
        """
        if self.free_list:
            return self.free_list.pop(0)
        return self.replacer.victim()

    def fetch_page(self, page_id: int) -> Optional[Frame]:
        """
        Fetches the requested page into a buffer pool frame.

        Logic:
        1. Check if page_id is already in self.page_table (Cache Hit):
           - Increment self.hits
           - Find its frame_id, increment frame.pin_count
           - Pin the frame in self.replacer so it cannot be evicted
           - Return the frame
        2. Cache Miss:
           - Increment self.misses
           - Find an available frame via self._find_available_frame()
           - If no frame is available (all frames pinned), return None!
           - If the chosen frame is currently holding another page (replacement):
             - If the old page is dirty (is_dirty == True):
               Write it to disk via self.disk_manager.write_page()
             - Remove the old page_id from self.page_table
           - Read the new page from disk via self.disk_manager.read_page(page_id)
           - Populate the frame: set page_id, pin_count = 1, is_dirty = False, data
           - Add new page_id -> frame_id to self.page_table
           - Pin the frame in self.replacer
           - Return the frame
        """
        # TODO(student): Implement fetch_page
        raise NotImplementedError

    def unpin_page(self, page_id: int, is_dirty: bool = False) -> bool:
        """
        Unpins a page from the buffer pool when an operator finishes using it.

        Logic:
        1. If page_id is not in self.page_table or frame.pin_count <= 0, return False.
        2. If is_dirty is True, set frame.is_dirty = True (cumulative flag: once dirty,
           it stays dirty until flushed to disk).
        3. Decrement frame.pin_count by 1.
        4. If frame.pin_count drops to 0, notify self.replacer.unpin(frame_id).
        5. Return True.
        """
        # TODO(student): Implement unpin_page
        raise NotImplementedError

    def flush_page(self, page_id: int) -> bool:
        """
        Flushes a specific page to disk if it is currently buffered in memory.

        Logic:
        1. If page_id is not in self.page_table, return False.
        2. If the frame is dirty, write its data to disk via self.disk_manager.write_page(),
           and reset frame.is_dirty to False.
        3. Return True.
        Note: flush_page does NOT evict or unpin the page; it simply synchronizes memory to disk.
        """
        # TODO(student): Implement flush_page
        raise NotImplementedError

    def flush_all(self) -> None:
        """Flushes all currently buffered dirty pages to disk."""
        # TODO(student): Implement flush_all
        raise NotImplementedError

    def new_page(self) -> Optional[Tuple[int, Frame]]:
        """
        Allocates a brand-new page on disk and brings it into a buffer pool frame.

        Logic:
        1. Find an available frame via self._find_available_frame().
           If no frame is available (all pinned), return None.
        2. Allocate a new page on disk via self.disk_manager.allocate_page() -> new_page_id.
        3. If the chosen frame held an existing dirty page, write it to disk and remove from page_table.
        4. Initialize the frame: page_id = new_page_id, pin_count = 1, is_dirty = False,
           data = blank bytearray(PAGE_SIZE).
        5. Update self.page_table and notify self.replacer.pin(frame_id).
        6. Return (new_page_id, frame).
        """
        # TODO(student): Implement new_page
        raise NotImplementedError

    def delete_page(self, page_id: int) -> bool:
        """
        Deallocates a page from the buffer pool and disk.

        Logic:
        1. If page_id is not in self.page_table, it is already not buffered; return True.
        2. If frame.pin_count > 0, the page is actively pinned and CANNOT be deleted; return False.
        3. Remove page_id from self.page_table.
        4. Remove frame from self.replacer via self.replacer.pin(frame_id).
        5. Reset the frame via frame.reset().
        6. Add frame_id back to self.free_list.
        7. Notify self.disk_manager.deallocate_page(page_id).
        8. Return True.
        """
        # TODO(student): Implement delete_page
        raise NotImplementedError

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
        if not self._cache:
            return None
        frame_id, _ = self._cache.popitem(last=False)
        return frame_id

    def pin(self, frame_id: int) -> None:
        """
        Removes a frame from the replacer because it is now pinned (in use).
        If frame_id is not in the replacer, this operation has no effect.
        """
        self._cache.pop(frame_id, None)

    def unpin(self, frame_id: int) -> None:
        """
        Adds a frame to the replacer because its pin_count reached 0 (eligible for eviction).
        If frame_id is already in the replacer, moves it to the most-recently-unpinned position.
        """
        if frame_id in self._cache:
            self._cache.move_to_end(frame_id)
        else:
            self._cache[frame_id] = True

    def size(self) -> int:
        """Returns the number of frames currently eligible for eviction."""
        return len(self._cache)


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

        Cache hit : bump hits, pin_count += 1, pin in replacer, return frame.
        Cache miss: bump misses, find a frame (free list or LRU victim); if none,
                    return None. Write back the old page if dirty, drop it from the
                    page table, read the new page, set pin_count = 1, register it.
        """
        # ---- Path 1: cache hit ----
        if page_id in self.page_table:
            self.hits += 1
            frame_id = self.page_table[page_id]
            frame = self.frames[frame_id]
            frame.pin_count += 1
            self.replacer.pin(frame_id)
            return frame

        # ---- Path 2: cache miss ----
        self.misses += 1
        frame_id = self._find_available_frame()
        if frame_id is None:
            return None  # every frame is pinned

        frame = self.frames[frame_id]

        # Replacement: write back old page if dirty, then forget it
        if frame.page_id != -1:
            if frame.is_dirty:
                self.disk_manager.write_page(frame.page_id, frame.data)
                frame.is_dirty = False
            del self.page_table[frame.page_id]

        # Load the requested page
        frame.data = self.disk_manager.read_page(page_id)
        frame.page_id = page_id
        frame.pin_count = 1
        frame.is_dirty = False

        self.page_table[page_id] = frame_id
        self.replacer.pin(frame_id)
        return frame

    def unpin_page(self, page_id: int, is_dirty: bool = False) -> bool:
        """
        Unpins a page when an operator finishes using it.
        The dirty flag is cumulative: it is only ever set here, never cleared.
        """
        if page_id not in self.page_table:
            return False
        frame_id = self.page_table[page_id]
        frame = self.frames[frame_id]
        if frame.pin_count <= 0:
            return False

        if is_dirty:
            frame.is_dirty = True
        frame.pin_count -= 1
        if frame.pin_count == 0:
            self.replacer.unpin(frame_id)
        return True

    def flush_page(self, page_id: int) -> bool:
        """
        Flushes a specific page to disk if it is buffered. Does NOT evict or unpin.
        """
        if page_id not in self.page_table:
            return False
        frame = self.frames[self.page_table[page_id]]
        if frame.is_dirty:
            self.disk_manager.write_page(page_id, frame.data)
            frame.is_dirty = False
        return True

    def flush_all(self) -> None:
        """Flushes all currently buffered dirty pages to disk."""
        for pid in list(self.page_table.keys()):
            self.flush_page(pid)

    def new_page(self) -> Optional[Tuple[int, Frame]]:
        """
        Allocates a brand-new page on disk and brings it into a buffer pool frame
        with pin_count = 1. Returns None if every frame is pinned.
        """
        frame_id = self._find_available_frame()
        if frame_id is None:
            return None

        new_page_id = self.disk_manager.allocate_page()
        frame = self.frames[frame_id]

        # Evict the old page if the frame was in use
        if frame.page_id != -1:
            if frame.is_dirty:
                self.disk_manager.write_page(frame.page_id, frame.data)
                frame.is_dirty = False
            del self.page_table[frame.page_id]

        frame.page_id = new_page_id
        frame.pin_count = 1
        frame.is_dirty = False
        frame.data = bytearray(PAGE_SIZE)

        self.page_table[new_page_id] = frame_id
        self.replacer.pin(frame_id)
        return new_page_id, frame

    def delete_page(self, page_id: int) -> bool:
        """
        Deallocates a page from the buffer pool and disk.
        Returns True if not buffered, False if pinned, True after successful delete.
        """
        if page_id not in self.page_table:
            return True

        frame_id = self.page_table[page_id]
        frame = self.frames[frame_id]
        if frame.pin_count > 0:
            return False

        del self.page_table[page_id]
        self.replacer.pin(frame_id)  # stop tracking in replacer
        frame.reset()
        self.free_list.append(frame_id)
        self.disk_manager.deallocate_page(page_id)
        return True
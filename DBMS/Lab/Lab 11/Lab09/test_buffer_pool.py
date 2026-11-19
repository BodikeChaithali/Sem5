"""
CSL303 / MAL505 — Database Management Systems
Lab 09 [BUILD] — Buffer Pool Manager Test Suite
Autograder tests for LRUReplacer and BufferPoolManager.
"""
import os
import tempfile
import pytest
from buffer_pool import BufferPoolManager, LRUReplacer, DiskManager, PAGE_SIZE


# ============================================================================
# LRUReplacer Unit Tests
# ============================================================================

def test_replacer_victim_empty():
    """An empty replacer should report size 0 and return None on victim()."""
    rep = LRUReplacer(5)
    assert rep.size() == 0
    assert rep.victim() is None


def test_replacer_lru_order():
    """Frames unpinned in sequence should be victimized in the exact same LRU order."""
    rep = LRUReplacer(5)
    rep.unpin(1)
    rep.unpin(2)
    rep.unpin(3)
    assert rep.size() == 3

    assert rep.victim() == 1
    assert rep.victim() == 2
    assert rep.victim() == 3
    assert rep.size() == 0
    assert rep.victim() is None


def test_replacer_pin_prevents_eviction():
    """Pinning an unpinned frame removes it from eviction eligibility."""
    rep = LRUReplacer(5)
    rep.unpin(10)
    rep.unpin(20)
    rep.unpin(30)

    # Pin frame 20: it must no longer be evictable
    rep.pin(20)
    assert rep.size() == 2

    assert rep.victim() == 10
    assert rep.victim() == 30
    assert rep.victim() is None


def test_replacer_reunpin_updates_lru():
    """Re-unpinning an existing candidate updates its timestamp to the most recent."""
    rep = LRUReplacer(5)
    rep.unpin(1)
    rep.unpin(2)
    # 1 was unpinned before 2, but re-unpinning 1 makes it the most recently unpinned
    rep.unpin(1)

    assert rep.victim() == 2
    assert rep.victim() == 1
    assert rep.victim() is None


# ============================================================================
# BufferPoolManager Unit Tests
# ============================================================================

def test_bpm_fetch_and_unpin():
    """Verify basic page allocation, pin count mechanics, and replacer synchronization."""
    with tempfile.NamedTemporaryFile() as tmp:
        dm = DiskManager(tmp.name)
        bpm = BufferPoolManager(3, dm)

        res = bpm.new_page()
        assert res is not None, "new_page() should return (page_id, frame)"
        pid, frame = res
        assert pid == 0
        assert frame.pin_count == 1
        assert frame.page_id == 0
        assert bpm.replacer.size() == 0, "Pinned page cannot be in replacer"

        # Unpin page successfully
        assert bpm.unpin_page(pid, is_dirty=False) is True
        assert frame.pin_count == 0
        assert bpm.replacer.size() == 1, "Unpinned page must be tracked in replacer"

        # Unpinning when pin_count is already 0 should return False
        assert bpm.unpin_page(pid, is_dirty=False) is False


def test_bpm_cache_hit_no_disk_io():
    """Fetching an already buffered page should hit the cache without physical disk I/O."""
    with tempfile.NamedTemporaryFile() as tmp:
        dm = DiskManager(tmp.name)
        bpm = BufferPoolManager(3, dm)

        pid, frame = bpm.new_page()
        frame.data[0:4] = b"TEST"
        bpm.unpin_page(pid, is_dirty=True)

        reads_before = dm.num_reads
        # Fetch page again while it still resides in the buffer pool
        f_again = bpm.fetch_page(pid)
        assert f_again is not None
        assert f_again.data[0:4] == b"TEST"
        assert dm.num_reads == reads_before, "Cache hit must not trigger physical disk read"
        assert bpm.hits == 1
        assert bpm.misses == 0
        bpm.unpin_page(pid, is_dirty=False)


def test_bpm_all_frames_pinned_returns_none():
    """When all buffer pool frames are pinned, allocation and fetch requests must return None."""
    with tempfile.NamedTemporaryFile() as tmp:
        dm = DiskManager(tmp.name)
        bpm = BufferPoolManager(2, dm)

        # Allocate 2 pages (fills all frames, both pinned)
        p0, f0 = bpm.new_page()
        p1, f1 = bpm.new_page()

        # Both frames are pinned (pin_count = 1). Cannot evict!
        assert bpm.new_page() is None, "new_page must return None when pool is full of pinned pages"

        # Allocate page 2 directly on disk, try to fetch it into memory
        dm.allocate_page()
        assert bpm.fetch_page(2) is None, "fetch_page must return None when all frames are pinned"

        # Unpin p0 -> frame becomes evictable
        bpm.unpin_page(p0, is_dirty=False)
        p2_res = bpm.new_page()
        assert p2_res is not None, "new_page should succeed once an unpinned victim exists"


def test_bpm_dirty_page_written_to_disk_on_eviction():
    """Dirty pages must be persisted to disk upon eviction, preserving mutations."""
    with tempfile.NamedTemporaryFile() as tmp:
        dm = DiskManager(tmp.name)
        bpm = BufferPoolManager(2, dm)

        p0, f0 = bpm.new_page()
        f0.data[:8] = b"DIRTY000"
        bpm.unpin_page(p0, is_dirty=True)

        p1, f1 = bpm.new_page()
        f1.data[:8] = b"CLEAN111"
        bpm.unpin_page(p1, is_dirty=False)

        writes_before = dm.num_writes
        # Allocating p2 forces eviction of LRU page (p0). Because p0 is dirty, it must be flushed!
        p2, f2 = bpm.new_page()
        bpm.unpin_page(p2, is_dirty=False)
        assert dm.num_writes == writes_before + 1, "Dirty victim frame must trigger disk write"

        # Read the raw disk block directly to verify durability
        raw_disk_page = dm.read_page(p0)
        assert raw_disk_page[:8] == b"DIRTY000", "Disk block must contain persisted updates"


def test_bpm_clean_page_not_rewritten():
    """Clean pages should be dropped from memory upon eviction without incurring disk writes."""
    with tempfile.NamedTemporaryFile() as tmp:
        dm = DiskManager(tmp.name)
        bpm = BufferPoolManager(2, dm)

        p0, f0 = bpm.new_page()
        bpm.unpin_page(p0, is_dirty=False)

        p1, f1 = bpm.new_page()
        bpm.unpin_page(p1, is_dirty=False)

        writes_before = dm.num_writes
        # Evicting clean page p0 should incur 0 disk writes
        p2, f2 = bpm.new_page()
        bpm.unpin_page(p2, is_dirty=False)
        assert dm.num_writes == writes_before, "Evicting a clean page must not write to disk"


def test_bpm_flush_page_and_flush_all():
    """Explicit flushes must synchronize dirty pages to disk and clear dirty flags."""
    with tempfile.NamedTemporaryFile() as tmp:
        dm = DiskManager(tmp.name)
        bpm = BufferPoolManager(3, dm)

        p0, f0 = bpm.new_page()
        f0.data[:5] = b"DATA0"
        f0.is_dirty = True

        p1, f1 = bpm.new_page()
        f1.data[:5] = b"DATA1"
        f1.is_dirty = True

        # Flush p0 individually
        assert bpm.flush_page(p0) is True
        assert f0.is_dirty is False, "flush_page must reset is_dirty to False"
        assert dm.read_page(p0)[:5] == b"DATA0"

        # Flush all remaining pages
        bpm.flush_all()
        assert f1.is_dirty is False
        assert dm.read_page(p1)[:5] == b"DATA1"


def test_bpm_delete_page_safety():
    """Pinned pages cannot be deleted; unpinned pages are removed and frame is freed."""
    with tempfile.NamedTemporaryFile() as tmp:
        dm = DiskManager(tmp.name)
        bpm = BufferPoolManager(2, dm)

        p0, f0 = bpm.new_page()
        # Cannot delete while actively pinned
        assert bpm.delete_page(p0) is False, "Cannot delete page with pin_count > 0"

        bpm.unpin_page(p0, is_dirty=False)
        assert bpm.delete_page(p0) is True
        assert p0 not in bpm.page_table, "Deleted page must be removed from page_table"


def test_bpm_workload_hit_ratios():
    """Evaluate hit ratios under temporal locality vs sequential flooding."""
    with tempfile.NamedTemporaryFile() as tmp:
        dm = DiskManager(tmp.name)
        # Allocate 10 pages on disk
        for _ in range(10):
            dm.allocate_page()

        # Scenario A: Working set of 3 pages with buffer pool size 5 (fits entirely in RAM)
        bpm_local = BufferPoolManager(5, dm)
        for _ in range(100):
            for pid in [0, 1, 2]:
                f = bpm_local.fetch_page(pid)
                bpm_local.unpin_page(pid, is_dirty=False)
        # 3 initial misses, followed by 297 consecutive hits -> 297/300 = 99% hit ratio
        assert bpm_local.hit_ratio > 0.95, f"Expected >95% hit ratio, got {bpm_local.hit_ratio:.2%}"

        # Scenario B: Sequential Flooding scan over 10 pages with buffer pool size 3
        bpm_scan = BufferPoolManager(3, dm)
        for _ in range(5):
            for pid in range(10):
                f = bpm_scan.fetch_page(pid)
                bpm_scan.unpin_page(pid, is_dirty=False)
        # Because table size (10) > pool size (3), every cyclic scan misses -> 0% hit ratio!
        assert bpm_scan.hit_ratio == 0.0, f"Expected 0% hit ratio under flooding, got {bpm_scan.hit_ratio:.2%}"

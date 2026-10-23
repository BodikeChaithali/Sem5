"""
Lab 7a autograder. Run:  pytest test_heapfile.py -q
All tests pass only when heapfile.py is fully and correctly implemented.
"""
import os
import tempfile

import heapfile as hf


def newfile():
    fd, path = tempfile.mkstemp(suffix=".heap")
    os.close(fd)
    os.remove(path)          # HeapFile re-creates it
    return path


# ---------- Page-level ----------

def test_page_insert_and_get():
    p = hf.Page()
    s0 = p.insert(b"hello")
    s1 = p.insert(b"world!!")
    assert (s0, s1) == (0, 1)
    assert p.get(s0) == b"hello"
    assert p.get(s1) == b"world!!"
    assert p.get(99) is None          # out of range


def test_page_delete_tombstones_and_stability():
    p = hf.Page()
    s0 = p.insert(b"aaa")
    s1 = p.insert(b"bbb")
    assert p.delete(s0) is True
    assert p.get(s0) is None
    assert p.get(s1) == b"bbb"        # neighbour untouched
    assert s1 == 1                    # slot numbers NOT renumbered
    assert p.delete(s0) is False      # deleting a tombstone -> False
    assert p.delete(999) is False     # out of range -> False


def test_page_fills_up_and_reports_full():
    p = hf.Page()
    rec = b"x" * 100
    n = 0
    while p.insert(rec) is not None:
        n += 1
    assert n >= 30
    assert p.insert(rec) is None      # genuinely full


def test_compact_reclaims_space_and_preserves_records():
    p = hf.Page()
    rec = b"y" * 200
    slots = []
    while True:
        s = p.insert(rec)
        if s is None:
            break
        slots.append(s)
    for s in slots[::2]:              # delete every other record
        p.delete(s)
    before = p.free_space()
    p.compact()
    after = p.free_space()
    assert after > before             # dead space reclaimed
    for s in slots[1::2]:             # survivors still readable at same slots
        assert p.get(s) == rec
    for s in slots[::2]:              # deleted stay deleted
        assert p.get(s) is None
    assert p.insert(rec) is not None  # room exists again


# ---------- HeapFile-level ----------

def test_heapfile_roundtrip_multi_page():
    path = newfile()
    try:
        h = hf.HeapFile(path)
        rids = [h.insert(f"rec{i}".encode()) for i in range(500)]
        for i, rid in enumerate(rids):
            assert h.get(rid) == f"rec{i}".encode()
        assert h.num_pages() >= 2     # must spill across pages
    finally:
        os.remove(path)


def test_heapfile_scan_reflects_deletes():
    path = newfile()
    try:
        h = hf.HeapFile(path)
        rids = [h.insert(f"v{i}".encode()) for i in range(200)]
        for rid in rids[::2]:
            h.delete(rid)
        seen = {rec for _, rec in h.scan()}
        assert b"v1" in seen and b"v0" not in seen
        assert len(seen) == 100
    finally:
        os.remove(path)


def test_persistence_across_reopen():
    path = newfile()
    try:
        h = hf.HeapFile(path)
        rid = h.insert(b"durable")
        del h
        h2 = hf.HeapFile(path)        # reopen the same file
        assert h2.get(rid) == b"durable"
    finally:
        os.remove(path)


def test_rid_addresses_are_stable_after_delete():
    path = newfile()
    try:
        h = hf.HeapFile(path)
        keep = h.insert(b"keep-me")
        drop = h.insert(b"drop-me")
        assert h.delete(drop) is True
        assert h.get(keep) == b"keep-me"   # untouched neighbour still resolves
    finally:
        os.remove(path)

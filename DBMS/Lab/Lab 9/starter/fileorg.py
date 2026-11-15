"""
Lab 7b, Part B [BUILD] — Heap vs sorted vs hashed file organizations (Lecture 21).

A tiny simulated disk stores pages. Every page you touch costs one I/O, and the
Disk counts them, so you can measure the costs from Lecture 21's table:

    Operation        Heap          Sorted              Hash
    point lookup     ~B/2 reads    ~log2(B) reads      ~1 read
    range lookup     B reads       descend + walk      all pages (scatter)
    insert           ~1 + 1        find page + write   ~1 + 1 (+ overflow)

A page is a dict:  {"recs": [(key, payload), ...], "next": page_id or None}
  * "recs" holds at most `cap` records.
  * "next" links a page to an OVERFLOW page (used by sorted and hashed files).

Rules (the tests check them):
  * Only Disk.read / Disk.write count as I/O. Always read a page through
    Disk.read before looking at its records; never peek at disk.pages directly.
  * Python-side metadata that a real system keeps in a header page or catalog
    is allowed WITHOUT I/O: the list of primary page ids (in order), the number
    of buckets, and the hash function.

Run the autograder:   pytest test_fileorg.py -q
Cost table:           python compare.py
"""
from __future__ import annotations
import copy


class Disk:
    """Provided: a page store that counts I/Os."""

    def __init__(self):
        self.pages = []
        self.reads = 0
        self.writes = 0

    def alloc(self) -> int:
        """Allocate an empty page and return its id (allocation itself is free)."""
        self.pages.append({"recs": [], "next": None})
        return len(self.pages) - 1

    def read(self, pid: int) -> dict:
        self.reads += 1
        return copy.deepcopy(self.pages[pid])

    def write(self, pid: int, page: dict) -> None:
        self.writes += 1
        self.pages[pid] = copy.deepcopy(page)

    def reset_counters(self) -> None:
        self.reads = self.writes = 0


# =============================== Heap (provided example) ===============================

class HeapFile:
    """Provided: unordered pages, insert appends to the last page."""

    def __init__(self, disk: Disk, cap: int = 4):
        self.disk, self.cap, self.pids = disk, cap, []

    def load(self, records) -> None:
        for k, v in records:
            self.insert(k, v)

    def insert(self, key, payload) -> None:
        if self.pids:
            last = self.pids[-1]
            page = self.disk.read(last)
            if len(page["recs"]) < self.cap:
                page["recs"].append((key, payload))
                self.disk.write(last, page)
                return
        pid = self.disk.alloc()
        self.disk.write(pid, {"recs": [(key, payload)], "next": None})
        self.pids.append(pid)

    def lookup(self, key):
        for pid in self.pids:                          # scan until found: ~B/2 reads
            for k, v in self.disk.read(pid)["recs"]:
                if k == key:
                    return v
        return None

    def range(self, lo, hi):
        out = []
        for pid in self.pids:                          # must read every page
            out += [(k, v) for k, v in self.disk.read(pid)["recs"] if lo <= k <= hi]
        return sorted(out)

    def num_pages(self) -> int:
        return len(self.pids)


# =============================== Sorted file ===============================

class SortedFile:
    """
    Primary pages hold records sorted by key, and the pages themselves are in
    key order: every key on primary page i is >= the first key of page i and
    < the first key of page i+1. Records that arrive when their page is full go
    to that page's OVERFLOW chain ("next" pointers), unsorted.
    """

    def __init__(self, disk: Disk, cap: int = 4):
        self.disk, self.cap, self.pids = disk, cap, []     # pids: primary pages in key order

    def load(self, records) -> None:
        """Provided: bulk-load sorted records, filling each primary page."""
        recs = sorted(records)
        for i in range(0, len(recs), self.cap):
            pid = self.disk.alloc()
            self.disk.write(pid, {"recs": recs[i:i + self.cap], "next": None})
            self.pids.append(pid)

    def insert(self, key, payload) -> None:
        """Provided: put the record on its primary page, or in its overflow chain."""
        if not self.pids:
            pid = self.disk.alloc()
            self.disk.write(pid, {"recs": [(key, payload)], "next": None})
            self.pids.append(pid)
            return
        pid = self.pids[self._find_page(key)]
        page = self.disk.read(pid)
        while len(page["recs"]) >= self.cap and page["next"] is not None:
            pid = page["next"]
            page = self.disk.read(pid)
        if len(page["recs"]) < self.cap:
            page["recs"].append((key, payload))
            page["recs"].sort()
            self.disk.write(pid, page)
        else:
            new = self.disk.alloc()
            self.disk.write(new, {"recs": [(key, payload)], "next": None})
            page["next"] = new
            self.disk.write(pid, page)

    # ------------------------------ TODO(student) ------------------------------

    def _find_page(self, key) -> int:
        """
        Binary search over the PRIMARY pages (self.pids, in key order). Return the
        index i of the last primary page whose FIRST key is <= key, or 0 if key
        is smaller than every first key. Each probe is one Disk.read.
        Target: about ceil(log2(B)) + 1 reads for B primary pages.
        """
        # TODO(student)
        raise NotImplementedError

    def lookup(self, key):
        """
        Return the payload for key, or None. Use _find_page, read that primary
        page, then follow its overflow chain ("next") if needed.
        """
        # TODO(student)
        raise NotImplementedError

    def range(self, lo, hi):
        """
        Return all (key, payload) with lo <= key <= hi, sorted by key.
        Find the primary page for `lo`, then walk primary pages in order (and
        each one's overflow chain) until a primary page's first key exceeds hi.
        Do NOT read pages that cannot contain keys in [lo, hi].
        """
        # TODO(student)
        raise NotImplementedError

    def num_pages(self) -> int:
        return len(self.pids)


# =============================== Hashed file ===============================

class HashedFile:
    """
    `nbuckets` primary pages; key k lives in bucket h(k) = k % nbuckets, either
    on the bucket's primary page or further down its overflow chain.
    """

    def __init__(self, disk: Disk, nbuckets: int = 8, cap: int = 4):
        self.disk, self.cap, self.nbuckets = disk, cap, nbuckets
        self.pids = []
        for _ in range(nbuckets):                      # provided: allocate the buckets
            pid = disk.alloc()
            disk.write(pid, {"recs": [], "next": None})
            self.pids.append(pid)

    def h(self, key) -> int:
        """Provided: the hash function (bucket number)."""
        return key % self.nbuckets

    def load(self, records) -> None:
        for k, v in records:
            self.insert(k, v)

    # ------------------------------ TODO(student) ------------------------------

    def insert(self, key, payload) -> None:
        """
        Read the bucket's primary page. If it has room, add the record and write
        it back. Otherwise follow "next" to the first page in the chain with
        room; if the whole chain is full, allocate a new overflow page, write
        it, link it from the last page, and write that last page.
        """
        # TODO(student)
        raise NotImplementedError

    def lookup(self, key):
        """Return the payload for key, or None: read the bucket page, then its chain."""
        # TODO(student)
        raise NotImplementedError

    def range(self, lo, hi):
        """
        Return all (key, payload) with lo <= key <= hi, sorted by key.
        Hashing scatters neighbouring keys, so every bucket page and every
        overflow page must be read.
        """
        # TODO(student)
        raise NotImplementedError

    def num_pages(self) -> int:
        """Provided: primary + overflow pages, counted without I/O."""
        n = 0
        for pid in self.pids:
            while pid is not None:
                n += 1
                pid = self.disk.pages[pid]["next"]
        return n

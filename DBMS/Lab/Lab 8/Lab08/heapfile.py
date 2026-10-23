"""
Lab 7a [BUILD] â€” A page-based heap file with slotted pages.

You implement the parts marked  # TODO(student).  Everything else is provided
so you can focus on the slotted-page mechanics from Lecture 20.

Page layout (all little-endian, see the `struct` formats below):

    byte 0                                             byte PAGE_SIZE
    â”Œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¬â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¬â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€  ...  â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”�
    â”‚  header  â”‚  slot dir â†’   â”‚   â†� free space â†’   â”‚ â†� records   â”‚
    â”‚ (4 bytes)â”‚ slot0 slot1 â€¦  â”‚                    â”‚  rec1 rec0  â”‚
    â””â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”´â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”´â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€  ...  â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”˜

  header  = (num_slots: u16, free_end: u16)
            free_end = lowest byte offset currently occupied by record data
            (starts at PAGE_SIZE; records grow downward from the end).
  slot[i] = (offset: u16, length: u16)   a deleted slot is the TOMBSTONE (0, 0)

  A record is addressed by its RID = (page_id, slot_no).  Slot numbers are
  STABLE: deleting a record tombstones its slot but never renumbers the others,
  because indexes elsewhere may hold that RID.

Run the autograder:   pytest test_heapfile.py
Quick manual check:   python heapfile.py
"""
from __future__ import annotations
import os
import struct
from collections import namedtuple

PAGE_SIZE = 4096
HEADER = struct.Struct("<HH")   # num_slots, free_end
SLOT = struct.Struct("<HH")     # offset, length
HEADER_SIZE = HEADER.size       # 4
SLOT_SIZE = SLOT.size           # 4
TOMBSTONE = (0, 0)              # a deleted slot

RID = namedtuple("RID", ["page_id", "slot_no"])


class Page:
    """A single fixed-size slotted page held in memory as a bytearray."""

    def __init__(self, data: bytes | None = None):
        if data is None:
            self.data = bytearray(PAGE_SIZE)
            HEADER.pack_into(self.data, 0, 0, PAGE_SIZE)   # num_slots=0, free_end=PAGE_SIZE
        else:
            assert len(data) == PAGE_SIZE, "a page must be exactly PAGE_SIZE bytes"
            self.data = bytearray(data)

    # ---- header helpers (provided) ----
    @property
    def num_slots(self) -> int:
        return HEADER.unpack_from(self.data, 0)[0]

    @property
    def free_end(self) -> int:
        return HEADER.unpack_from(self.data, 0)[1]

    def _set_header(self, num_slots: int, free_end: int) -> None:
        HEADER.pack_into(self.data, 0, num_slots, free_end)

    def _slot(self, i: int) -> tuple[int, int]:
        return SLOT.unpack_from(self.data, HEADER_SIZE + i * SLOT_SIZE)

    def _set_slot(self, i: int, offset: int, length: int) -> None:
        SLOT.pack_into(self.data, HEADER_SIZE + i * SLOT_SIZE, offset, length)

    def free_space(self) -> int:
        """Contiguous free bytes between the slot directory and the record region."""
        slot_dir_end = HEADER_SIZE + self.num_slots * SLOT_SIZE
        return self.free_end - slot_dir_end

    # ---- record operations ----
    def insert(self, record: bytes) -> int | None:
        if self.free_space() < len(record) + SLOT_SIZE:
            return None

        slot_no = self.num_slots
        new_offset = self.free_end - len(record)

        self.data[new_offset:new_offset + len(record)] = record
        self._set_slot(slot_no, new_offset, len(record))
        self._set_header(slot_no + 1, new_offset)

        return slot_no

    def get(self, slot_no: int) -> bytes | None:
        """Return the record bytes for slot_no, or None if out-of-range/tombstoned."""
        if slot_no < 0 or slot_no >= self.num_slots:
            return None
        offset, length = self._slot(slot_no)
        if (offset, length) == TOMBSTONE:
            return None
        return bytes(self.data[offset:offset + length])

    def delete(self, slot_no: int) -> bool:
        """
        Tombstone slot_no. Return True if a live record was deleted, False if the
        slot was out-of-range or already tombstoned. Do NOT renumber other slots.
        """
        if slot_no < 0 or slot_no >= self.num_slots:
            return False

        if self._slot(slot_no) == TOMBSTONE:
            return False

        self._set_slot(slot_no, *TOMBSTONE)
        return True

    def compact(self) -> None:
        live_records = []

        for slot_no in range(self.num_slots):
            if self._slot(slot_no) != TOMBSTONE:
                live_records.append((slot_no, self.get(slot_no)))

        cur_free_end = PAGE_SIZE

        for slot_no, record in live_records:
            new_offset = cur_free_end - len(record)

            self.data[new_offset:new_offset + len(record)] = record
            self._set_slot(slot_no, new_offset, len(record))

            cur_free_end = new_offset

        self._set_header(self.num_slots, cur_free_end)

    def live_slots(self):
        """Yield the slot numbers of all live (non-tombstoned) records."""
        for i in range(self.num_slots):
            if self._slot(i) != TOMBSTONE:
                yield i


class HeapFile:
    """A file of pages on disk; page k lives at byte offset k * PAGE_SIZE."""

    def __init__(self, path: str):
        self.path = path
        if not os.path.exists(path):
            open(path, "wb").close()

    def num_pages(self) -> int:
        return os.path.getsize(self.path) // PAGE_SIZE

    def read_page(self, page_id: int) -> Page:
        with open(self.path, "rb") as f:
            f.seek(page_id * PAGE_SIZE)
            return Page(f.read(PAGE_SIZE))

    def write_page(self, page_id: int, page: Page) -> None:
        with open(self.path, "r+b") as f:
            f.seek(page_id * PAGE_SIZE)
            f.write(page.data)

    def _append_page(self, page: Page) -> int:
        page_id = self.num_pages()
        with open(self.path, "ab") as f:
            f.write(page.data)
        return page_id

    def insert(self, record: bytes) -> RID:
        """Find the first page with room; else append a new page. (Provided.)"""
        if len(record) + SLOT_SIZE > PAGE_SIZE - HEADER_SIZE:
            raise ValueError("record too large for a page")
        for pid in range(self.num_pages()):
            page = self.read_page(pid)
            slot = page.insert(record)
            if slot is not None:
                self.write_page(pid, page)
                return RID(pid, slot)
        page = Page()
        slot = page.insert(record)
        pid = self._append_page(page)
        return RID(pid, slot)

    def get(self, rid: RID) -> bytes | None:
        if rid.page_id < 0 or rid.page_id >= self.num_pages():
            return None
        return self.read_page(rid.page_id).get(rid.slot_no)

    def delete(self, rid: RID) -> bool:
        if rid.page_id < 0 or rid.page_id >= self.num_pages():
            return False
        page = self.read_page(rid.page_id)
        if page.delete(rid.slot_no):
            self.write_page(rid.page_id, page)
            return True
        return False

    def scan(self):
        for page_id in range(self.num_pages()):
            page = self.read_page(page_id)

            for slot_no in page.live_slots():
                record = page.get(slot_no)
                yield (RID(page_id, slot_no), record)


if __name__ == "__main__":
    # tiny smoke test you can run once your methods work
    import tempfile
    path = os.path.join(tempfile.gettempdir(), "smoke.heap")
    if os.path.exists(path):
        os.remove(path)
    h = HeapFile(path)
    r1 = h.insert(b"hello")
    r2 = h.insert(b"world")
    print("get r1:", h.get(r1))
    print("delete r1:", h.delete(r1))
    print("scan:", list(h.scan()))
    os.remove(path)

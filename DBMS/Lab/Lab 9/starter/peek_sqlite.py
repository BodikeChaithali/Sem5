"""
Lab 7b, Part C [EXPLORE] — look inside a real SQLite database file (Lecture 21).

Usage:
    python peek_sqlite.py university.db            # file header + catalog + root pages
    python peek_sqlite.py university.db 2          # dump the header of page 2
    python peek_sqlite.py university.db 2 --cells  # ... and its cell pointer array

Nothing here needs to be edited. Read the code: it is a slotted-page reader.

SQLite facts used below (https://www.sqlite.org/fileformat2.html):
  * The file is a sequence of pages; page numbers start at 1, so page p lives
    at byte offset (p - 1) * page_size.
  * Bytes 0-99 of page 1 are the database file header. All multi-byte integers
    in the file are BIG-endian (Lab 7a used little-endian).
  * Every table or index is a B-tree. A B-tree page starts with a header
    (8 bytes on leaf pages, 12 on interior pages; on page 1 it starts at
    byte 100), followed by the CELL POINTER ARRAY: one 2-byte offset per cell.
    Cells (the records) are packed from the END of the page towards the front.
"""
import sqlite3
import struct
import sys

PAGE_TYPES = {2: "interior index", 5: "interior table", 10: "leaf index", 13: "leaf table"}


def file_header(path):
    with open(path, "rb") as f:
        h = f.read(100)
    page_size = struct.unpack(">H", h[16:18])[0]
    return {
        "magic": h[:16],
        "page_size": 65536 if page_size == 1 else page_size,
        "page_count": struct.unpack(">I", h[28:32])[0],
        "freelist_pages": struct.unpack(">I", h[36:40])[0],
        "schema_cookie": struct.unpack(">I", h[40:44])[0],
        "text_encoding": {1: "UTF-8", 2: "UTF-16le", 3: "UTF-16be"}.get(struct.unpack(">I", h[56:60])[0], "?"),
    }


def read_page(path, pgno, page_size):
    with open(path, "rb") as f:
        f.seek((pgno - 1) * page_size)            # page id -> byte offset: pure arithmetic
        return f.read(page_size)


def page_header(page, pgno):
    base = 100 if pgno == 1 else 0                # page 1 also carries the file header
    ptype = page[base]
    hdr = {
        "type": PAGE_TYPES.get(ptype, f"not a b-tree page ({ptype})"),
        "first_freeblock": struct.unpack(">H", page[base + 1:base + 3])[0],
        "num_cells": struct.unpack(">H", page[base + 3:base + 5])[0],
        "cell_content_start": struct.unpack(">H", page[base + 5:base + 7])[0] or 65536,
        "fragmented_bytes": page[base + 7],
    }
    hlen = 8
    if ptype in (2, 5):
        hdr["rightmost_child"] = struct.unpack(">I", page[base + 8:base + 12])[0]
        hlen = 12
    hdr["header_len"] = hlen
    ptr_start = base + hlen
    hdr["cell_pointers"] = [struct.unpack(">H", page[ptr_start + 2 * i:ptr_start + 2 * i + 2])[0]
                            for i in range(hdr["num_cells"])]
    hdr["cell_pointer_array_end"] = ptr_start + 2 * hdr["num_cells"]
    hdr["unallocated_gap"] = hdr["cell_content_start"] - hdr["cell_pointer_array_end"]
    return hdr


def catalog(path):
    con = sqlite3.connect(path)
    rows = con.execute("SELECT type, name, tbl_name, rootpage FROM sqlite_master ORDER BY rootpage").fetchall()
    con.close()
    return rows


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    path = sys.argv[1]
    fh = file_header(path)
    if len(sys.argv) == 2:
        print(f"magic          : {fh['magic']!r}")
        for k in ("page_size", "page_count", "freelist_pages", "schema_cookie", "text_encoding"):
            print(f"{k:<15}: {fh[k]}")
        print(f"file size      : {fh['page_size'] * fh['page_count']} bytes (page_size x page_count)\n")
        print(f"{'type':<7}{'name':<34}{'rootpage':>9}  root page type        cells")
        for typ, name, _tbl, root in catalog(path):
            if root:
                h = page_header(read_page(path, root, fh["page_size"]), root)
                print(f"{typ:<7}{name:<34}{root:>9}  {h['type']:<20}{h['num_cells']:>6}")
            else:
                print(f"{typ:<7}{name:<34}{'-':>9}")
        return
    pgno = int(sys.argv[2])
    h = page_header(read_page(path, pgno, fh["page_size"]), pgno)
    print(f"page {pgno} at byte offset {(pgno - 1) * fh['page_size']}")
    for k, v in h.items():
        if k != "cell_pointers":
            print(f"  {k:<24}: {v}")
    if "--cells" in sys.argv:
        print("  cell pointer array (slot -> offset of the cell in this page):")
        for i, off in enumerate(h["cell_pointers"]):
            print(f"    slot {i:>3} -> {off}")


if __name__ == "__main__":
    main()

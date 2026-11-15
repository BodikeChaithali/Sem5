"""
Lab 7b, Part A [BUILD] — Variable-length record encoding (Lecture 20).

A record is turned into bytes with the layout from Lecture 20 ("Record Binary
Format"). All integers are little-endian.

    offset 0        2              2+B             fixed_start        var_start
    ┌──────────────┬──────────────┬───────────────┬──────────────────┬──────────────────┐
    │ num_fields   │ NULL bitmap  │ offset array  │ fixed-length     │ variable-length  │
    │ u16          │ B bytes      │ u16 per       │ fields, schema   │ data, schema     │
    │              │ (1 bit/col)  │ VARCHAR field │ order            │ order            │
    └──────────────┴──────────────┴───────────────┴──────────────────┴──────────────────┘

  * NULL bitmap: B = ceil(num_fields / 8) bytes. Bit i is (byte i // 8, bit i % 8,
    least-significant bit first). Bit = 1 means field i is NULL.
  * Offset array: one u16 per VARCHAR field, in schema order, holding the END
    offset of that field's bytes, measured from the start of the record.
    VARCHAR k starts where VARCHAR k-1 ends (VARCHAR 0 starts at var_start).
  * Fixed fields keep their slot even when NULL (the bytes are zero), so every
    fixed field sits at a constant offset. A NULL VARCHAR stores zero bytes:
    its end offset equals the previous end offset.

Supported types:
    "INT"       4-byte signed integer   (struct "<i")
    "SMALLINT"  2-byte signed integer   (struct "<h")
    "FLOAT"     8-byte IEEE double      (struct "<d")
    ("CHAR", n) exactly n bytes, UTF-8, right-padded with spaces
    "VARCHAR"   variable length, UTF-8

Run the autograder:   pytest test_record.py -q
Quick demo:           python record.py
"""
from __future__ import annotations
import struct
from dataclasses import dataclass

U16 = struct.Struct("<H")
FIXED_FMT = {"INT": struct.Struct("<i"), "SMALLINT": struct.Struct("<h"), "FLOAT": struct.Struct("<d")}

# The University `student` table from the course dataset
STUDENT = [("id", "INT"), ("name", "VARCHAR"), ("dept_name", "VARCHAR"), ("tot_cred", "SMALLINT")]


def is_var(ftype) -> bool:
    return ftype == "VARCHAR"


def fixed_size(ftype) -> int:
    """Byte size of a fixed-length type."""
    if isinstance(ftype, tuple) and ftype[0] == "CHAR":
        return ftype[1]
    if ftype in FIXED_FMT:
        return FIXED_FMT[ftype].size
    raise ValueError(f"not a fixed-length type: {ftype!r}")


@dataclass(frozen=True)
class Layout:
    """Byte positions that depend only on the schema (computed once, provided)."""
    num_fields: int
    bitmap_off: int            # always 2
    bitmap_len: int            # B = ceil(num_fields / 8)
    offsets_off: int           # start of the offset array
    var_slot: dict             # field index -> position k in the offset array
    fixed_pos: dict            # field index -> byte offset of that fixed field
    var_start: int             # first byte of the variable-length data area


def layout(schema) -> Layout:
    """Provided: compute the constant part of the record layout for a schema."""
    n = len(schema)
    bitmap_len = (n + 7) // 8
    offsets_off = 2 + bitmap_len
    var_fields = [i for i, (_, t) in enumerate(schema) if is_var(t)]
    var_slot = {i: k for k, i in enumerate(var_fields)}
    pos = offsets_off + 2 * len(var_fields)
    fixed_pos = {}
    for i, (_, t) in enumerate(schema):
        if not is_var(t):
            fixed_pos[i] = pos
            pos += fixed_size(t)
    return Layout(n, 2, bitmap_len, offsets_off, var_slot, fixed_pos, pos)


def _pack_fixed(ftype, value) -> bytes:
    """Provided: bytes for one non-NULL fixed-length value."""
    if isinstance(ftype, tuple):
        raw = str(value).encode("utf-8")
        if len(raw) > ftype[1]:
            raise ValueError(f"value {value!r} longer than CHAR({ftype[1]})")
        return raw.ljust(ftype[1], b" ")
    return FIXED_FMT[ftype].pack(value)


def _unpack_fixed(ftype, raw: bytes):
    """Provided: inverse of _pack_fixed (CHAR values lose their trailing padding)."""
    if isinstance(ftype, tuple):
        return raw.decode("utf-8").rstrip(" ")
    return FIXED_FMT[ftype].unpack(raw)[0]


def is_null(data: bytes, i: int) -> bool:
    """Provided: True if bit i of the NULL bitmap is set."""
    return bool(data[2 + i // 8] & (1 << (i % 8)))


# ============================== TODO(student) ==============================

def encode(schema, values) -> bytes:
    """
    Serialise one row. `values` is a tuple/list with one entry per field;
    None means NULL. Raise ValueError if the number of values does not match
    the schema.

    Steps:
      1. L = layout(schema). Start a bytearray of length L.var_start (zeros).
      2. Write num_fields (u16) at offset 0.
      3. For each field i:
           - NULL: set bit i of the bitmap. Fixed fields keep their zero bytes;
             a NULL VARCHAR contributes no data.
           - fixed: write _pack_fixed(type, value) at L.fixed_pos[i].
           - VARCHAR: append value.encode("utf-8") to the variable area.
         After each VARCHAR (NULL or not), write its END offset into the offset
         array at L.offsets_off + 2 * L.var_slot[i].
      4. Return bytes(header_and_fixed + variable_area).
    """
    # TODO(student)
    raise NotImplementedError


def get_field(schema, data: bytes, i: int):
    """
    Return the value of field i WITHOUT decoding the other fields (Lecture 20,
    Quick Check 1). Use the NULL bitmap, the field's fixed position, or the
    offset array to jump straight to the bytes:
      - NULL bit set            -> None
      - fixed field             -> _unpack_fixed(type, data[pos : pos + size])
      - VARCHAR k               -> start = var_start if k == 0 else end of VARCHAR k-1
                                   end   = offset_array[k]
    Raise IndexError if i is out of range.
    """
    # TODO(student)
    raise NotImplementedError


def decode(schema, data: bytes) -> tuple:
    """
    Inverse of encode: return a tuple of Python values (None for NULL).
    Raise ValueError if num_fields stored in the record does not match the schema.
    Hint: once get_field works, this is short.
    """
    # TODO(student)
    raise NotImplementedError


# ============================== provided demo ==============================

def hexdump(data: bytes) -> str:
    return "\n".join(f"{o:04x}  " + " ".join(f"{b:02x}" for b in data[o:o + 16]) for o in range(0, len(data), 16))


if __name__ == "__main__":
    row = (128, "Zhang", "Comp. Sci.", 102)
    rec = encode(STUDENT, row)
    print(f"encode{row} -> {len(rec)} bytes")
    print(hexdump(rec))
    print("decode:", decode(STUDENT, rec))
    print("get_field(name):", get_field(STUDENT, rec, 1))
    print("with NULL dept:", decode(STUDENT, encode(STUDENT, (70557, "Snow", None, 0))))

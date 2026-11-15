# Lab 7b — Records, File Organizations & the Catalog (starter)

| File | You… |
|---|---|
| `record.py` | implement `encode`, `get_field`, `decode` (Part A) |
| `fileorg.py` | implement `SortedFile._find_page / lookup / range` and `HashedFile.insert / lookup / range` (Part B) |
| `compare.py` | run it to measure the Lecture 21 cost table |
| `peek_sqlite.py` | run it to look inside a SQLite file (Part C) |
| `test_record.py`, `test_fileorg.py` | do **not** edit (autograder) |

## Run

```bash
sqlite3 university.db < ../data/university.sql   # once, for Part C
pytest -q                                          # target: 21 passed
python record.py                                   # Part A demo
python compare.py                                  # Part B cost table
python peek_sqlite.py university.db                # Part C
```

See `../lab07b.md` for the full instructions and the `REPORT.md` questions.

## Submission checklist

- [ ] `record.py` and `fileorg.py` — all TODO methods implemented
- [ ] `pytest -q` shows `21 passed`
- [ ] `REPORT.md` — Questions 1–5

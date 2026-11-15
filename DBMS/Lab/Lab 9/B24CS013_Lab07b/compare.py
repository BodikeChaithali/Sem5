"""
Lab 7b, Part B — measure Lecture 21's cost table with your fileorg.py.

    python compare.py              # N = 4000 records, 10 records per page
    python compare.py 20000 10     # N, records per page

Prints the average page reads per operation for each organization. Paste the
table into REPORT.md (Question 3).
"""
import random
import sys

import fileorg as fo


def measure(make, keys, probes, ranges, inserts):
    d = fo.Disk()
    f = make(d)
    f.load([(k, "x") for k in keys])
    B = f.num_pages()
    row = {"pages": B}

    d.reset_counters()
    for k in probes:
        f.lookup(k)
    row["lookup"] = d.reads / len(probes)

    d.reset_counters()
    for lo, hi in ranges:
        f.range(lo, hi)
    row["range"] = d.reads / len(ranges)

    d.reset_counters()
    for k in inserts:
        f.insert(k, "new")
    row["insert"] = (d.reads + d.writes) / len(inserts)
    return row


def main():
    n = int(sys.argv[1]) if len(sys.argv) > 1 else 4000
    cap = int(sys.argv[2]) if len(sys.argv) > 2 else 10
    rng = random.Random(303)
    keys = rng.sample(range(10 * n), n)
    probes = rng.sample(keys, 200)
    width = 10 * n // 1000                         # ranges covering ~0.1% of the key space
    ranges = [(lo, lo + width) for lo in rng.sample(range(10 * n - width), 50)]
    inserts = [k for k in rng.sample(range(10 * n, 11 * n), 100)]

    orgs = {
        "Heap": lambda d: fo.HeapFile(d, cap=cap),
        "Sorted": lambda d: fo.SortedFile(d, cap=cap),
        "Hashed": lambda d: fo.HashedFile(d, nbuckets=max(1, int(n / (0.8 * cap))), cap=cap),
    }
    print(f"N = {n} records, {cap} records/page\n")
    print(f"{'Organization':<14}{'pages B':>9}{'lookup reads':>15}{'range reads':>14}{'insert I/Os':>14}")
    for name, make in orgs.items():
        r = measure(make, keys, probes, ranges, inserts)
        print(f"{name:<14}{r['pages']:>9}{r['lookup']:>15.1f}{r['range']:>14.1f}{r['insert']:>14.1f}")


if __name__ == "__main__":
    main()

import random
import bptree as bp


def test_search_on_empty_and_single():
    t = bp.BPlusTree(order=4)
    assert t.search(5) == []
    t.insert(5, "five")
    assert t.search(5) == ["five"]
    assert t.search(6) == []


def test_sequential_insert_and_search():
    t = bp.BPlusTree(order=4)

    for i in range(1, 51):
        t.insert(i, i * 10)

    for i in range(1, 51):
        assert t.search(i) == [i * 10]

    assert t.height() >= 2
    t.check_invariants()


def test_duplicate_keys_preserve_insertion_order():
    t = bp.BPlusTree(order=4)

    for value in ["a", "b", "c", "d"]:
        t.insert(3, value)

    assert t.search(3) == ["a", "b", "c", "d"]
    t.check_invariants()


def test_random_insert_keeps_invariants():
    t = bp.BPlusTree(order=4)
    keys = list(range(200))
    random.Random(303).shuffle(keys)

    ref = {}

    for k in keys:
        t.insert(k, k)
        ref[k] = [k]
        t.check_invariants()

    for k in range(200):
        assert t.search(k) == ref[k]


def test_range_scan_basic():
    t = bp.BPlusTree(order=4)

    for i in range(100):
        t.insert(i, i)

    assert list(t.range(10, 20)) == [
        (i, i) for i in range(10, 21)
    ]

    assert list(t.range(-5, 3)) == [
        (i, i) for i in range(4)
    ]

    assert list(t.range(97, 500)) == [
        (i, i) for i in range(97, 100)
    ]


def test_range_scan_on_shuffled_sparse_keys():
    t = bp.BPlusTree(order=6)
    keys = list(range(0, 300, 3))
    random.Random(7).shuffle(keys)

    for k in keys:
        t.insert(k, str(k))

    got = [k for k, _ in t.range(30, 90)]

    assert got == [
        k for k in range(30, 91) if k % 3 == 0
    ]


def test_large_tree_stays_shallow():
    t = bp.BPlusTree(order=50)

    for i in range(5000):
        t.insert(i, i)

    t.check_invariants()

    assert t.height() <= 4
    assert t.search(4999) == [4999]

    assert list(t.range(4990, 5100)) == [
        (i, i) for i in range(4990, 5000)
    ]


def test_duplicate_key_across_splits():
    t = bp.BPlusTree(order=4)

    for i in range(100):
        t.insert(10, f"value_{i}")
        t.insert(i, f"other_{i}")

    assert t.search(10) == [
        f"value_{i}" for i in range(100)
    ]

    t.check_invariants()

    results = [
        value for key, value in t.range(10, 10)
    ]

    assert results == [
        f"value_{i}" for i in range(100)
    ]


def test_delete_one_duplicate():
    t = bp.BPlusTree(order=4)

    t.insert(5, "a")
    t.insert(5, "b")
    t.insert(5, "c")

    assert t.delete(5, "b") is True
    assert t.search(5) == ["a", "c"]

    assert t.delete(5, "missing") is False
    assert t.search(5) == ["a", "c"]


def test_delete_last_value():
    t = bp.BPlusTree(order=4)

    t.insert(5, "a")

    assert t.delete(5, "a") is True
    assert t.search(5) == []

    assert t.delete(5, "a") is False


def test_delete_from_split_tree():
    t = bp.BPlusTree(order=4)

    for i in range(100):
        t.insert(i, f"v{i}")
        t.insert(i, f"duplicate{i}")

    for i in range(0, 100, 2):
        assert t.delete(i, f"v{i}")

    for i in range(100):
        expected = (
            [f"duplicate{i}"]
            if i % 2 == 0
            else [f"v{i}", f"duplicate{i}"]
        )
        assert t.search(i) == expected

    t.check_invariants()


def test_range_with_duplicate_keys():
    t = bp.BPlusTree(order=4)

    t.insert(1, "a")
    t.insert(1, "b")
    t.insert(2, "c")
    t.insert(2, "d")
    t.insert(3, "e")

    assert list(t.range(1, 2)) == [
        (1, "a"),
        (1, "b"),
        (2, "c"),
        (2, "d")
    ]


def test_delete_keeps_other_keys():
    t = bp.BPlusTree(order=4)

    for i in range(20):
        t.insert(i, i)

    assert t.delete(10, 10)
    assert t.search(10) == []

    for i in range(20):
        if i != 10:
            assert t.search(i) == [i]

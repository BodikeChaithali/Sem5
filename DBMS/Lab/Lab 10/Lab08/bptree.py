"""
Lab 8 [BUILD] — An in-memory B+tree (search, insert with splits, range scan).

Conventions (Lecture 23):
  - `order` = maximum number of CHILDREN per node.
  - a node holds at most `order - 1` keys  (self.max_keys).
  - LEAF nodes hold the data: parallel `keys` / `values`, plus a `next` pointer
    linking leaves left-to-right for range scans.
  - INTERNAL nodes hold `keys` (separators) and `children` (len = #keys + 1).
  - On overflow:
      * a LEAF split COPIES its middle key up (the value stays in the leaf),
      * an INTERNAL split PUSHES its middle key up (removed from this level).
  - The tree grows ONLY at the root.

Run the autograder:   pytest test_bptree.py -q
"""
from __future__ import annotations
from bisect import bisect_left, bisect_right

DEFAULT_ORDER = 4   # a node holds at most order-1 keys


class LeafNode:
    __slots__ = ("keys", "values", "next")

    def __init__(self):
        self.keys = []       # sorted keys
        self.values = []     # values, parallel to keys
        self.next = None     # right sibling leaf (for range scans)
    leaf = True


class InternalNode:
    __slots__ = ("keys", "children")

    def __init__(self):
        self.keys = []       # order-1 separators
        self.children = []   # order children (len == len(keys)+1)
    leaf = False


class BPlusTree:
    def __init__(self, order: int = DEFAULT_ORDER):
        assert order >= 3
        self.order = order
        self.root = LeafNode()

    @property
    def max_keys(self) -> int:
        return self.order - 1

    # ---------- provided: descend to the leaf that would hold `key` ----------
    def _find_leaf(self, key) -> LeafNode:
        node = self.root
        while not node.leaf:
            i = bisect_right(node.keys, key)   # first child whose range covers key
            node = node.children[i]
        return node

    # ============================ implemented methods ============================

    def search(self, key):
        """Return the value stored for `key`, or None if absent."""
        leaf = self._find_leaf(key)
        i = bisect_left(leaf.keys, key)
        if i < len(leaf.keys) and leaf.keys[i] == key:
            return leaf.values[i]
        return None

    def _split_leaf(self, leaf: LeafNode):
        """
        Split an overfull leaf. Upper half moves to a new right leaf, `next`
        links are fixed, and the right leaf's first key is COPIED up.
        """
        right = LeafNode()
        mid = len(leaf.keys) // 2

        right.keys = leaf.keys[mid:]
        right.values = leaf.values[mid:]
        leaf.keys = leaf.keys[:mid]
        leaf.values = leaf.values[:mid]

        right.next = leaf.next
        leaf.next = right

        return right.keys[0], right          # copy up: key stays in right leaf

    def _split_internal(self, node: InternalNode):
        """
        Split an overfull internal node. The middle key is PUSHED up
        (removed from this level) and the children are partitioned.
        """
        right = InternalNode()
        mid = len(node.keys) // 2
        sep = node.keys[mid]

        right.keys = node.keys[mid + 1:]
        node.keys = node.keys[:mid]

        right.children = node.children[mid + 1:]
        node.children = node.children[:mid + 1]

        return sep, right                    # push up: sep is in neither half

    def _insert(self, node, key, value):
        """
        Recursively insert into the subtree rooted at `node`.
        Returns None if no split occurred, else (separator, new_right_node).
        """
        if node.leaf:
            i = bisect_left(node.keys, key)
            if i < len(node.keys) and node.keys[i] == key:
                node.values[i] = value       # update existing key, no duplicate
                return None
            node.keys.insert(i, key)
            node.values.insert(i, value)
            if len(node.keys) > self.max_keys:
                return self._split_leaf(node)
            return None

        # Internal node
        idx = bisect_right(node.keys, key)
        result = self._insert(node.children[idx], key, value)
        if result is None:
            return None

        sep, new_right = result
        node.keys.insert(idx, sep)
        node.children.insert(idx + 1, new_right)

        if len(node.keys) > self.max_keys:
            return self._split_internal(node)
        return None

    def insert(self, key, value):
        """Public insert. Grows the tree only when the root splits."""
        result = self._insert(self.root, key, value)
        if result is not None:
            sep, right = result
            new_root = InternalNode()
            new_root.keys = [sep]
            new_root.children = [self.root, right]
            self.root = new_root

    def range(self, lo, hi):
        """Yield (key, value) pairs with lo <= key <= hi in ascending order."""
        curr = self._find_leaf(lo)
        while curr is not None:
            for k, v in zip(curr.keys, curr.values):
                if k > hi:
                    return
                if k >= lo:
                    yield (k, v)
            curr = curr.next

    # ========================= provided inspection helpers =========================

    def height(self) -> int:
        h, node = 1, self.root
        while not node.leaf:
            h += 1
            node = node.children[0]
        return h

    def leaf_depths(self):
        depths = []

        def walk(node, d):
            if node.leaf:
                depths.append(d)
            else:
                for c in node.children:
                    walk(c, d + 1)
        walk(self.root, 1)
        return depths

    def check_invariants(self) -> bool:
        """Raise AssertionError if any B+tree invariant is violated."""
        depths = self.leaf_depths()
        assert len(set(depths)) == 1, f"unbalanced: leaf depths {set(depths)}"
        min_keys = (self.order + 1) // 2 - 1   # ceil(order/2) - 1

        def walk(node, is_root):
            assert node.keys == sorted(node.keys), "keys not sorted"
            assert len(node.keys) <= self.max_keys, "node overfull"
            if node.leaf:
                if not is_root:
                    assert len(node.keys) >= min_keys, "leaf underfull"
            else:
                assert len(node.children) == len(node.keys) + 1, "bad child count"
                if not is_root:
                    assert len(node.keys) >= min_keys, "internal underfull"
                else:
                    assert len(node.keys) >= 1, "root internal needs a key"
                for c in node.children:
                    walk(c, False)
        walk(self.root, True)

        node = self.root
        while not node.leaf:
            node = node.children[0]
        chain = []
        while node is not None:
            chain.extend(node.keys)
            node = node.next
        assert chain == sorted(chain), "leaf chain not globally sorted"
        return True


if __name__ == "__main__":
    t = BPlusTree(order=4)
    for i in [5, 3, 8, 1, 9, 2, 7, 4, 6, 10]:
        t.insert(i, i * 100)
    print("search 7:", t.search(7))
    print("range 3..8:", list(t.range(3, 8)))
    print("height:", t.height())
    t.check_invariants()
    print("invariants OK")
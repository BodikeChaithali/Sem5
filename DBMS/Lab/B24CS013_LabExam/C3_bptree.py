from __future__ import annotations
from bisect import bisect_left, bisect_right

DEFAULT_ORDER = 4


class LeafNode:
    __slots__ = ("keys", "values", "next")

    def __init__(self):
        self.keys = []
        self.values = []
        self.next = None

    leaf = True


class InternalNode:
    __slots__ = ("keys", "children")

    def __init__(self):
        self.keys = []
        self.children = []

    leaf = False


class BPlusTree:
    def __init__(self, order: int = DEFAULT_ORDER):
        assert order >= 3
        self.order = order
        self.root = LeafNode()

    @property
    def max_keys(self):
        return self.order - 1

    def _find_leaf(self, key):
        node = self.root
        while not node.leaf:
            node = node.children[bisect_right(node.keys, key)]
        return node

    def search(self, key):
        """Return all values for key in insertion order."""
        leaf = self._find_leaf(key)
        i = bisect_left(leaf.keys, key)

        while leaf is not None:
            while i < len(leaf.keys):
                if leaf.keys[i] != key:
                    return []
                return list(leaf.values[i])
            leaf = leaf.next
            i = 0

        return []

    def _split_leaf(self, leaf):
        right = LeafNode()
        mid = len(leaf.keys) // 2

        right.keys = leaf.keys[mid:]
        right.values = leaf.values[mid:]

        leaf.keys = leaf.keys[:mid]
        leaf.values = leaf.values[:mid]

        right.next = leaf.next
        leaf.next = right

        return right.keys[0], right

    def _split_internal(self, node):
        right = InternalNode()
        mid = len(node.keys) // 2
        sep = node.keys[mid]

        right.keys = node.keys[mid + 1:]
        right.children = node.children[mid + 1:]

        node.keys = node.keys[:mid]
        node.children = node.children[:mid + 1]

        return sep, right

    def _insert(self, node, key, value):
        if node.leaf:
            i = bisect_left(node.keys, key)

            if i < len(node.keys) and node.keys[i] == key:
                node.values[i].append(value)
                return None

            node.keys.insert(i, key)
            node.values.insert(i, [value])

            if len(node.keys) > self.max_keys:
                return self._split_leaf(node)

            return None

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
        result = self._insert(self.root, key, value)

        if result is not None:
            sep, right = result
            new_root = InternalNode()
            new_root.keys = [sep]
            new_root.children = [self.root, right]
            self.root = new_root

    def delete(self, key, value):
        """Remove one occurrence of value for key, without rebalancing."""
        leaf = self._find_leaf(key)
        i = bisect_left(leaf.keys, key)

        while leaf is not None:
            while i < len(leaf.keys):
                if leaf.keys[i] != key:
                    return False

                values = leaf.values[i]

                try:
                    j = values.index(value)
                except ValueError:
                    return False

                values.pop(j)

                if not values:
                    leaf.keys.pop(i)
                    leaf.values.pop(i)

                return True

            leaf = leaf.next
            i = 0

        return False

    def range(self, lo, hi):
        """Yield every (key, value) pair in sorted key order."""
        if lo > hi:
            return

        curr = self._find_leaf(lo)

        while curr is not None:
            for k, values in zip(curr.keys, curr.values):
                if k > hi:
                    return

                if k >= lo:
                    for value in values:
                        yield (k, value)

            curr = curr.next

    def height(self):
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

    def check_invariants(self):
        depths = self.leaf_depths()
        assert len(set(depths)) == 1, "unbalanced tree"

        min_keys = (self.order + 1) // 2 - 1

        def walk(node, is_root):
            assert node.keys == sorted(node.keys), "keys not sorted"
            assert len(node.keys) <= self.max_keys, "node overfull"

            if node.leaf:
                assert len(node.keys) == len(node.values)
                for values in node.values:
                    assert isinstance(values, list)
                    assert len(values) > 0

                if not is_root:
                    assert len(node.keys) >= min_keys

            else:
                assert len(node.children) == len(node.keys) + 1

                if not is_root:
                    assert len(node.keys) >= min_keys
                else:
                    assert len(node.keys) >= 1

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

        assert chain == sorted(chain), "leaf chain not sorted"
        return True


if __name__ == "__main__":
    t = BPlusTree(order=4)

    for i in [5, 3, 8, 1, 9, 2, 7, 4, 6, 10]:
        t.insert(i, i * 100)

    t.insert(5, 500)
    t.insert(5, 501)

    print("search 5:", t.search(5))
    print("range 3..8:", list(t.range(3, 8)))

    print("delete:", t.delete(5, 500))
    print("search 5:", t.search(5))

    print("height:", t.height())
    t.check_invariants()
    print("invariants OK")

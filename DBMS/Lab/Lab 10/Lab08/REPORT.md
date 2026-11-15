# Lab 8 Report: B+tree Index

## Question 1: Copy Up vs. Push Up

In a B+tree only the leaves store real records. Internal nodes just store
signposts that tell a search which way to go. This is why the two splits are
different.

When a leaf splits, the separator is the first key of the right leaf, and that
key has a value attached to it. The parent only needs a copy of the key to use
as a signpost, so the real entry must stay in the leaf. When an internal node
splits, its middle key is only a signpost and has no record behind it. It moves
up to the parent and is removed from the level below.

**(a) If a leaf split pushed the separator up (removed it from the leaf):**
the key and its value would be gone from the leaf level. For example, after
splitting [10, 15, 20, 25], the key 20 would only exist as a signpost in the
parent. `search(20)` would return None, a range scan over 10..30 would skip 20,
and the leaf chain would no longer hold every record. The index would silently
lose data.

**(b) If an internal split copied the separator up (kept it in the child):**
the same signpost would appear in two levels. The child would keep a key that
has no extra child pointer to go with it, which breaks the rule
`len(children) == len(keys) + 1`. The tree would also store the same boundary
twice, which wastes space and can send searches into the wrong subtree.

## Question 2: Fanout and Height

**(a)** Let n be the order and h the height, with the leaves at level h.

- Maximum: every internal node has n children and every leaf holds n - 1 keys.
  That gives n^(h-1) leaves, so max keys = n^(h-1) * (n - 1).
- Minimum (for h >= 2): the root has at least 2 children and every other
  internal node has at least ceil(n/2) children. That gives at least
  2 * ceil(n/2)^(h-2) leaves, and each leaf holds at least ceil(n/2) - 1 keys.
  So min keys = 2 * ceil(n/2)^(h-2) * (ceil(n/2) - 1). For h = 1 the minimum is 1.

**(b)** With n = 50:

| Height | Maximum keys | Minimum keys |
|--------|--------------|--------------|
| h = 3  | 50^2 * 49 = 122,500   | 2 * 25 * 24 = 1,200  |
| h = 4  | 50^3 * 49 = 6,125,000 | 2 * 625 * 24 = 30,000 |

Since 5,000 is between 1,200 and 122,500 and below the 30,000 minimum for h = 4,
the tree in the test has height 3. This is within the test's limit of h <= 4.

**(c)** A balanced binary tree with 5,000 keys needs about
ceil(log2(5001)) = 13 levels. If each node sits on its own page, a lookup costs
about 13 random disk reads. The B+tree needs only 3. The root and upper levels
are normally kept in memory, so in practice a lookup often costs about 1 disk
read. The B+tree is much cheaper because each node has many children.

## Question 3: Range Scan Complexity

- **Using `range()`:** one descent visits 3 nodes (root, internal node, leaf).
  1,000 records at 50 per leaf fill about 20 leaves. The first leaf is already
  reached by the descent, so we visit 19 more by following `next` pointers.
  Total is about 3 + 19 = **22 nodes**, or 23 if the range does not line up
  exactly with the leaf boundaries.
- **Using 1,000 separate `search(k)` calls:** each call visits 3 nodes, so the
  total is 1,000 * 3 = **3,000 nodes**.
- **What this means:** the single scan is about 130 times cheaper. It pays for
  the tree descent only once and then reads neighbouring leaves in order, which
  is also friendly to disk reads. This is why B+tree indexes are used for
  BETWEEN conditions and ORDER BY.

## Question 4: In-Memory vs. Disk-Backed

- **What a child pointer is on disk:** it is a page ID (a page number inside
  the database file), not a memory address. A Python object reference only works
  inside one running process, so it cannot be saved to disk.
- **Role of the Buffer Pool Manager:** when the tree wants to go from a parent
  to a child, it asks the buffer pool for that page ID. If the page is already
  in memory, the pool returns it right away. If not, the pool reads it from disk
  into a free frame. If no frame is free, it first evicts a page that nobody is
  using.
- **Pinning and unpinning:** the tree pins a page while it is reading or
  changing it, so the pool cannot evict it at that moment. After the tree has
  fetched and pinned the child, it unpins the parent. If it changed the parent,
  it marks it dirty so it gets written back to disk. During a split, every page
  involved stays pinned until the change is finished. Pages that are unpinned
  can be evicted when the pool needs space.

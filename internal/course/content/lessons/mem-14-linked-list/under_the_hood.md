**Linked lists are slow on real hardware.** Each node is a separate heap
allocation, so walking the list means jumping around memory, and every
jump is a likely cache miss. A `std::vector` walks memory in order, and
the CPU prefetches the next cache line before you ask for it. For most
workloads a vector beats a list even with inserts in the middle. Bjarne
Stroustrup's talk on this is famous, and you'll measure it yourself in
phase 5.

**When lists win.** Lists are good when you hold pointers to nodes and
splice them in and out in O(1) without moving anything else: LRU
caches, free lists in allocators (phase 7), the kernel's run queues.
Those are usually *intrusive* lists, where the `next` pointer lives
inside the object rather than in a separate node.

**`std::forward_list` and `std::list`.** The standard library's singly
and doubly linked lists. `std::list::splice` moves nodes between lists
without allocating, and that's the main reason to pick it.

**Why `Node{v, std::move(head_)}` works.** `Node` is an *aggregate*, a
struct with no constructors, so brace initialisation fills its fields
in order. The `unique_ptr` field can only be *moved* in, hence the
`std::move`.

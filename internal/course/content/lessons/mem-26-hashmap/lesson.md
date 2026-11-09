---
title: "HashMap: open addressing"
minutes: 45
files: [hashmap.hpp]
run:
  args: ["the", "fox", "cat"]
  stdin: "the quick brown fox jumps over the lazy dog\nthe end\n"
deeper:
  - title: Open addressing (Wikipedia)
    url: https://en.wikipedia.org/wiki/Open_addressing
  - title: "Designing a fast hash table (Malte Skarupke)"
    url: https://probablydance.com/2017/02/26/i-wrote-the-fastest-hashtable/
---
A hash table turns a key into an array index: `hash(key) % capacity`.
Two keys can land on the same index, a **collision**, and the table
needs a plan for that.

`std::unordered_map` chains: each slot holds a linked list of the
entries that landed there. That's a heap node per entry and a pointer
chase per lookup. **Open addressing** keeps every entry in one flat
array instead. On a collision, it **probes** onward to the next slot
until it finds the key or an empty slot:

```
capacity 8, hash("fox") % 8 == 5, hash("cat") % 8 == 5
slot:  0    1    2    3    4    5     6     7
            ·    ·    ·    ·  [fox] [cat]   ·      cat probed on to 6
```

That's **linear probing**, the simplest scheme and very cache-friendly:
neighbours are next to each other in memory.

Two rules keep it fast and correct:

- **The capacity is a power of two**, so `% capacity` becomes
  `& (capacity - 1)`, a single AND instruction. Stepping on wraps the
  same way: `i = (i + 1) & mask`.
- **The table never fills up.** Probing only stops at the key or at an
  empty slot. `insert` grows the table (doubling it and re-inserting
  everything) before the load goes past 3/4.

Growing is written for you. This lesson is about finding slots.
`main.cpp` counts the words on standard input and looks up the ones you
name:

```
$ echo "the quick brown fox jumps over the lazy dog" | ./hashmap the fox cat
the: 2
fox: 1
cat: 0
8 different words, capacity 16
```

## Assignment

- [ ] `slot_for` probes from the key's hash to its slot, or the empty slot where it would go
- [ ] `find` (both versions) return the value or `nullptr`
- [ ] `insert` adds a key once, growing first when the load would pass 3/4
- [ ] `operator[]` returns the value, inserting `V()` when the key is missing

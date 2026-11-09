---
title: "HashMap: deletion and rehashing"
minutes: 35
files: [hashmap.hpp]
run:
  stdin: "set apple 3\nset pear 5\nset fig 1\nget apple\ndel apple\nget apple\ndel kiwi\nset fig 2\nlist\n"
deeper:
  - title: Lazy deletion (Wikipedia)
    url: https://en.wikipedia.org/wiki/Lazy_deletion
---
Deleting from an open-addressing table looks easy: find the key and
mark its slot empty. But a lookup stops at the first empty slot, so an
empty slot in the *middle* of a run cuts off everything after it:

```
"cat" probed past "fox" to reach slot 6.
slot:   5      6
      [fox]  [cat]      erase fox → slot 5 empty
        ·    [cat]      find cat: hash says 5, it's empty, stop. "Not found"!
```

The fix is a **tombstone**: a third slot state, `Deleted`. Lookups
step over it as if it were full, and inserts may reuse it.

Tombstones have a cost. They don't hold entries, but probes still walk
over them. A table that sees lots of inserts and erases fills up with
tombstones until every lookup is slow, or, with no `Empty` slots left,
never ends at all. So the load check counts **used** slots (full plus
deleted), and when it trips, the table **rehashes**: it rebuilds from
the live entries only. It doubles the capacity if the live entries need
it, and otherwise rebuilds at the same size, just to clear the
tombstones out.

Also new: `for_each(f)` calls `f(key, value)` for every entry, so you
can list the table. `main.cpp` is a tiny key-value shell:

```
$ ./hashmap
set apple 3
get apple       →  apple = 3
del apple       →  deleted apple
get apple       →  apple is not set
list            →  every entry, sorted
```

## Assignment

- [ ] `slot_for` steps over tombstones
- [ ] `erase` leaves a tombstone, and frees the key and value it held
- [ ] `insert` checks the key isn't already there, then reuses the first free or deleted slot
- [ ] `rehash` rebuilds without tombstones, and `insert` calls it when used slots pass 3/4
- [ ] `for_each` visits every live entry once

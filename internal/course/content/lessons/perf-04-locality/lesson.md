---
title: "Locality: rows vs columns"
minutes: 40
files: [locality.hpp]
program: locality
optimize: true
run: {}
deeper:
  - title: "CS:APP 6.5-6.6: writing cache-friendly code, and rearranging loops"
    url: https://csapp.cs.cmu.edu/
  - title: "CS:APP Cache Lab: the transpose part"
    url: http://csapp.cs.cmu.edu/3e/cachelab.pdf
---
C++ stores a matrix row after row. Element `(i, j)` of an `n`-column
matrix is at index `i * n + j`, so `(i, j + 1)` is the next 8 bytes
and `(i + 1, j)` is a whole row away. Two loops that do the same work:

```cpp
for (i) for (j) sum += m(i, j);   // walks along rows: 1 miss per 8 doubles
for (j) for (i) sum += m(i, j);   // walks down columns: 1 miss per double
```

The second one only gets any reuse if a whole column's worth of
lines stays in the cache until it comes back for the next column.
For a big matrix, it won't.

**Transpose** has the problem built in: it reads `a` along rows and
writes `out` down columns, or the other way round. Swapping the loops
just moves the bad walk to the other matrix. The fix is **blocking**
(or tiling): transpose one small `block × block` tile at a time. An
8 × 8 tile of doubles is 8 lines of `a` and 8 lines of `out`, which
stay in the cache while the tile is done, so every line is loaded
once.

```text
for each tile (ii, jj), stepping by block:
    for i in ii .. ii + block (but not past the edge)
        for j in jj .. jj + block (but not past the edge)
            out(j, i) = a(i, j)
```

**Multiply** usually needs no blocking, just the right loop order.
The textbook `ijk` order computes `c(i, j)` as a dot product of row
`i` of `a` with column `j` of `b`, so its inner loop walks down a
column. Move `j` innermost and every inner loop runs along rows:

```text
for i
    for k
        x = a(i, k)
        for j:  c(i, j) += x * b(k, j)
```

Same arithmetic, same answer, different order of memory accesses.
`c` has to start at zero for that to work.

The functions are templates. Run passes a plain `Matrix`. The tests
pass one that sends every element you touch through the cache
simulator from lesson 5.1.2, so they can count your misses exactly.
Don't create matrices inside your functions.

## Assignment

- [ ] `transpose_blocked` transposes one `block × block` tile at a time, including the ragged tiles at the edges
- [ ] `multiply_ikj` zeroes `c`, then multiplies with `j` as the innermost loop
- [ ] Run it. Why is 1024 × 1024 so much slower than 1000 × 1000 for the textbook loops?

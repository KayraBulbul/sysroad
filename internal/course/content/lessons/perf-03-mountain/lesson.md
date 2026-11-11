---
title: Build the memory mountain
minutes: 45
files: [mountain.cpp]
program: mountain
optimize: true
run: {}
deeper:
  - title: "CS:APP 6.6.1: The memory mountain"
    url: https://csapp.cs.cmu.edu/
  - title: "Gallery of Processor Cache Effects (Igor Ostrovsky)"
    url: https://igoro.com/archive/gallery-of-processor-cache-effects/
---
The simulator counted misses. Now measure what they cost on your own
machine. The **memory mountain**, from CS:APP, is one picture of your
whole memory hierarchy: read throughput for every combination of

- **working-set size**: how much memory the loop cycles through, from
  16 KB (fits in L1) to 64 MB (bigger than any cache), and
- **stride**: read every element, every 2nd, every 3rd, up to every
  12th. With 8-byte `long`s, a stride of 8 or more uses one value per
  64-byte line.

Small sizes and small strides make the summit. As the size grows, the
mountain drops off a cliff at each cache's capacity: those are the
**ridges** of temporal locality. As the stride grows, each line gives
you fewer values before the next miss: that's the **slope** of spatial
locality.

The loop at the heart of it is `read_stride`. Write it with four
running sums, not one:

```cpp
for (; i + 3 * stride < elems; i += 4 * stride) {
    a += data[i];
    b += data[i + stride];
    c += data[i + 2 * stride];
    d += data[i + 3 * stride];
}
// then one sum for the leftovers
```

With a single `sum`, each add waits for the one before it, and at one
add per clock cycle the adds become the limit instead of the memory.
Four independent sums let the CPU work on four at once. Lesson 5.2.3
covers why.

`build_mountain` measures each point. It doesn't call `measure` itself:
it takes a `Timer`, so `main.cpp` passes your harness from lesson 5.1.1
and the tests pass a fake. Wrap the read in `do_not_optimize`, or
`-O2` deletes the loop whose result you threw away. Divide the bytes
read by the nanoseconds, and you have bytes per nanosecond, which is
the same as GB/s.

Run prints the table for up to 64 MB, which takes a second or two. To
climb higher, build it yourself in the lesson folder:

```sh
clang++ -std=c++20 -O2 *.cpp -o mountain
./mountain 512           # up to 512 MB
./mountain 512 csv > mountain.csv
```

The CSV opens in any spreadsheet, which can draw it as a 3D surface.

## Assignment

- [ ] `read_stride` sums every `stride`th element below `elems`, with four running sums
- [ ] `bytes_read` counts the elements `read_stride` visits, in bytes
- [ ] `human_size` turns 16384 into `16K` and 1048576 into `1M`
- [ ] `build_mountain` times every size and stride and fills in `gbps`
- [ ] Run it, and find where your L1, L2 and L3 end

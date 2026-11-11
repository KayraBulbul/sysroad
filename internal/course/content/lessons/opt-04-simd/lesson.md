---
title: Auto-vectorisation and SIMD
minutes: 40
files: [simd.cpp]
program: simd
optimize: true
run: {}
deeper:
  - title: "Clang: auto-vectorisation in LLVM"
    url: https://llvm.org/docs/Vectorizers.html
  - title: "Clang language extensions: vectors"
    url: https://clang.llvm.org/docs/LanguageExtensions.html#vectors-and-extended-vectors
  - title: "Daniel Lemire's blog: SIMD in practice"
    url: https://lemire.me/blog/
---
A SIMD register holds several values side by side: 16 bytes (SSE on
x86-64, NEON on arm64), 32 with AVX2, 64 with AVX-512. One instruction
adds, compares or ANDs all of them at once. A loop that processes one
byte per instruction can process 16.

**Auto-vectorisation** is the compiler doing that for you. You've
already seen it in this phase: `ikj` multiply, struct-of-arrays and
`invert` all turned into SIMD at `-O2`. Ask clang to report what it
did:

```sh
clang++ -std=c++20 -O2 -c simd.cpp \
  -Rpass=loop-vectorize -Rpass-missed=loop-vectorize -Rpass-analysis=loop-vectorize
```

```text
simd.cpp:5:5: remark: the cost-model indicates that vectorization is not beneficial
```

That's the `is_ascii` in your starter code. A loop that can `return`
in the middle needs extra work to vectorise (which lane stopped it?),
and clang decides it isn't worth it. But you don't need to stop
early. OR every byte together, and look at the top bit once at the
end. The loop becomes straight-line work on every element, which
vectorises well. Checking a whole 1 MB buffer this way is faster than
checking half of it a byte at a time.

**When the compiler won't**, write the SIMD yourself. Intrinsics
(`_mm_cmpeq_epi8` on x86, `vceqq_u8` on arm64) are one way, but you'd
write everything twice. Clang and GCC also understand **vector
types**, which compile to SSE or NEON as needed:

```cpp
using u8x16 = std::uint8_t __attribute__((vector_size(16)));

u8x16 v = load16(s + i);         // 16 bytes
u8x16 m = v == splat16('\n');    // 0xff in each lane that matched, 0 elsewhere
```

`count_byte` is the core of `wc -l`. The plain loop does vectorise,
but badly: it adds into a 64-bit count, so clang widens every byte to
64 bits first and handles 2 per instruction. Keep the counts in
**byte lanes** instead. A match is `0xff`, which is -1 as a byte, so
subtracting the comparison adds 1 to each lane that matched:

```cpp
u8x16 counts{};
counts -= v == splat16(c);   // the result needs a cast to u8x16
```

A byte overflows at 256, so after at most 255 blocks, add the 16
lanes into a `std::size_t` total and start the lanes again from 0.
Handle the last `n % 16` bytes one at a time.

## Assignment

- [ ] `is_ascii` has no early exit, and clang reports it as vectorised
- [ ] `count_byte` compares 16 bytes at a time and counts in byte lanes
- [ ] It empties the lane counters at least every 255 blocks, and handles the leftover bytes
- [ ] Run it. How many bytes per nanosecond does `count_byte` get through?

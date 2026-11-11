**What your `count_byte` compiles to.** On x86-64 the inner loop is
`movdqu` (load 16 bytes), `pcmpeqb` (compare), `psubb` (subtract) and
the loop counter: about 2 cycles per 16 bytes. On arm64 it's `ldr q`,
`cmeq` and `sub`. The 255-block flush runs once per 4 KB and costs
nothing that matters.

**Wider vectors.** With `-march=native` on a PC with AVX2, clang
splits each 16-lane operation into halves of a 32-lane one, or you
can declare `vector_size(32)` yourself. AVX-512 has 64-byte
registers and *mask registers*, which make tails and comparisons
cheaper. Apple's NEON is fixed at 16 bytes but the M-series cores run
four NEON instructions per cycle, which makes up for it.

**Horizontal sums are slow.** Adding the 16 lanes of a vector
together needs a few shuffles. That's why you keep counting in lanes
and only add them up every 255 blocks. `psadbw` (sum of absolute
differences against zero) on x86, or `uaddlv` on arm64, adds them in
one instruction, and clang uses them when it recognises the pattern.

**The real `wc`.** GNU `wc -l` uses `memchr`-style SIMD to find
newlines, then on AVX2 machines a dedicated loop much like yours.
glibc's `memchr`, `strlen` and `memcmp` are all hand-written SIMD per
CPU generation, picked at load time (lesson 5.3.4 shows how).

**Beyond counting.** SIMD shines whenever the same operation applies
to many bytes: JSON parsing (simdjson parses over 2 GB/s), UTF-8
validation, base64, hashing, image filters, compression. The tricks
are the ones you just used: compare to get masks, combine masks with
AND and OR, count with subtraction, and avoid branches inside the
loop.

**`std::simd`.** C++26 adds `std::simd<T>` to the standard library, a
portable version of what vector types give you, with the same
operators. Libraries like Google's Highway do the same today and can
pick the best instruction set at run time.

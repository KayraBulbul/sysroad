---
title: "Make your wc as fast as GNU's"
minutes: 60
files: [count.cpp]
program: wc
optimize: true
run:
  args: ["--race", "64"]
deeper:
  - title: "GNU coreutils wc.c and wc_avx2.c (source)"
    url: https://github.com/coreutils/coreutils/blob/master/src/wc.c
  - title: "Beating C with 80 lines of Haskell: wc (and the replies it started)"
    url: https://chrispenner.ca/posts/wc
  - title: "Counting newlines really fast (Daniel Lemire)"
    url: https://lemire.me/blog/2017/02/14/how-fast-can-you-count-lines/
---
Your wc from phase 1 is correct. On a 64 MB file it takes about 90 ms,
under 1 GB/s. GNU's `wc -l` takes about 10 ms. Its full count, with
words, takes 140 ms, because it decodes UTF-8 to find Unicode spaces.
Time to beat both, using this chapter.

The course's wc is here with its counting moved into `count.cpp`:

- `count_chunk(st, p, n)` adds the lines, words and bytes of one chunk
  of input to `st`. A word can straddle two chunks, so `st.in_word`
  remembers whether the last byte was inside one.
- `count_fd(fd, out)` reads a file descriptor to the end with
  `read(2)` and feeds every chunk to `count_chunk`.

Both are written one byte at a time. Make them fast:

1. **Read in big chunks.** Each `read` is a system call, about a
   microsecond of fixed cost. 4 KB per call is 16,000 calls for 64 MB.
   256 KB is 256.
2. **Count lines with SIMD**, exactly like `count_byte` in lesson
   5.2.4.
3. **Count words with SIMD too.** A word starts at a byte that isn't
   whitespace and follows one that is. For 16 bytes at once:

```cpp
u8x16 v      = load16(p + i);       // bytes i to i + 15
u8x16 before = load16(p + i - 1);   // bytes i - 1 to i + 14: one to the left
starts -= ~spaces(v) & spaces(before);
```

where `spaces(v)` is `0xff` in each whitespace lane: equal to `' '`,
or between `'\t'` (9) and `'\r'` (13). The overlapping load is the
trick: it lines up every byte with the one before it, with no
shuffling. It needs a byte before `i`, so handle the first byte of
the chunk on its own, using `st.in_word`, and start the SIMD loop at
1. Finish with the leftover bytes one at a time, and set `st.in_word`
from the chunk's last byte.

Run writes 64 MB of text to a temporary file and races your
`count_fd` against your system's `wc` and `wc -l`.

## Assignment

- [ ] `count_fd` reads at least 64 KB per `read`, and retries when `read` is interrupted (`EINTR`)
- [ ] `count_chunk` counts lines and word starts 16 bytes at a time, emptying its lane counters every 255 blocks
- [ ] Chunks of any size give the same totals as one big chunk
- [ ] Run it. Do you beat `wc -l`?

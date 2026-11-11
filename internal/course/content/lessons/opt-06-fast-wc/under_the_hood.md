**Where the time goes now.** At 8 GB/s, counting isn't the limit any
more: copying the file from the kernel's page cache into your buffer
is. Each `read` is a `memcpy` of 256 KB inside the kernel. `mmap`ing
the file instead skips that copy, but sets up page tables for every
page, and for one sequential pass it's often no faster. GNU wc uses
`read`, and so does `cat`.

**Why GNU's full count is slow.** In a UTF-8 locale, GNU wc decodes
each multibyte character to check whether it's one of Unicode's
spaces (there are about 25, like the no-break space U+00A0 and the
ideographic space U+3000). That turns a byte loop into a decoding
loop. Run `LC_ALL=C wc` and it only uses ASCII whitespace, like
yours, and gets faster, but still counts a byte at a time. `wc -l`
skips all that and only looks for `'\n'`, with AVX2 where the CPU has
it.

**Your answers can differ from GNU's.** Text with a no-break space,
or any other Unicode space, has fewer words by your count than by
GNU's in a UTF-8 locale. Matching it exactly needs a UTF-8 decoder
that can tell where a multibyte space starts, which SIMD can do too:
simdutf validates and transcodes UTF-8 at many gigabytes per second.

**Bigger vectors.** With AVX2 the same loop takes 32 bytes per step,
and the bottleneck moves even more firmly to memory. On the Mac, NEON
stays at 16, but the M-series cores run four of these instructions
per cycle, so your loop runs about as fast as on the PC.

**The same tricks elsewhere.** "Find the bytes that differ from their
neighbour" is how SIMD CSV and JSON parsers find field boundaries,
and the overlapping load is common in hand-tuned string code
(`memchr`, `strlen` and friends handle their unaligned starts and
ends with it). Comparing against a set of characters is the core of
`strpbrk`, of tokenisers and of Hyperscan's regular expressions.

**A whole program, measured.** This is the order to do it in for any
program: measure first (`perf` showed you `count_chunk`), fix the
algorithm and the I/O pattern, then the loop, and only then reach for
SIMD. Each step was worth about 10×, and the order matters: SIMD on
4 KB reads would have saved much less.

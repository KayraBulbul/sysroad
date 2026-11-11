**What LTO does to `brighten`.** Without it, the loop in `main.cpp` is
a call per pixel: save registers, jump, compare twice, return. With
it, the linker sees the body, inlines it, notices the loop applies the
same clamp to consecutive bytes, and vectorises it with saturating
adds (`paddusb` on x86, `uqadd` on arm64) that brighten 16 or 32
pixels per instruction. On an i9-14900K that's 37 ms down to 3.4 ms
for 64 MB, the same speed-up you'd get by moving `brighten` into the
header as an `inline` function.

**How the object files change.** With `-flto=thin`, `pixel.o` isn't
machine code any more: it's LLVM bitcode (run `file` on it). The
linker hands all the bitcode to LLVM, which optimises across files and
only then generates code. *Thin* LTO keeps a summary of each file and
optimises the files in parallel using the summaries, so it scales to
huge programs. Full LTO (`-flto`) merges everything into one module,
which is slower to build and occasionally a little faster. A static
library of bitcode needs an archiver that understands it: `llvm-ar` on
Linux, which CMake picks for you, and Apple's `ar` on the Mac.

**Profile-guided optimisation.** PGO goes one step further: build with
`-fprofile-generate`, run the program on typical input, then rebuild
with `-fprofile-use`. The compiler then knows which branches are
taken and which functions are hot, and lays out and inlines code
accordingly. Chrome, Firefox and clang itself ship that way, for 10%
or more.

**Why `-O3` isn't the default everywhere.** It's not reliably faster.
More inlining and unrolling means more code, and a hot loop that no
longer fits in the 32 KB instruction cache, or the smaller micro-op
cache, can run slower. Most Linux distributions build packages with
`-O2`. CMake's `Release` uses `-O3` for historical reasons.

**Frame pointers cost about 1%.** `-fno-omit-frame-pointer` keeps
`rbp` (or `x29`) as a link to the caller's frame instead of a spare
register. That's what lets `perf` walk the stack cheaply. Apple's
arm64 ABI requires frame pointers anyway, so on the Mac the flag
changes nothing.

**Seeing what the optimiser decided.** `-Rpass=inline` and
`-Rpass=loop-vectorize` make clang print a remark for each inlining
and each vectorised loop, and `-Rpass-missed=...` for each one it
gave up on, often with the reason. Lesson 5.2.4 uses them.

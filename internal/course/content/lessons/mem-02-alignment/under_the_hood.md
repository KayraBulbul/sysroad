**What misalignment costs.** On x86-64, an ordinary misaligned load
works and is usually just as fast, unless it crosses a cache line (64
bytes). Then the CPU does two loads. Some SIMD instructions require
alignment and fault without it. Older ARM chips trapped. In C++ it's
undefined behaviour either way. UBSan flags it as `misaligned address`,
and you'll see that in 2.3.6.

**`alignas`.** You can ask for more alignment:
`struct alignas(64) Counter { std::atomic<long> n; };` puts each
`Counter` on its own cache line. That's the fix for *false sharing*,
where two threads update neighbouring variables and fight over one
cache line. It's in phase 8.

**`#pragma pack(1)`** removes padding. File formats and network headers
sometimes use it to match bytes on disk exactly, but then every
misaligned field read is slow or unsafe. Reading the fields one at a
time from a byte buffer, as the next lessons do, is the portable way.

**Check your answer with the compiler.** `offsetof(S, n)` from
`<cstddef>` gives a field's offset, and clang can print a whole layout:
`clang++ -Xclang -fdump-record-layouts -fsyntax-only file.cpp`.

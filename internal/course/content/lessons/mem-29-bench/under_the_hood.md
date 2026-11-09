**Why the linked list loses so badly.** Summing a `std::vector<int>`
reads memory in order, so the CPU's prefetcher fetches the next cache
line before you need it and the loop can be vectorised to add 8 ints at
a time. A `std::list` makes every step a pointer chase to wherever that
node was allocated. It's the same O(n), and often 10× or more slower.
Big-O counts operations. Real hardware charges for memory access
patterns.

**Why `String` can lose on short strings.** `std::string`'s members are
templates defined in a header, so the compiler sees their code at the
call site and inlines it: constructing a short string in a loop becomes
a few stores. `String`'s members live in `string.cpp`, a separate
*translation unit*, so every call is a real function call that the
optimiser can't see into. Build with `-flto` (link-time optimisation)
and the gap narrows a lot. Phase 5 covers inlining and LTO properly.

**Your HashMap vs `std::unordered_map`.** Open addressing keeps
everything in one array, while `unordered_map` allocates a node per
entry and follows a pointer per lookup. Flat tables usually win on
lookups, sometimes by 2–3×. Your results depend on the key pattern and
on how well `std::hash<int>` spreads keys for your mask (it doesn't).

**What `do_not_optimize` does.** `asm volatile("" : : "r,m"(value) : "memory")`
is an empty block of assembly that claims to read `value` and touch
all memory. The compiler can't see inside it, so it has to actually
compute `value` and can't move memory operations across it. It emits
no instructions. Google Benchmark's `benchmark::DoNotOptimize` is the
same trick.

**Microbenchmarks vs the real thing.** A loop that pushes 100,000 ints
runs with a hot cache and a branch predictor that's seen it all before.
Your program in the wild won't. Phase 5 uses `perf` to measure whole
programs (cache misses, branch misses, instructions per cycle), which is
what you'd use to decide what to optimise in practice.

**Run it twice.** Expect a few percent of variation between runs. If two
numbers are within that noise, they're the same. Benchmarking
frameworks run until the measurements settle and report a confidence
interval for this reason.

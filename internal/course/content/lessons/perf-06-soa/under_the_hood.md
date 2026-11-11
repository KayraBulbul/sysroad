**Counting the bytes.** `count_below` over a million AoS particles
touches every one of the 1.25 million lines they fill: 80 MB of
traffic. The
SoA version reads 4 MB. The 15-20× you see is mostly that ratio. The
step gains less because it needs 28 of every 80 bytes in AoS, so the
waste is under 3×, plus what SIMD adds.

**Why the compiler vectorises SoA and not AoS.** To do 8 particles'
`z += vz * dt` at once, the CPU loads 8 consecutive floats into one
register. In SoA they *are* consecutive. In AoS the 8 `z`s are 80
bytes apart, so the compiler would need gather instructions or a lot
of shuffling, and usually decides it isn't worth it.

**AoSoA.** SIMD-heavy code sometimes uses a hybrid: blocks of 8 or 16
particles, each block stored as a struct of small arrays. It keeps a
particle's fields near each other (good when you need all of them)
while still giving SIMD contiguous lanes. ISPC and many physics
engines use it.

**Where SoA shows up.** Game engines' entity-component systems store
each component type in its own array. Columnar databases (DuckDB,
ClickHouse, Parquet files) store each column separately for the same
reason: a query that sums one column reads only that column. Pandas
and Arrow DataFrames are SoA too, which is why row-by-row loops over
them are slow.

**When AoS wins.** If you always use every field of an element
together, or access elements at random (a hash table, a tree), AoS
puts everything you need in one or two lines. SoA would touch one
line per field. Measure the real access pattern before rearranging.

**`kinetic_energy` gains less than you'd think.** The sum is in a
`double`, and floating-point addition isn't associative, so clang
won't split it into several partial sums without `-ffast-math`. The
loop is limited by the chain of adds, not by memory. Lesson 5.2.3
breaks that chain by hand.

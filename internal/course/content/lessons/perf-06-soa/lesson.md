---
title: Struct of arrays vs array of structs
minutes: 35
files: [soa.cpp]
program: particles
optimize: true
run: {}
deeper:
  - title: "CppCon 2014: Mike Acton, Data-Oriented Design and C++"
    url: https://www.youtube.com/watch?v=rX0ItVEVjHc
  - title: "Data-Oriented Design (Richard Fabian), the book, free online"
    url: https://www.dataorienteddesign.com/dodbook/
---
The natural way to store a million particles is a
`std::vector<Particle>`: an **array of structs** (AoS). Each `Particle`
is 80 bytes. The physics step reads and writes 24 of them, and
`count_below` reads only `z`, 4 bytes. But the cache moves whole
64-byte lines, so a loop over `z` drags in every particle's name,
colour and timestamp too: 80 bytes of memory traffic for 4 useful
ones.

A **struct of arrays** (SoA) turns the layout sideways: one vector per
field.

```text
AoS:  [x y z vx vy vz m id name… color… t] [x y z vx vy vz m id name… …] …
SoA:  x:  [x x x x x x x x x x x x x x x x …]
      z:  [z z z z z z z z z z z z z z z z …]
      vx: [vx vx vx vx …]
      …
```

Now `count_below` reads one dense array of floats, 16 per cache line,
every byte of which it uses. The step reads six arrays, but only
the bytes it needs from each. As a bonus, `z_[i] += vz_[i] * dt` over
consecutive `i` is exactly the shape the compiler can turn into SIMD
instructions, doing 4 or 8 particles per instruction.

The fields the physics never reads (name, colour, spawn time) go in a
separate vector of small `Cold` structs. That's **hot/cold
splitting**, and it's often a good middle ground on its own: keep a
struct, but move the rarely used fields out of it.

The price is convenience. A particle is no longer an object you can
hold: adding one means pushing onto nine vectors, and reading one back
means gathering from all of them. That's `add` and `get`.

`aos.cpp` has the array-of-structs version. Write the same operations
for `ParticlesSoA`, with the same arithmetic in the same order, so the
results match exactly.

## Assignment

- [ ] `add` pushes each field onto its vector, and the cold fields onto `cold_`
- [ ] `size` and `get`, which puts particle `i` back together
- [ ] `step`, `kinetic_energy` and `count_below` do what `aos.cpp` does, one array per field
- [ ] Run it. Which operation gains the most, and why that one?

---
title: Loop unrolling and dependency chains
minutes: 35
files: [chains.cpp]
program: chains
optimize: true
run: {}
deeper:
  - title: "CS:APP 5.7-5.10: understanding modern processors, loop unrolling, parallelism"
    url: https://csapp.cs.cmu.edu/
  - title: "uops.info: latency and throughput of every x86 instruction"
    url: https://uops.info/table.html
  - title: "Agner Fog's microarchitecture guide"
    url: https://www.agner.org/optimize/microarchitecture.pdf
---
A modern core can start several instructions every cycle: two or more
loads, two floating-point adds, an integer add or four, a branch. It
looks ahead in your loop for work that's ready, and starts it out of
order. What it can't do is start an instruction before its inputs
exist. Two numbers describe each operation:

- **latency**: cycles from start until the result can be used. A
  floating-point add takes 2-4, a multiply 3-5, an integer add 1.
- **throughput**: how many can start per cycle. Usually 2 FP adds, 2
  multiplies, 2-3 loads.

Look at the obvious sum:

```cpp
for (i) total += a[i];
```

Every add needs the previous `total`. That's a **dependency chain**,
and it runs at one add per *latency*, however many adders the core
has. The loads aren't the problem: they don't depend on each other, so
the core can do them ahead of time.

The fix is several independent chains:

```cpp
for (i = 0; i + 4 <= n; i += 4) {
    s0 += a[i];  s1 += a[i + 1];  s2 += a[i + 2];  s3 += a[i + 3];
}
// one more loop for the 0-3 elements left over
return (s0 + s1) + (s2 + s3);
```

Now four adds are in flight at once, and the loop can approach one
add per *throughput*. This is **loop unrolling with multiple
accumulators**. Unrolling alone (`total += a[i]; total += a[i + 1];`)
saves loop overhead but keeps the single chain.

Why doesn't the compiler do it for you? Because floating-point
addition isn't associative: `(a + b) + c` can differ from
`a + (b + c)` in the last bit, and reordering would change your
answer. It does it for integers, where the result is the same.

Polynomials have the same problem. Horner's rule,
`result = a[i] + x * result`, does the fewest operations, but each
step is a multiply then an add on the chain. Split the polynomial in
two, with even and odd coefficients, and you get two chains in x²:

```text
p(x) = (a0 + a2·x² + a4·x⁴ + …) + x · (a1 + a3·x² + a5·x⁴ + …)
```

Run measures **cycles per element** (CPE): the time per element
divided by the length of one cycle. It finds the length of a cycle by
timing a chain of integer adds, which take one cycle each.

## Assignment

- [ ] `sum2` keeps two running totals, `sum4` four, and both handle any `n`
- [ ] `poly_split` evaluates the polynomial as two Horner chains in x², even and odd
- [ ] Run it. Where does doubling the accumulators stop halving the CPE?

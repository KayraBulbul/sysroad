**The Patriot bug was trap 2.** In 1991 a Patriot missile battery
counted time in tenths of a second and multiplied by a 24-bit
approximation of 0.1. After 100 hours of running, the clock was off by
0.34 seconds. An incoming missile travels half a kilometre in that
time, so the system looked in the wrong place and missed it.

**Why 0.1 isn't exact.** A fraction has a finite binary form only when
its denominator is a power of two. 1/2, 1/4 and 3/8 are exact. 1/10
isn't, the same way 1/3 isn't in decimal: it's 0.000110011001100…
repeating forever, cut off after 53 bits.

**Printing hides it.** `std::format("{}", x)` and Python's `repr`
print the *shortest* decimal that reads back as the same double. So
0.1 prints as `0.1` even though the stored value is
0.1000000000000000055511151231257827. And `0.1 + 0.2` prints as
0.30000000000000004 because a shorter string would read back as a
different double, the one that 0.3 becomes.

**Money.** Banks and payment APIs use integers (cents, or
thousandths of a cent) or a decimal type. Stripe's API takes amounts
as whole numbers of the smallest currency unit for this reason.

**Adding many numbers well.** When you really do have to sum a lot of
floats, order matters: adding small numbers to a huge running total
loses them. *Kahan summation* keeps a second variable holding the
rounding error of each step and feeds it back in. Python's
`math.fsum` goes further and returns the exactly rounded sum.

**`-ffast-math` breaks trap 3's fix.** That flag lets the compiler
assume NaN and infinity never occur, so it may delete your
`std::isnan` check as dead code. It also lets it reorder additions,
which changes results. It's off by default for good reasons.

**NaN and sorting.** `std::sort` requires that `<` is a consistent
ordering. With NaNs in the data it isn't, and the sort can produce
garbage or run off the end of the array. Remove or partition out the
NaNs first.

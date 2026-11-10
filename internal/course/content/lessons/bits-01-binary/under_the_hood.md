**`x & (x - 1)` works because of borrowing.** Subtracting 1 turns the
lowest 1 bit into 0 and every 0 below it into 1: `1011 0000 - 1` is
`1010 1111`. Everything above that bit is untouched, so ANDing the two
keeps the high part and wipes out the lowest 1.

**The CPU has an instruction for it.** x86-64 has `popcnt` and ARM has
`cnt`. `std::popcount` from `<bit>` compiles down to them, and clang
even recognises the `x &= x - 1` loop and replaces the whole loop with
the instruction when the target allows it. `<bit>` has more that are
worth knowing: `std::countl_zero`, `std::countr_zero`, `std::has_single_bit`
(your `is_power_of_two`), `std::bit_width` and `std::rotl`.

**Why XOR shows up everywhere.** `a ^ a` is 0 and `a ^ 0` is `a`, so
XORing with the same value twice gets you back where you started. That
makes it its own inverse, which is why simple ciphers, checksums, RAID
parity and the "swap without a temporary" trick all use it. Compilers
zero a register with `xor %eax, %eax` because it's the shortest way to
write 0.

**Octal still exists.** A literal starting with a plain `0` is base 8:
`0755` is 493. It survives because Unix permissions are three bits per
group, and one octal digit is exactly three bits. It also means `010`
is 8, which has surprised people for fifty years.

**Use unsigned types for bit work.** Shifting or flipping a signed
number drags the sign bit into it. Lesson 4.1.3 shows what the sign
bit means, and 4.1.4 shows how small types get widened before the
operators even see them.

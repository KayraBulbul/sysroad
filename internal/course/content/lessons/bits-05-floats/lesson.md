---
title: IEEE 754 floats by hand
minutes: 30
files: [ieee.cpp]
run:
  args: ["0.1"]
deeper:
  - title: Float Exposed (click the bits)
    url: https://float.exposed/
  - title: "What Every Computer Scientist Should Know About Floating-Point Arithmetic"
    url: https://docs.oracle.com/cd/E19957-01/806-3568/ncg_goldberg.html
  - title: "Floating point visually explained (Fabien Sanglard)"
    url: https://fabiensanglard.net/floating_point_visually_explained/
---
In lesson 2.1.1 you split a `float` into its three fields. Now for
what they mean. A float is scientific notation in binary: a sign, a
number between 1 and 2, and a power of two.

```
6.5  =  110.1 in binary  =  1.101 x 2^2

 0   1000 0001   101 0000 0000 0000 0000 0000      0x40d00000
sign  exponent   fraction (23 bits)
       129 = 2 + 127
```

Two details save bits. The exponent is stored **plus 127**, so the
field is never negative. And because the number in front always
starts with `1.`, that 1 isn't stored at all, only the 23 bits after
the point. So for most bit patterns the value is

```
(-1)^sign  x  (1 + fraction / 2^23)  x  2^(exponent - 127)
```

The smallest and largest exponent fields are reserved, which gives
five kinds of float:

- exponent 1 to 254: a **normal** number, the formula above
- exponent 0, fraction 0: **zero**. There are two, +0 and -0
- exponent 0, fraction not 0: a **subnormal**. There's no hidden 1,
  and the power is fixed at -126: `(fraction / 2^23) x 2^-126`. They
  fill the gap between the smallest normal number and zero
- exponent 255, fraction 0: **infinity**, positive or negative
- exponent 255, fraction not 0: **NaN**, "not a number", what `0.0 / 0.0` gives

With 24 bits of precision, a float holds every whole number up to
2^24 = 16777216 exactly, and then starts skipping: between 2^24 and
2^25 it can only hold the even ones, then only multiples of 4, and so
on. A value in between is **rounded to the nearest** float, and when
it's exactly halfway, to the one whose last bit is 0 ("ties to even").
The same thing happens to 0.1, which has no finite binary form, just
as 1/3 has no finite decimal one.

`main.cpp` is done. `std::ldexp(x, n)` from `<cmath>` computes
`x * 2^n` exactly, which is the tool for `value_of`. Don't use
`std::bit_cast` or `memcpy` in this lesson: the point is to do the
arithmetic the CPU does.

```
$ ./ieee 0.1
0.1 is stored as 0x3dcccccd
sign 0  exponent 123  fraction 0x4ccccd
normal: 1.60000002384185791015625 x 2^-4
exactly 0.100000001490116119384765625
$ ./ieee int 16777217
16777217 as a float is 0x4b800000
exactly 16777216
```

## Assignment

- [ ] `classify` says which of the five kinds a bit pattern is
- [ ] `value_of` computes the value from the three fields
- [ ] `from_uint` builds the bits of the float nearest to an integer, rounding ties to even

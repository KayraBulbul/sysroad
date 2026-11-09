---
title: Objects are bytes
minutes: 15
files: [bytes.cpp]
run:
  args: ["float", "-0.5"]
deeper:
  - title: Object representation (cppreference)
    url: https://en.cppreference.com/w/cpp/language/object#Object_representation_and_value_representation
  - title: std::bit_cast (cppreference)
    url: https://en.cppreference.com/w/cpp/numeric/bit_cast
  - title: Float Exposed (play with float bits)
    url: https://float.exposed/
---
Every object in memory is a run of bytes: an `int` is 4 of them, a
`double` is 8. `sizeof` tells you how many bytes. A `const unsigned char*`
lets you read them one at a time, because the language allows any object
to be read as `unsigned char` bytes.

```cpp
int x = 1;
auto* p = reinterpret_cast<const unsigned char*>(&x);
for (std::size_t i = 0; i < sizeof x; ++i)
    std::cout << int(p[i]) << ' ';   // 1 0 0 0
```

The `1` comes first. x86-64 and ARM Macs are both **little-endian**:
they store the least significant byte at the lowest address. Lesson 2.1.4
handles the other order.

A `float` is bytes too, in a different code. The 32 bits split into a
sign (1 bit), an exponent (8 bits, stored plus 127) and a mantissa (23
bits). To get them out as a `uint32_t`, copy the bytes with
`std::memcpy` or use C++20's `std::bit_cast<std::uint32_t>(f)`.
Casting `&f` to `std::uint32_t*` and reading through it is undefined
behaviour. Lesson 2.3.6 explains why.

`main.cpp` is done. It reads a type and a value and prints the bytes:

```
$ ./bytes float -0.5
float -0.5: 4 bytes
00 00 00 bf
sign 1  exponent 126 (2^-1)  mantissa 0x000000
```

## Assignment

- [ ] `hex_bytes` prints `n` bytes as two-digit lowercase hex, separated by single spaces
- [ ] `bits_of` returns a float's 32 bits as a `std::uint32_t`
- [ ] `parts_of` splits a float into sign, exponent and mantissa

`midpoint`: do the addition in a wider type, where it can't overflow,
then narrow the result. It always fits back in an `int`:
`return static_cast<int>((static_cast<long long>(a) + b) / 2);`.
---
`rotl`: reduce the amount first, `n %= 32;`, and return `x` when it's
0, since `x >> 32` is the oversized shift. C++20's `std::rotl` from
`<bit>` does all of this for you.
---
`read_u32`: `std::uint32_t v; std::memcpy(&v, p, sizeof v); return v;`.
`memcpy` copies bytes, so alignment doesn't matter, and the compiler
turns it into a single load wherever the CPU allows that.
---
`has_adjacent_equal`: compare `v[i]` with `v[i + 1]` only while
`i + 1 < v.size()`. (Not `i < v.size() - 1`: for an empty vector
`v.size() - 1` wraps around to a huge number.)
---
`magnitude`: do the negation in `unsigned`, where wrapping is defined:
`return x < 0 ? 0u - static_cast<unsigned>(x) : static_cast<unsigned>(x);`.

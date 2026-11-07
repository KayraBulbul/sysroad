`parse_size`: look at the last character first. If it's a letter, pick
the multiplier and `s.remove_suffix(1)`. Then parse what's left as a
`std::uint64_t` with `from_chars`.
---
Overflow: before multiplying, check
`if (n > std::numeric_limits<std::uint64_t>::max() / mult) return std::nullopt;`.
That checks `n * mult` without ever computing a wrapped-around result.
---
`main`: `if (auto n = parse_size(arg)) std::cout << *n << '\n'; else { std::cerr << ...; status = 1; }`.

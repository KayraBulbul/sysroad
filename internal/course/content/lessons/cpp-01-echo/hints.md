Only add a space *between* arguments, not before the first one. Checking
`if (!out.empty())` breaks when the first argument is `""`, so track the
index instead: `for (std::size_t i = 0; i < args.size(); ++i)` and add
`' '` when `i > 0`.
---
To drop a leading `-n`: `if (!args.empty() && args[0] == "-n")`. Comparing a
`std::string` with a string literal using `==` compares the text, unlike
in C.
---
Erasing the first element works:
`args.erase(args.begin());`. So does building the vector from `argv + 2`
when `-n` is there.

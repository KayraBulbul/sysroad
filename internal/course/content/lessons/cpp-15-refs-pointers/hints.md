`find_value`: loop with a reference so you get the element itself, not a
copy: `for (int& x : v) if (x == target) return &x;`. After the loop,
`return nullptr;`.
---
`trim`: find the first and last non-space characters with
`s.find_first_not_of(" \t\n\r\f\v")` and `s.find_last_not_of(...)`. If
the first is `std::string::npos`, the whole string is whitespace, so
`s.clear()`. Otherwise `s = s.substr(first, last - first + 1);`.
---
`longest`: start with `const std::string* best = nullptr;`, and for each
`const std::string& w : words`, take `&w` when `best == nullptr ||
w.size() > best->size()`.

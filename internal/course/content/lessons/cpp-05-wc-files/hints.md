Start with the easy cases:

```cpp
Options o = parse_options(args);
if (o.bad != 0) { err << "wc: invalid option -- '" << o.bad << "'\n"; return 1; }
if (o.files.empty()) { out << format_counts(count_stream(in), o) << '\n'; return 0; }
```
---
For each name in `o.files`, keep `Counts total;` and `int status = 0;`
outside the loop. `"-"` reads `in`. Anything else gets an `std::ifstream`.
---
```cpp
std::ifstream f{std::string(name), std::ios::binary};
if (!f) {
    err << "wc: " << name << ": " << std::strerror(errno) << '\n';
    status = 1;
    continue;
}
```
---
After counting a file, add each field to the total:
`total.lines += c.lines;` and the same for words and bytes. Print
`format_counts(total, o) << " total\n"` when `o.files.size() > 1`.

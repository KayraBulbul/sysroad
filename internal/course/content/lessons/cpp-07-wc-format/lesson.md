---
title: GNU-exact column widths
minutes: 15
files: [wc.cpp]
run:
  args: ["a.txt", "b.txt"]
  files:
    a.txt: "hello world\n"
    b.txt: "one\ntwo three\n"
deeper:
  - title: std::filesystem::file_size (cppreference)
    url: https://en.cppreference.com/w/cpp/filesystem/file_size
  - title: "GNU wc: compute_number_width (source)"
    url: https://github.com/coreutils/coreutils/blob/master/src/wc.c
---
Your wc pads every column to 7. GNU's doesn't:

```text
$ wc a.txt b.txt
 1  2 12 a.txt
 2  3 14 b.txt
 3  5 26 total
$ wc -l a.txt
1 a.txt
```

GNU works out the width **before reading anything**, from the file
sizes: the total can't have more digits than the total byte count. The
rule:

1. If there's **one input and one column**, the width is 1.
2. Otherwise, add up the sizes of the inputs that are **regular files**,
   and the width is that sum's number of digits.
3. If any input is stdin (no arguments, or `-`) or exists but isn't a
   regular file (a directory, say), the width is at least 7. Its size is
   unknown.
4. Files that don't exist count for nothing.

`<filesystem>` answers both questions without opening the file:

```cpp
namespace fs = std::filesystem;
std::error_code ec;
if (fs::is_regular_file(path, ec)) {
    total += fs::file_size(path, ec);
}
```

Passing an `std::error_code` makes these functions report problems through
`ec` instead of throwing. A missing file just isn't "regular". Note that
they take a `fs::path`, which you can build from a `string_view`.

**A width chosen at run time.** `std::format` reads a width from the
arguments when you write a nested `{}`:

```cpp
std::format("{:>{}}", n, width)  // n, right-aligned, width wide
```

## Assignment

- [ ] Implement `column_width` with the four rules
- [ ] Make `format_counts` use the width it's given
- [ ] Compute the width once in `run_wc`, where the TODO is

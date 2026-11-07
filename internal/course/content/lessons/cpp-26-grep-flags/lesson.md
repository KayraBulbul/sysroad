---
title: "grep: -i, -v, -n, -c"
minutes: 20
files: [grep.cpp]
run:
  args: ["-in", "todo", "main.cpp"]
  files:
    main.cpp: "int main() {\n    // TODO: parse args\n    run();\n    // todo: clean up\n}\n"
deeper:
  - title: std::search (cppreference)
    url: https://en.cppreference.com/w/cpp/algorithm/search
---
Four flags cover most of what you'll use grep for:

```text
$ grep -in todo main.cpp
2:    // TODO: parse args
4:    // todo: clean up
$ grep -c todo *.cpp
main.cpp:1
util.cpp:0
```

- **`-i`** ignores case. Compare letters with `std::tolower`, on the
  `unsigned char` value as always.
- **`-v`** inverts: select the lines that *don't* contain the pattern.
- **`-n`** puts the line number (counting from 1) and a `:` before each
  line, after the file name: `main.cpp:2:...`.
- **`-c`** prints only how many lines were selected, one count per file
  (`name:count`, or just the count for a single input). With `-v`, it
  counts the non-matching lines.

The exit status follows the *selected* lines. `grep -v x` exits 0 if any
line lacks an `x`.

Parsing is done. `parse_grep_options` accepts flags anywhere and bundled
(`-in`), takes the first other argument as the pattern, and the rest as
files. Your work is `matches` and `grep_stream`.

**Case-insensitive search without copying.** You could lowercase every
line into a new string. `std::search` with a comparison lambda does it in
place:

```cpp
auto same = [](char a, char b) {
    return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
};
bool found = std::search(line.begin(), line.end(), pat.begin(), pat.end(), same) != line.end();
```

`std::search` finds the first place the pattern's range appears inside
the line's range. One edge: it returns `line.end()` for an empty pattern,
so treat an empty pattern as always found.

## Assignment

- [ ] `matches`: `-i` and `-v`
- [ ] `grep_stream`: `-n` line numbers
- [ ] `grep_stream`: `-c` prints `prefix` and the count instead of the lines

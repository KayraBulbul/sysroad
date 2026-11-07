---
title: "wc flags: -l, -w, -c"
minutes: 15
files: [wc.cpp]
run:
  args: ["-lw"]
  stdin: "the quick brown fox\njumps over\n"
deeper:
  - title: std::basic_string_view (cppreference)
    url: https://en.cppreference.com/w/cpp/string/basic_string_view
---
Real `wc` lets you pick the columns:

```text
$ printf 'one two\nthree\n' | wc -l
1
$ printf 'one two\nthree\n' | wc -lw
      2       3
```

Flags can come one at a time (`-l -w`) or bundled (`-lw`). The columns
always print in the same order (lines, words, bytes), whatever order the
flags came in. With no flags you get all three.

**argv as a vector.** `main(int argc, char* argv[])` hands you C strings.
Wrap them once and work with views from then on:

```cpp
std::vector<std::string_view> args(argv + 1, argv + argc);
```

That's the vector's *range constructor*: it builds one `string_view` from
each `char*` between the two pointers. `argv[0]` is the program's name,
so the range starts at `argv + 1`.

**A struct for the result.** Parsing gives back several facts, so return a
struct instead of passing pointers in:

```cpp
struct Options {
    bool lines = false;  // default member initialisers:
    bool words = false;  // every Options starts out all-false
    bool bytes = false;
    char bad = 0;        // the unknown flag letter, if any
};
```

`string_view` has what you need for parsing: `arg.starts_with('-')`,
`arg.size()`, and `arg.substr(1)` (everything after the dash), with no
copying.

**The spec**

- An argument that starts with `-` and is longer than one character is a
  group of flag letters: `l`, `w` or `c`.
- Any other letter is an error. Store it in `bad` and stop parsing.
- If no column was asked for, turn all three on.
- Arguments that aren't flags are ignored for now. Files come next lesson.
- `format_counts` returns the chosen columns, each right-aligned to 7
  characters and separated by one space, then `'\n'`. Exact GNU widths
  come in lesson 1.1.7.
- On a bad flag, `main` prints `wc: invalid option -- 'x'` to `std::cerr`
  and returns 1.

`count_stream` from last lesson is already in your file.

## Assignment

- [ ] Implement `parse_options`
- [ ] Implement `format_counts`
- [ ] Wire both into `main`, including the error for a bad flag

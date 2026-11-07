---
title: "grep: -r over a directory tree"
minutes: 30
files: [grep.cpp]
run:
  args: ["-rn", "TODO", "src"]
  files:
    src/main.cpp: "int main() {\n    // TODO: flags\n}\n"
    src/util/strings.cpp: "// helpers\n// TODO: unicode\n"
    src/util/README: "nothing to do here\n"
    src/logo.bin: "PNG\0\0TODO\0"
deeper:
  - title: std::filesystem::recursive_directory_iterator (cppreference)
    url: https://en.cppreference.com/w/cpp/filesystem/recursive_directory_iterator
---
The chapter project: make your grep search whole directory trees, the way
you'd hunt for a function name in a codebase.

```text
$ grep -rn TODO src
src/main.cpp:2:    // TODO: flags
src/util/strings.cpp:2:// TODO: unicode
grep: src/logo.bin: binary file matches
```

This one is a **project**: the spec and the tests, and the rest is up to
you. Your grep from last lesson is the starter, and `-r` is already
parsed into `o.recursive`.

**The spec**

- **`-r`** makes directory arguments get searched: every regular file
  below them, at any depth, including hidden ones. Entries that are
  **symlinks are skipped** (GNU's `-r` doesn't follow them, `-R` does).
  Files named directly on the command line are searched as usual.
- **Order:** within each directory argument, search the files in sorted
  path order. (GNU uses the order the OS returns, which varies. Sorted
  is predictable, so it's testable.)
- **No file arguments** with `-r` means `.`, and then the names are shown
  *without* the leading `./`: `src/main.cpp`, not `./src/main.cpp`.
- **Names:** shown when there's more than one argument, or when `-r` is
  searching a directory. A lone file with `-r` has no prefix, as before.
- **Binary files:** a file containing a zero byte is binary. GNU treats
  zero bytes as line ends too. If any of its lines would be selected,
  print nothing for it on `out`, and write `grep: NAME: binary file
  matches` to `err` instead. This isn't an error, so the status stays 0.
  With `-c`, counts print normally.
- Without `-r`, a directory is still an error (`Is a directory`, status
  2), and with `-c` GNU also prints `NAME:0` for it. The starter already
  does that.

Tools you'll want: `std::filesystem::recursive_directory_iterator`,
`entry.is_symlink()`, `entry.is_regular_file()`, `std::sort` on
`fs::path`, and `path.lexically_relative(".")`. To check a file for zero
bytes you'll need its contents in memory first:
`std::string data(std::istreambuf_iterator<char>(f), {});`.

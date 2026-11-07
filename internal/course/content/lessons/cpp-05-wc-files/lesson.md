---
title: Files, totals and errors
minutes: 20
files: [wc.cpp]
run:
  args: ["a.txt", "nope.txt", "b.txt"]
  files:
    a.txt: "hello world\n"
    b.txt: "one\ntwo three\n"
deeper:
  - title: std::basic_ifstream (cppreference)
    url: https://en.cppreference.com/w/cpp/io/basic_ifstream
  - title: std::strerror (cppreference)
    url: https://en.cppreference.com/w/cpp/string/byte/strerror
---
Now `wc` reads files, names them, and adds a total:

```text
$ wc a.txt nope.txt b.txt
      1       2      12 a.txt
wc: nope.txt: No such file or directory
      2       3      14 b.txt
      3       5      26 total
$ echo $?
1
```

A missing file isn't fatal. wc reports it on **stderr**, carries on with
the others, and exits 1 at the end so scripts can tell something went
wrong.

**Opening a file.** `std::ifstream` is an `istream` that reads a file, so
it plugs straight into `count_stream`:

```cpp
std::ifstream f(std::string(name), std::ios::binary);
if (!f) {
    // couldn't open it: errno says why
}
Counts c = count_stream(f);
```

`std::ios::binary` means "give me the bytes exactly". It only makes a
difference on Windows, but it says what you mean. The file closes itself
when `f` goes out of scope. That's RAII, which is chapter 1.3.

**Why it failed.** When opening fails, the C library leaves an error number
in `errno` (from `<cerrno>`). `std::strerror(errno)` (from `<cstring>`)
turns it into the familiar message. Read it straight away, before anything
else can overwrite it.

**Streams as parameters.** The real work goes in a function that takes its
streams as arguments, so the tests can hand it string streams instead of
the terminal:

```cpp
int run_wc(const std::vector<std::string_view>& args,
           std::istream& in, std::ostream& out, std::ostream& err);
```

`main` just calls `run_wc(args, std::cin, std::cout, std::cerr)`.

Two things in the starter have already changed: `parse_options` now
collects non-flag arguments into `o.files`, and `format_counts` no longer
adds the `'\n'`, so you can put a name after it.

**The spec**

- On a bad flag, write `wc: invalid option -- 'x'` to `err` and return 1.
- No files: count `in` and print the columns with no name.
- Each file prints its columns, a space, its name, then `'\n'`. The name
  `-` means "read `in`".
- If a file can't be opened, write `wc: NAME: REASON` to `err`, skip it, and
  return 1 at the end.
- With more than one file argument, finish with a `total` line, even if
  some of them failed.

## Assignment

- [ ] Count each file, or `in` when there are none
- [ ] Report files that can't be opened, and keep going
- [ ] Print the `total` line and return the right exit status
